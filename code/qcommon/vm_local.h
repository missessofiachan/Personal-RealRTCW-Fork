/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "q_shared.h"
#include "qcommon.h"

// Max number of arguments to pass from engine to vm's vmMain function.
// command number + 12 arguments
#define MAX_VMMAIN_ARGS 13
#define MAX_VMMAIN_CALL_ARGS MAX_VMMAIN_ARGS

// Max number of arguments to pass from a vm to engine's syscall handler function for the vm.
// syscall number + 15 arguments
#define MAX_VMSYSCALL_ARGS 16

// don't change, this is hardcoded into x86 VMs, opStack protection relies
// on this
#define	OPSTACK_SIZE	1024
#define	OPSTACK_MASK	(OPSTACK_SIZE-1)

// Hardcoded in q3asm a reserved at end of bss
#define	PROGRAM_STACK_SIZE	0x10000
#define	PROGRAM_STACK_MASK	(PROGRAM_STACK_SIZE-1)

#ifndef S_COLOR_WARNING
#define S_COLOR_WARNING S_COLOR_YELLOW
#endif

#define MAX_OPSTACK_SIZE 512
#define PROC_OPSTACK_SIZE 128

typedef union vmFunc_u {
	byte		*ptr;
	void (*func)(void);
} vmFunc_t;

typedef struct {
	int32_t	value;     // 32
	byte	op;        // 8
	byte	opStack;   // 8
	unsigned jused:1;  // this instruction is a jump target
	unsigned swtch:1;  // indirect jump
	unsigned safe:1;   // non-masked OP_STORE*
	unsigned endp:1;   // for last OP_LEAVE instruction
	unsigned fpu:1;    // load into FPU register
	unsigned njump:1;  // near jump
} instruction_t;

extern cvar_t *vm_rtChecks;

#define VM_RTCHECK_PSTACK  1
#define VM_RTCHECK_OPSTACK 2
#define VM_RTCHECK_JUMP    4
#define VM_RTCHECK_DATA    8

typedef enum {
	VM_FREE = -1,
	VM_GAME = 0,
	VM_CGAME,
	VM_UI,
	VM_COUNT
} vmIndex_t;


typedef enum {
	OP_UNDEF, 

	OP_IGNORE, 

	OP_BREAK,

	OP_ENTER,
	OP_LEAVE,
	OP_CALL,
	OP_PUSH,
	OP_POP,

	OP_CONST,
	OP_LOCAL,

	OP_JUMP,

	//-------------------

	OP_EQ,
	OP_NE,

	OP_LTI,
	OP_LEI,
	OP_GTI,
	OP_GEI,

	OP_LTU,
	OP_LEU,
	OP_GTU,
	OP_GEU,

	OP_EQF,
	OP_NEF,

	OP_LTF,
	OP_LEF,
	OP_GTF,
	OP_GEF,

	//-------------------

	OP_LOAD1,
	OP_LOAD2,
	OP_LOAD4,
	OP_STORE1,
	OP_STORE2,
	OP_STORE4,				// *(stack[top-1]) = stack[top]
	OP_ARG,

	OP_BLOCK_COPY,

	//-------------------

	OP_SEX8,
	OP_SEX16,

	OP_NEGI,
	OP_ADD,
	OP_SUB,
	OP_DIVI,
	OP_DIVU,
	OP_MODI,
	OP_MODU,
	OP_MULI,
	OP_MULU,

	OP_BAND,
	OP_BOR,
	OP_BXOR,
	OP_BCOM,

	OP_LSH,
	OP_RSHI,
	OP_RSHU,

	OP_NEGF,
	OP_ADDF,
	OP_SUBF,
	OP_DIVF,
	OP_MULF,

	OP_CVIF,
	OP_CVFI,
	OP_MAX
} opcode_t;

#define JUMP	(1<<0)
#define FPU		(1<<1)

typedef struct opcode_info_s
{
	int	size;
	int	stack;
	int	nargs;
	int	flags; // rhs type cast
} opcode_info_t;

extern const opcode_info_t ops[ OP_MAX ];

#define CPU_FCOM   1
#define CPU_MMX    2
#define CPU_SSE    4
#define CPU_SSE2   8
#define CPU_SSE3   16
#define CPU_SSE41  32

#define CPU_Flags  (CPU_FCOM | CPU_SSE | CPU_SSE2 | CPU_SSE41)



typedef int	vmptr_t;

typedef struct vmSymbol_s {
	struct vmSymbol_s	*next;
	int		symValue;
	int		profileCount;
	char	symName[1];		// variable sized
} vmSymbol_t;

#define	VM_OFFSET_PROGRAM_STACK		0
#define	VM_OFFSET_SYSTEM_CALL		4

struct vm_s {
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE
    int			programStack;		// the vm may be recursively entered
    intptr_t			(*systemCall)( intptr_t *parms );

	//------------------------------------
   
	char		name[MAX_QPATH];
	void	*searchPath;				// hint for FS_ReadFileDir()

	// for dynamic linked modules
	void		*dllHandle;
	vmMainProc	entryPoint;
	void (*destroy)(vm_t* self);

	// for interpreted modules
	qboolean	currentlyInterpreting;

	qboolean	compiled;
	vmFunc_t	codeBase;
	int			entryOfs;
	int			codeLength;

	intptr_t	*instructionPointers;
	int			instructionCount;

	byte		*dataBase;
	int			dataMask;
	int			dataAlloc;			// actually allocated

	int			heapLength;			// length of QVMs data
	int			heapAlloc;			// QVM's current allocate point
	int			heapAllocTop;		// QVM's current temporary memory allocate point

	int			stackBottom;		// if programStack < stackBottom, error

	int			numSymbols;
	struct vmSymbol_s	*symbols;

	int			callLevel;		// counts recursive VM_Call
	int			breakFunction;		// increment breakCount on function entry to this
	int			breakCount;

	int32_t		*jumpTableTargets;
	int			numJumpTableTargets;

	// New JIT fields
	unsigned int codeSize;
	int32_t     *opStack;
	int32_t     *opStackTop;
	uint32_t    crc32sum;
	qboolean    forceDataMask;
	uint32_t    exactDataLength;
	uint32_t    programStackExtra;
	int         syscallCount;
	int         index;
};


extern	vm_t	*currentVM;
extern	int		vm_debugLevel;

qboolean VM_Compile( vm_t *vm, vmHeader_t *header );
int32_t	VM_CallCompiled( vm_t *vm, int32_t *args );

const char *VM_LoadInstructions( const byte *code_pos, int codeLength, int instructionCount, instruction_t *buf );
const char *VM_CheckInstructions( instruction_t *buf,
								int instructionCount,
								const int32_t *jumpTableTargets,
								int numJumpTableTargets,
								int dataLength );
void VM_ReplaceInstructions( vm_t *vm, instruction_t *buf );

void VM_PrepareInterpreter( vm_t *vm, vmHeader_t *header );
int	VM_CallInterpreted( vm_t *vm, int *args );

vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value );
int VM_SymbolToValue( vm_t *vm, const char *symbol );
const char *VM_ValueToSymbol( vm_t *vm, int value );
void VM_LogSyscalls( int *args );

void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n);
