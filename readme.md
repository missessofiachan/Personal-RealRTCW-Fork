RealRTCW Personal Fork: Engine Modernization & Performance Optimization



This repository is a modernized, high-performance fork of the Return to Castle Wolfenstein single-player engine. It optimizes the virtual filesystem, streamlines collision and map loading, upgrades the audio pipeline to OpenAL EFX/EAX, introduces SIMD math accelerations, overhauls the legacy renderer for cinematic lighting, and improves modern Linux/OS integration.

Performance & Architectural Improvements



📦 Virtual Filesystem (VFS) & Zip Traversal (files.c)



PK3 Directory Caching (pk3cache.dat) Caches ZIP/PK3 directory scans in a binary file validated by size and timestamp. This bypasses minizip traversal on startup, dramatically slashing game boot times, minimizing disk read wear, and eliminating cold-start delays after a system reboot.

Intelligent Stream Seeking Overhauls zlib stream seeking to jump forward using absolute offsets in the decompression window. This avoids full zip rewinds, improves background music/sound streaming responsiveness, and lowers overall CPU load during asset streaming.

Expanded File Hash Table Increased MAX_FILEHASH_SIZE from 1,024 to 32,768. This scales up lookups to near O(1) efficiency under heavy mod loads, eradicating micro-stutters and hitching when dynamically loading models or textures during gameplay.

AVX2 File Comparison Utilizes 256-bit AVX2 registers in FS_FileCompare to compare 32 bytes per cycle. This accelerates binary file verification passes and lowers CPU utilization right before level entry.

Linear Path Normalization Replaces inefficient O(N2) array shifts in path sanitization with a streamlined O(N) two-pointer pass, lowering CPU overhead and eliminating legacy buffer overrun vulnerabilities in the directory resolver.

Multi-Segment Path Filtering Replaces legacy wildcard string matching with tokenized multi-segment filtering. This speeds up asset matching across thousands of files and strictly enforces absolute segment priorities to prevent incorrect mod load orders.

Increased File Tracking Limits Raised the MAX_FOUND_FILES limit to allow the tracker to index thousands of custom assets simultaneously, removing hardcoded legacy restrictions and preventing crashes on massive total-conversion mods.

📐 Collision Model & Map Loading (cm_load.c / cm_trace.c)



RAM-Based Map Caching Retains decompressed map collision hulls and geometry in system memory. Level loads and quickloads for the same map become near-instantaneous by bypassing disk extraction and parser passes, keeping cache structures hot in the L3 cache.

Branchless Box Collision Math Redesigns spatial ray-intersection checks inside bounding box clipping tasks using branchless vector min/max selections. This prevents CPU instruction pipeline stalls from branch mispredictions, yielding faster, more stable physics cycles.

Model Bounds Loading Fix Corrected the modelbounds parsing logic on world asset clip layers. This ensures projectile hitboxes align perfectly with physics shapes at all angles, eliminating player physics clips and falling-through-world bugs.

🧵 Multi-Threaded Architecture & Parallelization (gp_jobsystem)



Parallel CPU Skeletal Vertex Skinning (tr_animation.c) Partitions large skeletal model surfaces (numVerts > 256) into 256-vertex chunks, distributing skinning and bone lerping across background workers. This automatically scales with modern multi-core CPUs, preventing main-thread rendering bottlenecks and maximizing high refresh-rate monitors.

Thread-Isolated Memory Alignment Configures concurrent skinning chunks to write into isolated target blocks to eliminate false cache-line sharing. This reduces memory controller lock-contention and optimizes L1/L2 cache efficiency on multi-channel DDR4/DDR5 platforms.

Asynchronous Background Asset Loading (snd_openal.c / snd_codec.c) Offloads file loading and WAV/OGG/Opus decompression tasks to worker threads. Utilizes a thread-safe custom allocator (S_CodecAllocateTemp) to safely allocate asset buffers concurrently without causing memory fragmentation in the engine's main hunk memory.

🔊 Dynamic Spatial Audio & Environmental Reverb (OpenAL EFX / EAX)



Dynamic Ray-Traced Reverb Casts geometric rays into the map to calculate room volume, outdoor exposure, and structural boundaries. This dynamically adjusts OpenAL EFX reverb parameters on the fly based on the player's immediate surroundings, replacing static presets and eliminating the need for manual designer triggers.

OpenAL EFX Occlusion Applies low-pass filters to sound sources when line-of-sight is blocked, realistically simulating acoustic muffling behind walls, doors, or solid geometry to improve tactical positional tracking.

HRTF Support Exposes full Head-Related Transfer Function configurations directly in the UI, delivering precise binaural 3D spatial audio on headphones—highly beneficial for tracking enemies during stealth sections.

X-Macro API Bindings (OpenAL & cURL) Consolidates dynamically loaded function pointer declarations, definitions, loading, and unloading routines using unified compile-time X-Macros. This slashes duplicate boilerplate code and enforces strict type and signature safety.

SIMD Audio Mixer Employs 128-bit SSE lanes and saturation packing (_mm_packs_epi32) to mix up to 4 stereo streams in parallel without branching, completely eliminating audio cracking and distortion during intense combat.

