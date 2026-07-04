# 🚀 RealRTCW Personal Fork: Engine Modernization & Performance Optimization

This repository contains a modernized, high-performance fork of the Return to Castle Wolfenstein single-player engine core. By aggressively eliminating legacy scalar constraints, flattening high-frequency conditional branches, optimizing virtual filesystem bottlenecks, and introducing real-time ray-traced audio architectures, this fork transforms the historical codebase into a highly responsive, SIMD-driven pipeline engineered for modern x86_64 systems.

---

## 🛠️ Performance & Architectural Modernizations

### 📦 The Virtual Filesystem Tier (`files.c`)
* **PK3 Structural Index Cache (`pk3cache.dat`)**: Replaced expensive startup ZIP directory scans with an instant binary index cache database. Validated by OS file size and modification time stamps, it bypasses standard minizip traversal to accelerate mod asset loading by up to 100x.
* **Intelligent Stream Seeking**: Overhauled the raw inflation seek pipeline. The engine calculates absolute offsets and jumps forward within the active `zlib` sliding window stream, completely eliminating the legacy penalty of full decompression rewinds.
* **Elastic Hash Table Scalability**: Expanded `MAX_FILEHASH_SIZE` from `1024` to `32768`, drastically flattening hash collision chains down to near $O(1)$ lookup complexity for heavy total-conversion map modifications.
* **256-Bit AVX2 File Evaluator**: Integrated hand-tuned AVX2 vector instructions directly into binary file checking routines (`FS_FileCompare`). It streams up to 32 bytes concurrently per clock cycle directly into hardware YMM vector registers with zero loop branching.
* **Single-Pass Path Sanitization**: Rewrote legacy path normalizations from slow $O(N^2)$ structural array shifts into a highly responsive two-pointer reader/writer path configuration that strips duplicate or invalid separators in a single $O(N)$ pass.
* **Multi-Segment Path Filtering**: Completely replaced the old string matching layout with a high-performance multi-segment tokenization system, drastically accelerating file lookups and matching passes across thousands of files.
* **Expanded File Found Capacity**: Increased `MAX_FOUND_FILES` tracking limits to handle massive mod installations seamlessly without crashing or truncating file assets.

### 🔊 Dynamic Spatial Audio & Environmental Reverb (OpenAL EFX / EAX)
* **Dynamic Ray-Traced Reverb**: Implemented a highly responsive acoustic tracing engine. The game dynamically casts geometric rays into the active map layout to calculate real-time room volume, outdoor exposure, and structural boundaries, adjusting OpenAL EFX reverb parameters instantly on the fly.
* **OpenAL EFX Occlusion Filtering**: Injected native acoustic occlusion filters. Sound sources are dynamically processed through low-pass frequency filters when geometric line-of-sight is obstructed, realistically muffling sound propagation behind walls, doors, and solid brushes.
* **EAX Preset Architecture & Crossfading**: Integrated advanced EAX environmental reverb presets featuring seamless crossfading transitions, allowing smooth acoustic morphing when transitioning from tight indoor corridors to wide-open outdoor areas.
* **Native 3D HRTF Subsystem**: Exposed full Head-Related Transfer Function (HRTF) configuration controls directly to the user interface, unlocking authentic binaural 3D audio spatialization for precise directional audio tracking.
* **Streamlined X-Macro Subsystem Initialization**: Consolidated and optimized the entire OpenAL function pointer allocation backend using compile-time X-Macros, eliminating runtime overhead and improving code maintainability.
* **Hardware-Saturated SIMD Audio Mixer**: Leveraged 128-bit vector lanes and hardware-level saturation packing primitives (`_mm_packs_epi32`) to mix audio channels in parallel chunks, combining and clamping up to 4 stereo audio streams concurrently in a single CPU cycle to completely avoid dynamic boundary branches.

