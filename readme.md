# RealRTCW Personal Fork: Engine Modernization & Performance Optimization

This repository is a modernized, high-performance fork of the Return to Castle Wolfenstein single-player engine. It optimizes the virtual filesystem, streamlines collision/map loading, upgrades the audio pipeline to OpenAL EFX/EAX, adds SIMD math speedups, and improves modern Linux/OS integration.

---

## Performance & Architectural Improvements

### 📦 Virtual Filesystem (VFS) & Zip Traversal (`files.c`)
* **PK3 Directory Caching (`pk3cache.dat`)**: Caches ZIP/PK3 directory scans in a binary cache file validated by size and timestamp.
  * *Benefit*: Bypasses minizip traversal on startup, significantly reducing game load times.
* **Intelligent Stream Seeking**: Overhauled zlib stream seeking to jump forward using absolute offsets in the decompression window.
  * *Benefit*: Avoids full zip rewinds and decompression passes during seek operations.
* **Expanded File Hash Table**: Increased `MAX_FILEHASH_SIZE` from 1,024 to 32,768.
  * *Benefit*: Reduces hash collision chains, keeping file lookup times near $O(1)$ under heavy mod loads.
* **AVX2 File Comparison**: Uses 256-bit AVX2 registers in `FS_FileCompare` to compare 32 bytes per cycle.
  * *Benefit*: Speeds up binary file verification and comparison passes.
* **Linear Path Normalization**: Replaced $O(N^2)$ array shifts in path sanitization with a single $O(N)$ two-pointer pass.
  * *Benefit*: Lowers CPU overhead when resolving long path sequences.
* **Multi-Segment Path Filtering**: Replaced legacy wildcard string matching with tokenized multi-segment filtering.
  * *Benefit*: Speeds up directory searches and asset matches across thousands of files.
* **Increased File Tracking Limits**: Raised `MAX_FOUND_FILES` limit.
  * *Benefit*: Prevents engine crashes when launching massive total-conversion mods.

### 📐 Collision Model & Map Loading (`cm_load.c` / `cm_trace.c`)
* **RAM-Based Map Caching**: Retains decompressed map collision hulls and geometry in system memory.
  * *Benefit*: Level loads and quickloads for the same map are near-instantaneous by bypassing disk extraction and parser passes.
* **Consolidated Map Streaming**: Removed the redundant `cm_stream.c` file and integrated dynamic map-switching directly into `cm_load.c` and the server startup pipeline.
  * *Benefit*: Streamlines code architecture and eliminates loading hitches during transitions.
* **Branchless Box Collision Math**: Redesigned spatial ray-intersection checks inside bounding box clipping tasks using branchless vector min/max selections.
  * *Benefit*: Prevents CPU instruction pipeline stalls by eliminating branch mispredictions.
* **Model Bounds Loading Fix**: Corrected modelbounds parsing logic.
  * *Benefit*: Prevents collision and tracking errors on world asset clip layers.

### 🔊 Dynamic Spatial Audio & Environmental Reverb (OpenAL EFX / EAX)
* **Dynamic Ray-Traced Reverb**: Casts geometric rays into the map to calculate room volume, outdoor exposure, and structural boundaries.
  * *Benefit*: Dynamically adjusts OpenAL EFX reverb parameters on the fly based on the player's immediate physical environment.
* **OpenAL EFX Occlusion**: Applies low-pass filters to sound sources when line-of-sight is blocked.
  * *Benefit*: Simulates realistic acoustic muffling behind walls, doors, or solid brushes.
* **EAX Reverb Presets & Crossfading**: Supports EAX presets with crossfading.
  * *Benefit*: Ensures smooth acoustic transitions when moving between indoor corridors and open outdoor zones.
* **HRTF Support**: Exposed Head-Related Transfer Function configuration in the UI.
  * *Benefit*: Delivers binaural 3D spatial audio for precise directional tracking on headphones.
* **X-Macro OpenAL Binding**: Uses compile-time X-Macros to initialize OpenAL function pointers.
  * *Benefit*: Removes runtime binding boilerplate, decreasing initialization overhead.
* **SIMD Audio Mixer**: Uses 128-bit SSE lanes and saturation packing (`_mm_packs_epi32`).
  * *Benefit*: Mixes up to 4 stereo streams in parallel without branching, preventing audio cracking during heavy combat.

### 📐 Core Math & Renderer Geometry (`q_math.c` / `q_shared.h` / `tr_mesh.c`)
* **SIMD Vector Normalization**: Replaced scalar vector normalizations with SSE intrinsics and reciprocal square root approximations (`_mm_rsqrt_ps`).
  * *Benefit*: Speeds up coordinate transformation math.