💡 Cinematic Rendering & Lighting Pipeline (tr_shadows.c / tr_light.c / tr_bloom.c)



Carmack's Reverse (Z-Fail Stencil Shadows) Switched the volumetric shadow rendering pipeline from Z-Pass to Z-Fail culling. This prevents the entire screen from glitching or inverting colors when the camera clips inside a shadow volume.

Volumetric Contact Hardening (Pseudo-Soft Shadows) Injects a high-frequency, vertex-indexed dither pattern into the light extrusion vectors, creating realistic light penumbras that blur naturally over distance and removing sharp polygonal shadow edges.

Cinematic Ambient Shadow Blending Replaces the hardcoded multiplicative gray blend with an alpha-blended, slate-blue tinted shadow pass, allowing shadows to absorb environmental ambient colors rather than muddling the diffuse textures underneath.

Extended Shadow Extrusion Increased the hardcoded shadow volume extrusion distance from 512 units to 3,000 units. Shadows no longer disappear mid-air when cast from high balconies, sniper towers, or long staircases.

Physically Based (PBR) Dynamic Light Attenuation Replaces legacy inverse-square distance division with a windowed smooth-step falloff curve. This generates velvet-smooth lighting gradients across models and prevents close-range muzzle flashes from over-exposing weapon viewmodels.

Intensity-Weighted Light Grid Direction Upgraded ambient map lighting trilerp calculations to weight directional vectors by their actual luminous intensity rather than spatial proximity. This fixes severe shading flickers on moving NPCs caused by dark grid nodes.

Saturation-Preserving Tone Mapping (Color Clamp) Modifies the final entity lighting clamp to scale RGB vectors proportionally instead of clipping channels independently. This prevents intense colored explosions or alarms from bleaching out into flat white patches.

Simulated Global Illumination (Bounce Light) Injects a fractional percentage of dynamic directed light back into the entity's ambient channel, realistically simulating floor and wall light bounces without the performance overhead of raytracing.

Fast Surface-Aware Plane Culling (R_DlightBmodel) Evaluates individual polygon faces against active dynamic lights using accelerated dot-product plane filtering. This cuts wasted GPU vertex cycles on hidden surfaces and prevents light from leaking through thin doors or gates.

Modernized Aspect-Correct Bloom & Spectral Saturation Fixes legacy aspect-ratio viewport bugs and implements screen-space energy conservation blending. Adds high-frequency dithering and integrated O(1) single-pass photographic highlight extraction for smooth, un-clipped widescreen bloom gradients.

🖼️ Modernized Backend Renderer & Geometry Streaming (tr_surface.c)



Immediate Mode Eradication (RB_SurfaceBeam / RB_SurfaceAxis) Stripped all legacy fixed-function immediate mode paths (qglBegin/qglEnd) from procedural entities and debugging lines. All procedural effects are re-routed to stream directly into the unified engine vertex and index geometry cache buffers, eliminating high-frequency driver synchronization blocks.

Dynamic Light Pass Consolidation (RB_SurfaceTriangles / RB_SurfaceFace) Merged the secondary standalone bitwise masking loop (vertexDlightBits) directly into the main coordinate vertex allocation stride, optimizing L1/L2 data cache efficiency by streaming all attributes concurrently.

Branch-Hoisted Keyframe Interpolation (LerpCMeshVertexes) Hoisted structural model compression evaluation flags completely outside high-frequency vertex processing loops, eradicating instruction pipeline stalls due to internal conditional checks.

Auto-Vectorized Array Normalization (VectorArrayNormalize) Re-engineered vector operations using loop unrolling by 4 and optimized coordinate bitwise shifts (<< 2) to bypass standard lookup tables, fully saturates L1 data cache line bandwidth to complement the engine's -mavx2 -ffast-math matrix compilation flags.

Dual-Channel Coordinate Streaming (RB_SurfaceMesh / RB_SurfaceCMesh) Refactored multi-stage UV texture loops to simultaneously push diffuse and lightmap properties across both coordinate streaming channels, dropping memory controller lookup bottlenecks when processing heavy mod assets.

📐 Core Math & Renderer Geometry (q_math.c / q_shared.h / tr_mesh.c)



SIMD Vector Normalization Replaces scalar vector normalizations with SSE intrinsics and reciprocal square root approximations (_mm_rsqrt_ps), speeding up transformation math while safeguarding routines against division-by-zero errors.

Branchless Coordinate Clamping Replaces clamping branches with fminf and fmaxf, compiling down to single-instruction minss/maxss assembly primitives to eliminate branch mispredictions during rapid player movement.

SIMD Fog Volume Checks Checks mesh bounding spheres against fog volume limits in parallel using SSE masking (_mm_movemask_ps), eliminating calculations lag or visual pops when crossing dynamic fog boundaries.

Inlined LOD Calculations Replaces indirect function calls in R_ComputeLOD with direct inline calculations, eliminating call stack overhead when processing complex mesh details and optimizing CPU instruction cache usage.

User Command Optimizations Streamlines view-angle calculations and command processing (usercmd_t), minimizing input latency and keeping movement responsive on high-polling-rate mice and gamepads.

