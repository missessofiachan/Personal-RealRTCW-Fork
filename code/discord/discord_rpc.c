#include "discord_rpc.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#define getpid GetCurrentProcessId
#else
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#ifndef ARCH_STRING
#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_STRING "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define ARCH_STRING "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_STRING "arm64"
#else
#define ARCH_STRING "x86_64"
#endif
#endif

#include "../client/client.h"

// Connection states managed exclusively by the background thread
typedef enum
{
    DISCORD_STATE_DISCONNECTED,
    DISCORD_STATE_CONNECTED_WAIT_READY,
    DISCORD_STATE_READY
} discordState_t;

#ifdef WIN32
static HANDLE discord_pipe = INVALID_HANDLE_VALUE;
static HANDLE worker_thread = NULL;
static CRITICAL_SECTION payload_mutex;
#else
static int discord_fd = -1;
static pthread_t worker_thread;
static pthread_mutex_t payload_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static discordState_t discord_conn_state = DISCORD_STATE_DISCONNECTED;
static char pending_payload[1024] = "";
static qboolean pending_update = qfalse;
static qboolean thread_active = qfalse;
static time_t next_discord_connect_time = 0;

static char discord_mapname[64] = "";
static char discord_skill[64] = "";
static char discord_display[64] = "";
static char discord_cs_message[128] = ""; // CS_MESSAGE (worldspawn map title)
static char discord_cs_missionstats[128] =
    ""; // CS_MISSIONSTATS (live level stats)
static char discord_fs_game[64] = ""; // fs_game (active mod folder name)
static int discord_needs_update = 0;
static time_t next_allowed_update_time = 0; // Anti-spam throttling timer
static time_t start_time = 0;

// Dynamic state-tracking variables to prevent redundant parsing/spamming on
// main thread
static int discord_last_health = -1;
static int discord_last_wave = -1;
static int discord_last_kills = -1;
static int discord_last_score = -1;
static int discord_last_weapon = -1;

// Persistent stream framing buffers for background thread
static char incoming_buf[4096];
static int incoming_len = 0;

#include "discord_data.h"

#ifdef __GNUC__
static void discord_log(const char* fmt, ...) __attribute__ ((format (gnu_printf, 1, 2)));
#endif

static void discord_log(const char* fmt, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    Q_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Com_DPrintf("[Discord] %s", buf);
}

static void ExtractMapBaseName(const char* in, char* out, int maxlen)
{
    const char* start = strrchr(in, '/');
    start = start ? start + 1 : in;
    const char* end = strrchr(start, '.');
    int len = end ? (int)(end - start) : (int)strlen(start);
    if (len >= maxlen)
        len = maxlen - 1;
    memcpy(out, start, len);
    out[len] = '\0';
}

static void StripColorCodes(const char* in, char* out, int maxlen)
{
    int j = 0;
    for (int i = 0; in[i] && j < maxlen - 1; i++)
    {
        if (in[i] == '^' && in[i + 1] != '\0')
        {
            char c = in[i + 1];
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z'))
            {
                i++;
                continue;
            }
        }
        out[j++] = in[i];
    }
    out[j] = '\0';
}

