# Codebase File Map

**Root Directory:** `RealRTCW/`

## Directory Tree
```text
RealRTCW/
├── .gitattributes
├── .gitignore
├── COPYING.txt
├── HOWTO-Build (linux).md
├── Makefile
├── README.txt
├── READMEbase.md
├── code/
│   ├── AL/
│   │   ├── al.h
│   │   ├── alc.h
│   │   ├── alext.h
│   │   ├── efx-creative.h
│   │   ├── efx-presets.h
│   │   └── efx.h
│   ├── SDL3/
│   │   └── include/
│   │       └── SDL3/
│   │           ├── SDL.h
│   │           ├── SDL_assert.h
│   │           ├── SDL_asyncio.h
│   │           ├── SDL_atomic.h
│   │           ├── SDL_audio.h
│   │           ├── SDL_begin_code.h
│   │           ├── SDL_bits.h
│   │           ├── SDL_blendmode.h
│   │           ├── SDL_camera.h
│   │           ├── SDL_clipboard.h
│   │           ├── SDL_close_code.h
│   │           ├── SDL_copying.h
│   │           ├── SDL_cpuinfo.h
│   │           ├── SDL_dialog.h
│   │           ├── SDL_dlopennote.h
│   │           ├── SDL_egl.h
│   │           ├── SDL_endian.h
│   │           ├── SDL_error.h
│   │           ├── SDL_events.h
│   │           ├── SDL_filesystem.h
│   │           ├── SDL_gamepad.h
│   │           ├── SDL_gpu.h
│   │           ├── SDL_guid.h
│   │           ├── SDL_haptic.h
│   │           ├── SDL_hidapi.h
│   │           ├── SDL_hints.h
│   │           ├── SDL_init.h
│   │           ├── SDL_intrin.h
│   │           ├── SDL_iostream.h
│   │           ├── SDL_joystick.h
│   │           ├── SDL_keyboard.h
│   │           ├── SDL_keycode.h
│   │           ├── SDL_loadso.h
│   │           ├── SDL_locale.h
│   │           ├── SDL_log.h
│   │           ├── SDL_main.h
│   │           ├── SDL_main_impl.h
│   │           ├── SDL_messagebox.h
│   │           ├── SDL_metal.h
│   │           ├── SDL_misc.h
│   │           ├── SDL_mouse.h
│   │           ├── SDL_mutex.h
│   │           ├── SDL_oldnames.h
│   │           ├── SDL_opengl.h
│   │           ├── SDL_opengl_glext.h
│   │           ├── SDL_opengles.h
│   │           ├── SDL_opengles2.h
│   │           ├── SDL_opengles2_gl2.h
│   │           ├── SDL_opengles2_gl2ext.h
│   │           ├── SDL_opengles2_gl2platform.h
│   │           ├── SDL_opengles2_khrplatform.h
│   │           ├── SDL_pen.h
│   │           ├── SDL_pixels.h
│   │           ├── SDL_platform.h
│   │           ├── SDL_platform_defines.h
│   │           ├── SDL_power.h
│   │           ├── SDL_process.h
│   │           ├── SDL_properties.h
│   │           ├── SDL_rect.h
│   │           ├── SDL_render.h
│   │           ├── SDL_revision.h
│   │           ├── SDL_scancode.h
│   │           ├── SDL_sensor.h
│   │           ├── SDL_stdinc.h
│   │           ├── SDL_storage.h
│   │           ├── SDL_surface.h
│   │           ├── SDL_system.h
│   │           ├── SDL_test.h
│   │           ├── SDL_test_assert.h
│   │           ├── SDL_test_common.h
│   │           ├── SDL_test_compare.h
│   │           ├── SDL_test_crc32.h
│   │           ├── SDL_test_font.h
│   │           ├── SDL_test_fuzzer.h
│   │           ├── SDL_test_harness.h
│   │           ├── SDL_test_log.h
│   │           ├── SDL_test_md5.h
│   │           ├── SDL_test_memory.h
│   │           ├── SDL_thread.h
│   │           ├── SDL_time.h
│   │           ├── SDL_timer.h
│   │           ├── SDL_touch.h
│   │           ├── SDL_tray.h
│   │           ├── SDL_version.h
│   │           ├── SDL_video.h
│   │           └── SDL_vulkan.h
│   ├── asm/
│   │   ├── ftola.asm
│   │   ├── ftola.c
│   │   ├── matha.s
│   │   ├── qasm-inline.h
│   │   ├── qasm.h
│   │   ├── snapvector.asm
│   │   ├── snapvector.c
│   │   ├── snd_mixa.s
│   │   └── vm_x86_64.asm
│   ├── botlib/
│   │   ├── aasfile.h
│   │   ├── be_aas.h
│   │   ├── be_aas_bsp.h
│   │   ├── be_aas_bspq3.c
│   │   ├── be_aas_cluster.c
│   │   ├── be_aas_cluster.h
│   │   ├── be_aas_debug.c
│   │   ├── be_aas_debug.h
│   │   ├── be_aas_def.h
│   │   ├── be_aas_entity.c
│   │   ├── be_aas_entity.h
│   │   ├── be_aas_file.c
│   │   ├── be_aas_file.h
│   │   ├── be_aas_funcs.h
│   │   ├── be_aas_main.c
│   │   ├── be_aas_main.h
│   │   ├── be_aas_move.c
│   │   ├── be_aas_move.h
│   │   ├── be_aas_optimize.c
│   │   ├── be_aas_optimize.h
│   │   ├── be_aas_reach.c
│   │   ├── be_aas_reach.h
│   │   ├── be_aas_route.c
│   │   ├── be_aas_route.h
│   │   ├── be_aas_routealt.c
│   │   ├── be_aas_routealt.h
│   │   ├── be_aas_routetable.c
│   │   ├── be_aas_routetable.h
│   │   ├── be_aas_sample.c
│   │   ├── be_aas_sample.h
│   │   ├── be_ai_char.c
│   │   ├── be_ai_char.h
│   │   ├── be_ai_chat.c
│   │   ├── be_ai_chat.h
│   │   ├── be_ai_gen.c
│   │   ├── be_ai_gen.h
│   │   ├── be_ai_goal.c
│   │   ├── be_ai_goal.h
│   │   ├── be_ai_move.c
│   │   ├── be_ai_move.h
│   │   ├── be_ai_weap.c
│   │   ├── be_ai_weap.h
│   │   ├── be_ai_weight.c
│   │   ├── be_ai_weight.h
│   │   ├── be_ea.c
│   │   ├── be_ea.h
│   │   ├── be_interface.c
│   │   ├── be_interface.h
│   │   ├── botai.h
│   │   ├── botlib.h
│   │   ├── l_crc.c
│   │   ├── l_crc.h
│   │   ├── l_libvar.c
│   │   ├── l_libvar.h
│   │   ├── l_log.c
│   │   ├── l_log.h
│   │   ├── l_memory.c
│   │   ├── l_memory.h
│   │   ├── l_precomp.c
│   │   ├── l_precomp.h
│   │   ├── l_script.c
│   │   ├── l_script.h
│   │   ├── l_struct.c
│   │   ├── l_struct.h
│   │   └── l_utils.h
│   ├── cgame/
│   │   ├── cg_atmospheric.c
│   │   ├── cg_consolecmds.c
│   │   ├── cg_draw.c
│   │   ├── cg_drawtools.c
│   │   ├── cg_effects.c
│   │   ├── cg_ents.c
│   │   ├── cg_event.c
│   │   ├── cg_flamethrower.c
│   │   ├── cg_flashlight.c
│   │   ├── cg_flashlight.h
│   │   ├── cg_info.c
│   │   ├── cg_local.h
│   │   ├── cg_localents.c
│   │   ├── cg_main.c
│   │   ├── cg_marks.c
│   │   ├── cg_newdraw.c
│   │   ├── cg_particles.c
│   │   ├── cg_players.c
│   │   ├── cg_playerstate.c
│   │   ├── cg_predict.c
│   │   ├── cg_public.h
│   │   ├── cg_scoreboard.c
│   │   ├── cg_servercmds.c
│   │   ├── cg_snapshot.c
│   │   ├── cg_sound.c
│   │   ├── cg_syscalls.asm
│   │   ├── cg_syscalls.c
│   │   ├── cg_trails.c
│   │   ├── cg_view.c
│   │   └── cg_weapons.c
│   ├── client/
│   │   ├── cl_avi.c
│   │   ├── cl_cgame.c
│   │   ├── cl_cin.c
│   │   ├── cl_console.c
│   │   ├── cl_curl.c
│   │   ├── cl_curl.h
│   │   ├── cl_input.c
│   │   ├── cl_keys.c
│   │   ├── cl_main.c
│   │   ├── cl_net_chan.c
│   │   ├── cl_parse.c
│   │   ├── cl_scrn.c
│   │   ├── cl_ui.c
│   │   ├── client.h
│   │   ├── keycodes.h
│   │   ├── keys.h
│   │   ├── libmumblelink.c
│   │   ├── libmumblelink.h
│   │   ├── qal.c
│   │   ├── qal.h
│   │   ├── snd_adpcm.c
│   │   ├── snd_altivec.c
│   │   ├── snd_codec.c
│   │   ├── snd_codec.h
│   │   ├── snd_codec_ogg.c
│   │   ├── snd_codec_opus.c
│   │   ├── snd_codec_wav.c
│   │   ├── snd_dma.c
│   │   ├── snd_efx.c
│   │   ├── snd_efx.h
│   │   ├── snd_local.h
│   │   ├── snd_main.c
│   │   ├── snd_mem.c
│   │   ├── snd_mix.c
│   │   ├── snd_openal.c
│   │   ├── snd_public.h
│   │   └── snd_wavelet.c
│   ├── curl-8.8.0/
│   │   └── include/
│   │       ├── Makefile.am
│   │       ├── Makefile.in
│   │       ├── README.md
│   │       └── curl/
│   │           ├── Makefile.am
│   │           ├── Makefile.in
│   │           ├── curl.h
│   │           ├── curlver.h
│   │           ├── easy.h
│   │           ├── header.h
│   │           ├── mprintf.h
│   │           ├── multi.h
│   │           ├── options.h
│   │           ├── stdcheaders.h
│   │           ├── system.h
│   │           ├── typecheck-gcc.h
│   │           ├── urlapi.h
│   │           └── websockets.h
│   ├── discord/
│   │   ├── discord_data.c
│   │   ├── discord_data.h
│   │   ├── discord_rpc.c
│   │   └── discord_rpc.h
│   ├── freetype-2.9/
│   │   ├── include/
│   │   │   ├── freetype/
│   │   │   │   ├── config/
│   │   │   │   │   ├── ftconfig.h
│   │   │   │   │   ├── ftheader.h
│   │   │   │   │   ├── ftmodule.h
│   │   │   │   │   ├── ftoption.h
│   │   │   │   │   └── ftstdlib.h
│   │   │   │   ├── freetype.h
│   │   │   │   ├── ftadvanc.h
│   │   │   │   ├── ftbbox.h
│   │   │   │   ├── ftbdf.h
│   │   │   │   ├── ftbitmap.h
│   │   │   │   ├── ftbzip2.h
│   │   │   │   ├── ftcache.h
│   │   │   │   ├── ftchapters.h
│   │   │   │   ├── ftcid.h
│   │   │   │   ├── ftdriver.h
│   │   │   │   ├── fterrdef.h
│   │   │   │   ├── fterrors.h
│   │   │   │   ├── ftfntfmt.h
│   │   │   │   ├── ftgasp.h
│   │   │   │   ├── ftglyph.h
│   │   │   │   ├── ftgxval.h
│   │   │   │   ├── ftgzip.h
│   │   │   │   ├── ftimage.h
│   │   │   │   ├── ftincrem.h
│   │   │   │   ├── ftlcdfil.h
│   │   │   │   ├── ftlist.h
│   │   │   │   ├── ftlzw.h
│   │   │   │   ├── ftmac.h
│   │   │   │   ├── ftmm.h
│   │   │   │   ├── ftmodapi.h
│   │   │   │   ├── ftmoderr.h
│   │   │   │   ├── ftotval.h
│   │   │   │   ├── ftoutln.h
│   │   │   │   ├── ftparams.h
│   │   │   │   ├── ftpfr.h
│   │   │   │   ├── ftrender.h
│   │   │   │   ├── ftsizes.h
│   │   │   │   ├── ftsnames.h
│   │   │   │   ├── ftstroke.h
│   │   │   │   ├── ftsynth.h
│   │   │   │   ├── ftsystem.h
│   │   │   │   ├── fttrigon.h
│   │   │   │   ├── fttypes.h
│   │   │   │   ├── ftwinfnt.h
│   │   │   │   ├── internal/
│   │   │   │   │   ├── autohint.h
│   │   │   │   │   ├── cffotypes.h
│   │   │   │   │   ├── cfftypes.h
│   │   │   │   │   ├── ftcalc.h
│   │   │   │   │   ├── ftdebug.h
│   │   │   │   │   ├── ftdrv.h
│   │   │   │   │   ├── ftgloadr.h
│   │   │   │   │   ├── fthash.h
│   │   │   │   │   ├── ftmemory.h
│   │   │   │   │   ├── ftobjs.h
│   │   │   │   │   ├── ftpic.h
│   │   │   │   │   ├── ftpsprop.h
│   │   │   │   │   ├── ftrfork.h
│   │   │   │   │   ├── ftserv.h
│   │   │   │   │   ├── ftstream.h
│   │   │   │   │   ├── fttrace.h
│   │   │   │   │   ├── ftvalid.h
│   │   │   │   │   ├── internal.h
│   │   │   │   │   ├── psaux.h
│   │   │   │   │   ├── pshints.h
│   │   │   │   │   ├── services/
│   │   │   │   │   │   ├── svbdf.h
│   │   │   │   │   │   ├── svcfftl.h
│   │   │   │   │   │   ├── svcid.h
│   │   │   │   │   │   ├── svfntfmt.h
│   │   │   │   │   │   ├── svgldict.h
│   │   │   │   │   │   ├── svgxval.h
│   │   │   │   │   │   ├── svkern.h
│   │   │   │   │   │   ├── svmetric.h
│   │   │   │   │   │   ├── svmm.h
│   │   │   │   │   │   ├── svotval.h
│   │   │   │   │   │   ├── svpfr.h
│   │   │   │   │   │   ├── svpostnm.h
│   │   │   │   │   │   ├── svprop.h
│   │   │   │   │   │   ├── svpscmap.h
│   │   │   │   │   │   ├── svpsinfo.h
│   │   │   │   │   │   ├── svsfnt.h
│   │   │   │   │   │   ├── svttcmap.h
│   │   │   │   │   │   ├── svtteng.h
│   │   │   │   │   │   ├── svttglyf.h
│   │   │   │   │   │   └── svwinfnt.h
│   │   │   │   │   ├── sfnt.h
│   │   │   │   │   ├── t1types.h
│   │   │   │   │   └── tttypes.h
│   │   │   │   ├── t1tables.h
│   │   │   │   ├── ttnameid.h
│   │   │   │   ├── tttables.h
│   │   │   │   └── tttags.h
│   │   │   └── ft2build.h
│   │   └── src/
│   │       ├── autofit/
│   │       │   ├── afangles.c
│   │       │   ├── afangles.h
│   │       │   ├── afblue.c
│   │       │   ├── afblue.cin
│   │       │   ├── afblue.dat
│   │       │   ├── afblue.h
│   │       │   ├── afblue.hin
│   │       │   ├── afcjk.c
│   │       │   ├── afcjk.h
│   │       │   ├── afcover.h
│   │       │   ├── afdummy.c
│   │       │   ├── afdummy.h
│   │       │   ├── aferrors.h
│   │       │   ├── afglobal.c
│   │       │   ├── afglobal.h
│   │       │   ├── afhints.c
│   │       │   ├── afhints.h
│   │       │   ├── afindic.c
│   │       │   ├── afindic.h
│   │       │   ├── aflatin.c
│   │       │   ├── aflatin.h
│   │       │   ├── aflatin2.c
│   │       │   ├── aflatin2.h
│   │       │   ├── afloader.c
│   │       │   ├── afloader.h
│   │       │   ├── afmodule.c
│   │       │   ├── afmodule.h
│   │       │   ├── afpic.c
│   │       │   ├── afpic.h
│   │       │   ├── afranges.c
│   │       │   ├── afranges.h
│   │       │   ├── afscript.h
│   │       │   ├── afshaper.c
│   │       │   ├── afshaper.h
│   │       │   ├── afstyles.h
│   │       │   ├── aftypes.h
│   │       │   ├── afwarp.c
│   │       │   ├── afwarp.h
│   │       │   ├── afwrtsys.h
│   │       │   └── autofit.c
│   │       ├── base/
│   │       │   ├── basepic.c
│   │       │   ├── basepic.h
│   │       │   ├── ftadvanc.c
│   │       │   ├── ftapi.c
│   │       │   ├── ftbase.c
│   │       │   ├── ftbase.h
│   │       │   ├── ftbbox.c
│   │       │   ├── ftbdf.c
│   │       │   ├── ftbitmap.c
│   │       │   ├── ftcalc.c
│   │       │   ├── ftcid.c
│   │       │   ├── ftdbgmem.c
│   │       │   ├── ftdebug.c
│   │       │   ├── ftfntfmt.c
│   │       │   ├── ftfstype.c
│   │       │   ├── ftgasp.c
│   │       │   ├── ftgloadr.c
│   │       │   ├── ftglyph.c
│   │       │   ├── ftgxval.c
│   │       │   ├── fthash.c
│   │       │   ├── ftinit.c
│   │       │   ├── ftlcdfil.c
│   │       │   ├── ftmac.c
│   │       │   ├── ftmm.c
│   │       │   ├── ftobjs.c
│   │       │   ├── ftotval.c
│   │       │   ├── ftoutln.c
│   │       │   ├── ftpatent.c
│   │       │   ├── ftpfr.c
│   │       │   ├── ftpic.c
│   │       │   ├── ftpsprop.c
│   │       │   ├── ftrfork.c
│   │       │   ├── ftsnames.c
│   │       │   ├── ftstream.c
│   │       │   ├── ftstroke.c
│   │       │   ├── ftsynth.c
│   │       │   ├── ftsystem.c
│   │       │   ├── fttrigon.c
│   │       │   ├── fttype1.c
│   │       │   ├── ftutil.c
│   │       │   ├── ftwinfnt.c
│   │       │   ├── md5.c
│   │       │   └── md5.h
│   │       ├── bdf/
│   │       │   ├── README
│   │       │   ├── bdf.c
│   │       │   ├── bdf.h
│   │       │   ├── bdfdrivr.c
│   │       │   ├── bdfdrivr.h
│   │       │   ├── bdferror.h
│   │       │   └── bdflib.c
│   │       ├── bzip2/
│   │       │   └── ftbzip2.c
│   │       ├── cache/
│   │       │   ├── ftcache.c
│   │       │   ├── ftcbasic.c
│   │       │   ├── ftccache.c
│   │       │   ├── ftccache.h
│   │       │   ├── ftccback.h
│   │       │   ├── ftccmap.c
│   │       │   ├── ftcerror.h
│   │       │   ├── ftcglyph.c
│   │       │   ├── ftcglyph.h
│   │       │   ├── ftcimage.c
│   │       │   ├── ftcimage.h
│   │       │   ├── ftcmanag.c
│   │       │   ├── ftcmanag.h
│   │       │   ├── ftcmru.c
│   │       │   ├── ftcmru.h
│   │       │   ├── ftcsbits.c
│   │       │   └── ftcsbits.h
│   │       ├── cff/
│   │       │   ├── cff.c
│   │       │   ├── cffcmap.c
│   │       │   ├── cffcmap.h
│   │       │   ├── cffdrivr.c
│   │       │   ├── cffdrivr.h
│   │       │   ├── cfferrs.h
│   │       │   ├── cffgload.c
│   │       │   ├── cffgload.h
│   │       │   ├── cffload.c
│   │       │   ├── cffload.h
│   │       │   ├── cffobjs.c
│   │       │   ├── cffobjs.h
│   │       │   ├── cffparse.c
│   │       │   ├── cffparse.h
│   │       │   ├── cffpic.c
│   │       │   ├── cffpic.h
│   │       │   └── cfftoken.h
│   │       ├── cid/
│   │       │   ├── ciderrs.h
│   │       │   ├── cidgload.c
│   │       │   ├── cidgload.h
│   │       │   ├── cidload.c
│   │       │   ├── cidload.h
│   │       │   ├── cidobjs.c
│   │       │   ├── cidobjs.h
│   │       │   ├── cidparse.c
│   │       │   ├── cidparse.h
│   │       │   ├── cidriver.c
│   │       │   ├── cidriver.h
│   │       │   ├── cidtoken.h
│   │       │   └── type1cid.c
│   │       ├── gxvalid/
│   │       │   ├── README
│   │       │   ├── gxvalid.c
│   │       │   ├── gxvalid.h
│   │       │   ├── gxvbsln.c
│   │       │   ├── gxvcommn.c
│   │       │   ├── gxvcommn.h
│   │       │   ├── gxverror.h
│   │       │   ├── gxvfeat.c
│   │       │   ├── gxvfeat.h
│   │       │   ├── gxvfgen.c
│   │       │   ├── gxvjust.c
│   │       │   ├── gxvkern.c
│   │       │   ├── gxvlcar.c
│   │       │   ├── gxvmod.c
│   │       │   ├── gxvmod.h
│   │       │   ├── gxvmort.c
│   │       │   ├── gxvmort.h
│   │       │   ├── gxvmort0.c
│   │       │   ├── gxvmort1.c
│   │       │   ├── gxvmort2.c
│   │       │   ├── gxvmort4.c
│   │       │   ├── gxvmort5.c
│   │       │   ├── gxvmorx.c
│   │       │   ├── gxvmorx.h
│   │       │   ├── gxvmorx0.c
│   │       │   ├── gxvmorx1.c
│   │       │   ├── gxvmorx2.c
│   │       │   ├── gxvmorx4.c
│   │       │   ├── gxvmorx5.c
│   │       │   ├── gxvopbd.c
│   │       │   ├── gxvprop.c
│   │       │   └── gxvtrak.c
│   │       ├── gzip/
│   │       │   ├── adler32.c
│   │       │   ├── ftgzip.c
│   │       │   ├── ftzconf.h
│   │       │   ├── infblock.c
│   │       │   ├── infblock.h
│   │       │   ├── infcodes.c
│   │       │   ├── infcodes.h
│   │       │   ├── inffixed.h
│   │       │   ├── inflate.c
│   │       │   ├── inftrees.c
│   │       │   ├── inftrees.h
│   │       │   ├── infutil.c
│   │       │   ├── infutil.h
│   │       │   ├── zlib.h
│   │       │   ├── zutil.c
│   │       │   └── zutil.h
│   │       ├── lzw/
│   │       │   ├── ftlzw.c
│   │       │   ├── ftzopen.c
│   │       │   └── ftzopen.h
│   │       ├── otvalid/
│   │       │   ├── otvalid.c
│   │       │   ├── otvalid.h
│   │       │   ├── otvbase.c
│   │       │   ├── otvcommn.c
│   │       │   ├── otvcommn.h
│   │       │   ├── otverror.h
│   │       │   ├── otvgdef.c
│   │       │   ├── otvgpos.c
│   │       │   ├── otvgpos.h
│   │       │   ├── otvgsub.c
│   │       │   ├── otvjstf.c
│   │       │   ├── otvmath.c
│   │       │   ├── otvmod.c
│   │       │   └── otvmod.h
│   │       ├── pcf/
│   │       │   ├── README
│   │       │   ├── pcf.c
│   │       │   ├── pcf.h
│   │       │   ├── pcfdrivr.c
│   │       │   ├── pcfdrivr.h
│   │       │   ├── pcferror.h
│   │       │   ├── pcfread.c
│   │       │   ├── pcfread.h
│   │       │   ├── pcfutil.c
│   │       │   └── pcfutil.h
│   │       ├── pfr/
│   │       │   ├── pfr.c
│   │       │   ├── pfrcmap.c
│   │       │   ├── pfrcmap.h
│   │       │   ├── pfrdrivr.c
│   │       │   ├── pfrdrivr.h
│   │       │   ├── pfrerror.h
│   │       │   ├── pfrgload.c
│   │       │   ├── pfrgload.h
│   │       │   ├── pfrload.c
│   │       │   ├── pfrload.h
│   │       │   ├── pfrobjs.c
│   │       │   ├── pfrobjs.h
│   │       │   ├── pfrsbit.c
│   │       │   ├── pfrsbit.h
│   │       │   └── pfrtypes.h
│   │       ├── psaux/
│   │       │   ├── afmparse.c
│   │       │   ├── afmparse.h
│   │       │   ├── cffdecode.c
│   │       │   ├── cffdecode.h
│   │       │   ├── psarrst.c
│   │       │   ├── psarrst.h
│   │       │   ├── psaux.c
│   │       │   ├── psauxerr.h
│   │       │   ├── psauxmod.c
│   │       │   ├── psauxmod.h
│   │       │   ├── psblues.c
│   │       │   ├── psblues.h
│   │       │   ├── psconv.c
│   │       │   ├── psconv.h
│   │       │   ├── pserror.c
│   │       │   ├── pserror.h
│   │       │   ├── psfixed.h
│   │       │   ├── psfont.c
│   │       │   ├── psfont.h
│   │       │   ├── psft.c
│   │       │   ├── psft.h
│   │       │   ├── psglue.h
│   │       │   ├── pshints.c
│   │       │   ├── pshints.h
│   │       │   ├── psintrp.c
│   │       │   ├── psintrp.h
│   │       │   ├── psobjs.c
│   │       │   ├── psobjs.h
│   │       │   ├── psread.c
│   │       │   ├── psread.h
│   │       │   ├── psstack.c
│   │       │   ├── psstack.h
│   │       │   ├── pstypes.h
│   │       │   ├── t1cmap.c
│   │       │   ├── t1cmap.h
│   │       │   ├── t1decode.c
│   │       │   └── t1decode.h
│   │       ├── pshinter/
│   │       │   ├── pshalgo.c
│   │       │   ├── pshalgo.h
│   │       │   ├── pshglob.c
│   │       │   ├── pshglob.h
│   │       │   ├── pshinter.c
│   │       │   ├── pshmod.c
│   │       │   ├── pshmod.h
│   │       │   ├── pshnterr.h
│   │       │   ├── pshpic.c
│   │       │   ├── pshpic.h
│   │       │   ├── pshrec.c
│   │       │   └── pshrec.h
│   │       ├── psnames/
│   │       │   ├── psmodule.c
│   │       │   ├── psmodule.h
│   │       │   ├── psnamerr.h
│   │       │   ├── psnames.c
│   │       │   ├── pspic.c
│   │       │   ├── pspic.h
│   │       │   └── pstables.h
│   │       ├── raster/
│   │       │   ├── ftmisc.h
│   │       │   ├── ftraster.c
│   │       │   ├── ftraster.h
│   │       │   ├── ftrend1.c
│   │       │   ├── ftrend1.h
│   │       │   ├── raster.c
│   │       │   ├── rasterrs.h
│   │       │   ├── rastpic.c
│   │       │   └── rastpic.h
│   │       ├── sfnt/
│   │       │   ├── pngshim.c
│   │       │   ├── pngshim.h
│   │       │   ├── sfdriver.c
│   │       │   ├── sfdriver.h
│   │       │   ├── sferrors.h
│   │       │   ├── sfnt.c
│   │       │   ├── sfntpic.c
│   │       │   ├── sfntpic.h
│   │       │   ├── sfobjs.c
│   │       │   ├── sfobjs.h
│   │       │   ├── ttbdf.c
│   │       │   ├── ttbdf.h
│   │       │   ├── ttcmap.c
│   │       │   ├── ttcmap.h
│   │       │   ├── ttcmapc.h
│   │       │   ├── ttkern.c
│   │       │   ├── ttkern.h
│   │       │   ├── ttload.c
│   │       │   ├── ttload.h
│   │       │   ├── ttmtx.c
│   │       │   ├── ttmtx.h
│   │       │   ├── ttpost.c
│   │       │   ├── ttpost.h
│   │       │   ├── ttsbit.c
│   │       │   └── ttsbit.h
│   │       ├── smooth/
│   │       │   ├── ftgrays.c
│   │       │   ├── ftgrays.h
│   │       │   ├── ftsmerrs.h
│   │       │   ├── ftsmooth.c
│   │       │   ├── ftsmooth.h
│   │       │   ├── ftspic.c
│   │       │   ├── ftspic.h
│   │       │   └── smooth.c
│   │       ├── tools/
│   │       │   ├── afblue.pl
│   │       │   ├── apinames.c
│   │       │   ├── chktrcmp.py
│   │       │   ├── cordic.py
│   │       │   ├── docmaker/
│   │       │   │   ├── content.py
│   │       │   │   ├── docbeauty.py
│   │       │   │   ├── docmaker.py
│   │       │   │   ├── formatter.py
│   │       │   │   ├── sources.py
│   │       │   │   ├── tohtml.py
│   │       │   │   └── utils.py
│   │       │   ├── ftfuzzer/
│   │       │   │   ├── README
│   │       │   │   ├── ftfuzzer.cc
│   │       │   │   ├── ftmutator.cc
│   │       │   │   ├── rasterfuzzer.cc
│   │       │   │   └── runinput.cc
│   │       │   ├── ftrandom/
│   │       │   │   ├── Makefile
│   │       │   │   ├── README
│   │       │   │   └── ftrandom.c
│   │       │   ├── glnames.py
│   │       │   ├── no-copyright
│   │       │   ├── test_afm.c
│   │       │   ├── test_bbox.c
│   │       │   ├── test_trig.c
│   │       │   ├── update-copyright
│   │       │   └── update-copyright-year
│   │       ├── truetype/
│   │       │   ├── truetype.c
│   │       │   ├── ttdriver.c
│   │       │   ├── ttdriver.h
│   │       │   ├── tterrors.h
│   │       │   ├── ttgload.c
│   │       │   ├── ttgload.h
│   │       │   ├── ttgxvar.c
│   │       │   ├── ttgxvar.h
│   │       │   ├── ttinterp.c
│   │       │   ├── ttinterp.h
│   │       │   ├── ttobjs.c
│   │       │   ├── ttobjs.h
│   │       │   ├── ttpic.c
│   │       │   ├── ttpic.h
│   │       │   ├── ttpload.c
│   │       │   ├── ttpload.h
│   │       │   ├── ttsubpix.c
│   │       │   └── ttsubpix.h
│   │       ├── type1/
│   │       │   ├── t1afm.c
│   │       │   ├── t1afm.h
│   │       │   ├── t1driver.c
│   │       │   ├── t1driver.h
│   │       │   ├── t1errors.h
│   │       │   ├── t1gload.c
│   │       │   ├── t1gload.h
│   │       │   ├── t1load.c
│   │       │   ├── t1load.h
│   │       │   ├── t1objs.c
│   │       │   ├── t1objs.h
│   │       │   ├── t1parse.c
│   │       │   ├── t1parse.h
│   │       │   ├── t1tokens.h
│   │       │   └── type1.c
│   │       ├── type42/
│   │       │   ├── t42drivr.c
│   │       │   ├── t42drivr.h
│   │       │   ├── t42error.h
│   │       │   ├── t42objs.c
│   │       │   ├── t42objs.h
│   │       │   ├── t42parse.c
│   │       │   ├── t42parse.h
│   │       │   ├── t42types.h
│   │       │   └── type42.c
│   │       └── winfonts/
│   │           ├── fnterrs.h
│   │           ├── winfnt.c
│   │           └── winfnt.h
│   ├── game/
│   │   ├── ai_cast.c
│   │   ├── ai_cast.h
│   │   ├── ai_cast_characters.c
│   │   ├── ai_cast_debug.c
│   │   ├── ai_cast_events.c
│   │   ├── ai_cast_fight.c
│   │   ├── ai_cast_fight.h
│   │   ├── ai_cast_func_attack.c
│   │   ├── ai_cast_func_boss1.c
│   │   ├── ai_cast_func_hein.c
│   │   ├── ai_cast_funcs.c
│   │   ├── ai_cast_global.h
│   │   ├── ai_cast_loadouts.c
│   │   ├── ai_cast_script.c
│   │   ├── ai_cast_script_actions.c
│   │   ├── ai_cast_script_ents.c
│   │   ├── ai_cast_sight.c
│   │   ├── ai_cast_survival.c
│   │   ├── ai_cast_think.c
│   │   ├── ai_chat.c
│   │   ├── ai_chat.h
│   │   ├── ai_cmd.c
│   │   ├── ai_cmd.h
│   │   ├── ai_dmnet.c
│   │   ├── ai_dmnet.h
│   │   ├── ai_dmq3.c
│   │   ├── ai_dmq3.h
│   │   ├── ai_main.c
│   │   ├── ai_main.h
│   │   ├── ai_team.c
│   │   ├── ai_team.h
│   │   ├── bg_animation.c
│   │   ├── bg_jaymod.h
│   │   ├── bg_lib.c
│   │   ├── bg_lib.h
│   │   ├── bg_local.h
│   │   ├── bg_misc.c
│   │   ├── bg_pmove.c
│   │   ├── bg_public.h
│   │   ├── bg_slidemove.c
│   │   ├── chars.h
│   │   ├── g_active.c
│   │   ├── g_alarm.c
│   │   ├── g_bot.c
│   │   ├── g_client.c
│   │   ├── g_cmds.c
│   │   ├── g_combat.c
│   │   ├── g_func_decs.h
│   │   ├── g_func_decs.h.orig
│   │   ├── g_funcs.h
│   │   ├── g_funcs.h.orig
│   │   ├── g_items.c
│   │   ├── g_local.h
│   │   ├── g_main.c
│   │   ├── g_mem.c
│   │   ├── g_misc.c
│   │   ├── g_missile.c
│   │   ├── g_mover.c
│   │   ├── g_props.c
│   │   ├── g_public.h
│   │   ├── g_save.c
│   │   ├── g_script.c
│   │   ├── g_script_actions.c
│   │   ├── g_session.c
│   │   ├── g_spawn.c
│   │   ├── g_survival.h
│   │   ├── g_survival_buy.c
│   │   ├── g_survival_misc.c
│   │   ├── g_survival_score.c
│   │   ├── g_svcmds.c
│   │   ├── g_syscalls.asm
│   │   ├── g_syscalls.c
│   │   ├── g_target.c
│   │   ├── g_team.c
│   │   ├── g_team.h
│   │   ├── g_tramcar.c
│   │   ├── g_trigger.c
│   │   ├── g_utils.c
│   │   ├── g_weapon.c
│   │   ├── inv.h
│   │   ├── match.h
│   │   └── syn.h
│   ├── jpeg-8c/
│   │   ├── README
│   │   ├── ioquake3-changes.diff
│   │   ├── jaricom.c
│   │   ├── jcapimin.c
│   │   ├── jcapistd.c
│   │   ├── jcarith.c
│   │   ├── jccoefct.c
│   │   ├── jccolor.c
│   │   ├── jcdctmgr.c
│   │   ├── jchuff.c
│   │   ├── jcinit.c
│   │   ├── jcmainct.c
│   │   ├── jcmarker.c
│   │   ├── jcmaster.c
│   │   ├── jcomapi.c
│   │   ├── jconfig.h
│   │   ├── jcparam.c
│   │   ├── jcprepct.c
│   │   ├── jcsample.c
│   │   ├── jctrans.c
│   │   ├── jdapimin.c
│   │   ├── jdapistd.c
│   │   ├── jdarith.c
│   │   ├── jdatadst.c
│   │   ├── jdatasrc.c
│   │   ├── jdcoefct.c
│   │   ├── jdcolor.c
│   │   ├── jdct.h
│   │   ├── jddctmgr.c
│   │   ├── jdhuff.c
│   │   ├── jdinput.c
│   │   ├── jdmainct.c
│   │   ├── jdmarker.c
│   │   ├── jdmaster.c
│   │   ├── jdmerge.c
│   │   ├── jdpostct.c
│   │   ├── jdsample.c
│   │   ├── jdtrans.c
│   │   ├── jerror.c
│   │   ├── jerror.h
│   │   ├── jfdctflt.c
│   │   ├── jfdctfst.c
│   │   ├── jfdctint.c
│   │   ├── jidctflt.c
│   │   ├── jidctfst.c
│   │   ├── jidctint.c
│   │   ├── jinclude.h
│   │   ├── jmemmgr.c
│   │   ├── jmemnobs.c
│   │   ├── jmemsys.h
│   │   ├── jmorecfg.h
│   │   ├── jpegint.h
│   │   ├── jpeglib.h
│   │   ├── jquant1.c
│   │   ├── jquant2.c
│   │   ├── jutils.c
│   │   └── jversion.h
│   ├── libogg-1.3.5/
│   │   ├── AUTHORS
│   │   ├── COPYING
│   │   ├── include/
│   │   │   ├── Makefile.am
│   │   │   ├── Makefile.in
│   │   │   └── ogg/
│   │   │       ├── Makefile.am
│   │   │       ├── Makefile.in
│   │   │       ├── config_types.h
│   │   │       ├── config_types.h.in
│   │   │       ├── ogg.h
│   │   │       └── os_types.h
│   │   └── src/
│   │       ├── Makefile.am
│   │       ├── Makefile.in
│   │       ├── bitwise.c
│   │       ├── crctable.h
│   │       └── framing.c
│   ├── libs/
│   │   ├── linux32/
│   │   │   ├── libSDL2-2.0.so.0.12.0
│   │   │   ├── libSDL2-2.0.so.0.18.0
│   │   │   ├── libSDL2-2.0.so.0.4.1
│   │   │   ├── libSDL2-2.0.so.0.9.0
│   │   │   └── libSDL2main.a
│   │   ├── linux64/
│   │   │   ├── libSDL2-2.0.so.0.12.0
│   │   │   ├── libSDL2-2.0.so.0.18.0
│   │   │   ├── libSDL2-2.0.so.0.4.1
│   │   │   ├── libSDL2-2.0.so.0.9.0
│   │   │   └── libSDL2main.a
│   │   ├── macosx/
│   │   │   ├── libSDL2-2.0.0.dylib
│   │   │   ├── libSDL2main.a
│   │   │   └── libopenal.dylib
│   │   ├── win32/
│   │   │   ├── SDL3.lib
│   │   │   ├── libSDL3.dll.a
│   │   │   └── libcurl.a
│   │   └── win64/
│   │       ├── SDL3.lib
│   │       ├── libSDL3.dll.a
│   │       └── libcurl.a
│   ├── libvorbis-1.3.7/
│   │   ├── AUTHORS
│   │   ├── COPYING
│   │   ├── include/
│   │   │   ├── Makefile.am
│   │   │   ├── Makefile.in
│   │   │   └── vorbis/
│   │   │       ├── Makefile.am
│   │   │       ├── Makefile.in
│   │   │       ├── codec.h
│   │   │       ├── vorbisenc.h
│   │   │       └── vorbisfile.h
│   │   └── lib/
│   │       ├── CMakeLists.txt
│   │       ├── Makefile.am
│   │       ├── Makefile.in
│   │       ├── analysis.c
│   │       ├── backends.h
│   │       ├── barkmel.c
│   │       ├── bitrate.c
│   │       ├── bitrate.h
│   │       ├── block.c
│   │       ├── books/
│   │       │   ├── Makefile.am
│   │       │   ├── Makefile.in
│   │       │   ├── coupled/
│   │       │   │   ├── Makefile.am
│   │       │   │   ├── Makefile.in
│   │       │   │   ├── res_books_51.h
│   │       │   │   └── res_books_stereo.h
│   │       │   ├── floor/
│   │       │   │   ├── Makefile.am
│   │       │   │   ├── Makefile.in
│   │       │   │   └── floor_books.h
│   │       │   └── uncoupled/
│   │       │       ├── Makefile.am
│   │       │       ├── Makefile.in
│   │       │       └── res_books_uncoupled.h
│   │       ├── codebook.c
│   │       ├── codebook.h
│   │       ├── codec_internal.h
│   │       ├── envelope.c
│   │       ├── envelope.h
│   │       ├── floor0.c
│   │       ├── floor1.c
│   │       ├── highlevel.h
│   │       ├── info.c
│   │       ├── lookup.c
│   │       ├── lookup.h
│   │       ├── lookup_data.h
│   │       ├── lookups.pl
│   │       ├── lpc.c
│   │       ├── lpc.h
│   │       ├── lsp.c
│   │       ├── lsp.h
│   │       ├── mapping0.c
│   │       ├── masking.h
│   │       ├── mdct.c
│   │       ├── mdct.h
│   │       ├── misc.h
│   │       ├── modes/
│   │       │   ├── Makefile.am
│   │       │   ├── Makefile.in
│   │       │   ├── floor_all.h
│   │       │   ├── psych_11.h
│   │       │   ├── psych_16.h
│   │       │   ├── psych_44.h
│   │       │   ├── psych_8.h
│   │       │   ├── residue_16.h
│   │       │   ├── residue_44.h
│   │       │   ├── residue_44p51.h
│   │       │   ├── residue_44u.h
│   │       │   ├── residue_8.h
│   │       │   ├── setup_11.h
│   │       │   ├── setup_16.h
│   │       │   ├── setup_22.h
│   │       │   ├── setup_32.h
│   │       │   ├── setup_44.h
│   │       │   ├── setup_44p51.h
│   │       │   ├── setup_44u.h
│   │       │   ├── setup_8.h
│   │       │   └── setup_X.h
│   │       ├── os.h
│   │       ├── psy.c
│   │       ├── psy.h
│   │       ├── psytune.c
│   │       ├── registry.c
│   │       ├── registry.h
│   │       ├── res0.c
│   │       ├── scales.h
│   │       ├── sharedbook.c
│   │       ├── smallft.c
│   │       ├── smallft.h
│   │       ├── synthesis.c
│   │       ├── tone.c
│   │       ├── vorbisenc.c
│   │       ├── vorbisfile.c
│   │       ├── window.c
│   │       └── window.h
│   ├── mimalloc/
│   │   ├── .gitattributes
│   │   ├── .gitignore
│   │   ├── CMakeLists.txt
│   │   ├── LICENSE
│   │   ├── azure-pipelines.yml
│   │   ├── cmake/
│   │   │   ├── JoinPaths.cmake
│   │   │   ├── mimalloc-config-version.cmake
│   │   │   └── mimalloc-config.cmake
│   │   ├── doc/
│   │   │   ├── bench-2020/
│   │   │   │   ├── bench-c5-18xlarge-2020-01-20-a.svg
│   │   │   │   ├── bench-c5-18xlarge-2020-01-20-b.svg
│   │   │   │   ├── bench-c5-18xlarge-2020-01-20-rss-a.svg
│   │   │   │   ├── bench-c5-18xlarge-2020-01-20-rss-b.svg
│   │   │   │   ├── bench-r5a-1.svg
│   │   │   │   ├── bench-r5a-12xlarge-2020-01-16-a.svg
│   │   │   │   ├── bench-r5a-12xlarge-2020-01-16-b.svg
│   │   │   │   ├── bench-r5a-2.svg
│   │   │   │   ├── bench-r5a-rss-1.svg
│   │   │   │   ├── bench-r5a-rss-2.svg
│   │   │   │   ├── bench-spec-rss.svg
│   │   │   │   ├── bench-spec.svg
│   │   │   │   ├── bench-z4-1.svg
│   │   │   │   ├── bench-z4-2.svg
│   │   │   │   ├── bench-z4-rss-1.svg
│   │   │   │   └── bench-z4-rss-2.svg
│   │   │   ├── bench-2021/
│   │   │   │   ├── bench-amd5950x-2021-01-30-a.svg
│   │   │   │   ├── bench-amd5950x-2021-01-30-b.svg
│   │   │   │   ├── bench-c5-18xlarge-2021-01-30-a.svg
│   │   │   │   ├── bench-c5-18xlarge-2021-01-30-b.svg
│   │   │   │   ├── bench-c5-18xlarge-2021-01-30-rss-a.svg
│   │   │   │   ├── bench-c5-18xlarge-2021-01-30-rss-b.svg
│   │   │   │   └── bench-macmini-2021-01-30.svg
│   │   │   ├── doxyfile
│   │   │   ├── ds-logo.jpg
│   │   │   ├── ds-logo.png
│   │   │   ├── mimalloc-doc.h
│   │   │   ├── mimalloc-doxygen.css
│   │   │   ├── mimalloc-logo-100.png
│   │   │   ├── mimalloc-logo.png
│   │   │   ├── mimalloc-logo.svg
│   │   │   ├── spades-logo.png
│   │   │   └── unreal-logo.svg
│   │   ├── docs/
│   │   │   ├── annotated.html
│   │   │   ├── annotated_dup.js
│   │   │   ├── bc_s.png
│   │   │   ├── bdwn.png
│   │   │   ├── bench.html
│   │   │   ├── build.html
│   │   │   ├── classes.html
│   │   │   ├── closed.png
│   │   │   ├── doc.png
│   │   │   ├── doxygen.css
│   │   │   ├── doxygen.png
│   │   │   ├── dynsections.js
│   │   │   ├── environment.html
│   │   │   ├── folderclosed.png
│   │   │   ├── folderopen.png
│   │   │   ├── functions.html
│   │   │   ├── functions_vars.html
│   │   │   ├── group__aligned.html
│   │   │   ├── group__aligned.js
│   │   │   ├── group__analysis.html
│   │   │   ├── group__analysis.js
│   │   │   ├── group__analysis_structmi__heap__area__t.js
│   │   │   ├── group__cpp.html
│   │   │   ├── group__cpp.js
│   │   │   ├── group__extended.html
│   │   │   ├── group__extended.js
│   │   │   ├── group__heap.html
│   │   │   ├── group__heap.js
│   │   │   ├── group__malloc.html
│   │   │   ├── group__malloc.js
│   │   │   ├── group__options.html
│   │   │   ├── group__options.js
│   │   │   ├── group__posix.html
│   │   │   ├── group__posix.js
│   │   │   ├── group__typed.html
│   │   │   ├── group__typed.js
│   │   │   ├── group__zeroinit.html
│   │   │   ├── group__zeroinit.js
│   │   │   ├── index.html
│   │   │   ├── jquery.js
│   │   │   ├── mimalloc-doc_8h_source.html
│   │   │   ├── mimalloc-doxygen.css
│   │   │   ├── mimalloc-logo.svg
│   │   │   ├── modules.html
│   │   │   ├── modules.js
│   │   │   ├── nav_f.png
│   │   │   ├── nav_g.png
│   │   │   ├── nav_h.png
│   │   │   ├── navtree.css
│   │   │   ├── navtree.js
│   │   │   ├── navtreedata.js
│   │   │   ├── navtreeindex0.js
│   │   │   ├── open.png
│   │   │   ├── overrides.html
│   │   │   ├── pages.html
│   │   │   ├── resize.js
│   │   │   ├── search/
│   │   │   │   ├── all_0.html
│   │   │   │   ├── all_0.js
│   │   │   │   ├── all_1.html
│   │   │   │   ├── all_1.js
│   │   │   │   ├── all_2.html
│   │   │   │   ├── all_2.js
│   │   │   │   ├── all_3.html
│   │   │   │   ├── all_3.js
│   │   │   │   ├── all_4.html
│   │   │   │   ├── all_4.js
│   │   │   │   ├── all_5.html
│   │   │   │   ├── all_5.js
│   │   │   │   ├── all_6.html
│   │   │   │   ├── all_6.js
│   │   │   │   ├── all_7.html
│   │   │   │   ├── all_7.js
│   │   │   │   ├── all_8.html
│   │   │   │   ├── all_8.js
│   │   │   │   ├── all_9.html
│   │   │   │   ├── all_9.js
│   │   │   │   ├── all_a.html
│   │   │   │   ├── all_a.js
│   │   │   │   ├── all_b.html
│   │   │   │   ├── all_b.js
│   │   │   │   ├── all_c.html
│   │   │   │   ├── all_c.js
│   │   │   │   ├── all_d.html
│   │   │   │   ├── all_d.js
│   │   │   │   ├── classes_0.html
│   │   │   │   ├── classes_0.js
│   │   │   │   ├── close.png
│   │   │   │   ├── enums_0.html
│   │   │   │   ├── enums_0.js
│   │   │   │   ├── enumvalues_0.html
│   │   │   │   ├── enumvalues_0.js
│   │   │   │   ├── enumvalues_1.html
│   │   │   │   ├── enumvalues_1.js
│   │   │   │   ├── functions_0.html
│   │   │   │   ├── functions_0.js
│   │   │   │   ├── functions_1.html
│   │   │   │   ├── functions_1.js
│   │   │   │   ├── groups_0.html
│   │   │   │   ├── groups_0.js
│   │   │   │   ├── groups_1.html
│   │   │   │   ├── groups_1.js
│   │   │   │   ├── groups_2.html
│   │   │   │   ├── groups_2.js
│   │   │   │   ├── groups_3.html
│   │   │   │   ├── groups_3.js
│   │   │   │   ├── groups_4.html
│   │   │   │   ├── groups_4.js
│   │   │   │   ├── groups_5.html
│   │   │   │   ├── groups_5.js
│   │   │   │   ├── groups_6.html
│   │   │   │   ├── groups_6.js
│   │   │   │   ├── groups_7.html
│   │   │   │   ├── groups_7.js
│   │   │   │   ├── groups_8.html
│   │   │   │   ├── groups_8.js
│   │   │   │   ├── mag_sel.png
│   │   │   │   ├── nomatches.html
│   │   │   │   ├── pages_0.html
│   │   │   │   ├── pages_0.js
│   │   │   │   ├── pages_1.html
│   │   │   │   ├── pages_1.js
│   │   │   │   ├── pages_2.html
│   │   │   │   ├── pages_2.js
│   │   │   │   ├── pages_3.html
│   │   │   │   ├── pages_3.js
│   │   │   │   ├── pages_4.html
│   │   │   │   ├── pages_4.js
│   │   │   │   ├── search.css
│   │   │   │   ├── search.js
│   │   │   │   ├── search_l.png
│   │   │   │   ├── search_m.png
│   │   │   │   ├── search_r.png
│   │   │   │   ├── searchdata.js
│   │   │   │   ├── typedefs_0.html
│   │   │   │   ├── typedefs_0.js
│   │   │   │   ├── typedefs_1.html
│   │   │   │   ├── typedefs_1.js
│   │   │   │   ├── typedefs_2.html
│   │   │   │   ├── typedefs_2.js
│   │   │   │   ├── variables_0.html
│   │   │   │   ├── variables_0.js
│   │   │   │   ├── variables_1.html
│   │   │   │   ├── variables_1.js
│   │   │   │   ├── variables_2.html
│   │   │   │   ├── variables_2.js
│   │   │   │   ├── variables_3.html
│   │   │   │   └── variables_3.js
│   │   │   ├── splitbar.png
│   │   │   ├── sync_off.png
│   │   │   ├── sync_on.png
│   │   │   ├── tab_a.png
│   │   │   ├── tab_b.png
│   │   │   ├── tab_h.png
│   │   │   ├── tab_s.png
│   │   │   ├── tabs.css
│   │   │   └── using.html
│   │   ├── ide/
│   │   │   ├── vs2017/
│   │   │   │   ├── mimalloc-override-test.vcxproj
│   │   │   │   ├── mimalloc-override-test.vcxproj.filters
│   │   │   │   ├── mimalloc-override.vcxproj
│   │   │   │   ├── mimalloc-override.vcxproj.filters
│   │   │   │   ├── mimalloc-test-stress.vcxproj
│   │   │   │   ├── mimalloc-test-stress.vcxproj.filters
│   │   │   │   ├── mimalloc-test.vcxproj
│   │   │   │   ├── mimalloc-test.vcxproj.filters
│   │   │   │   ├── mimalloc.sln
│   │   │   │   ├── mimalloc.vcxproj
│   │   │   │   └── mimalloc.vcxproj.filters
│   │   │   ├── vs2019/
│   │   │   │   ├── mimalloc-override-test.vcxproj
│   │   │   │   ├── mimalloc-override.vcxproj
│   │   │   │   ├── mimalloc-override.vcxproj.filters
│   │   │   │   ├── mimalloc-test-api.vcxproj
│   │   │   │   ├── mimalloc-test-stress.vcxproj
│   │   │   │   ├── mimalloc-test.vcxproj
│   │   │   │   ├── mimalloc.sln
│   │   │   │   ├── mimalloc.vcxproj
│   │   │   │   └── mimalloc.vcxproj.filters
│   │   │   └── vs2022/
│   │   │       ├── mimalloc-override-test.vcxproj
│   │   │       ├── mimalloc-override.vcxproj
│   │   │       ├── mimalloc-override.vcxproj.filters
│   │   │       ├── mimalloc-test-api.vcxproj
│   │   │       ├── mimalloc-test-stress.vcxproj
│   │   │       ├── mimalloc-test.vcxproj
│   │   │       ├── mimalloc.sln
│   │   │       ├── mimalloc.vcxproj
│   │   │       └── mimalloc.vcxproj.filters
│   │   ├── include/
│   │   │   ├── mimalloc/
│   │   │   │   ├── atomic.h
│   │   │   │   ├── internal.h
│   │   │   │   ├── prim.h
│   │   │   │   ├── track.h
│   │   │   │   └── types.h
│   │   │   ├── mimalloc-new-delete.h
│   │   │   ├── mimalloc-override.h
│   │   │   └── mimalloc.h
│   │   ├── mimalloc.pc.in
│   │   ├── readme.md
│   │   ├── src/
│   │   │   ├── alloc-aligned.c
│   │   │   ├── alloc-override.c
│   │   │   ├── alloc-posix.c
│   │   │   ├── alloc.c
│   │   │   ├── arena.c
│   │   │   ├── bitmap.c
│   │   │   ├── bitmap.h
│   │   │   ├── heap.c
│   │   │   ├── init.c
│   │   │   ├── options.c
│   │   │   ├── os.c
│   │   │   ├── page-queue.c
│   │   │   ├── page.c
│   │   │   ├── prim/
│   │   │   │   ├── osx/
│   │   │   │   │   ├── alloc-override-zone.c
│   │   │   │   │   └── prim.c
│   │   │   │   ├── prim.c
│   │   │   │   ├── readme.md
│   │   │   │   ├── unix/
│   │   │   │   │   └── prim.c
│   │   │   │   ├── wasi/
│   │   │   │   │   └── prim.c
│   │   │   │   └── windows/
│   │   │   │       ├── etw-mimalloc.wprp
│   │   │   │       ├── etw.h
│   │   │   │       ├── etw.man
│   │   │   │       ├── prim.c
│   │   │   │       └── readme.md
│   │   │   ├── random.c
│   │   │   ├── segment-map.c
│   │   │   ├── segment.c
│   │   │   ├── static.c
│   │   │   └── stats.c
│   │   └── test/
│   │       ├── CMakeLists.txt
│   │       ├── main-override-static.c
│   │       ├── main-override.c
│   │       ├── main-override.cpp
│   │       ├── main.c
│   │       ├── readme.md
│   │       ├── test-api-fill.c
│   │       ├── test-api.c
│   │       ├── test-stress.c
│   │       ├── test-wrong.c
│   │       └── testhelper.h
│   ├── null/
│   │   ├── mac_net.c
│   │   ├── null_client.c
│   │   ├── null_glimp.c
│   │   ├── null_input.c
│   │   ├── null_main.c
│   │   ├── null_net.c
│   │   └── null_snddma.c
│   ├── opus-1.5.2/
│   │   ├── AUTHORS
│   │   ├── COPYING
│   │   ├── celt/
│   │   │   ├── _kiss_fft_guts.h
│   │   │   ├── arch.h
│   │   │   ├── arm/
│   │   │   │   ├── arm2gnu.pl
│   │   │   │   ├── arm_celt_map.c
│   │   │   │   ├── armcpu.c
│   │   │   │   ├── armcpu.h
│   │   │   │   ├── armopts.s.in
│   │   │   │   ├── celt_fft_ne10.c
│   │   │   │   ├── celt_mdct_ne10.c
│   │   │   │   ├── celt_neon_intr.c
│   │   │   │   ├── celt_pitch_xcorr_arm-gnu.S
│   │   │   │   ├── celt_pitch_xcorr_arm.s
│   │   │   │   ├── fft_arm.h
│   │   │   │   ├── fixed_arm64.h
│   │   │   │   ├── fixed_armv4.h
│   │   │   │   ├── fixed_armv5e.h
│   │   │   │   ├── kiss_fft_armv4.h
│   │   │   │   ├── kiss_fft_armv5e.h
│   │   │   │   ├── mdct_arm.h
│   │   │   │   ├── pitch_arm.h
│   │   │   │   └── pitch_neon_intr.c
│   │   │   ├── bands.c
│   │   │   ├── bands.h
│   │   │   ├── celt.c
│   │   │   ├── celt.h
│   │   │   ├── celt_decoder.c
│   │   │   ├── celt_encoder.c
│   │   │   ├── celt_lpc.c
│   │   │   ├── celt_lpc.h
│   │   │   ├── cpu_support.h
│   │   │   ├── cwrs.c
│   │   │   ├── cwrs.h
│   │   │   ├── ecintrin.h
│   │   │   ├── entcode.c
│   │   │   ├── entcode.h
│   │   │   ├── entdec.c
│   │   │   ├── entdec.h
│   │   │   ├── entenc.c
│   │   │   ├── entenc.h
│   │   │   ├── fixed_debug.h
│   │   │   ├── fixed_generic.h
│   │   │   ├── float_cast.h
│   │   │   ├── kiss_fft.c
│   │   │   ├── kiss_fft.h
│   │   │   ├── laplace.c
│   │   │   ├── laplace.h
│   │   │   ├── mathops.c
│   │   │   ├── mathops.h
│   │   │   ├── mdct.c
│   │   │   ├── mdct.h
│   │   │   ├── meson.build
│   │   │   ├── mfrngcod.h
│   │   │   ├── mips/
│   │   │   │   ├── celt_mipsr1.h
│   │   │   │   ├── fixed_generic_mipsr1.h
│   │   │   │   ├── kiss_fft_mipsr1.h
│   │   │   │   ├── mdct_mipsr1.h
│   │   │   │   ├── pitch_mipsr1.h
│   │   │   │   └── vq_mipsr1.h
│   │   │   ├── modes.c
│   │   │   ├── modes.h
│   │   │   ├── opus_custom_demo.c
│   │   │   ├── os_support.h
│   │   │   ├── pitch.c
│   │   │   ├── pitch.h
│   │   │   ├── quant_bands.c
│   │   │   ├── quant_bands.h
│   │   │   ├── rate.c
│   │   │   ├── rate.h
│   │   │   ├── stack_alloc.h
│   │   │   ├── static_modes_fixed.h
│   │   │   ├── static_modes_fixed_arm_ne10.h
│   │   │   ├── static_modes_float.h
│   │   │   ├── static_modes_float_arm_ne10.h
│   │   │   ├── tests/
│   │   │   │   ├── meson.build
│   │   │   │   ├── test_unit_cwrs32.c
│   │   │   │   ├── test_unit_dft.c
│   │   │   │   ├── test_unit_entropy.c
│   │   │   │   ├── test_unit_laplace.c
│   │   │   │   ├── test_unit_mathops.c
│   │   │   │   ├── test_unit_mdct.c
│   │   │   │   ├── test_unit_rotation.c
│   │   │   │   └── test_unit_types.c
│   │   │   ├── vq.c
│   │   │   ├── vq.h
│   │   │   └── x86/
│   │   │       ├── celt_lpc_sse.h
│   │   │       ├── celt_lpc_sse4_1.c
│   │   │       ├── pitch_avx.c
│   │   │       ├── pitch_sse.c
│   │   │       ├── pitch_sse.h
│   │   │       ├── pitch_sse2.c
│   │   │       ├── pitch_sse4_1.c
│   │   │       ├── vq_sse.h
│   │   │       ├── vq_sse2.c
│   │   │       ├── x86_arch_macros.h
│   │   │       ├── x86_celt_map.c
│   │   │       ├── x86cpu.c
│   │   │       └── x86cpu.h
│   │   ├── include/
│   │   │   ├── meson.build
│   │   │   ├── opus.h
│   │   │   ├── opus_custom.h
│   │   │   ├── opus_defines.h
│   │   │   ├── opus_multistream.h
│   │   │   ├── opus_projection.h
│   │   │   └── opus_types.h
│   │   ├── silk/
│   │   │   ├── A2NLSF.c
│   │   │   ├── API.h
│   │   │   ├── CNG.c
│   │   │   ├── HP_variable_cutoff.c
│   │   │   ├── Inlines.h
│   │   │   ├── LPC_analysis_filter.c
│   │   │   ├── LPC_fit.c
│   │   │   ├── LPC_inv_pred_gain.c
│   │   │   ├── LP_variable_cutoff.c
│   │   │   ├── MacroCount.h
│   │   │   ├── MacroDebug.h
│   │   │   ├── NLSF2A.c
│   │   │   ├── NLSF_VQ.c
│   │   │   ├── NLSF_VQ_weights_laroia.c
│   │   │   ├── NLSF_decode.c
│   │   │   ├── NLSF_del_dec_quant.c
│   │   │   ├── NLSF_encode.c
│   │   │   ├── NLSF_stabilize.c
│   │   │   ├── NLSF_unpack.c
│   │   │   ├── NSQ.c
│   │   │   ├── NSQ.h
│   │   │   ├── NSQ_del_dec.c
│   │   │   ├── PLC.c
│   │   │   ├── PLC.h
│   │   │   ├── SigProc_FIX.h
│   │   │   ├── VAD.c
│   │   │   ├── VQ_WMat_EC.c
│   │   │   ├── ana_filt_bank_1.c
│   │   │   ├── arm/
│   │   │   │   ├── LPC_inv_pred_gain_arm.h
│   │   │   │   ├── LPC_inv_pred_gain_neon_intr.c
│   │   │   │   ├── NSQ_del_dec_arm.h
│   │   │   │   ├── NSQ_del_dec_neon_intr.c
│   │   │   │   ├── NSQ_neon.c
│   │   │   │   ├── NSQ_neon.h
│   │   │   │   ├── SigProc_FIX_armv4.h
│   │   │   │   ├── SigProc_FIX_armv5e.h
│   │   │   │   ├── arm_silk_map.c
│   │   │   │   ├── biquad_alt_arm.h
│   │   │   │   ├── biquad_alt_neon_intr.c
│   │   │   │   ├── macros_arm64.h
│   │   │   │   ├── macros_armv4.h
│   │   │   │   └── macros_armv5e.h
│   │   │   ├── biquad_alt.c
│   │   │   ├── bwexpander.c
│   │   │   ├── bwexpander_32.c
│   │   │   ├── check_control_input.c
│   │   │   ├── code_signs.c
│   │   │   ├── control.h
│   │   │   ├── control_SNR.c
│   │   │   ├── control_audio_bandwidth.c
│   │   │   ├── control_codec.c
│   │   │   ├── debug.c
│   │   │   ├── debug.h
│   │   │   ├── dec_API.c
│   │   │   ├── decode_core.c
│   │   │   ├── decode_frame.c
│   │   │   ├── decode_indices.c
│   │   │   ├── decode_parameters.c
│   │   │   ├── decode_pitch.c
│   │   │   ├── decode_pulses.c
│   │   │   ├── decoder_set_fs.c
│   │   │   ├── define.h
│   │   │   ├── enc_API.c
│   │   │   ├── encode_indices.c
│   │   │   ├── encode_pulses.c
│   │   │   ├── errors.h
│   │   │   ├── fixed/
│   │   │   │   ├── LTP_analysis_filter_FIX.c
│   │   │   │   ├── LTP_scale_ctrl_FIX.c
│   │   │   │   ├── apply_sine_window_FIX.c
│   │   │   │   ├── arm/
│   │   │   │   │   ├── warped_autocorrelation_FIX_arm.h
│   │   │   │   │   └── warped_autocorrelation_FIX_neon_intr.c
│   │   │   │   ├── autocorr_FIX.c
│   │   │   │   ├── burg_modified_FIX.c
│   │   │   │   ├── corrMatrix_FIX.c
│   │   │   │   ├── encode_frame_FIX.c
│   │   │   │   ├── find_LPC_FIX.c
│   │   │   │   ├── find_LTP_FIX.c
│   │   │   │   ├── find_pitch_lags_FIX.c
│   │   │   │   ├── find_pred_coefs_FIX.c
│   │   │   │   ├── k2a_FIX.c
│   │   │   │   ├── k2a_Q16_FIX.c
│   │   │   │   ├── main_FIX.h
│   │   │   │   ├── mips/
│   │   │   │   │   ├── noise_shape_analysis_FIX_mipsr1.h
│   │   │   │   │   └── warped_autocorrelation_FIX_mipsr1.h
│   │   │   │   ├── noise_shape_analysis_FIX.c
│   │   │   │   ├── pitch_analysis_core_FIX.c
│   │   │   │   ├── process_gains_FIX.c
│   │   │   │   ├── regularize_correlations_FIX.c
│   │   │   │   ├── residual_energy16_FIX.c
│   │   │   │   ├── residual_energy_FIX.c
│   │   │   │   ├── schur64_FIX.c
│   │   │   │   ├── schur_FIX.c
│   │   │   │   ├── structs_FIX.h
│   │   │   │   ├── vector_ops_FIX.c
│   │   │   │   ├── warped_autocorrelation_FIX.c
│   │   │   │   └── x86/
│   │   │   │       ├── burg_modified_FIX_sse4_1.c
│   │   │   │       └── vector_ops_FIX_sse4_1.c
│   │   │   ├── float/
│   │   │   │   ├── LPC_analysis_filter_FLP.c
│   │   │   │   ├── LPC_inv_pred_gain_FLP.c
│   │   │   │   ├── LTP_analysis_filter_FLP.c
│   │   │   │   ├── LTP_scale_ctrl_FLP.c
│   │   │   │   ├── SigProc_FLP.h
│   │   │   │   ├── apply_sine_window_FLP.c
│   │   │   │   ├── autocorrelation_FLP.c
│   │   │   │   ├── burg_modified_FLP.c
│   │   │   │   ├── bwexpander_FLP.c
│   │   │   │   ├── corrMatrix_FLP.c
│   │   │   │   ├── encode_frame_FLP.c
│   │   │   │   ├── energy_FLP.c
│   │   │   │   ├── find_LPC_FLP.c
│   │   │   │   ├── find_LTP_FLP.c
│   │   │   │   ├── find_pitch_lags_FLP.c
│   │   │   │   ├── find_pred_coefs_FLP.c
│   │   │   │   ├── inner_product_FLP.c
│   │   │   │   ├── k2a_FLP.c
│   │   │   │   ├── main_FLP.h
│   │   │   │   ├── noise_shape_analysis_FLP.c
│   │   │   │   ├── pitch_analysis_core_FLP.c
│   │   │   │   ├── process_gains_FLP.c
│   │   │   │   ├── regularize_correlations_FLP.c
│   │   │   │   ├── residual_energy_FLP.c
│   │   │   │   ├── scale_copy_vector_FLP.c
│   │   │   │   ├── scale_vector_FLP.c
│   │   │   │   ├── schur_FLP.c
│   │   │   │   ├── sort_FLP.c
│   │   │   │   ├── structs_FLP.h
│   │   │   │   ├── warped_autocorrelation_FLP.c
│   │   │   │   ├── wrappers_FLP.c
│   │   │   │   └── x86/
│   │   │   │       └── inner_product_FLP_avx2.c
│   │   │   ├── gain_quant.c
│   │   │   ├── init_decoder.c
│   │   │   ├── init_encoder.c
│   │   │   ├── inner_prod_aligned.c
│   │   │   ├── interpolate.c
│   │   │   ├── lin2log.c
│   │   │   ├── log2lin.c
│   │   │   ├── macros.h
│   │   │   ├── main.h
│   │   │   ├── meson.build
│   │   │   ├── mips/
│   │   │   │   ├── NSQ_del_dec_mipsr1.h
│   │   │   │   ├── macros_mipsr1.h
│   │   │   │   └── sigproc_fix_mipsr1.h
│   │   │   ├── pitch_est_defines.h
│   │   │   ├── pitch_est_tables.c
│   │   │   ├── process_NLSFs.c
│   │   │   ├── quant_LTP_gains.c
│   │   │   ├── resampler.c
│   │   │   ├── resampler_down2.c
│   │   │   ├── resampler_down2_3.c
│   │   │   ├── resampler_private.h
│   │   │   ├── resampler_private_AR2.c
│   │   │   ├── resampler_private_IIR_FIR.c
│   │   │   ├── resampler_private_down_FIR.c
│   │   │   ├── resampler_private_up2_HQ.c
│   │   │   ├── resampler_rom.c
│   │   │   ├── resampler_rom.h
│   │   │   ├── resampler_structs.h
│   │   │   ├── shell_coder.c
│   │   │   ├── sigm_Q15.c
│   │   │   ├── sort.c
│   │   │   ├── stereo_LR_to_MS.c
│   │   │   ├── stereo_MS_to_LR.c
│   │   │   ├── stereo_decode_pred.c
│   │   │   ├── stereo_encode_pred.c
│   │   │   ├── stereo_find_predictor.c
│   │   │   ├── stereo_quant_pred.c
│   │   │   ├── structs.h
│   │   │   ├── sum_sqr_shift.c
│   │   │   ├── table_LSF_cos.c
│   │   │   ├── tables.h
│   │   │   ├── tables_LTP.c
│   │   │   ├── tables_NLSF_CB_NB_MB.c
│   │   │   ├── tables_NLSF_CB_WB.c
│   │   │   ├── tables_gain.c
│   │   │   ├── tables_other.c
│   │   │   ├── tables_pitch_lag.c
│   │   │   ├── tables_pulses_per_block.c
│   │   │   ├── tests/
│   │   │   │   ├── meson.build
│   │   │   │   └── test_unit_LPC_inv_pred_gain.c
│   │   │   ├── tuning_parameters.h
│   │   │   ├── typedef.h
│   │   │   └── x86/
│   │   │       ├── NSQ_del_dec_avx2.c
│   │   │       ├── NSQ_del_dec_sse4_1.c
│   │   │       ├── NSQ_sse4_1.c
│   │   │       ├── SigProc_FIX_sse.h
│   │   │       ├── VAD_sse4_1.c
│   │   │       ├── VQ_WMat_EC_sse4_1.c
│   │   │       ├── main_sse.h
│   │   │       └── x86_silk_map.c
│   │   └── src/
│   │       ├── analysis.c
│   │       ├── analysis.h
│   │       ├── extensions.c
│   │       ├── mapping_matrix.c
│   │       ├── mapping_matrix.h
│   │       ├── meson.build
│   │       ├── mlp.c
│   │       ├── mlp.h
│   │       ├── mlp_data.c
│   │       ├── opus.c
│   │       ├── opus_compare.c
│   │       ├── opus_decoder.c
│   │       ├── opus_demo.c
│   │       ├── opus_encoder.c
│   │       ├── opus_multistream.c
│   │       ├── opus_multistream_decoder.c
│   │       ├── opus_multistream_encoder.c
│   │       ├── opus_private.h
│   │       ├── opus_projection_decoder.c
│   │       ├── opus_projection_encoder.c
│   │       ├── repacketizer.c
│   │       └── repacketizer_demo.c
│   ├── opusfile-0.12/
│   │   ├── AUTHORS
│   │   ├── COPYING
│   │   ├── include/
│   │   │   └── opusfile.h
│   │   └── src/
│   │       ├── http.c
│   │       ├── info.c
│   │       ├── internal.c
│   │       ├── internal.h
│   │       ├── opusfile.c
│   │       ├── stream.c
│   │       ├── wincerts.c
│   │       └── winerrno.h
│   ├── qcommon/
│   │   ├── cm_load.c
│   │   ├── cm_local.h
│   │   ├── cm_patch.c
│   │   ├── cm_patch.h
│   │   ├── cm_polylib.c
│   │   ├── cm_polylib.h
│   │   ├── cm_public.h
│   │   ├── cm_test.c
│   │   ├── cm_trace.c
│   │   ├── cmd.c
│   │   ├── common.c
│   │   ├── cvar.c
│   │   ├── files.c
│   │   ├── gp_jobsystem.c
│   │   ├── gp_jobsystem.h
│   │   ├── huffman.c
│   │   ├── json.h
│   │   ├── md4.c
│   │   ├── md5.c
│   │   ├── msg.c
│   │   ├── net_chan.c
│   │   ├── net_ip.c
│   │   ├── puff.c
│   │   ├── puff.h
│   │   ├── q_math.c
│   │   ├── q_platform.h
│   │   ├── q_shared.c
│   │   ├── q_shared.h
│   │   ├── qcommon.h
│   │   ├── qfiles.h
│   │   ├── surfaceflags.h
│   │   ├── vm.c
│   │   ├── vm_armv7l.c
│   │   ├── vm_interpreted.c
│   │   ├── vm_local.h
│   │   ├── vm_none.c
│   │   ├── vm_optimize.h
│   │   ├── vm_powerpc.c
│   │   ├── vm_powerpc_asm.c
│   │   ├── vm_powerpc_asm.h
│   │   ├── vm_sparc.c
│   │   ├── vm_sparc.h
│   │   └── vm_x86.c
│   ├── renderer/
│   │   ├── anorms256.h
│   │   ├── iqm.h
│   │   ├── qgl.h
│   │   ├── tr_altivec.c
│   │   ├── tr_animation.c
│   │   ├── tr_backend.c
│   │   ├── tr_bloom.c
│   │   ├── tr_bsp.c
│   │   ├── tr_cmds.c
│   │   ├── tr_cmesh.c
│   │   ├── tr_curve.c
│   │   ├── tr_flares.c
│   │   ├── tr_font.c
│   │   ├── tr_image.c
│   │   ├── tr_image_bmp.c
│   │   ├── tr_image_jpg.c
│   │   ├── tr_image_pcx.c
│   │   ├── tr_image_png.c
│   │   ├── tr_image_tga.c
│   │   ├── tr_init.c
│   │   ├── tr_light.c
│   │   ├── tr_local.h
│   │   ├── tr_main.c
│   │   ├── tr_marks.c
│   │   ├── tr_mesh.c
│   │   ├── tr_model.c
│   │   ├── tr_model_iqm.c
│   │   ├── tr_noise.c
│   │   ├── tr_public.h
│   │   ├── tr_scene.c
│   │   ├── tr_shade.c
│   │   ├── tr_shade_calc.c
│   │   ├── tr_shader.c
│   │   ├── tr_shadows.c
│   │   ├── tr_sky.c
│   │   ├── tr_subs.c
│   │   ├── tr_surface.c
│   │   ├── tr_types.h
│   │   └── tr_world.c
│   ├── sdl/
│   │   ├── sdl_gamma.c
│   │   ├── sdl_glimp.c
│   │   ├── sdl_icon.h
│   │   ├── sdl_input.c
│   │   └── sdl_snd.c
│   ├── server/
│   │   ├── server.h
│   │   ├── sv_bot.c
│   │   ├── sv_ccmds.c
│   │   ├── sv_client.c
│   │   ├── sv_game.c
│   │   ├── sv_init.c
│   │   ├── sv_main.c
│   │   ├── sv_net_chan.c
│   │   ├── sv_snapshot.c
│   │   └── sv_world.c
│   ├── splines/
│   │   ├── math_angles.cpp
│   │   ├── math_angles.h
│   │   ├── math_matrix.cpp
│   │   ├── math_matrix.h
│   │   ├── math_quaternion.cpp
│   │   ├── math_quaternion.h
│   │   ├── math_vector.cpp
│   │   ├── math_vector.h
│   │   ├── q_parse.cpp
│   │   ├── q_shared.cpp
│   │   ├── q_splineshared.h
│   │   ├── splines.cpp
│   │   ├── splines.h
│   │   ├── util_list.h
│   │   ├── util_str.cpp
│   │   └── util_str.h
│   ├── steam/
│   │   ├── steam.c
│   │   └── steam.h
│   ├── steamshim/
│   │   ├── LICENSE.txt
│   │   ├── launcher/
│   │   │   ├── LaunchRTCW.sln
│   │   │   ├── LaunchRTCW.vcxproj
│   │   │   ├── LaunchRTCW.vcxproj.filters
│   │   │   ├── make.sh
│   │   │   ├── steam/
│   │   │   │   ├── isteamapps.h
│   │   │   │   ├── isteamappticket.h
│   │   │   │   ├── isteamclient.h
│   │   │   │   ├── isteamcontroller.h
│   │   │   │   ├── isteamdualsense.h
│   │   │   │   ├── isteamfriends.h
│   │   │   │   ├── isteamgamecoordinator.h
│   │   │   │   ├── isteamgameserver.h
│   │   │   │   ├── isteamgameserverstats.h
│   │   │   │   ├── isteamhtmlsurface.h
│   │   │   │   ├── isteamhttp.h
│   │   │   │   ├── isteaminput.h
│   │   │   │   ├── isteaminventory.h
│   │   │   │   ├── isteammatchmaking.h
│   │   │   │   ├── isteammusic.h
│   │   │   │   ├── isteamnetworking.h
│   │   │   │   ├── isteamnetworkingmessages.h
│   │   │   │   ├── isteamnetworkingsockets.h
│   │   │   │   ├── isteamnetworkingutils.h
│   │   │   │   ├── isteamparentalsettings.h
│   │   │   │   ├── isteamps3overlayrenderer.h
│   │   │   │   ├── isteamremoteplay.h
│   │   │   │   ├── isteamremotestorage.h
│   │   │   │   ├── isteamscreenshots.h
│   │   │   │   ├── isteamtimeline.h
│   │   │   │   ├── isteamugc.h
│   │   │   │   ├── isteamuser.h
│   │   │   │   ├── isteamuserstats.h
│   │   │   │   ├── isteamutils.h
│   │   │   │   ├── isteamvideo.h
│   │   │   │   ├── matchmakingtypes.h
│   │   │   │   ├── steam_api.h
│   │   │   │   ├── steam_api_common.h
│   │   │   │   ├── steam_api_flat.h
│   │   │   │   ├── steam_api_internal.h
│   │   │   │   ├── steam_gameserver.h
│   │   │   │   ├── steamclientpublic.h
│   │   │   │   ├── steamencryptedappticket.h
│   │   │   │   ├── steamhttpenums.h
│   │   │   │   ├── steamnetworkingfakeip.h
│   │   │   │   ├── steamnetworkingtypes.h
│   │   │   │   ├── steamps3params.h
│   │   │   │   ├── steamtypes.h
│   │   │   │   └── steamuniverse.h
│   │   │   ├── steamshim
│   │   │   ├── steamshim_parent.cpp
│   │   │   ├── win64/
│   │   │   └── x64/
│   │   │       └── Release/
│   │   │           ├── LaunchRTCW.log
│   │   │           ├── LaunchRTCW.tlog/
│   │   │           │   ├── CL.command.1.tlog
│   │   │           │   ├── CL.read.1.tlog
│   │   │           │   ├── CL.write.1.tlog
│   │   │           │   ├── LaunchRTCW.lastbuildstate
│   │   │           │   ├── LaunchRTCW.write.1u.tlog
│   │   │           │   ├── link.command.1.tlog
│   │   │           │   ├── link.read.1.tlog
│   │   │           │   └── link.write.1.tlog
│   │   │           ├── LaunchRTCW.vcxproj.FileListAbsolute.txt
│   │   │           ├── launcher.exe.recipe
│   │   │           ├── launcher.iobj
│   │   │           ├── launcher.ipdb
│   │   │           └── steamshim_parent.obj
│   │   ├── steam_appid.txt
│   │   ├── steamshim_child.c
│   │   └── steamshim_child.h
│   ├── sys/
│   │   ├── con_log.c
│   │   ├── con_passive.c
│   │   ├── con_tty.c
│   │   ├── con_win32.c
│   │   ├── sys_loadlib.h
│   │   ├── sys_local.h
│   │   ├── sys_main.c
│   │   ├── sys_osx.m
│   │   ├── sys_unix.c
│   │   ├── sys_win32.c
│   │   ├── win_manifest.xml
│   │   ├── win_resource.h
│   │   └── win_resource.rc
│   ├── tools/
│   │   ├── asm/
│   │   │   ├── README.Id
│   │   │   ├── cmdlib.c
│   │   │   ├── cmdlib.h
│   │   │   ├── lib.txt
│   │   │   ├── mathlib.h
│   │   │   ├── notes.txt
│   │   │   ├── ops.txt
│   │   │   ├── opstrings.h
│   │   │   └── q3asm.c
│   │   ├── lcc/
│   │   │   ├── COPYRIGHT
│   │   │   ├── LOG
│   │   │   ├── README
│   │   │   ├── README.id
│   │   │   ├── cpp/
│   │   │   │   ├── cpp.c
│   │   │   │   ├── cpp.h
│   │   │   │   ├── eval.c
│   │   │   │   ├── getopt.c
│   │   │   │   ├── hideset.c
│   │   │   │   ├── include.c
│   │   │   │   ├── lex.c
│   │   │   │   ├── macro.c
│   │   │   │   ├── nlist.c
│   │   │   │   ├── tokens.c
│   │   │   │   └── unix.c
│   │   │   ├── doc/
│   │   │   │   ├── 4.html
│   │   │   │   ├── bprint.1
│   │   │   │   ├── bprint.pdf
│   │   │   │   ├── install.html
│   │   │   │   ├── lcc.1
│   │   │   │   └── lcc.pdf
│   │   │   ├── etc/
│   │   │   │   ├── bytecode.c
│   │   │   │   └── lcc.c
│   │   │   ├── lburg/
│   │   │   │   ├── gram.c
│   │   │   │   ├── gram.y
│   │   │   │   ├── lburg.1
│   │   │   │   ├── lburg.c
│   │   │   │   └── lburg.h
│   │   │   └── src/
│   │   │       ├── alloc.c
│   │   │       ├── bind.c
│   │   │       ├── bytecode.c
│   │   │       ├── c.h
│   │   │       ├── config.h
│   │   │       ├── dag.c
│   │   │       ├── dagcheck.md
│   │   │       ├── decl.c
│   │   │       ├── enode.c
│   │   │       ├── error.c
│   │   │       ├── event.c
│   │   │       ├── expr.c
│   │   │       ├── gen.c
│   │   │       ├── init.c
│   │   │       ├── inits.c
│   │   │       ├── input.c
│   │   │       ├── lex.c
│   │   │       ├── list.c
│   │   │       ├── main.c
│   │   │       ├── null.c
│   │   │       ├── output.c
│   │   │       ├── prof.c
│   │   │       ├── profio.c
│   │   │       ├── simp.c
│   │   │       ├── stmt.c
│   │   │       ├── string.c
│   │   │       ├── sym.c
│   │   │       ├── symbolic.c
│   │   │       ├── token.h
│   │   │       ├── trace.c
│   │   │       ├── tree.c
│   │   │       └── types.c
│   │   └── stringify.c
│   ├── ui/
│   │   ├── ui_atoms.c
│   │   ├── ui_gameinfo.c
│   │   ├── ui_local.h
│   │   ├── ui_main.c
│   │   ├── ui_players.c
│   │   ├── ui_public.h
│   │   ├── ui_shared.c
│   │   ├── ui_shared.h
│   │   ├── ui_syscalls.asm
│   │   └── ui_syscalls.c
│   └── zlib-1.3.2/
│       ├── adler32.c
│       ├── crc32.c
│       ├── crc32.h
│       ├── crypt.h
│       ├── gzguts.h
│       ├── inffast.c
│       ├── inffast.h
│       ├── inffixed.h
│       ├── inflate.c
│       ├── inflate.h
│       ├── inftrees.c
│       ├── inftrees.h
│       ├── ints.h
│       ├── ioapi.c
│       ├── ioapi.h
│       ├── unzip.c
│       ├── unzip.h
│       ├── zconf.h
│       ├── zconf.h.in
│       ├── zlib.h
│       ├── zutil.c
│       └── zutil.h
├── compile.sh
├── cross-make-mingw.sh
├── cross-make-mingw64.sh
├── find.py
├── main/
│   └── ui/
│       └── menudef.h
├── make-macosx-app.sh
├── make-macosx-ub.sh
├── make-macosx.sh
├── media/
│   └── scripts/
│       ├── characters.mtr
│       ├── models.mtr
│       ├── particle.mtr
│       └── terrain.mtr
├── misc/
│   ├── iortcw.icns
│   ├── iortcw.svg
│   ├── wolf.ico
│   ├── wolf.svg
│   └── wolf512.png
├── package_mod.sh
├── readme.md
├── sdk/
│   ├── .map sources/
│   ├── BSPCUI/
│   │   ├── .gitattributes
│   │   ├── .gitignore
│   │   ├── BSPCUI.csproj
│   │   ├── BSPCUI.ico
│   │   ├── BSPCUI.sln
│   │   ├── MainForm.Designer.cs
│   │   ├── MainForm.cs
│   │   ├── MainForm.resx
│   │   └── Program.cs
│   ├── buttons templates/
│   │   ├── realrtcw_999.png
│   │   ├── realrtcw_bonus_back.png
│   │   ├── realrtcw_chapter1.png
│   │   ├── realrtcw_chapter2.png
│   │   ├── realrtcw_chapter3.png
│   │   ├── realrtcw_chapter4.png
│   │   ├── realrtcw_chapter5.png
│   │   ├── realrtcw_chapter6.png
│   │   ├── realrtcw_chapter7.png
│   │   ├── realrtcw_chapters.png
│   │   ├── realrtcw_chapters_back.png
│   │   ├── realrtcw_hardcore.png
│   │   ├── realrtcw_ingame.png
│   │   ├── realrtcw_ingame_controls.png
│   │   ├── realrtcw_ingame_game.png
│   │   ├── realrtcw_ingame_load.png
│   │   ├── realrtcw_ingame_main.png
│   │   ├── realrtcw_ingame_quit.png
│   │   ├── realrtcw_ingame_save.png
│   │   ├── realrtcw_ingame_system.png
│   │   ├── realrtcw_ironman.png
│   │   ├── realrtcw_load.png
│   │   ├── realrtcw_load_back.png
│   │   ├── realrtcw_load_load.png
│   │   ├── realrtcw_logo.png
│   │   ├── realrtcw_main.png
│   │   ├── realrtcw_main_addons.png
│   │   ├── realrtcw_main_credits.png
│   │   ├── realrtcw_main_loadgame.png
│   │   ├── realrtcw_main_newgame.png
│   │   ├── realrtcw_main_options.png
│   │   ├── realrtcw_main_quit.png
│   │   ├── realrtcw_main_training.png
│   │   ├── realrtcw_nightmare.png
│   │   ├── realrtcw_options.png
│   │   ├── realrtcw_options_back.png
│   │   ├── realrtcw_options_controls.png
│   │   ├── realrtcw_options_defaults.png
│   │   ├── realrtcw_options_game.png
│   │   ├── realrtcw_options_system.png
│   │   ├── realrtcw_play.png
│   │   ├── realrtcw_play_2.png
│   │   ├── realrtcw_play_back.png
│   │   ├── realrtcw_play_bonus.png
│   │   ├── realrtcw_play_skill0.png
│   │   ├── realrtcw_play_skill1.png
│   │   ├── realrtcw_play_skill2.png
│   │   ├── realrtcw_play_skill3.png
│   │   ├── realrtcw_play_skill4.png
│   │   ├── realrtcw_skills_bonus.png
│   │   └── realrtcw_walkpark.png
│   ├── levelshots templates/
│   │   ├── assault.jpg
│   │   ├── baseout.jpg
│   │   ├── boss1.jpg
│   │   ├── boss2.jpg
│   │   ├── castle.jpg
│   │   ├── chateau.jpg
│   │   ├── church.jpg
│   │   ├── crypt1.jpg
│   │   ├── crypt2.jpg
│   │   ├── dam.jpg
│   │   ├── dark.jpg
│   │   ├── dig.jpg
│   │   ├── dig_s.jpg
│   │   ├── end.jpg
│   │   ├── escape1.jpg
│   │   ├── escape2.jpg
│   │   ├── factory.jpg
│   │   ├── forest.jpg
│   │   ├── norway.jpg
│   │   ├── rocket.jpg
│   │   ├── sfm.jpg
│   │   ├── swf.jpg
│   │   ├── trainyard.jpg
│   │   ├── tram.jpg
│   │   ├── village1.jpg
│   │   ├── village1_s.jpg
│   │   ├── village2.jpg
│   │   └── xlabs.jpg
│   ├── realrtcw-devguide.md
│   ├── realrtcw-itemslist.md
│   └── rtcw-bspc-custom/
│       ├── .gitignore
│       ├── COPYING.txt
│       ├── README.txt
│       ├── main/
│       │   └── ui/
│       │       └── menudef.h
│       ├── src/
│       │   ├── Makefile
│       │   ├── botai/
│       │   │   ├── ai_chat.c
│       │   │   ├── ai_chat.h
│       │   │   ├── ai_cmd.c
│       │   │   ├── ai_cmd.h
│       │   │   ├── ai_dmnet.c
│       │   │   ├── ai_dmnet.h
│       │   │   ├── ai_dmq3.c
│       │   │   ├── ai_dmq3.h
│       │   │   ├── ai_main.c
│       │   │   ├── ai_main.h
│       │   │   ├── ai_team.c
│       │   │   ├── ai_team.h
│       │   │   ├── botai.h
│       │   │   ├── chars.h
│       │   │   ├── inv.h
│       │   │   ├── match.h
│       │   │   └── syn.h
│       │   ├── botlib/
│       │   │   ├── aasfile.h
│       │   │   ├── be_aas.h
│       │   │   ├── be_aas_bsp.h
│       │   │   ├── be_aas_bspq3.c
│       │   │   ├── be_aas_cluster.c
│       │   │   ├── be_aas_cluster.h
│       │   │   ├── be_aas_debug.c
│       │   │   ├── be_aas_debug.h
│       │   │   ├── be_aas_def.h
│       │   │   ├── be_aas_entity.c
│       │   │   ├── be_aas_entity.h
│       │   │   ├── be_aas_file.c
│       │   │   ├── be_aas_file.h
│       │   │   ├── be_aas_funcs.h
│       │   │   ├── be_aas_main.c
│       │   │   ├── be_aas_main.h
│       │   │   ├── be_aas_move.c
│       │   │   ├── be_aas_move.h
│       │   │   ├── be_aas_optimize.c
│       │   │   ├── be_aas_optimize.h
│       │   │   ├── be_aas_reach.c
│       │   │   ├── be_aas_reach.h
│       │   │   ├── be_aas_route.c
│       │   │   ├── be_aas_route.h
│       │   │   ├── be_aas_routealt.c
│       │   │   ├── be_aas_routealt.h
│       │   │   ├── be_aas_routetable.c
│       │   │   ├── be_aas_routetable.h
│       │   │   ├── be_aas_sample.c
│       │   │   ├── be_aas_sample.h
│       │   │   ├── be_ai_char.c
│       │   │   ├── be_ai_chat.c
│       │   │   ├── be_ai_gen.c
│       │   │   ├── be_ai_goal.c
│       │   │   ├── be_ai_move.c
│       │   │   ├── be_ai_weap.c
│       │   │   ├── be_ai_weight.c
│       │   │   ├── be_ai_weight.h
│       │   │   ├── be_ea.c
│       │   │   ├── be_interface.c
│       │   │   ├── be_interface.h
│       │   │   ├── botlib.h
│       │   │   ├── botlib.vcproj
│       │   │   ├── botlib.vcxproj
│       │   │   ├── botlib.vcxproj.filters
│       │   │   ├── l_crc.c
│       │   │   ├── l_crc.h
│       │   │   ├── l_libvar.c
│       │   │   ├── l_libvar.h
│       │   │   ├── l_log.c
│       │   │   ├── l_log.h
│       │   │   ├── l_memory.c
│       │   │   ├── l_memory.h
│       │   │   ├── l_precomp.c
│       │   │   ├── l_precomp.h
│       │   │   ├── l_script.c
│       │   │   ├── l_script.h
│       │   │   ├── l_struct.c
│       │   │   ├── l_struct.h
│       │   │   └── l_utils.h
│       │   ├── bspc/
│       │   │   ├── _files.c
│       │   │   ├── aas_areamerging.c
│       │   │   ├── aas_areamerging.h
│       │   │   ├── aas_cfg.c
│       │   │   ├── aas_cfg.h
│       │   │   ├── aas_create.c
│       │   │   ├── aas_create.h
│       │   │   ├── aas_edgemelting.c
│       │   │   ├── aas_edgemelting.h
│       │   │   ├── aas_facemerging.c
│       │   │   ├── aas_facemerging.h
│       │   │   ├── aas_file.c
│       │   │   ├── aas_file.h
│       │   │   ├── aas_gsubdiv.c
│       │   │   ├── aas_gsubdiv.h
│       │   │   ├── aas_map.c
│       │   │   ├── aas_map.h
│       │   │   ├── aas_prunenodes.c
│       │   │   ├── aas_prunenodes.h
│       │   │   ├── aas_store.c
│       │   │   ├── aas_store.h
│       │   │   ├── be_aas_bspc.c
│       │   │   ├── be_aas_bspc.h
│       │   │   ├── brushbsp.c
│       │   │   ├── bspc.c
│       │   │   ├── bspc.vcxproj
│       │   │   ├── bspc.vcxproj.filters
│       │   │   ├── csg.c
│       │   │   ├── faces.c
│       │   │   ├── glfile.c
│       │   │   ├── l_bsp_ent.c
│       │   │   ├── l_bsp_ent.h
│       │   │   ├── l_bsp_hl.c
│       │   │   ├── l_bsp_hl.h
│       │   │   ├── l_bsp_q1.c
│       │   │   ├── l_bsp_q1.h
│       │   │   ├── l_bsp_q2.c
│       │   │   ├── l_bsp_q2.h
│       │   │   ├── l_bsp_q3.c
│       │   │   ├── l_bsp_q3.h
│       │   │   ├── l_bsp_sin.c
│       │   │   ├── l_bsp_sin.h
│       │   │   ├── l_cmd.c
│       │   │   ├── l_cmd.h
│       │   │   ├── l_log.c
│       │   │   ├── l_log.h
│       │   │   ├── l_math.c
│       │   │   ├── l_math.h
│       │   │   ├── l_mem.c
│       │   │   ├── l_mem.h
│       │   │   ├── l_poly.c
│       │   │   ├── l_poly.h
│       │   │   ├── l_qfiles.c
│       │   │   ├── l_qfiles.h
│       │   │   ├── l_threads.c
│       │   │   ├── l_threads.h
│       │   │   ├── l_utils.c
│       │   │   ├── l_utils.h
│       │   │   ├── leakfile.c
│       │   │   ├── map.c
│       │   │   ├── map_hl.c
│       │   │   ├── map_q1.c
│       │   │   ├── map_q2.c
│       │   │   ├── map_q3.c
│       │   │   ├── map_sin.c
│       │   │   ├── nodraw.c
│       │   │   ├── portals.c
│       │   │   ├── prtfile.c
│       │   │   ├── q2files.h
│       │   │   ├── q3files.h
│       │   │   ├── qbsp.h
│       │   │   ├── qfiles.h
│       │   │   ├── sinfiles.h
│       │   │   ├── textures.c
│       │   │   ├── tree.c
│       │   │   └── writebsp.c
│       │   ├── cgame/
│       │   │   ├── cg_consolecmds.c
│       │   │   ├── cg_draw.c
│       │   │   ├── cg_drawtools.c
│       │   │   ├── cg_effects.c
│       │   │   ├── cg_ents.c
│       │   │   ├── cg_event.c
│       │   │   ├── cg_flamethrower.c
│       │   │   ├── cg_info.c
│       │   │   ├── cg_local.h
│       │   │   ├── cg_localents.c
│       │   │   ├── cg_main.c
│       │   │   ├── cg_marks.c
│       │   │   ├── cg_newDraw.c
│       │   │   ├── cg_particles.c
│       │   │   ├── cg_players.c
│       │   │   ├── cg_playerstate.c
│       │   │   ├── cg_predict.c
│       │   │   ├── cg_public.h
│       │   │   ├── cg_scoreboard.c
│       │   │   ├── cg_servercmds.c
│       │   │   ├── cg_snapshot.c
│       │   │   ├── cg_sound.c
│       │   │   ├── cg_syscalls.c
│       │   │   ├── cg_trails.c
│       │   │   ├── cg_view.c
│       │   │   ├── cg_weapons.c
│       │   │   ├── cgame.def
│       │   │   ├── cgame.vcproj
│       │   │   ├── cgame.vcxproj
│       │   │   ├── cgame.vcxproj.filters
│       │   │   └── tr_types.h
│       │   ├── client/
│       │   │   ├── cl_cgame.c
│       │   │   ├── cl_cin.c
│       │   │   ├── cl_console.c
│       │   │   ├── cl_input.c
│       │   │   ├── cl_keys.c
│       │   │   ├── cl_main.c
│       │   │   ├── cl_net_chan.c
│       │   │   ├── cl_parse.c
│       │   │   ├── cl_scrn.c
│       │   │   ├── cl_ui.c
│       │   │   ├── client.h
│       │   │   ├── keys.h
│       │   │   ├── snd_adpcm.c
│       │   │   ├── snd_dma.c
│       │   │   ├── snd_local.h
│       │   │   ├── snd_mem.c
│       │   │   ├── snd_mix.c
│       │   │   ├── snd_public.h
│       │   │   └── snd_wavelet.c
│       │   ├── extractfuncs/
│       │   │   ├── ChangeLog
│       │   │   ├── Conscript
│       │   │   ├── extractfuncs.bat
│       │   │   ├── extractfuncs.c
│       │   │   ├── extractfuncs.vcproj
│       │   │   ├── extractfuncs.vcxproj
│       │   │   ├── extractfuncs.vcxproj.filters
│       │   │   ├── l_log.c
│       │   │   ├── l_log.h
│       │   │   ├── l_memory.c
│       │   │   ├── l_memory.h
│       │   │   ├── l_precomp.c
│       │   │   ├── l_precomp.h
│       │   │   ├── l_script.c
│       │   │   └── l_script.h
│       │   ├── ft2/
│       │   │   ├── ahangles.c
│       │   │   ├── ahangles.h
│       │   │   ├── ahglobal.c
│       │   │   ├── ahglobal.h
│       │   │   ├── ahglyph.c
│       │   │   ├── ahglyph.h
│       │   │   ├── ahhint.c
│       │   │   ├── ahhint.h
│       │   │   ├── ahloader.h
│       │   │   ├── ahmodule.c
│       │   │   ├── ahmodule.h
│       │   │   ├── ahoptim.c
│       │   │   ├── ahoptim.h
│       │   │   ├── ahtypes.h
│       │   │   ├── autohint.h
│       │   │   ├── freetype.h
│       │   │   ├── ftcalc.c
│       │   │   ├── ftcalc.h
│       │   │   ├── ftconfig.h
│       │   │   ├── ftdebug.c
│       │   │   ├── ftdebug.h
│       │   │   ├── ftdriver.h
│       │   │   ├── fterrors.h
│       │   │   ├── ftextend.c
│       │   │   ├── ftextend.h
│       │   │   ├── ftglyph.c
│       │   │   ├── ftglyph.h
│       │   │   ├── ftgrays.c
│       │   │   ├── ftgrays.h
│       │   │   ├── ftimage.h
│       │   │   ├── ftinit.c
│       │   │   ├── ftlist.c
│       │   │   ├── ftlist.h
│       │   │   ├── ftmemory.h
│       │   │   ├── ftmm.c
│       │   │   ├── ftmm.h
│       │   │   ├── ftmodule.h
│       │   │   ├── ftnames.c
│       │   │   ├── ftnames.h
│       │   │   ├── ftobjs.c
│       │   │   ├── ftobjs.h
│       │   │   ├── ftoption.h
│       │   │   ├── ftoutln.c
│       │   │   ├── ftoutln.h
│       │   │   ├── ftraster.c
│       │   │   ├── ftraster.h
│       │   │   ├── ftrend1.c
│       │   │   ├── ftrend1.h
│       │   │   ├── ftrender.h
│       │   │   ├── ftsmooth.c
│       │   │   ├── ftsmooth.h
│       │   │   ├── ftstream.c
│       │   │   ├── ftstream.h
│       │   │   ├── ftsystem.c
│       │   │   ├── ftsystem.h
│       │   │   ├── fttypes.h
│       │   │   ├── psnames.h
│       │   │   ├── sfdriver.c
│       │   │   ├── sfdriver.h
│       │   │   ├── sfnt.h
│       │   │   ├── sfobjs.c
│       │   │   ├── sfobjs.h
│       │   │   ├── t1tables.h
│       │   │   ├── ttcmap.c
│       │   │   ├── ttcmap.h
│       │   │   ├── ttdriver.c
│       │   │   ├── ttdriver.h
│       │   │   ├── tterrors.h
│       │   │   ├── ttgload.c
│       │   │   ├── ttgload.h
│       │   │   ├── ttinterp.c
│       │   │   ├── ttinterp.h
│       │   │   ├── ttload.c
│       │   │   ├── ttload.h
│       │   │   ├── ttnameid.h
│       │   │   ├── ttobjs.c
│       │   │   ├── ttobjs.h
│       │   │   ├── ttpload.c
│       │   │   ├── ttpload.h
│       │   │   ├── ttpost.c
│       │   │   ├── ttpost.h
│       │   │   ├── ttsbit.c
│       │   │   ├── ttsbit.h
│       │   │   ├── tttables.h
│       │   │   ├── tttags.h
│       │   │   └── tttypes.h
│       │   ├── game/
│       │   │   ├── ai_cast.c
│       │   │   ├── ai_cast.h
│       │   │   ├── ai_cast_characters.c
│       │   │   ├── ai_cast_debug.c
│       │   │   ├── ai_cast_events.c
│       │   │   ├── ai_cast_fight.c
│       │   │   ├── ai_cast_fight.h
│       │   │   ├── ai_cast_func_attack.c
│       │   │   ├── ai_cast_func_boss1.c
│       │   │   ├── ai_cast_funcs.c
│       │   │   ├── ai_cast_global.h
│       │   │   ├── ai_cast_script.c
│       │   │   ├── ai_cast_script_actions.c
│       │   │   ├── ai_cast_script_ents.c
│       │   │   ├── ai_cast_sight.c
│       │   │   ├── ai_cast_think.c
│       │   │   ├── be_aas.h
│       │   │   ├── be_ai_char.h
│       │   │   ├── be_ai_chat.h
│       │   │   ├── be_ai_gen.h
│       │   │   ├── be_ai_goal.h
│       │   │   ├── be_ai_move.h
│       │   │   ├── be_ai_weap.h
│       │   │   ├── be_ea.h
│       │   │   ├── bg_animation.c
│       │   │   ├── bg_lib.c
│       │   │   ├── bg_local.h
│       │   │   ├── bg_misc.c
│       │   │   ├── bg_pmove.c
│       │   │   ├── bg_public.h
│       │   │   ├── bg_slidemove.c
│       │   │   ├── botlib.h
│       │   │   ├── g_active.c
│       │   │   ├── g_alarm.c
│       │   │   ├── g_bot.c
│       │   │   ├── g_client.c
│       │   │   ├── g_cmds.c
│       │   │   ├── g_combat.c
│       │   │   ├── g_func_decs.h
│       │   │   ├── g_funcs.h
│       │   │   ├── g_items.c
│       │   │   ├── g_local.h
│       │   │   ├── g_main.c
│       │   │   ├── g_mem.c
│       │   │   ├── g_misc.c
│       │   │   ├── g_missile.c
│       │   │   ├── g_mover.c
│       │   │   ├── g_props.c
│       │   │   ├── g_public.h
│       │   │   ├── g_save.c
│       │   │   ├── g_script.c
│       │   │   ├── g_script_actions.c
│       │   │   ├── g_session.c
│       │   │   ├── g_spawn.c
│       │   │   ├── g_svcmds.c
│       │   │   ├── g_syscalls.c
│       │   │   ├── g_target.c
│       │   │   ├── g_team.c
│       │   │   ├── g_team.h
│       │   │   ├── g_tramcar.c
│       │   │   ├── g_trigger.c
│       │   │   ├── g_utils.c
│       │   │   ├── g_weapon.c
│       │   │   ├── game.def
│       │   │   ├── game.vcproj
│       │   │   ├── game.vcxproj
│       │   │   ├── game.vcxproj.filters
│       │   │   ├── q_math.c
│       │   │   ├── q_shared.c
│       │   │   ├── q_shared.h
│       │   │   └── surfaceflags.h
│       │   ├── idLib/
│       │   │   ├── idAudio.h
│       │   │   ├── idAudioHardware.h
│       │   │   ├── idLib.h
│       │   │   ├── idQ3Asnd.cpp
│       │   │   ├── idSound.cpp
│       │   │   ├── idSound.h
│       │   │   ├── idSpeaker.cpp
│       │   │   └── idSpeaker.h
│       │   ├── jpeg-6/
│       │   │   ├── jcapimin.c
│       │   │   ├── jccoefct.c
│       │   │   ├── jccolor.c
│       │   │   ├── jcdctmgr.c
│       │   │   ├── jchuff.c
│       │   │   ├── jchuff.h
│       │   │   ├── jcinit.c
│       │   │   ├── jcmainct.c
│       │   │   ├── jcmarker.c
│       │   │   ├── jcmaster.c
│       │   │   ├── jcomapi.c
│       │   │   ├── jconfig.h
│       │   │   ├── jcparam.c
│       │   │   ├── jcphuff.c
│       │   │   ├── jcprepct.c
│       │   │   ├── jcsample.c
│       │   │   ├── jctrans.c
│       │   │   ├── jdapimin.c
│       │   │   ├── jdapistd.c
│       │   │   ├── jdatadst.c
│       │   │   ├── jdatasrc.c
│       │   │   ├── jdcoefct.c
│       │   │   ├── jdcolor.c
│       │   │   ├── jdct.h
│       │   │   ├── jddctmgr.c
│       │   │   ├── jdhuff.c
│       │   │   ├── jdhuff.h
│       │   │   ├── jdinput.c
│       │   │   ├── jdmainct.c
│       │   │   ├── jdmarker.c
│       │   │   ├── jdmaster.c
│       │   │   ├── jdpostct.c
│       │   │   ├── jdsample.c
│       │   │   ├── jdtrans.c
│       │   │   ├── jerror.c
│       │   │   ├── jerror.h
│       │   │   ├── jfdctflt.c
│       │   │   ├── jidctflt.c
│       │   │   ├── jinclude.h
│       │   │   ├── jmemmgr.c
│       │   │   ├── jmemnobs.c
│       │   │   ├── jmemsys.h
│       │   │   ├── jmorecfg.h
│       │   │   ├── jpegint.h
│       │   │   ├── jpeglib.h
│       │   │   ├── jutils.c
│       │   │   └── jversion.h
│       │   ├── macosx/
│       │   │   ├── BuildRelease
│       │   │   ├── CGMouseDeltaFix.h
│       │   │   ├── CGMouseDeltaFix.m
│       │   │   ├── CGPrivateAPI.h
│       │   │   ├── GenerateQGL.pl
│       │   │   ├── Performance.rtf
│       │   │   ├── Q3Controller.h
│       │   │   ├── Q3Controller.m
│       │   │   ├── Quake3.icns
│       │   │   ├── Quake3.nib/
│       │   │   │   ├── classes.nib
│       │   │   │   ├── info.nib
│       │   │   │   └── objects.nib
│       │   │   ├── RecordDemo.zsh
│       │   │   ├── WolfSP.pbproj/
│       │   │   │   └── project.pbxproj
│       │   │   ├── banner.jpg
│       │   │   ├── macosx_display.h
│       │   │   ├── macosx_display.m
│       │   │   ├── macosx_glimp.h
│       │   │   ├── macosx_glimp.m
│       │   │   ├── macosx_glsmp_mutex.m
│       │   │   ├── macosx_glsmp_null.m
│       │   │   ├── macosx_glsmp_ports.m
│       │   │   ├── macosx_input.m
│       │   │   ├── macosx_local.h
│       │   │   ├── macosx_qgl.h
│       │   │   ├── macosx_sndcore.m
│       │   │   ├── macosx_snddma.m
│       │   │   ├── macosx_sys.m
│       │   │   ├── macosx_timers.h
│       │   │   ├── macosx_timers.m
│       │   │   └── timedemo.zsh
│       │   ├── qcommon/
│       │   │   ├── cm_load.c
│       │   │   ├── cm_local.h
│       │   │   ├── cm_patch.c
│       │   │   ├── cm_patch.h
│       │   │   ├── cm_polylib.c
│       │   │   ├── cm_polylib.h
│       │   │   ├── cm_public.h
│       │   │   ├── cm_test.c
│       │   │   ├── cm_trace.c
│       │   │   ├── cmd.c
│       │   │   ├── common.c
│       │   │   ├── cvar.c
│       │   │   ├── files.c
│       │   │   ├── huffman.c
│       │   │   ├── md4.c
│       │   │   ├── msg.c
│       │   │   ├── net_chan.c
│       │   │   ├── qcommon.h
│       │   │   ├── qfiles.h
│       │   │   ├── unzip.c
│       │   │   ├── unzip.h
│       │   │   ├── vm.c
│       │   │   ├── vm_interpreted.c
│       │   │   ├── vm_local.h
│       │   │   └── vm_x86.c
│       │   ├── renderer/
│       │   │   ├── anorms256.h
│       │   │   ├── qgl.h
│       │   │   ├── qgl_linked.h
│       │   │   ├── ref_trin.def
│       │   │   ├── renderer.vcproj
│       │   │   ├── renderer.vcxproj
│       │   │   ├── renderer.vcxproj.filters
│       │   │   ├── tr_animation.c
│       │   │   ├── tr_backend.c
│       │   │   ├── tr_bsp.c
│       │   │   ├── tr_cmds.c
│       │   │   ├── tr_cmesh.c
│       │   │   ├── tr_curve.c
│       │   │   ├── tr_flares.c
│       │   │   ├── tr_font.c
│       │   │   ├── tr_image.c
│       │   │   ├── tr_init.c
│       │   │   ├── tr_light.c
│       │   │   ├── tr_local.h
│       │   │   ├── tr_main.c
│       │   │   ├── tr_marks.c
│       │   │   ├── tr_mesh.c
│       │   │   ├── tr_model.c
│       │   │   ├── tr_noise.c
│       │   │   ├── tr_public.h
│       │   │   ├── tr_scene.c
│       │   │   ├── tr_shade.c
│       │   │   ├── tr_shade_calc.c
│       │   │   ├── tr_shader.c
│       │   │   ├── tr_shadows.c
│       │   │   ├── tr_sky.c
│       │   │   ├── tr_surface.c
│       │   │   └── tr_world.c
│       │   ├── server/
│       │   │   ├── server.h
│       │   │   ├── sv_bot.c
│       │   │   ├── sv_ccmds.c
│       │   │   ├── sv_client.c
│       │   │   ├── sv_game.c
│       │   │   ├── sv_init.c
│       │   │   ├── sv_main.c
│       │   │   ├── sv_net_chan.c
│       │   │   ├── sv_snapshot.c
│       │   │   └── sv_world.c
│       │   ├── splines/
│       │   │   ├── Splines.vcproj
│       │   │   ├── Splines.vcxproj
│       │   │   ├── Splines.vcxproj.filters
│       │   │   ├── math_angles.cpp
│       │   │   ├── math_angles.h
│       │   │   ├── math_matrix.cpp
│       │   │   ├── math_matrix.h
│       │   │   ├── math_quaternion.cpp
│       │   │   ├── math_quaternion.h
│       │   │   ├── math_vector.cpp
│       │   │   ├── math_vector.h
│       │   │   ├── q_parse.cpp
│       │   │   ├── q_shared.cpp
│       │   │   ├── q_splineshared.h
│       │   │   ├── splines.cpp
│       │   │   ├── splines.h
│       │   │   ├── util_list.h
│       │   │   ├── util_str.cpp
│       │   │   └── util_str.h
│       │   ├── ui/
│       │   │   ├── keycodes.h
│       │   │   ├── ui.def
│       │   │   ├── ui.vcproj
│       │   │   ├── ui.vcxproj
│       │   │   ├── ui.vcxproj.filters
│       │   │   ├── ui_atoms.c
│       │   │   ├── ui_gameinfo.c
│       │   │   ├── ui_local.h
│       │   │   ├── ui_main.c
│       │   │   ├── ui_players.c
│       │   │   ├── ui_public.h
│       │   │   ├── ui_shared.c
│       │   │   ├── ui_shared.h
│       │   │   ├── ui_syscalls.c
│       │   │   └── ui_util.c
│       │   ├── unix/
│       │   │   ├── ChangeLog
│       │   │   ├── Conscript-bspc
│       │   │   ├── Conscript-cgame
│       │   │   ├── Conscript-client
│       │   │   ├── Conscript-game
│       │   │   ├── Conscript-setup
│       │   │   ├── Conscript-ui
│       │   │   ├── Construct
│       │   │   ├── README.EULA
│       │   │   ├── README.Linux
│       │   │   ├── README.Q3Test
│       │   │   ├── bspc.vpj
│       │   │   ├── build_setup.sh
│       │   │   ├── build_tarball.sh
│       │   │   ├── cgame.vpj
│       │   │   ├── client.vpj
│       │   │   ├── cons
│       │   │   ├── extractfuncs.vpj
│       │   │   ├── ftol.nasm
│       │   │   ├── game.vpj
│       │   │   ├── linux_common.c
│       │   │   ├── linux_glimp.c
│       │   │   ├── linux_joystick.c
│       │   │   ├── linux_local.h
│       │   │   ├── linux_qgl.c
│       │   │   ├── linux_snd.c
│       │   │   ├── matha.s
│       │   │   ├── pcons-2.3.1
│       │   │   ├── qasm.h
│       │   │   ├── quake3.xpm
│       │   │   ├── snapvector.nasm
│       │   │   ├── snd_mixa.s
│       │   │   ├── sys_dosa.s
│       │   │   ├── ui.vpj
│       │   │   ├── unix_glw.h
│       │   │   ├── unix_main.c
│       │   │   ├── unix_net.c
│       │   │   ├── unix_shared.c
│       │   │   ├── vm_x86.c
│       │   │   ├── vm_x86a.s
│       │   │   └── wolf.vpw
│       │   ├── wolf.sln
│       │   ├── wolf.vcproj
│       │   ├── wolf.vcxproj
│       │   └── wolf.vcxproj.filters
│       └── uncrustify.cfg
├── ui/
│   ├── ingame_system.menu
│   └── main.menu
├── uncrustify.cfg
└── voip-readme.txt
```
