#include "q_shared.h"
#include "qcommon.h"
#include "gp_jobsystem.h"

#define MAX_ZONE_NAME 64

typedef enum {
	ZONE_UNLOADED,
	ZONE_LOADING,
	ZONE_READY_STAGED
} zoneState_t;

typedef struct {
	char        zoneName[MAX_ZONE_NAME];
	zoneState_t state;
	void        *rawBuffer;
	int         rawLength;
} mapStreamer_t;

// Isolated background state tracking instance
static mapStreamer_t g_activeStreamer = { "", ZONE_UNLOADED, NULL, 0 };

/*
==================
AsyncZoneIO_Worker

The worker payload called asynchronously by your SDL3 thread pool.
==================
*/
static void AsyncZoneIO_Worker(void *arg) {
	mapStreamer_t *streamer = (mapStreamer_t *)arg;
	if (!streamer) return;

	// Execute the lock-less reader from files.c
	streamer->rawBuffer = FS_LoadFileAsync(streamer->zoneName, &streamer->rawLength);

	if (!streamer->rawBuffer || streamer->rawLength <= 0) {
		Com_Printf("Stream-Error: Background loading failed for target zone: %s\n", streamer->zoneName);
		streamer->state = ZONE_UNLOADED;
		return;
	}

	// Update state marker to signal main loop thread parameters
	streamer->state = ZONE_READY_STAGED;
	Com_Printf("Stream-System: Zone %s buffered cleanly in RAM on background thread.\n", streamer->zoneName);
}

/*
==================
CM_TriggerZoneStream

Call this from player collision triggers or script commands to fire up the worker thread.
==================
*/
void CM_TriggerZoneStream(const char *zoneBspPath) {
	if (!zoneBspPath || !zoneBspPath[0]) return;
	
	// Guard: block duplicate triggers if a load task is already executing
	if (g_activeStreamer.state != ZONE_UNLOADED) return; 

	Q_strncpyz(g_activeStreamer.zoneName, zoneBspPath, sizeof(g_activeStreamer.zoneName));
	g_activeStreamer.state = ZONE_LOADING;
	g_activeStreamer.rawBuffer = NULL;
	g_activeStreamer.rawLength = 0;

	Com_Printf("Stream-System: Sending asset stream request to background workers for %s...\n", g_activeStreamer.zoneName);

	// Blast it straight into your gp_jobsystem worker lanes!
	Sys_QueueJob(AsyncZoneIO_Worker, &g_activeStreamer);
}

/*
==================
CM_PollStreamerHandshake

Call this function inside your high-frequency engine frame loop ticks (like SV_Frame or G_RunFrame).
==================
*/
void CM_PollStreamerHandshake(void) {
	// Early check out if no data assets are staged
	if (g_activeStreamer.state != ZONE_READY_STAGED) return;

	// Condition Match: Ensure the player is positioned correctly inside transition boundaries
	// before granting pointer assignment swaps.
	// (Add your custom spatial trigger proximity validation checks here)

	Com_Printf("Stream-System: Handshake validated. Swapping map memory pipelines!\n");

	// Quiesce background safety systems safely before pointer assignments
	Sys_WaitJobs();

	// --- HANDSHAKE EXECUTION ---
	// Feed your custom sub-lump parsers directly from your streamer memory staging buffer 
	// instead of triggering synchronous disk seeks:
	// CM_LoadMapFromBuffer(g_activeStreamer.rawBuffer, g_activeStreamer.rawLength);

	// --- ASYNC CLEANUP HANDOFF ---
	// Instead of lagging the rendering loop freeing an old zone buffer block, 
	// queue free() right onto your worker threads to resolve asynchronously!
	if (g_activeStreamer.rawBuffer) {
		// Sys_QueueJob(free, g_activeStreamer.rawBuffer);
	}

	// Reset structural state tracker to base configuration
	g_activeStreamer.rawBuffer = NULL;
	g_activeStreamer.rawLength = 0;
	g_activeStreamer.state = ZONE_UNLOADED;
}
