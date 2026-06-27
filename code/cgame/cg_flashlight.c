#include "cg_flashlight.h"
#include "cg_local.h"

qboolean cg_flashlightOn = qfalse;
float cg_flashlightEnergy = 1.0f; // range 0.0f to 1.0f

void CG_setClientFlags(void);

// Flashlight mechanics constants
#define FLASHLIGHT_MIN_ENERGY_TO_TURN_ON 0.10f
#define FLASHLIGHT_DRAIN_TIME_MS 20000.0f    // 20 seconds to drain
#define FLASHLIGHT_RECHARGE_TIME_MS 10000.0f // 10 seconds to recharge

// Flashlight projection constants
#define FLASHLIGHT_MAX_DISTANCE 600.0f
#define FLASHLIGHT_DISTANCE_THRESHOLD 100.0f
#define FLASHLIGHT_SPOT_SIZE_BASE 220.0f
#define FLASHLIGHT_SPOT_SIZE_SCALE 0.07f

void CG_Flashlight_Toggle_f(void) {
  if (!cg_flashlightOn && cg_flashlightEnergy < FLASHLIGHT_MIN_ENERGY_TO_TURN_ON) {
    return;
  }

  cg_flashlightOn = !cg_flashlightOn;

  if (cg.snap) {
    if (cgs.media.selectSound) {
      trap_S_StartLocalSound(cgs.media.selectSound, CHAN_LOCAL);
    }
    CG_setClientFlags();
  }
}

void CG_Flashlight_Frame(void) {
  trace_t tr;
  vec3_t end;
  float distance;
  float spotSize;
  
  // Inertia states
  static vec3_t currentLightDir = {0};
  static qboolean initialized = qfalse;

  if (!cg.snap) {
    return;
  }

  // 1. Drain battery if ON, recharge if OFF
  if (cg_flashlightOn) {
    cg_flashlightEnergy -= (float)cg.frametime / FLASHLIGHT_DRAIN_TIME_MS;
    if (cg_flashlightEnergy <= 0.0f) {
      cg_flashlightEnergy = 0.0f;
      cg_flashlightOn = qfalse;
      if (cgs.media.selectSound) {
        trap_S_StartLocalSound(cgs.media.selectSound, CHAN_LOCAL);
      }
      CG_setClientFlags();
    }
  } else {
    cg_flashlightEnergy += (float)cg.frametime / FLASHLIGHT_RECHARGE_TIME_MS;
    if (cg_flashlightEnergy > 1.0f) {
      cg_flashlightEnergy = 1.0f;
    }
  }

  if (!cg_flashlightOn) {
    initialized = qfalse; // Reset inertia when turned off
    return;
  }

  // 2. Calculate View Inertia (Sway)
  if (!initialized) {
    VectorCopy(cg.refdef.viewaxis[0], currentLightDir);
    initialized = qtrue;
  }

  // cg_flashlightSway controls the lag. 15.0 is a good baseline. Higher = less lag.
  float swaySpeed = cg_flashlightSway.value > 0 ? cg_flashlightSway.value : 15.0f;
  float lagFactor = swaySpeed * (cg.frametime / 1000.0f); 
  if (lagFactor > 1.0f) lagFactor = 1.0f;

  currentLightDir[0] += (cg.refdef.viewaxis[0][0] - currentLightDir[0]) * lagFactor;
  currentLightDir[1] += (cg.refdef.viewaxis[0][1] - currentLightDir[1]) * lagFactor;
  currentLightDir[2] += (cg.refdef.viewaxis[0][2] - currentLightDir[2]) * lagFactor;
  VectorNormalize(currentLightDir);

  // 3. Calculate Mounting Offset (Right Shoulder)
  vec3_t flashOrigin;
  VectorCopy(cg.refdef.vieworg, flashOrigin);
  // Offset: 8 units right, 4 units down
  VectorMA(flashOrigin, 8.0f, cg.refdef.viewaxis[1], flashOrigin); 
  VectorMA(flashOrigin, -4.0f, cg.refdef.viewaxis[2], flashOrigin);

  // 4. Project the spot light using inertia direction and offset origin
  VectorMA(flashOrigin, FLASHLIGHT_MAX_DISTANCE, currentLightDir, end);
  CG_Trace(&tr, flashOrigin, NULL, NULL, end, cg.predictedPlayerState.clientNum, MASK_SOLID);

  distance = tr.fraction * FLASHLIGHT_MAX_DISTANCE;
  spotSize = cg_flashlightRadius.value > 0 ? cg_flashlightRadius.value : 280.0f;
  if (distance > FLASHLIGHT_DISTANCE_THRESHOLD) {
    spotSize = FLASHLIGHT_SPOT_SIZE_BASE + (distance * FLASHLIGHT_SPOT_SIZE_SCALE);
  }

  // 5. Intensity, Battery Flicker, and Constant Noise
  float intensityScale = 1.0f;

  if (cg_flashlightEnergy < 0.25f) {
    float factor = cg_flashlightEnergy / 0.25f; // 1.0 down to 0.0
    if (random() > factor) {
      intensityScale = 0.1f + 0.15f * random(); // Extreme dim/flicker
    } else {
      intensityScale = 0.4f + 0.6f * factor;    // Smooth drain dimming
    }
  }

  // Apply constant, subtle analog noise to intensity (voltage hum)
  intensityScale *= (0.96f + (crandom() * 0.04f));

  vec3_t jitteredEndpos;
  float rJitter = 1.0f + (crandom() * 0.015f);

  VectorCopy(tr.endpos, jitteredEndpos);
  jitteredEndpos[0] += crandom() * 0.4f;
  jitteredEndpos[1] += crandom() * 0.4f;
  jitteredEndpos[2] += crandom() * 0.4f;

  // Retrieve color from CVARs
  float rColor = cg_flashlightColorR.value * intensityScale;
  float gColor = cg_flashlightColorG.value * intensityScale;
  float bColor = cg_flashlightColorB.value * intensityScale;

  // Concentrated bright hotspot (inner cone)
  trap_R_AddLightToScene(jitteredEndpos, spotSize * 0.5f * rJitter, rColor, gColor, bColor, 0);

  // Wide, dim ambient spill (outer cone)
  trap_R_AddLightToScene(jitteredEndpos, spotSize * 1.6f * rJitter, rColor * 0.35f, gColor * 0.35f, bColor * 0.35f, 0);

  // Player-centric ambient light (illuminates hands/weapons)
  vec3_t playerLightOrg;
  VectorMA(cg.refdef.vieworg, 32.0f, cg.refdef.viewaxis[0], playerLightOrg);
  trap_R_AddLightToScene(playerLightOrg, 150.0f * rJitter, rColor * 0.40f, gColor * 0.40f, bColor * 0.40f, 0);
}

