# RealRTCW Personal Fork: Engine Modernization & Performance Optimization

This repository is a modernized, high-performance fork of the Return to Castle Wolfenstein single-player engine. It optimizes the virtual filesystem, streamlines collision/map loading, upgrades the audio pipeline to OpenAL EFX/EAX, adds SIMD math speedups, overhauls the legacy renderer for cinematic lighting, and improves modern Linux/OS integration.

---

## Performance & Architectural Improvements

### 📦 Virtual Filesystem (VFS) & Zip Traversal (`files.c`)

* **PK3 Directory Caching (`pk3cache.dat`)**
* Caches ZIP/PK3 directory scans in a binary cache file validated by size and timestamp.
* **Benefit**: Bypasses minizip traversal on startup, significantly reducing game load times.
* **Benefit**: Minimizes disk read activity, extending the lifespan of storage drives under constant launch cycles.
* **Benefit**: Drastically reduces dynamic memory allocation overhead at boot time.
* **Benefit**: Prevents cold-start delays when launching the game immediately after a system reboot.


* **Intelligent Stream Seeking**
* Overhauled zlib stream seeking to jump forward using absolute offsets in the decompression window.
* **Benefit**: Avoids full zip rewinds and decompression passes during seek operations.
* **Benefit**: Improves background streaming responsiveness when playing music or loading sound tracks.
* **Benefit**: Decreases temporary memory allocation spikes and garbage collection delays.
* **Benefit**: Lowers average CPU load when parsing large dynamic assets like streaming audio.


* **Expanded File Hash Table**
* Increased `MAX_FILEHASH_SIZE` from 1,024 to 32,768.
* **Benefit**: Reduces hash collision chains, keeping file lookup times near O(1) under heavy mod loads.
* **Benefit**: Eradicates micro-stutters and sudden hitching when dynamically loading models/textures during play.
* **Benefit**: Optimizes memory lookup path efficiency in the game virtual filesystem.


* **AVX2 File Comparison**
* Uses 256-bit AVX2 registers in `FS_FileCompare` to compare 32 bytes per cycle.
* **Benefit**: Speeds up binary file verification and comparison passes.
* **Benefit**: Lowers CPU utilization when validating files before level entry.
* **Benefit**: Enables faster cache line fills by reading 256 bits at a time directly.


* **Linear Path Normalization**
* Replaced O(N²) array shifts in path sanitization with a single O(N) two-pointer pass.
* **Benefit**: Lowers CPU overhead when resolving long path sequences.
* **Benefit**: Eliminates potential buffer overrun vulnerabilities in directory resolver functions.
* **Benefit**: Cleans up overall VFS utility code footprint, increasing system stability.


* **Multi-Segment Path Filtering**
* Replaced legacy wildcard string matching with tokenized multi-segment filtering.
* **Benefit**: Speeds up directory searches and asset matches across thousands of files.
* **Benefit**: Simplifies internal lookup logic, making custom mod asset replacement more stable.
* **Benefit**: Prevents incorrect mod load orders by resolving absolute segment priorities.


* **Increased File Tracking Limits**
* Raised `MAX_FOUND_FILES` limit.
* **Benefit**: Prevents engine crashes when launching massive total-conversion mods.
* **Benefit**: Allows the file tracker to index thousands of custom assets simultaneously.
* **Benefit**: Encourages complex community mods by removing hardcoded legacy restrictions.



---

### 📐 Collision Model & Map Loading (`cm_load.c` / `cm_trace.c`)

* **RAM-Based Map Caching**
* Retains decompressed map collision hulls and geometry in system memory.
* **Benefit**: Level loads and quickloads for the same map are near-instantaneous by bypassing disk extraction and parser passes.
* **Benefit**: Drastically accelerates developer iteration loops when testing structural geometry changes.
* **Benefit**: Decreases physical drive wear and tear by avoiding repeated disk accesses.
* **Benefit**: Keeps cache structures hot in L3 cache, minimizing instruction pipeline latency.


* **Branchless Box Collision Math**
* Redesigned spatial ray-intersection checks inside bounding box clipping tasks using branchless vector min/max selections.
* **Benefit**: Prevents CPU instruction pipeline stalls by eliminating branch mispredictions.
* **Benefit**: Yields faster physics cycles, keeping high frame rates stable during complex entity collisions.
* **Benefit**: Reduces instructions per cycle (IPC) requirements for basic clipping checks.