static void EscapeJsonString(const char* in, char* out, int maxlen)
{
    int j = 0;
    for (int i = 0; in[i] && j < maxlen - 5; i++)
    {
        if (in[i] == '"')
        {
            out[j++] = '\\';
            out[j++] = '"';
        }
        else if (in[i] == '\\')
        {
            out[j++] = '\\';
            out[j++] = '\\';
        }
        else if (in[i] == '\n')
        {
            out[j++] = '\\';
            out[j++] = 'n';
        }
        else if (in[i] == '\r')
        {
            out[j++] = '\\';
            out[j++] = 'r';
        }
        else if (in[i] == '\t')
        {
            out[j++] = '\\';
            out[j++] = 't';
        }
        else
        {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
}

static void Discord_ShutdownSocket(void)
{
#ifdef WIN32
    if (discord_pipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(discord_pipe);
        discord_pipe = INVALID_HANDLE_VALUE;
        discord_log("Discord: Connection closed.\n");
    }
#else
    if (discord_fd >= 0)
    {
        close(discord_fd);
        discord_fd = -1;
        discord_log("Discord: Connection closed.\n");
    }
#endif
    discord_conn_state = DISCORD_STATE_DISCONNECTED;
    incoming_len = 0;
}

static int Discord_WriteAll(const void* buf, size_t len)
{
#ifdef WIN32
    if (discord_pipe == INVALID_HANDLE_VALUE)
        return 0;

    DWORD written = 0;
    if (!WriteFile(discord_pipe, buf, (DWORD)len, &written, NULL) ||
        written != len)
    {
        discord_log("Discord: Write error occurred.\n");
        Discord_ShutdownSocket();
        return 0;
    }
    return 1;
#else
    if (discord_fd < 0)
        return 0;

    size_t total_sent = 0;
    const char* ptr = (const char*)buf;

    while (total_sent < len)
    {
        ssize_t sent =
            send(discord_fd, ptr + total_sent, len - total_sent, MSG_NOSIGNAL);
        if (sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            discord_log("Discord: Write error occurred (%d).\n", errno);
            Discord_ShutdownSocket();
            return 0;
        }
        else if (sent == 0)
        {
            discord_log("Discord: Connection lost during send operation.\n");
            Discord_ShutdownSocket();
            return 0;
        }
        total_sent += sent;
    }
    return 1;
#endif
}

static int Discord_Connect(void)
{
    if (discord_conn_state != DISCORD_STATE_DISCONNECTED)
        return 1;

    time_t cur = time(NULL);
    if (cur < next_discord_connect_time)
        return 0;

#ifdef WIN32
    discord_log("Discord: Connecting (Windows Named Pipe)...\n");
    char pipe_path[128];
    for (int i = 0; i < 10; i++)
    {
        snprintf(pipe_path, sizeof(pipe_path), "\\\\.\\pipe\\discord-ipc-%d", i);
        HANDLE pipe = CreateFileA(pipe_path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL);
        if (pipe != INVALID_HANDLE_VALUE)
        {
            discord_pipe = pipe;
            discord_conn_state = DISCORD_STATE_CONNECTED_WAIT_READY;
            incoming_len = 0;

            const char* handshake_json =
                "{\"v\":1,\"client_id\":\"1500118711774744737\"}";
            uint32_t header[2];
            header[0] = 0;
            header[1] = (uint32_t)strlen(handshake_json);

            if (!Discord_WriteAll(header, sizeof(header)) ||
                !Discord_WriteAll(handshake_json, header[1]))
            {
                return 0;
            }
            discord_log("Discord: Connected successfully to %s\n", pipe_path);
            return 1;
        }
    }
#else
    discord_log("Discord: Connecting (Linux Unix Socket)...\n");
    const char* dirs[] = {
        getenv("XDG_RUNTIME_DIR"), getenv("TMPDIR"),
        getenv("TMP"), getenv("TEMP"), "/tmp"
    };
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    for (int d = 0; d < 5; d++)
    {
        if (!dirs[d] || !dirs[d][0])
            continue;
        for (int i = 0; i < 10; i++)
        {
            snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/discord-ipc-%d",
                     dirs[d], i);
            int fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd < 0)
                continue;

            if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
            {
                discord_fd = fd;
                discord_conn_state = DISCORD_STATE_CONNECTED_WAIT_READY;
                incoming_len = 0;

                const char* handshake_json =
                    "{\"v\":1,\"client_id\":\"1500118711774744737\"}";
                uint32_t header[2];
                header[0] = 0;
                header[1] = (uint32_t)strlen(handshake_json);

                if (!Discord_WriteAll(header, sizeof(header)) ||
                    !Discord_WriteAll(handshake_json, header[1]))
                {
                    return 0;
                }
                discord_log("Discord: Connected successfully to %s\n", addr.sun_path);
                return 1;
            }
            close(fd);
        }
    }
#endif

    next_discord_connect_time = cur + 5;
    discord_log("Discord: Failed to find any active Discord IPC path.\n");
    return 0;
}

