#ifndef CG_FLASHLIGHT_H
#define CG_FLASHLIGHT_H

// Do not include cg_local.h here to prevent duplicate header inclusion conflicts
// qboolean is defined in q_shared.h/cg_local.h which should be included before this file in .c files

#include "../qcommon/q_shared.h"

extern qboolean cg_flashlightOn;
extern float cg_flashlightEnergy;

void CG_Flashlight_Toggle_f( void );
void CG_Flashlight_Frame( void );
void CG_Flashlight_DrawHUD( void );

#endif // CG_FLASHLIGHT_H