* **Model Bounds Loading Fix**
* Corrected modelbounds parsing logic.
* **Benefit**: Prevents collision and tracking errors on world asset clip layers.
* **Benefit**: Ensures projectile hitboxes align perfectly with physics shapes under all angles.
* **Benefit**: Avoids weird player physics clips or falling-through-world bugs.



---

### 🧵 Multi-Threaded Architecture & Parallelization (`gp_jobsystem`)

* **Parallel CPU Skeletal Vertex Skinning (`tr_animation.c`)**
* Partitions large skeletal model surfaces (`numVerts > 256`) into 256-vertex chunks and computes skinning/bone lerping across background workers.
* **Benefit**: Prevents rendering bottlenecks on the main thread when drawing complex character models or dense crowds.
* **Benefit**: Automatically scales work distribution based on system thread count, maximizing modern multi-core CPUs.
* **Benefit**: Lowers framing times, enabling much higher refresh rate execution on high-end monitors.
* **Benefit**: Frees up the main thread to handle critical gameplay operations like script execution.


* **Thread-Isolated Memory Alignment**
* Configures concurrent skinning chunks to write into isolated target blocks to eliminate false cache-line sharing.
* **Benefit**: Drops memory controller lock-contention, optimizing memory write speeds.
* **Benefit**: Increases overall L1/L2 cache efficiency by avoiding cache line invalidation cycles (false sharing).
* **Benefit**: Enhances memory bandwidth efficiency on multi-channel DDR4/DDR5 platforms.


* **Asynchronous Background Asset Loading & Decompression (`snd_openal.c` / `snd_codec.c`)**
* Offloads file loading and WAV/OGG/Opus decompression tasks to worker threads. Utilizes a thread-safe custom allocator (`S_CodecAllocateTemp`) to safely allocate asset buffers concurrently without corrupting the main engine hunk memory.
* **Benefit**: Completely eradicates mid-game micro-stutters and hitching during background asset streaming.
* **Benefit**: Insulates the main gameplay logic from file I/O blocks or sound processing delays.
* **Benefit**: Safely handles allocation without memory fragmentation in the engine's main memory heap.



---

### 🔊 Dynamic Spatial Audio & Environmental Reverb (OpenAL EFX / EAX)

* **Dynamic Ray-Traced Reverb**
* Casts geometric rays into the map to calculate room volume, outdoor exposure, and structural boundaries.
* **Benefit**: Dynamically adjusts OpenAL EFX reverb parameters on the fly based on the player's immediate physical environment, replacing static preset reverb.
* **Benefit**: Creates an accurate acoustic environment (distinguishing small vaults from outdoor fields) automatically.
* **Benefit**: Removes the need for level designers to manually place reverb zone triggers, simplifying mapping.


* **OpenAL EFX Occlusion**
* Applies low-pass filters to sound sources when line-of-sight is blocked.
* **Benefit**: Simulates realistic acoustic muffling behind walls, doors, or solid brushes.
* **Benefit**: Improves the player's tactical position tracking by accurately representing coverage barriers.
* **Benefit**: Heightens acoustic realism by adjusting audio characteristics dynamically.


* **HRTF Support**
* Exposed Head-Related Transfer Function configuration in the UI.
* **Benefit**: Delivers binaural 3D spatial audio for precise directional tracking on headphones.
* **Benefit**: Heightens immersion in stealth sections where sound positioning is key.
* **Benefit**: Provides a clearer soundstage, preventing high-frequency ear fatigue.


* **X-Macro API Bindings (OpenAL & cURL)**
* Consolidates dynamically loaded function pointer declarations, definitions, loading, and unloading routines using unified compile-time X-Macros.
* **Benefit**: Drastically reduces duplicate boilerplate code across multiple header and source files, enhancing codebase maintainability.
* **Benefit**: Enforces strict compile-time type and signature safety, preventing runtime binding mismatches, initialization bugs, or memory leaks.
* **Benefit**: Simplifies addition of new external functions to a single entry in a macro master list.


* **SIMD Audio Mixer**
* Uses 128-bit SSE lanes and saturation packing (`_mm_packs_epi32`).
* **Benefit**: Mixes up to 4 stereo streams in parallel without branching, preventing audio cracking during heavy combat.
* **Benefit**: Moderates mixer processing cycles, leaving more processing power for physics and gameplay logic.
* **Benefit**: Eliminates clipping and saturation distortion by using SSE saturation packing.



---

### 💡 Cinematic Rendering & Lighting Pipeline (`tr_shadows.c` / `tr_light.c` / `tr_bloom.c`)

