# RealRTCW Personal Fork: Engine Modernization & Performance Optimization

This repository is a modernized, high-performance fork of the **Return to Castle Wolfenstein** single-player engine[cite: 1]. It optimizes the virtual filesystem, streamlines collision and map loading, upgrades the audio pipeline to OpenAL EFX/EAX, introduces SIMD math accelerations, overhauls the legacy renderer for cinematic lighting, and delivers seamless integration for modern OS environments (including immutable Linux distros)[cite: 1].

---

## 🛠️ Architectural Overview

| Module | Focus Area | Key Technologies & Core Files |
| :--- | :--- | :--- |
| **Virtual Filesystem** | VFS Performance & I/O Reduction | `files.c`, AVX2, Directory Caching, Tokenized Filtering[cite: 1] |
| **Collision & Maps** | Physics Hulls & Spatial Queries | `cm_load.c`, `cm_trace.c`, Branchless Box Optimization[cite: 1] |
| **Multithreading** | Parallelization & Job Dispatching | `gp_jobsystem`, Skeletal Skinning, Async Audio Codecs[cite: 1] |
| **Audio Pipeline** | Spatial Audio & Reverb Simulation | OpenAL EFX, EAX, 128-bit SSE Audio Mixer[cite: 1] |
| **Renderer & Shading** | Cinematic Visuals & Tech Modernization | Z-Fail (Carmack's Reverse), Pseudo-Soft Shadows, PBR Attenuation[cite: 1] |
| **QVM Compiler** | Execution Performance & Security | `vm_x86.c`, `vm.c`, x86_64 JIT, W^X Memory Hardening[cite: 1] |
| **Memory Management** | Low-Latency Allocation | `mimalloc`, Eager Page Commits, 2MB Huge Pages[cite: 1] |

---

## ⚡ Performance & Architectural Improvements

### 📦 Virtual Filesystem (VFS) & Zip Traversal (`files.c`)
* **PK3 Directory Caching (`pk3cache.dat`)**: Caches ZIP/PK3 directory scans in a binary file validated by size and timestamp[cite: 1]. This bypasses minizip traversal on startup, dramatically slashing game boot times and eliminating cold-start delays[cite: 1].
* **Intelligent Stream Seeking**: Overhauls zlib stream seeking to jump forward using absolute offsets in the decompression window, improving background asset streaming responsiveness while reducing CPU load[cite: 1].
* **Expanded File Hash Table**: Increased `MAX_FILEHASH_SIZE` from 1,024 to 32,768, scaling up lookups to near $O(1)$ efficiency under heavy mod loads to eradicate micro-stutters[cite: 1].
* **AVX2 File Comparison**: Utilizes 256-bit AVX2 registers in `FS_FileCompare` to compare 32 bytes per cycle, accelerating binary verification passes prior to level entry[cite: 1].
* **Linear Path Normalization**: Replaces inefficient $O(N^2)$ array shifts in path sanitization with a streamlined $O(N)$ two-pointer pass, lowering CPU overhead and eliminating legacy buffer overrun vulnerabilities[cite: 1].
* **Multi-Segment Path Filtering**: Replaces legacy wildcard string matching with tokenized multi-segment filtering to speed up asset matching across thousands of files and strictly enforce absolute segment priorities[cite: 1].
* **Increased File Tracking Limits**: Raised the `MAX_FOUND_FILES` limit to allow the tracker to index thousands of custom assets simultaneously, preventing crashes on massive total-conversion mods[cite: 1].

### 📐 Collision Model & Map Loading (`cm_load.c` / `cm_trace.c`)
* **RAM-Based Map Caching**: Retains decompressed map collision hulls and geometry in system memory, making level loads and quickloads near-instantaneous by bypassing disk extraction[cite: 1].
* **Branchless Box Collision Math**: Redesigns spatial ray-intersection checks inside bounding box clipping tasks using branchless vector min/max selections, preventing CPU instruction pipeline stalls from branch mispredictions[cite: 1].
* **Model Bounds Loading Fix**: Corrected the modelbounds parsing logic on world asset clip layers, ensuring projectile hitboxes align perfectly with physics shapes at all angles[cite: 1].

### 🧵 Multi-Threaded Architecture & Parallelization (`gp_jobsystem`)
* **Parallel CPU Skeletal Vertex Skinning (`tr_animation.c`)**: Partitions large skeletal model surfaces (`numVerts > 256`) into 256-vertex chunks, distributing skinning across background workers to maximize high refresh-rate monitors[cite: 1].
* **Thread-Isolated Memory Alignment**: Configures concurrent skinning chunks to write into isolated target blocks to eliminate false cache-line sharing, optimizing L1/L2 cache efficiency[cite: 1].
* **Asynchronous Background Asset Loading (`snd_openal.c` / `snd_codec.c`)**: Offloads file loading and WAV/OGG/Opus decompression tasks to worker threads using a thread-safe custom allocator (`S_CodecAllocateTemp`)[cite: 1].

### 🔊 Dynamic Spatial Audio & Environmental Reverb (OpenAL EFX / EAX)
* **Dynamic Ray-Traced Reverb**: Casts geometric rays into the map to calculate room volume, outdoor exposure, and structural boundaries to dynamically adjust OpenAL EFX reverb parameters on the fly[cite: 1].
* **OpenAL EFX Occlusion**: Applies low-pass filters to sound sources when line-of-sight is blocked, realistically simulating acoustic muffling behind walls, doors, or solid geometry[cite: 1].
* **HRTF Support**: Exposes full Head-Related Transfer Function configurations directly in the UI, delivering precise binaural 3D spatial audio on headphones[cite: 1].
* **X-Macro API Bindings**: Consolidates dynamically loaded function pointer declarations, definitions, loading, and unloading routines for OpenAL and cURL using unified compile-time X-Macros[cite: 1].
* **SIMD Audio Mixer**: Employs 128-bit SSE lanes and saturation packing (`_mm_packs_epi32`) to mix up to 4 stereo streams in parallel without branching, completely eliminating audio cracking[cite: 1].

### 💡 Cinematic Rendering & Lighting Pipeline (`tr_shadows.c` / `tr_light.c` / `tr_bloom.c`)
* **Carmack's Reverse (Z-Fail Stencil Shadows)**: Switched the volumetric shadow rendering pipeline from Z-Pass to Z-Fail culling to prevent the entire screen from glitching when the camera clips inside a shadow volume[cite: 1].
* **Volumetric Contact Hardening (Pseudo-Soft Shadows)**: Injects a high-frequency, vertex-indexed dither pattern into the light extrusion vectors, creating realistic light penumbras that blur naturally over distance[cite: 1].
* **Cinematic Ambient Shadow Blending**: Replaces the hardcoded multiplicative gray blend with an alpha-blended, slate-blue tinted shadow pass, allowing shadows to absorb environmental ambient colors[cite: 1].
* **Extended Shadow Extrusion**: Increased the hardcoded shadow volume extrusion distance from 512 units to 3,000 units so shadows no longer disappear mid-air from high balconies[cite: 1].
* **Physically Based Dynamic Light Attenuation**: Replaces legacy inverse-square distance division with a windowed smooth-step falloff curve to generate smooth lighting gradients across models[cite: 1].
* **Intensity-Weighted Light Grid Direction**: Upgraded ambient map lighting trilerp calculations to weight directional vectors by their actual luminous intensity rather than spatial proximity, fixing severe shading flickers on moving NPCs[cite: 1].
* **Saturation-Preserving Tone Mapping (Color Clamp)**: Modifies the final entity lighting clamp to scale RGB vectors proportionally instead of clipping channels independently, preventing intense explosions from bleaching out[cite: 1].
* **Simulated Global Illumination (Bounce Light)**: Injects a fractional percentage of dynamic directed light back into the entity's ambient channel, simulating floor and wall light bounces without raytracing overhead[cite: 1].
* **Fast Surface-Aware Plane Culling (`R_DlightBmodel`)**: Evaluates individual polygon faces against active dynamic lights using accelerated dot-product plane filtering, cutting wasted GPU vertex cycles[cite: 1].
* **Modernized Aspect-Correct Bloom & Spectral Saturation**: Fixes legacy aspect-ratio viewport bugs and implements screen-space energy conservation blending alongside integrated $O(1)$ single-pass photographic highlight extraction[cite: 1].

### 🖼️ Modernized Backend Renderer & Geometry Streaming (`tr_surface.c`)
* **Immediate Mode Eradication (`RB_SurfaceBeam` / `RB_SurfaceAxis`)**: Stripped all legacy fixed-function immediate mode paths (`qglBegin`/`qglEnd`) from procedural entities, re-routing them to stream directly into the unified engine vertex and index geometry cache buffers[cite: 1].
* **Dynamic Light Pass Consolidation (`RB_SurfaceTriangles` / `RB_SurfaceFace`)**: Merged the secondary standalone bitwise masking loop (`vertexDlightBits`) directly into the main coordinate vertex allocation stride to optimize cache efficiency[cite: 1].
* **Branch-Hoisted Keyframe Interpolation (`LerpCMeshVertexes`)**: Hoisted structural model compression evaluation flags completely outside high-frequency vertex processing loops, eradicating instruction pipeline stalls[cite: 1].
* **Auto-Vectorized Array Normalization (`VectorArrayNormalize`)**: Re-engineered vector operations using loop unrolling by 4 and optimized coordinate bitwise shifts (`<< 2`) to fully saturate L1 data cache line bandwidth[cite: 1].
* **Dual-Channel Coordinate Streaming (`RB_SurfaceMesh` / `RB_SurfaceCMesh`)**: Refactored multi-stage UV texture loops to simultaneously push diffuse and lightmap properties across both coordinate streaming channels[cite: 1].

### 📐 Core Math & Renderer Geometry (`q_math.c` / `q_shared.h` / `tr_mesh.c`)
* **SIMD Vector Normalization**: Replaces scalar vector normalizations with SSE intrinsics and reciprocal square root approximations (`_mm_rsqrt_ps`), speeding up transformation math while safeguarding routines against division-by-zero errors[cite: 1].
* **Branchless Coordinate Clamping**: Replaces clamping branches with `fminf` and `fmaxf`, compiling down to single-instruction `minss`/`maxss` assembly primitives[cite: 1].
* **SIMD Fog Volume Checks**: Checks mesh bounding spheres against fog volume limits in parallel using SSE masking (`_mm_movemask_ps`), eliminating calculation lag or visual pops when crossing dynamic fog boundaries[cite: 1].
* **Inlined LOD Calculations**: Replaces indirect function calls in `R_ComputeLOD` with direct inline calculations, eliminating call stack overhead when processing complex mesh details[cite: 1].
* **User Command Optimizations**: Streamlines view-angle calculations and command processing (`usercmd_t`), minimizing input latency for high-polling-rate mice and gamepads[cite: 1].

### 🤖 Bot Intelligence Optimization (`ai_main.c` / `g_bot.c` / `ai_cast_sight.c`)
* **Fast Bot Pathing & Vectorized Transforms**: Uses hardware-native rounding (`roundf`) for fast angle wrapping in navigation meshes alongside SSE unaligned loads/stores (`_mm_loadu_ps` / `_mm_storeu_ps`) to copy 3D transform structures in single operations[cite: 1].
* **Vector Dot Product FOV Check**: Replaces nested conditional loops and expensive `AngleMod` calculations in visibility checks with a 3D vector dot product, bypassing multiple trigonometric conversions per sight check[cite: 1].
* **Early-Exit PVS Check**: Runs a fast Potential Visibility Set (`trap_InPVS`) check at the start of visibility routines to instantly discard calculations for characters separated by solid world geometry[cite: 1].
* **Proximity-Based Dynamic Time-Slicing**: Dynamically adjusts sight check delays based on the 3D distance between characters, lowering latency to 100ms for close-range actors while safely scaling up delays for far-away actors to 800ms[cite: 1].
* **Cached Animation Speed Lookup**: Caches movement speeds retrieved from the character animation script based on `aiState`, eradicating redundant complex animation script parsing and indexing operations per character every frame[cite: 1].

### ⚡ Modernized x86_64 JIT QVM Compiler (`vm_x86.c` / `vm.c`)
* **Dynamic Register Caching**: Caches QVM stack variables directly in x86-64 registers (`RAX`, `RCX`, `RDX`) via `vm_optimize.h`, cutting stack memory traffic by up to 80% and accelerating virtual machine execution toward native speeds[cite: 1].
* **W^X Memory Hardening**: Transitioned JIT buffer allocation to a strict Write-or-Execute model using `mprotect`, switching memory pages to `PROT_READ | PROT_EXEC` after code emission to secure the engine against dynamic code injection vulnerabilities[cite: 1].
* **Instruction Merging**: Merges related VM instructions into single machine code blocks and maps floating-point ceil/floor operations directly to hardware SSE4.1 `roundss` instructions, reducing the instruction cache footprint[cite: 1].
* **QVM Verification & Fixups**: Introduces pre-compilation instruction verification (`VM_CheckInstructions`), LCC compiler instruction fixups (`VM_Fixup`), and CRC32 lookup validation to prevent crash loops when executing legacy or malformed custom mods[cite: 1].
* **JIT Enabled by Default**: Configured the engine to default to JIT execution (`vm_* 1`) for game, cgame, and UI modules, unlocking maximum mod execution performance out of the box without manual user overrides[cite: 1].

---

## 🎮 UI, Gameplay Systems, & Dev Infrastructure

### ⚡ Low-Latency Memory Management (`mimalloc`)
> **Architectural Shift:** The legacy custom multi-segment zone sub-allocator (`Z_Malloc` and `Z_Free` in `common.c`) has been completely replaced with a direct mapping to Microsoft `mimalloc`. This eliminates double-allocation layers and mitigates memory-management execution overhead[cite: 1].

* **Eager Page Commits**: Configured memory segments to commit eagerly (`mi_option_eager_commit`) to physical RAM upon request, eliminating runtime micro-stutters and frame hitches when dynamic assets are loaded during high-action scenes[cite: 1].
* **Large OS Pages**: Enforces 2MB huge page allocation targets (`mi_option_large_os_pages`) on compatible Linux and Windows kernels to dramatically minimize CPU Translation Lookaside Buffer (TLB) cache misses[cite: 1].
* **Lock-Free Thread Safety**: Removed legacy spinlocks from engine memory pools. Multi-threaded worker tasks and job systems compile down to lock-free allocation paths using mimalloc's internal thread-local heap cache structures[cite: 1].
* **Diagnostic Telemetry**: Introduced the `/mi_stats` console command, exposing low-level allocator pool metrics, active blocks, and fragmentation reports live inside the developer console[cite: 1].

### 🎛️ Feature Additions & Enhancements
* **"Sofia" Advanced Settings Menu**: Features a custom settings tab to control HRTF, audio occlusion, reverb, and Discord options. The UI layout has been overhauled to eliminate text clipping or overlapping fields[cite: 1].
* **Discord Rich Presence**: Integrates game state (active mod, weapons with dynamic emojis, score/kills) into Discord via background thread sockets without introducing gameplay micro-stutters[cite: 1].
* **Tactical Flashlight**: Adds a battery-powered flashlight featuring dynamic dimming curves, standalone HUD indicators, and dedicated AI detection paths for enhanced stealth options[cite: 1].
* **WWII Realistic Bullet Physics & Ballistics**: Adds an option to toggle physical bullet projectiles instead of standard hitscan traces. Flight speeds are modeled after real-world muzzle velocities of WWII weapons (e.g., Luger, Colt, MP40, Mauser, Venom minigun) with optional gravity-drop trajectory simulations, while retaining full compatibility with headshot zones, damage falloff, and material penetration[cite: 1].
* **High-DPI Console & OS Enhancements**: Scales the developer console layout dynamically for 1440p and 4K displays. The engine also now automatically pauses and mutes audio when the application window loses focus (Alt-Tab)[cite: 1].
* **Bazzite & Distrobox Dev Support**: Configured custom build scripts for compiling within `Bazzite-dev-nvidia` Distrobox containers, streamlining dev environment setup on immutable Linux operating systems[cite: 1].
* **Modernized Dependencies & CI/CD**: Added native support for Steam Workshop paths on Linux, updated zlib from 1.2.11 to 1.3.2, and automated package compilation and release distribution packaging through streamlined CI/CD build scripts[cite: 1].
