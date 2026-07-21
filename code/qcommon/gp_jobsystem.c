#include "q_shared.h"
#include "gp_jobsystem.h"
#include <SDL3/SDL.h>

#define MAX_JOBS 2048                // Must be a power of two
#define JOB_MASK (MAX_JOBS - 1)      // Fast bitwise mask
#define MAX_WORKERS 32
#define SPIN_LIMIT 2000              // Micro-spin iterations before OS sleep

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
    // Set thread priority for high-performance job processing
    SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    while (1) {
        int spinCount = 0;

        // --- Phase 1: Micro-spin wait before sleeping ---
        while (spinCount < SPIN_LIMIT) {
            if (jobHead != jobTail || shutdownWorkers) {
                break;
            }
            spinCount++;
            SDL_DelayNS(0); // Yield CPU timeslice briefly without full OS sleep
        }

        // --- Phase 2: Lock and sleep if still no work ---
        SDL_LockMutex(queueMutex);
        while (jobHead == jobTail && !shutdownWorkers) {
            SDL_WaitCondition(queueCond, queueMutex);
        }

        if (shutdownWorkers && jobHead == jobTail) {
            SDL_UnlockMutex(queueMutex);
            break;
        }

        // Pop job using fast bitwise mask
        job_t job = jobQueue[jobHead];
        jobHead = (jobHead + 1) & JOB_MASK;
        SDL_UnlockMutex(queueMutex);

        // Execute work
        if (job.work) {
            job.work(job.arg);
        }

        // Decrement active jobs and signal wait condition if done
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

    int numCores = SDL_GetNumLogicalCPUCores();
    numWorkers = numCores - 1; // Leave 1 core for OS / main thread
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
        work(arg);
        return;
    }

    SDL_LockMutex(queueMutex);
    
    int nextTail = (jobTail + 1) & JOB_MASK;
    if (nextTail == jobHead) {
        // Queue full fallback
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

    // --- Main Thread Work Stealing ---
    while (1) {
        job_t job = {0};
        qboolean hasJob = qfalse;

        SDL_LockMutex(queueMutex);
        if (activeJobs == 0) {
            SDL_UnlockMutex(queueMutex);
            break; // All jobs finished!
        }

        // If jobs are remaining in the queue, pop and execute one on the main thread
        if (jobHead != jobTail) {
            job = jobQueue[jobHead];
            jobHead = (jobHead + 1) & JOB_MASK;
            hasJob = qtrue;
        } else {
            // Queue is empty, but worker threads are still finishing active jobs
            SDL_WaitCondition(waitCond, queueMutex);
            SDL_UnlockMutex(queueMutex);
            continue;
        }
        SDL_UnlockMutex(queueMutex);

        // Execute stolen job on the main thread
        if (hasJob) {
            if (job.work) {
                job.work(job.arg);
            }

            SDL_LockMutex(queueMutex);
            activeJobs--;
            if (activeJobs == 0) {
                SDL_BroadcastCondition(waitCond);
            }
            SDL_UnlockMutex(queueMutex);
        }
    }
}

int Sys_GetNumWorkers(void) {
    return numWorkers;
}