// HUD layout constants
#define FLASHLIGHT_HUD_X 465
#define FLASHLIGHT_HUD_Y 450
#define FLASHLIGHT_BAR_WIDTH 100
#define FLASHLIGHT_BAR_HEIGHT 6

void CG_Flashlight_DrawHUD(void) {
  static qhandle_t flashlightIcon = 0;
  if (!flashlightIcon) {
    flashlightIcon = trap_R_RegisterShader("gfx/2d/flashlight_icon");
  }

  vec4_t color = {1.0f, 1.0f, 1.0f, 0.8f};       // Dynamic color
  vec4_t boxcolor = {0.05f, 0.05f, 0.05f, 0.9f}; // Dark background box
  vec4_t iconcolor = {0.9f, 0.9f, 0.9f, 1.0f};   // White icon

  // Calculate dynamic color: White -> Yellow -> Red
  if (cg_flashlightEnergy >= 1.0f) {
    color[2] = 1.0f;
  } else if (cg_flashlightEnergy < 0.66f) {
    color[2] = 0.0f;
  } else {
    color[2] = (cg_flashlightEnergy - 0.66f) / 0.34f;
  }

  if (cg_flashlightEnergy > 0.30f) {
    color[1] = 1.0f;
  } else if (cg_flashlightEnergy < 0.15f) {
    color[1] = 0.0f;
  } else {
    color[1] = (cg_flashlightEnergy - 0.15f) / 0.15f;
  }

  // Don't draw if flashlight is off and battery is fully charged (keeps HUD clean)
  if (!cg_flashlightOn && cg_flashlightEnergy >= 1.0f) {
    return;
  }

  if (!cg.snap || cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.cameraMode) {
    return;
  }

  // Draw black background box (encompasses icon and bar)
  CG_FillRect(FLASHLIGHT_HUD_X - 20, FLASHLIGHT_HUD_Y - 4,
              FLASHLIGHT_BAR_WIDTH + 24, FLASHLIGHT_BAR_HEIGHT + 8, boxcolor);

  // Draw flashlight icon
  if (flashlightIcon) {
    trap_R_SetColor(iconcolor);
    CG_DrawPic(FLASHLIGHT_HUD_X - 16, FLASHLIGHT_HUD_Y - 2, 10, 10,
               flashlightIcon);
    trap_R_SetColor(NULL);
  }

  // Draw bar fill
  CG_FillRect(FLASHLIGHT_HUD_X, FLASHLIGHT_HUD_Y,
              (int)(FLASHLIGHT_BAR_WIDTH * cg_flashlightEnergy),
              FLASHLIGHT_BAR_HEIGHT, color);
}