* **Branchless Coordinate Clamping**: Replaced clamping branches with `fminf` and `fmaxf`, compiling down to single-instruction `minss`/`maxss` assembly primitives.
  * *Benefit*: Avoids branch mispredictions during player movement and camera updates.
* **SIMD Fog Volume Checks**: Checks mesh bounding spheres against fog volume limits in parallel using SSE masking (`_mm_movemask_ps`).
  * *Benefit*: Speeds up atmospheric fog visibility calculations.
* **Inlined LOD Calculations**: Replaced indirect function calls in `R_ComputeLOD` with direct inline calculations.
  * *Benefit*: Eliminates call stack overhead when processing complex mesh details.
* **User Command Optimizations**: Streamlined view-angle calculations and command processing (`usercmd_t`).
  * *Benefit*: Reduces per-frame CPU consumption during high-rate player movements.

### 🤖 Bot Intelligence Optimization (`ai_main.c` / `g_bot.c`)
* **Fast Bot Pathing**: Uses hardware-native rounding (`roundf`) for fast angle wrapping in bot path navigation.
  * *Benefit*: Decreases bot decision-making overhead.
* **Vectorized Transform Copies**: Uses SSE unaligned loads/stores (`_mm_loadu_ps` / `_mm_storeu_ps`) to copy 3D transform structures in single operations.
  * *Benefit*: Minimizes memory bandwidth usage when updating bot structures.

### ⚡ Modernized x86_64 JIT QVM Compiler (`vm_x86.c` / `vm.c` / `vm_local.h`)
* **Dynamic Register Caching**: Caches QVM stack variables directly in x86-64 registers (RAX, RCX, RDX) via `vm_optimize.h`.
  * *Benefit*: Cuts stack memory traffic by up to 80%, accelerating virtual machine execution toward native speeds.
* **W^X Memory Hardening**: Transitioned JIT buffer allocation to a Write-or-Execute model using `mprotect` to switch memory pages to `PROT_READ | PROT_EXEC` after code emission.
  * *Benefit*: Protects the engine against dynamic code injection vulnerabilities.
* **Instruction Merging**: Merges related VM instructions into single machine code blocks and maps floating-point ceil/floor operations directly to SSE4.1 `roundss` instructions.
  * *Benefit*: Reduces instruction cache footprint and improves execution pipelining.
* **QVM Verification & Fixups**: Added pre-compilation instruction verification (`VM_CheckInstructions`), LCC compiler instruction fixups (`VM_Fixup`), and CRC32 lookup validation.
  * *Benefit*: Prevents crash loops when executing legacy or corrupt custom mods.
* **JIT Enabled by Default**: Configured the engine to default to JIT execution (`vm_* 1`) for game, cgame, and UI modules.
  * *Benefit*: Unlocks maximum mod execution performance out of the box.

---

## UI, Gameplay Systems, & Dev Infrastructure

* **\"Sofia\" Advanced Settings Menu**: Added a custom settings tab to control HRTF, audio occlusion, reverb, and Discord options. Refactored UI menu layouts to clean up padding.
  * *Benefit*: Provides a clean, accessible layout for new settings without text clipping or overlapping fields.
* **Discord Rich Presence**: Integrates game state (active mod, weapons with dynamic emojis, score/kills) into Discord via background thread sockets.
  * *Benefit*: Displays rich activity status to friends without introducing gameplay micro-stutters.
* **Tactical Flashlight**: Added a battery-powered flashlight with dynamic dimming curves, HUD indicators, and AI detection paths.
  * *Benefit*: Enhances tactical options with realistic battery consumption and AI stealth responses.
* **High-DPI Console Scaling**: Scales the console layout dynamically.
  * *Benefit*: Ensures the console is fully legible on modern high-resolution 1440p and 4K displays.
* **Bazzite & Distrobox Dev Support**: Configured build scripts for compiling within `Bazzite-dev-nvidia` Distrobox containers.
  * *Benefit*: Streamlines dev environment setup on immutable Linux OS configurations.
* **Engine Updates**: Added support for Steam Workshop paths on Linux, and updated zlib from `1.2.11` to `1.3.2`.
  * *Benefit*: Extends compatibility to modern workshop structures and speeds up file decompression.
* **Focus Loss Muting/Pausing**: Automatically pauses the game and mutes audio when the application window loses focus.
  * *Benefit*: Eliminates noise pollution and prevents in-game player deaths during Alt-Tabbing.
* **CI/CD Automation**: Automated package compilation and packaging through build scripts.
  * *Benefit*: Eliminates manual steps when building game release distributions.

---

## Hardware Architecture & Build Matrix

To build this modernized engine environment, the compilation pipeline targets native advanced hardware optimization flags:

```bash
# Target Compiler Flags for Optimized Native Execution
-mavx2 -mfma -msse4.1 -O3
```