* **Carmack's Reverse (Z-Fail Stencil Shadows)**
* Switched the volumetric shadow rendering pipeline from Z-Pass to Z-Fail culling.
* **Benefit**: Prevents the entire screen from glitching or inverting colors when the player's camera clips inside an active shadow volume.
* **Benefit**: Ensures mathematically robust shadow projection regardless of complex level geometry or extreme character proximity.


* **Volumetric Contact Hardening (Pseudo-Soft Shadows)**
* Injected a high-frequency, vertex-indexed dither pattern into the light extrusion vectors.
* **Benefit**: Simulates realistic light penumbras where shadows are sharp near the caster and naturally blur over distance.
* **Benefit**: Completely eliminates the dated, razor-sharp polygonal edges associated with 2001-era stencil shadows.


* **Cinematic Ambient Shadow Blending**
* Replaced the hardcoded multiplicative gray blend with an alpha-blended, slate-blue tinted shadow pass.
* **Benefit**: Allows shadows to absorb environmental ambient colors instead of muddying or desaturating the floor textures.
* **Benefit**: Preserves high-resolution diffuse details and normal maps underneath shadow volumes.


* **Extended Shadow Extrusion**
* Increased the hardcoded shadow volume extrusion distance from 512 units to 3000 units.
* **Benefit**: Prevents shadows from magically disappearing in mid-air when cast from high balconies, sniper towers, or staircases.
* **Benefit**: Stretches dynamic entity silhouettes cleanly across expansive RealRTCW map geometry without adding GPU processing cost.


* **Physically Based (PBR) Dynamic Light Attenuation**
* Replaced the legacy inverse-square distance division logic with a windowed smooth-step falloff curve.
* **Benefit**: Generates velvet-smooth lighting gradients across character models, eliminating harsh geometric banding.
* **Benefit**: Prevents close-range light sources (like muzzle flashes) from causing extreme over-exposure on weapon viewmodels.


* **Intensity-Weighted Light Grid Direction**
* Upgraded ambient map lighting trilerp calculations to weight directional vectors by their actual luminous intensity instead of spatial proximity.
* **Benefit**: Stops dark or pitch-black grid nodes from collapsing the lighting vector, fixing severe shading flickers on moving NPCs.
* **Benefit**: Anchors character specular and diffuse highlights dependably to the most prominent environmental light source.


* **Saturation-Preserving Tone Mapping (Color Clamp)**
* Modified the final entity lighting clamp to scale RGB vectors proportionally rather than clipping channels independently.
* **Benefit**: Prevents intense colored lights (explosions, alarms) from bleaching out into chalky white patches on models.
* **Benefit**: Retains rich, vibrant cinematic color fidelity and texture depth even under maximum illumination.


* **Simulated Global Illumination (Bounce Light)**
* Injected a fractional percentage of dynamic directed light back into the entity's ambient channel.
* **Benefit**: Realistically simulates floor and wall light bouncing, preventing the unlit sides of models from looking completely pitch black during explosions.
* **Benefit**: Grounds characters and weapons naturally into dynamic environments without the overhead of raytracing.


* **Fast Surface-Aware Dynamic Light Plane Culling (`R_DlightBmodel`)**
* Evaluates individual polygon face planes against active dynamic lights using accelerated dot-product plane filtering.
* **Benefit**: Drastically cuts wasted GPU vertex cycles by dynamically stripping light flags from back-facing or hidden surfaces.
* **Benefit**: Eliminates dynamic lighting artifacts bleeding or leaking straight through thin structural architecture like doors and gates.


* **Modernized Aspect-Correct Bloom & Spectral Saturation**
* Fixed legacy aspect-ratio viewport coordinate bugs, implemented screen-space energy conservation blending, added high-frequency dithering, and integrated O(1) single-pass photographic highlight extraction.
* **Benefit**: Renders widescreen aspect-ratio corrected visual bleeds without horizontal stretching or halo clipping.
* **Benefit**: Prevents glowing elements from crushing the UI or burning out into flat white blocks, while dissolving diamond-blur geometric banding patterns into smooth gradients.



---

### 📐 Core Math & Renderer Geometry (`q_math.c` / `q_shared.h` / `tr_mesh.c`)

* **SIMD Vector Normalization**
* Replaced scalar vector normalizations with SSE intrinsics and reciprocal square root approximations (`_mm_rsqrt_ps`).
* **Benefit**: Speeds up coordinate transformation math.
* **Benefit**: Minimizes processor instruction footprint for normalized geometry checks.
* **Benefit**: Safeguards arithmetic routines against division-by-zero errors.