static void Discord_Pump(void)
{
    if (discord_conn_state == DISCORD_STATE_DISCONNECTED)
        return;

#ifdef WIN32
    DWORD avail = 0;
    if (!PeekNamedPipe(discord_pipe, NULL, 0, NULL, &avail, NULL))
    {
        discord_log("Discord: Pipe error during Peek.\n");
        Discord_ShutdownSocket();
        return;
    }

    if (avail == 0)
        return;

    DWORD bytesRead = 0;
    if (!ReadFile(discord_pipe, incoming_buf + incoming_len,
                  sizeof(incoming_buf) - incoming_len - 1, &bytesRead, NULL))
    {
        discord_log("Discord: Read error.\n");
        Discord_ShutdownSocket();
        return;
    }

    incoming_len += bytesRead;
#else
    ssize_t r = recv(discord_fd, incoming_buf + incoming_len,
                     sizeof(incoming_buf) - incoming_len - 1, MSG_DONTWAIT);
    if (r < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        discord_log("Discord: Read error %d.\n", errno);
        Discord_ShutdownSocket();
        return;
    }
    else if (r == 0)
    {
        discord_log("Discord: EOF received (Discord closed).\n");
        Discord_ShutdownSocket();
        return;
    }

    incoming_len += r;
#endif

    incoming_buf[incoming_len] = '\0';

    while (incoming_len >= 8)
    {
        uint32_t op, payload_len;
        memcpy(&op, incoming_buf, 4);
        memcpy(&payload_len, incoming_buf + 4, 4);

        if (8 + payload_len >= sizeof(incoming_buf))
        {
            discord_log("Discord: Received payload size too large (%u bytes).\n", payload_len);
            Discord_ShutdownSocket();
            return;
        }

        if (incoming_len < 8 + payload_len)
        {
            break;
        }

        char boundary_char = incoming_buf[8 + payload_len];
        incoming_buf[8 + payload_len] = '\0';

        char* payload = incoming_buf + 8;
        discord_log("Discord reply (op %u, len %u): %s\n", op, payload_len,
                    payload);

        if (discord_conn_state == DISCORD_STATE_CONNECTED_WAIT_READY &&
            strstr(payload, "\"evt\":\"READY\""))
        {
            discord_conn_state = DISCORD_STATE_READY;
            discord_log("Discord: Ready event received.\n");
        }

        incoming_buf[8 + payload_len] = boundary_char;

        int packet_total_size = 8 + payload_len;
        memmove(incoming_buf, incoming_buf + packet_total_size,
                incoming_len - packet_total_size);
        incoming_len -= packet_total_size;
    }
}

