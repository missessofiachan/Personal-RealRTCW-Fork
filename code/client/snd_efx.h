#ifndef __SND_EFX_H__
#define __SND_EFX_H__

#define REVERB_PRESET_OUTDOORS   0
#define REVERB_PRESET_BUNKER     1
#define REVERB_PRESET_ROOM       2
#define REVERB_PRESET_LARGE_HALL 3
#define REVERB_PRESET_CRYPT      4
#define REVERB_PRESET_TUNNEL     5
#define REVERB_PRESET_SEWER      6
#define REVERB_PRESET_DEPOT      7

void S_EFX_Init(void);
void S_EFX_Shutdown(void);
void S_EFX_SetPreset(int presetNum);
void S_EFX_AttachSource(unsigned int alSource);

#endif // __SND_EFX_H__
