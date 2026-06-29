#ifndef GP_JOBSYSTEM_H
#define GP_JOBSYSTEM_H

void Sys_InitJobSystem(void);
void Sys_ShutdownJobSystem(void);
void Sys_QueueJob(void (*work)(void*), void* arg);
void Sys_WaitJobs(void);
int  Sys_GetNumWorkers(void);

#endif // GP_JOBSYSTEM_H