#ifdef WIN32
DWORD WINAPI Discord_WorkerThread(LPVOID lpParam)
{


#else
void* Discord_WorkerThread(void* arg)
{
#endif
    discord_log("Discord: Background worker thread started.\n");
    while (thread_active)
    {
        char active_payload[1024] = "";
        qboolean has_update = qfalse;

        // Mutex protected swap of pending payload
#ifdef WIN32
        EnterCriticalSection(&payload_mutex);
#else
        pthread_mutex_lock(&payload_mutex);
#endif
        if (pending_update)
        {
            Q_strncpyz(active_payload, pending_payload, sizeof(active_payload));
            pending_update = qfalse;
            has_update = qtrue;
        }
#ifdef WIN32
        LeaveCriticalSection(&payload_mutex);
#else
        pthread_mutex_unlock(&payload_mutex);
#endif

        // Handle background connection and dispatching
        if (Discord_Connect())
        {
            Discord_Pump();
            if (has_update && discord_conn_state == DISCORD_STATE_READY)
            {
                uint32_t header[2];
                header[0] = 1;
                header[1] = (uint32_t)strlen(active_payload);
                if (Discord_WriteAll(header, sizeof(header)) &&
                    Discord_WriteAll(active_payload, header[1]))
                {
                    discord_log("Discord: Rich Presence update dispatched to client.\n");
                }
            }
        }
        else
        {
            // If disconnected, don't spin, drop pending update state so we don't
            // block
            if (has_update)
            {
                discord_log("Discord: Dropped update (not connected to client).\n");
            }
        }

#ifdef WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }

    Discord_ShutdownSocket();
    discord_log("Discord: Background worker thread terminated.\n");
    return 0;
}

void Discord_Init(void)
{
    if (thread_active)
        return;

    thread_active = qtrue;
    pending_update = qfalse;
    pending_payload[0] = '\0';
    next_discord_connect_time = 0;
    discord_conn_state = DISCORD_STATE_DISCONNECTED;

#ifdef WIN32
    InitializeCriticalSection(&payload_mutex);
    worker_thread = CreateThread(NULL, 0, Discord_WorkerThread, NULL, 0, NULL);
#else
    pthread_create(&worker_thread, NULL, Discord_WorkerThread, NULL);
#endif
}

void Discord_Shutdown(void)
{
    if (!thread_active)
        return;

    thread_active = qfalse;

#ifdef WIN32
    if (worker_thread != NULL)
    {
        WaitForSingleObject(worker_thread, 1000);
        CloseHandle(worker_thread);
        worker_thread = NULL;
    }
    DeleteCriticalSection(&payload_mutex);
#else
    pthread_join(worker_thread, NULL);
    pthread_mutex_destroy(&payload_mutex);
#endif
}

static void Discord_Update(void)
{
    if (!discord_needs_update)
        return;

    time_t cur_time = time(NULL);
    if (cur_time < next_allowed_update_time)
    {
        return;
    }

    discord_needs_update = 0;
    next_allowed_update_time = cur_time + 4;

    char details[256] = "";
    char state[256] = "";
    char timestamp_json[64] = "";

    // DYNAMIC MAIN MENU CHECK
    if (strcmp(discord_display, "#status_mainmenu") == 0 || !discord_display[0])
    {
        const char* mod_name = GetModDisplayName(discord_fs_game);
        snprintf(details, sizeof(details), "Main Menu");
        if (mod_name)
        {
            snprintf(state, sizeof(state), "%s", mod_name);
        }
        else
        {
            snprintf(state, sizeof(state), "Return to Castle Wolfenstein");
        }
    }
    else
    {
        int skill_int = atoi(discord_skill);
        const char* skill_name = GetFriendlySkillName(skill_int);

        char basename[64];
        ExtractMapBaseName(discord_mapname, basename, sizeof(basename));
        const char* dict_name = GetFriendlyMapName(basename);

        if (dict_name != NULL)
        {
            if (skill_name && skill_name[0])
            {
                snprintf(details, sizeof(details), "🕹️ %s (%s)", dict_name, skill_name);
            }
            else
            {
                snprintf(details, sizeof(details), "🕹️ %s", dict_name);
            }
        }
        else if (discord_cs_message[0])
        {
            char clean[128];
            StripColorCodes(discord_cs_message, clean, sizeof(clean));
            if (skill_name && skill_name[0])
            {
                snprintf(details, sizeof(details), "🕹️ %s (%s)", clean, skill_name);
            }
            else
            {
                snprintf(details, sizeof(details), "🕹️ %s", clean);
            }
        }
        else
        {
            const char* mod_name = GetModDisplayName(discord_fs_game);
            if (mod_name)
            {
                if (skill_name && skill_name[0])
                {
                    snprintf(details, sizeof(details), "🕹️ %s: %s (%s)", mod_name, basename, skill_name);
                }
                else
                {
                    snprintf(details, sizeof(details), "🕹️ %s: %s", mod_name, basename);
                }
            }
            else
            {
                if (skill_name && skill_name[0])
                {
                    snprintf(details, sizeof(details), "🕹️ %s (%s)", basename, skill_name);
                }
                else
                {
                    snprintf(details, sizeof(details), "🕹️ %s", basename);
                }
            }
        }

        // --- Dynamic Multi-Part State Builder ---
        qboolean is_survival =
            (Cvar_VariableIntegerValue("g_gametype") == 3); // GT_SURVIVAL = 3

        char health_str[96] = "";
        if (clc.state == CA_ACTIVE && cl.snap.valid)
        {
            int health = cl.snap.ps.stats[STAT_HEALTH];
            if (health < 0)
                health = 0;
            int max_health = cl.snap.ps.stats[STAT_MAX_HEALTH];
            if (max_health <= 0)
                max_health = 100;
            snprintf(health_str, sizeof(health_str), "❤️ %d/%d", health, max_health);
        }

        char wave_str[64] = "";
        char score_str[64] = "";
        char kills_str[64] = "";
        if (is_survival && clc.state == CA_ACTIVE && cl.snap.valid)
        {
            int wave = cl.snap.ps.persistant[PERS_WAVES];
            int kills = cl.snap.ps.persistant[PERS_KILLS];
            int points = cl.snap.ps.persistant[PERS_SCORE];
            snprintf(wave_str, sizeof(wave_str), "Wave %d", wave);
            snprintf(score_str, sizeof(score_str), "✪ Score %d", points);
            snprintf(kills_str, sizeof(kills_str), "☠️ Kills: %d", kills);
        }

        char secrets_str[64] = "";
        char treasure_str[64] = "";
        if (discord_cs_missionstats[0] &&
            strncmp(discord_cs_missionstats, "s=", 2) == 0)
        {
            char stats_buf[128];
            Q_strncpyz(stats_buf, discord_cs_missionstats + 2, sizeof(stats_buf));

            int sec = 0, sec_total = 0, treas = 0, treas_total = 0;
            sscanf(stats_buf, ",%*d,%*d,%*d,%*d,%*d,%d,%d,%d,%d", &sec, &sec_total,
                   &treas, &treas_total);

            if (sec_total > 0)
            {
                snprintf(secrets_str, sizeof(secrets_str), "🔍 Secrets: %d/%d", sec,
                         sec_total);
            }
            if (treas_total > 0)
            {
                snprintf(treasure_str, sizeof(treasure_str), "💰 Treasure: %d/%d", treas,
                         treas_total);
            }
        }

        char weap_str[64] = "";
        if (clc.state == CA_ACTIVE && cl.snap.valid)
        {
            int active_weap = cl.snap.ps.weapon;
            const char* weap_name = GetFriendlyWeaponName(active_weap);
            if (weap_name)
            {
                snprintf(weap_str, sizeof(weap_str), "%s", weap_name);
            }
        }

        // Build components array safely
        char parts[8][96];
        int num_parts = 0;

        // Part 1: Survival Wave Counter / Stats
        if (wave_str[0])
        {
            Q_strncpyz(parts[num_parts++], wave_str, sizeof(parts[0]));
        }
        if (score_str[0])
        {
            Q_strncpyz(parts[num_parts++], score_str, sizeof(parts[0]));
        }
        if (kills_str[0])
        {
            Q_strncpyz(parts[num_parts++], kills_str, sizeof(parts[0]));
        }

        // Part 2: Live Player Health Info
        if (health_str[0])
        {
            Q_strncpyz(parts[num_parts++], health_str, sizeof(parts[0]));
        }

        // Part 3: Active Weapon Info
        if (weap_str[0])
        {
            Q_strncpyz(parts[num_parts++], weap_str, sizeof(parts[0]));
        }

        // Part 4: Level Mission Statistics
        if (secrets_str[0])
        {
            Q_strncpyz(parts[num_parts++], secrets_str, sizeof(parts[0]));
        }
        if (treasure_str[0])
        {
            Q_strncpyz(parts[num_parts++], treasure_str, sizeof(parts[0]));
        }

        // Assembly loop
        state[0] = '\0';
        for (int i = 0; i < num_parts; i++)
        {
            Q_strcat(state, sizeof(state), parts[i]);
            if (i < num_parts - 1)
            {
                Q_strcat(state, sizeof(state), " • ");
            }
        }
    }

    if (start_time != 0)
    {
        snprintf(timestamp_json, sizeof(timestamp_json),
                 "\"timestamps\":{\"start\":%lld},", (long long)start_time);
    }

    // Escape details and state string for safe JSON construction
    char escaped_details[512];
    char escaped_state[512];
    EscapeJsonString(details, escaped_details, sizeof(escaped_details));
    EscapeJsonString(state, escaped_state, sizeof(escaped_state));

    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{"
             "\"cmd\":\"SET_ACTIVITY\","
             "\"args\":{"
             "\"pid\":%d,"
             "\"activity\":{"
             "\"details\":\"%s\","
             "\"state\":\"%s\","
             "%s"
             "\"assets\":{"
             "\"large_image\":\"realrtcw\","
             "\"large_text\":\"RealRTCW\""
             "}"
             "}"
             "},"
             "\"nonce\":\"1\""
             "}",
             (int)getpid(), escaped_details, escaped_state, timestamp_json);

    // Push new payload to worker thread
#ifdef WIN32
    EnterCriticalSection(&payload_mutex);
#else
    pthread_mutex_lock(&payload_mutex);
#endif
    Q_strncpyz(pending_payload, payload, sizeof(pending_payload));
    pending_update = qtrue;
#ifdef WIN32
    LeaveCriticalSection(&payload_mutex);
#else
    pthread_mutex_unlock(&payload_mutex);
#endif
}

void Discord_RunFrame(void)
{
    qboolean changed = qfalse;

    // Initialize background thread on the first run frame if not already running
    if (!thread_active)
    {
        Discord_Init();
    }

    // GLOBAL MOD CHECKING: Always monitor active mod directory, even at main menu
    const char* fsgame_val = Cvar_VariableString("fs_game");
    if (strcmp(discord_fs_game, fsgame_val) != 0)
    {
        Q_strncpyz(discord_fs_game, fsgame_val, sizeof(discord_fs_game));
        changed = qtrue;
    }

    if (clc.state >= CA_CONNECTED)
    {
        if (strcmp(discord_display, "#status_map") != 0)
        {
            Q_strncpyz(discord_display, "#status_map", sizeof(discord_display));
            changed = qtrue;
        }
        if (strcmp(discord_mapname, cl.mapname) != 0)
        {
            Q_strncpyz(discord_mapname, cl.mapname, sizeof(discord_mapname));
            changed = qtrue;
            start_time = time(NULL);
        }

        const char* cs_msg = "";
        if (cl.gameState.stringOffsets[CS_MESSAGE])
        {
            cs_msg = cl.gameState.stringData + cl.gameState.stringOffsets[CS_MESSAGE];
        }
        if (strcmp(discord_cs_message, cs_msg) != 0)
        {
            Q_strncpyz(discord_cs_message, cs_msg, sizeof(discord_cs_message));
            changed = qtrue;
        }

        const char* cs_stats = "";
        if (cl.gameState.stringOffsets[CS_MISSIONSTATS])
        {
            cs_stats =
                cl.gameState.stringData + cl.gameState.stringOffsets[CS_MISSIONSTATS];
        }
        if (strcmp(discord_cs_missionstats, cs_stats) != 0)
        {
            Q_strncpyz(discord_cs_missionstats, cs_stats,
                       sizeof(discord_cs_missionstats));
            changed = qtrue;
        }

        const char* skill_val = Cvar_VariableString("g_gameskill");
        if (strcmp(discord_skill, skill_val) != 0)
        {
            Q_strncpyz(discord_skill, skill_val, sizeof(discord_skill));
            changed = qtrue;
        }

        // MONITOR REAL-TIME PLAYER DATA CHANGES
        if (clc.state == CA_ACTIVE && cl.snap.valid)
        {
            int cur_health = cl.snap.ps.stats[STAT_HEALTH];
            if (cur_health < 0)
                cur_health = 0;
            if (cur_health != discord_last_health)
            {
                discord_last_health = cur_health;
                changed = qtrue;
            }

            int cur_wave = cl.snap.ps.persistant[PERS_WAVES];
            if (cur_wave != discord_last_wave)
            {
                discord_last_wave = cur_wave;
                changed = qtrue;
            }

            int cur_kills = cl.snap.ps.persistant[PERS_KILLS];
            if (cur_kills != discord_last_kills)
            {
                discord_last_kills = cur_kills;
                changed = qtrue;
            }

            int cur_score = cl.snap.ps.persistant[PERS_SCORE];
            if (cur_score != discord_last_score)
            {
                discord_last_score = cur_score;
                changed = qtrue;
            }

            int cur_weap = cl.snap.ps.weapon;
            if (cur_weap != discord_last_weapon)
            {
                discord_last_weapon = cur_weap;
                changed = qtrue;
            }
        }

        if (changed)
        {
            discord_needs_update = 1;
        }
    }
    else
    {
        if (strcmp(discord_display, "#status_mainmenu") != 0)
        {
            Q_strncpyz(discord_display, "#status_mainmenu", sizeof(discord_display));
            discord_mapname[0] = '\0';
            discord_skill[0] = '\0';
            discord_cs_message[0] = '\0';
            discord_cs_missionstats[0] = '\0';
            discord_last_health = -1;
            discord_last_wave = -1;
            discord_last_kills = -1;
            discord_last_score = -1;
            discord_last_weapon = -1;
            start_time = 0;
            changed = qtrue;
        }

        if (changed)
        {
            discord_needs_update = 1;
        }
    }

    if (discord_needs_update)
    {
        Discord_Update();
    }
}