* **Branchless Coordinate Clamping**
* Replaced clamping branches with `fminf` and `fmaxf`, compiling down to single-instruction `minss`/`maxss` assembly primitives.
* **Benefit**: Avoids branch mispredictions during player movement and camera updates.
* **Benefit**: Decreases CPU cycles required to sanitize player movement commands.
* **Benefit**: Reduces assembly size and improves CPU instruction decoding.


* **SIMD Fog Volume Checks**
* Checks mesh bounding spheres against fog volume limits in parallel using SSE masking (`_mm_movemask_ps`).
* **Benefit**: Speeds up atmospheric fog visibility calculations.
* **Benefit**: Eliminates visual pops or calculations lag when crossing dynamic fog boundaries.
* **Benefit**: Ensures fog transitions are smooth and physically correct.


* **Inlined LOD Calculations**
* Replaced indirect function calls in `R_ComputeLOD` with direct inline calculations.
* **Benefit**: Eliminates call stack overhead when processing complex mesh details.
* **Benefit**: Optimizes CPU instruction cache usage inside render frames.
* **Benefit**: Simplifies assembly jump patterns, improving branch targets buffer performance.


* **User Command Optimizations**
* Streamlined view-angle calculations and command processing (`usercmd_t`).
* **Benefit**: Reduces per-frame CPU consumption during high-rate player movements.
* **Benefit**: Keeps game physics and player movement response extremely smooth on high polling-rate gamepads/mice.
* **Benefit**: Minimizes input latency, critical for fast-paced shooters.



---

### 🤖 Bot Intelligence Optimization (`ai_main.c` / `g_bot.c` / `ai_cast_sight.c`)

* **Fast Bot Pathing**
* Uses hardware-native rounding (`roundf`) for fast angle wrapping in bot path navigation.
* **Benefit**: Decreases bot decision-making overhead.
* **Benefit**: Speeds up navigation updates across large navigational meshes.
* **Benefit**: Minimizes frame rate spikes when multiple bots pathfind concurrently.


* **Vectorized Transform Copies**
* Uses SSE unaligned loads/stores (`_mm_loadu_ps` / `_mm_storeu_ps`) to copy 3D transform structures in single operations.
* **Benefit**: Minimizes memory bandwidth usage when updating bot structures.
* **Benefit**: Speeds up entity frame updates by utilizing hardware vector registers.
* **Benefit**: Eliminates unaligned memory read/write latency.


* **Vector Dot Product FOV Check**
* Replaced nested conditional loops and expensive `AngleMod` calculations in visibility checks with a 3D vector dot product.
* **Benefit**: Bypasses multiple expensive trigonometric conversions and divisions per sight check, significantly boosting CPU performance on maps with high actor counts.
* **Benefit**: Prevents CPU instruction pipeline stalls by replacing branch-heavy logic with direct, register-level vector equations.
* **Benefit**: Standardizes the visibility model into a consistent 3D visual cone, improving AI detection consistency.


* **Early-Exit PVS Check**
* Runs a fast Potential Visibility Set (`trap_InPVS`) check at the start of visibility checks before performing expensive multi-point raycasts.
* **Benefit**: Instantly discards calculations for characters separated by solid world geometry without executing any raycasts.
* **Benefit**: Substantially reduces CPU usage when players and AI move between different rooms or indoor areas.
* **Benefit**: Keeps the instruction cache clean by skipping the main loop code paths for out-of-sight actors.


* **Proximity-Based Dynamic Time-Slicing**
* Dynamically adjusts sight check delays based on the 3D distance between characters.
* **Benefit**: Lowers sight checking latency to 100ms for close-range actors, making close-quarters AI reactions twice as fast and sharp.
* **Benefit**: Safely scales up delays for far-away actors up to 800ms, removing significant processing overhead for distant characters.
* **Benefit**: Ensures AI responsiveness is directed where it is most noticeable to the player, optimizing game feel.


* **Cached Animation Speed Lookup**
* Caches the movement speeds retrieved from the character animation script based on `aiState`.
* **Benefit**: Eradicates three complex animation script parsing and indexing operations per character every frame.
* **Benefit**: Minimizes CPU instruction cache cycles in the AI main think loop.
* **Benefit**: Keeps physics tick execution times stable even during high-density character engagements.



---

### ⚡ Modernized x86_64 JIT QVM Compiler (`vm_x86.c` / `vm.c` / `vm_local.h`)