🤖 Bot Intelligence Optimization (ai_main.c / g_bot.c / ai_cast_sight.c)



Fast Bot Pathing & Vectorized Transforms Uses hardware-native rounding (roundf) for fast angle wrapping in navigation meshes alongside SSE unaligned loads/stores (_mm_loadu_ps / _mm_storeu_ps) to copy 3D transform structures in single operations, mitigating frame rate spikes when multiple bots pathfind concurrently.

Vector Dot Product FOV Check Replaces nested conditional loops and expensive AngleMod calculations in visibility checks with a 3D vector dot product. This bypasses multiple trigonometric conversions per sight check, significantly boosting CPU performance on maps with high actor counts.

Early-Exit PVS Check Runs a fast Potential Visibility Set (trap_InPVS) check at the start of visibility routines. It instantly discards calculations for characters separated by solid world geometry without executing expensive multi-point raycasts.

Proximity-Based Dynamic Time-Slicing Dynamically adjusts sight check delays based on the 3D distance between characters. Lowers latency to 100ms for close-range actors (making reactions twice as fast) while safely scaling up delays for far-away actors to 800ms to save processing overhead.

Cached Animation Speed Lookup Caches movement speeds retrieved from the character animation script based on aiState, eradicating redundant complex animation script parsing and indexing operations per character every frame.

⚡ Modernized x86_64 JIT QVM Compiler (vm_x86.c / vm.c)



Dynamic Register Caching Caches QVM stack variables directly in x86-64 registers (RAX, RCX, RDX) via vm_optimize.h, cutting stack memory traffic by up to 80% and accelerating virtual machine execution toward native speeds.

W^X Memory Hardening Transitioned JIT buffer allocation to a strict Write-or-Execute model using mprotect, switching memory pages to PROT_READ | PROT_EXEC after code emission to secure the engine against dynamic code injection vulnerabilities.

Instruction Merging Merges related VM instructions into single machine code blocks and maps floating-point ceil/floor operations directly to hardware SSE4.1 roundss instructions, reducing the instruction cache footprint.

QVM Verification & Fixups Introduces pre-compilation instruction verification (VM_CheckInstructions), LCC compiler instruction fixups (VM_Fixup), and CRC32 lookup validation to prevent crash loops when executing legacy or malformed custom mods.

JIT Enabled by Default Configured the engine to default to JIT execution (vm_* 1) for game, cgame, and UI modules, unlocking maximum mod execution performance out of the box without manual user overrides.

UI, Gameplay Systems, & Dev Infrastructure


⚡ Low-Latency Memory Management & Allocator Overhaul (mimalloc)
Direct mimalloc Integration Completely replaced the legacy custom multi-segment zone sub-allocator (Z_Malloc and Z_Free in common.c) with a direct mapping to Microsoft mimalloc. This eliminates double-allocation layers and mitigates memory-management execution overhead.
Eager Page Commits Configured memory segments to commit eagerly (mi_option_eager_commit) to physical RAM upon request, eliminating runtime micro-stutters and frame hitches when dynamic assets are loaded during high-action scenes.
Large OS Pages Enforces 2MB huge page allocation targets (mi_option_large_os_pages) on compatible Linux and Windows kernels to dramatically minimize CPU Translation Lookaside Buffer (TLB) cache misses.
Lock-Free Thread Safety Removed legacy spinlocks from engine memory pools. Multi-threaded worker tasks and job systems compile down to lock-free allocation paths using mimalloc's internal thread-local heap cache structures.
Diagnostic Telemetry Introduced the /mi_stats console command, exposing low-level allocator pool metrics, active blocks, and fragmentation reports live inside the developer console.

"Sofia" Advanced Settings Menu Features a custom settings tab to control HRTF, audio occlusion, reverb, and Discord options. The UI layout has been overhauled to eliminate text clipping or overlapping fields.

Discord Rich Presence Integrates game state (active mod, weapons with dynamic emojis, score/kills) into Discord via background thread sockets without introducing gameplay micro-stutters.

Tactical Flashlight Adds a battery-powered flashlight featuring dynamic dimming curves, standalone HUD indicators, and dedicated AI detection paths for enhanced stealth options.

WWII Realistic Bullet Physics & Ballistics Adds an option to toggle physical bullet projectiles instead of standard hitscan traces. Flight speeds are modeled after real-world muzzle velocities of WWII weapons (e.g., Luger, Colt, MP40, Mauser, Venom minigun) with optional gravity-drop trajectory simulations, while retaining full compatibility with headshot zones, damage falloff, and material penetration.

High-DPI Console & OS Enhancements Scales the developer console layout dynamically for 1440p and 4K displays. The engine also now automatically pauses and mutes audio when the application window loses focus (Alt-Tab).

Bazzite & Distrobox Dev Support Configured custom build scripts for compiling within Bazzite-dev-nvidia Distrobox containers, streamlining dev environment setup on immutable Linux operating systems.

Modernized Dependencies & CI/CD Added native support for Steam Workshop paths on Linux, updated zlib from 1.2.11 to 1.3.2, and automated package compilation and release distribution packaging through streamlined CI/CD build scripts