### 📐 Core Math & Renderer Geometry Engines (`q_math.c` / `q_shared.h` / `tr_mesh.c`)
* **Vectorized Norm Primitives**: Upgraded 3D vector normalizations using 128-bit SIMD intrinsics. Costly legacy scalar math pipelines are replaced with hardware-native inverse square root approximations (`_mm_rsqrt_ps`).
* **Branchless Boundary Clamping**: Eliminated CPU branch misprediction penalties by converting coordinate clamping gates to hardware-native `fminf` and `fmaxf` functions, flattening paths into single-cycle assembly primitives (`minss`/`maxss`).
* **Parallel 3-Axis Fog Masking**: Accelerated atmospheric fog matching calculations by loading mesh bounding sphere coordinates and fog volume limits directly into SSE vector lanes, processing three-dimensional spatial bounding checks across all three axes simultaneously via parallel masking (`_mm_movemask_ps`).
* **Direct Truncation Bypass**: Replaced serialized indirect function pointer calls inside `R_ComputeLOD` with direct inline integer typecasting to un-serialize the instruction scheduling engine.
* **User Command & Angle Optimization**: Streamlined view-angle calculations and optimized high-frequency user command handling (`usercmd_t`) to drop processing overhead during intense player movement frames.

### 💥 Multi-Threaded Job System & Collision Engine (`cm_trace.c`)
* **Parallelized Particle Subsystem**: Decoupled the engine core's frame dependencies by implementing a multi-threaded background job engine, allowing parallelized runtime processing of engine particle updates.
* **Branchless Box Collision Math**: Redesigned spatial ray-intersection logic inside bounding box clipping tasks using branchless vector min/max selections to eliminate structural branch chains.
* **Model Bounds Loading Fix**: Corrected historical modelbounds parsing logic to guarantee error-free tracking of world asset collision layers.

### 🤖 Bot Intelligence Optimization (`ai_main.c` / `g_bot.c`)
* **Branchless Route Wrapping**: Injected mathematical angle calculation wrapping using hardware rounding primitives (`roundf`), allowing pathing bots to instantly calculate shortest-turn steering paths.
* **SIMD Snapshot Gatherers**: Integrated 128-bit unaligned memory instructions (`_mm_storeu_ps` / `_mm_loadu_ps`) to copy transform state structures to entity fields simultaneously.

---

## 🎨 UI, Gameplay Systems, & Dev Infrastructure

* **The "Sofia" Custom Settings Subsystem**: Designed and integrated a bespoke user interface tab dedicated to housing advanced options, including custom audio controls (HRTF, occlusion, reverb behaviors) and dedicated Discord parameters. UI menu layout constraints and padding metrics were fully refactored to eliminate clipping bugs and remove legacy duplicate option listings.
* **Native Discord Rich Presence**: Implemented deep gameplay integration featuring lookups for customized mods, active weapons labeled with dynamic emojis, live kills/score mapping, and background thread socket isolation. Includes API throttling protection to protect system performance.
* **Tactical Flashlight Subsystem**: Introduced a functional flashlight mechanic complete with dynamic battery dimming curves, customized HUD indicators, and reactive AI detection paths.
* **Dynamic High-DPI Console Scaling**: Replaced legacy fixed pixel console metrics with an adaptable system that handles character sizing gracefully on modern high-resolution monitors.
* **Linux Containerization & Distrobox Support**: Built native build-time environment detection tailored for containerized development environments, explicitly supporting compilation inside `Bazzite-dev-nvidia` containers with automated context messaging.
* **Modernized Foundations**: Native support for Linux Steam Workshop directories alongside upgrading the core decompression backbone from Zlib `1.2.11` to the highly optimized Zlib `1.3.2` framework.
* **Focus Loss Automation**: Injected automated game pausing and sound muting routines triggered instantly when the application loses operating system window focus.
* **CI/CD Automation**: Authored complete local build and deployment shell scripts to completely automate compile-to-package pipelines.

---

## 🛠️ Hardware Architecture & Build Matrix

To build this modernized engine environment, the compilation pipeline targets native advanced hardware optimization flags:

```bash
# Target Compiler Flags for Optimized Native Execution
-mavx2 -mfma -msse4.1 -O3