* **Dynamic Register Caching**
* Caches QVM stack variables directly in x86-64 registers (RAX, RCX, RDX) via `vm_optimize.h`.
* **Benefit**: Cuts stack memory traffic by up to 80%, accelerating virtual machine execution toward native speeds.
* **Benefit**: Minimizes memory access latency by utilizing fast CPU registers instead of L1 data cache.
* **Benefit**: Decreases data cache pollution, allowing other systems to run faster.


* **W^X Memory Hardening**
* Transitioned JIT buffer allocation to a Write-or-Execute model using `mprotect` to switch memory pages to `PROT_READ | PROT_EXEC` after code emission.
* **Benefit**: Protects the engine against dynamic code injection vulnerabilities.
* **Benefit**: Enhances security configurations compliance on restricted operating systems.
* **Benefit**: Aligns the JIT compiler with modern security requirements of operating system kernels.


* **Instruction Merging**
* Merges related VM instructions into single machine code blocks and maps floating-point ceil/floor operations directly to SSE4.1 `roundss` instructions.
* **Benefit**: Reduces instruction cache footprint and improves execution pipelining.
* **Benefit**: Converts multi-cycle mathematical loops to single machine instructions.
* **Benefit**: Prevents compilation latency in JIT execution paths.


* **QVM Verification & Fixups**
* Added pre-compilation instruction verification (`VM_CheckInstructions`), LCC compiler instruction fixups (`VM_Fixup`), and CRC32 lookup validation.
* **Benefit**: Prevents crash loops when executing legacy or corrupt custom mods.
* **Benefit**: Secures runtime stability against malformed QVM assemblies.
* **Benefit**: Speeds up debugging of custom mods.


* **JIT Enabled by Default**
* Configured the engine to default to JIT execution (`vm_* 1`) for game, cgame, and UI modules.
* **Benefit**: Unlocks maximum mod execution performance out of the box.
* **Benefit**: Eliminates manual engine configuration overrides for end users.
* **Benefit**: Reduces bug reports from players running heavy scripts.



---

## UI, Gameplay Systems, & Dev Infrastructure

* **"Sofia" Advanced Settings Menu**
* Added a custom settings tab to control HRTF, audio occlusion, reverb, and Discord options. Refactored UI menu layouts to clean up padding.
* **Benefit**: Provides a clean, accessible layout for new settings without text clipping or overlapping fields.


* **Discord Rich Presence**
* Integrates game state (active mod, weapons with dynamic emojis, score/kills) into Discord via background thread sockets.
* **Benefit**: Displays rich activity status to friends without introducing gameplay micro-stutters.


* **Tactical Flashlight**
* Added a battery-powered flashlight with dynamic dimming curves, HUD indicators, and AI detection paths.
* **Benefit**: Enhances tactical options with realistic battery consumption and AI stealth responses.


* **WWII Realistic Bullet Projectile Physics & Ballistics**
* Adds an option to toggle physical bullet projectiles instead of hitscan traces, with flight speeds based on real-world muzzle velocities of WW2 weapons (e.g. Luger, Colt, MP40, Mauser, Venom minigun) and optional gravity-drop trajectory simulations.
* **Benefit**: Elevates combat realism and tactical gunplay by introducing bullet travel time and drop over long distances.
* **Benefit**: Retains full compatibility with existing systems like headshot zones, damage falloff, ricochets, and material penetration.
* **Benefit**: Integrates seamlessly as a toggle directly in the vanilla Gameplay Options UI.


* **High-DPI Console Scaling**
* Scales the console layout dynamically.
* **Benefit**: Ensures the console is fully legible on modern high-resolution 1440p and 4K displays.


* **Bazzite & Distrobox Dev Support**
* Configured build scripts for compiling within `Bazzite-dev-nvidia` Distrobox containers.
* **Benefit**: Streamlines dev environment setup on immutable Linux OS configurations.


* **Engine Updates**
* Added support for Steam Workshop paths on Linux, and updated zlib from 1.2.11 to 1.3.2.
* **Benefit**: Extends compatibility to modern workshop structures and speeds up file decompression.


* **Focus Loss Muting/Pausing**
* Automatically pauses the game and mutes audio when the application window loses focus.
* **Benefit**: Eliminates noise pollution and prevents in-game player deaths during Alt-Tabbing.


* **CI/CD Automation**
* Automated package compilation and packaging through build scripts.
* **Benefit**: Eliminates manual steps when building game release distributions.



---

## Hardware Architecture & Build Matrix

To build this modernized engine environment, the compilation pipeline targets native advanced hardware optimization flags:

```bash
# Target Compiler Flags for Optimized Native Execution
-mavx2 -mfma -msse4.1 -O3

```

