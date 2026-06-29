#include "q_shared.h"
#include "gp_jobsystem.h"
#include <SDL3/SDL.h>

#define MAX_JOBS 256
#define MAX_WORKERS 32

typedef struct {
	void (*work)(void*);
	void *arg;
} job_t;

static job_t jobQueue[MAX_JOBS];
static int jobHead = 0;
static int jobTail = 0;
static int activeJobs = 0;

static SDL_Thread *workers[MAX_WORKERS];
static int numWorkers = 0;
static qboolean shutdownWorkers = qfalse;

static SDL_Mutex *queueMutex = NULL;
static SDL_Condition *queueCond = NULL;
static SDL_Condition *waitCond = NULL;

static int WorkerThread(void *data) {
	while (1) {
		SDL_LockMutex(queueMutex);
		while (jobHead == jobTail && !shutdownWorkers) {
			SDL_WaitCondition(queueCond, queueMutex);
		}

		if (shutdownWorkers && jobHead == jobTail) {
			SDL_UnlockMutex(queueMutex);
			break;
		}

		// Pop job
		job_t job = jobQueue[jobHead];
		jobHead = (jobHead + 1) % MAX_JOBS;
		SDL_UnlockMutex(queueMutex);

		// Execute work
		if (job.work) {
			job.work(job.arg);
		}

		// Decrement active jobs and signal wait condition
		SDL_LockMutex(queueMutex);
		activeJobs--;
		if (activeJobs == 0) {
			SDL_BroadcastCondition(waitCond);
		}
		SDL_UnlockMutex(queueMutex);
	}
	return 0;
}

void Sys_InitJobSystem(void) {
	if (queueMutex) return;

	queueMutex = SDL_CreateMutex();
	queueCond = SDL_CreateCondition();
	waitCond = SDL_CreateCondition();
	shutdownWorkers = qfalse;
	jobHead = 0;
	jobTail = 0;
	activeJobs = 0;

	// Query number of logical CPU cores
	int numCores = SDL_GetNumLogicalCPUCores();
	numWorkers = numCores - 1;
	if (numWorkers < 1) numWorkers = 1;
	if (numWorkers > MAX_WORKERS) numWorkers = MAX_WORKERS;

	for (int i = 0; i < numWorkers; i++) {
		char name[32];
		Com_sprintf(name, sizeof(name), "Worker_%d", i);
		workers[i] = SDL_CreateThread(WorkerThread, name, NULL);
	}
}

void Sys_ShutdownJobSystem(void) {
	if (!queueMutex) return;

	SDL_LockMutex(queueMutex);
	shutdownWorkers = qtrue;
	SDL_BroadcastCondition(queueCond);
	SDL_UnlockMutex(queueMutex);

	for (int i = 0; i < numWorkers; i++) {
		if (workers[i]) {
			SDL_WaitThread(workers[i], NULL);
			workers[i] = NULL;
		}
	}

	SDL_DestroyMutex(queueMutex);
	SDL_DestroyCondition(queueCond);
	SDL_DestroyCondition(waitCond);

	queueMutex = NULL;
	queueCond = NULL;
	waitCond = NULL;
	numWorkers = 0;
}

void Sys_QueueJob(void (*work)(void*), void* arg) {
	if (!queueMutex) {
		// Fallback to synchronous execution if job system is not initialized
		work(arg);
		return;
	}

	SDL_LockMutex(queueMutex);
	
	// Check if queue is full
	int nextTail = (jobTail + 1) % MAX_JOBS;
	if (nextTail == jobHead) {
		// Queue full, execute synchronously as fallback
		SDL_UnlockMutex(queueMutex);
		work(arg);
		return;
	}

	jobQueue[jobTail].work = work;
	jobQueue[jobTail].arg = arg;
	jobTail = nextTail;
	activeJobs++;

	SDL_SignalCondition(queueCond);
	SDL_UnlockMutex(queueMutex);
}

void Sys_WaitJobs(void) {
	if (!queueMutex) return;

	SDL_LockMutex(queueMutex);
	while (activeJobs > 0) {
		SDL_WaitCondition(waitCond, queueMutex);
	}
	SDL_UnlockMutex(queueMutex);
}

int Sys_GetNumWorkers(void) {
	return numWorkers;
}
