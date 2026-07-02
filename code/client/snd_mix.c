/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// snd_mix.c -- portable code to mix sounds for snd_dma.c

#include "client.h"
#include "snd_local.h"

static portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
static int snd_vol;

int*     snd_p;  
int      snd_linear_count;
short*   snd_out;

#if	!id386                                        // if configured not to use asm

void S_WriteLinearBlastStereo16 (void)
{
    int i = 0;

#ifndef Q3_VM
    // Process 8 audio samples at a time (4 stereo pairs) concurrently
    for (; i + 7 < snd_linear_count; i += 8)
    {
        // Load 8 32-bit mixed integer samples from the paintbuffer
        __m128i in_lo = _mm_loadu_si128((const __m128i*)&snd_p[i]);
        __m128i in_hi = _mm_loadu_si128((const __m128i*)&snd_p[i + 4]);

        // Arithmetic shift right by 8 bits across all channels
        __m128i shifted_lo = _mm_srai_epi32(in_lo, 8);
        __m128i shifted_hi = _mm_srai_epi32(in_hi, 8);

        // Pack 32-bit integers down to 16-bit shorts with hardware saturation clamping
        __m128i packed = _mm_packs_epi32(shifted_lo, shifted_hi);

        // Blast the 8 packed 16-bit stereo samples to the output audio buffer
        _mm_storeu_si128((__m128i*)&snd_out[i], packed);
    }
#endif

    // Clean scalar fallback for remaining samples or QVM compatibility
    for (; i < snd_linear_count; i += 2)
    {
        int val;

        val = snd_p[i] >> 8;
        if (val > 0x7fff)
            snd_out[i] = 0x7fff;
        else if (val < -32768)
            snd_out[i] = -32768;
        else
            snd_out[i] = val;

        val = snd_p[i + 1] >> 8;
        if (val > 0x7fff)
            snd_out[i + 1] = 0x7fff;
        else if (val < -32768)
            snd_out[i + 1] = -32768;
        else
            snd_out[i + 1] = val;
    }
}
#elif defined(__GNUC__)
// uses snd_mixa.s
void S_WriteLinearBlastStereo16 (void);
#else

__declspec( naked ) void S_WriteLinearBlastStereo16 (void)
{
	__asm {

 push edi
 push ebx
 mov ecx,ds:dword ptr[snd_linear_count]
 mov ebx,ds:dword ptr[snd_p]
 mov edi,ds:dword ptr[snd_out]
LWLBLoopTop:
 mov eax,ds:dword ptr[-8+ebx+ecx*4]
 sar eax,8
 cmp eax,07FFFh
 jg LClampHigh
 cmp eax,0FFFF8000h
 jnl LClampDone
 mov eax,0FFFF8000h
 jmp LClampDone
LClampHigh:
 mov eax,07FFFh
LClampDone:
 mov edx,ds:dword ptr[-4+ebx+ecx*4]
 sar edx,8
 cmp edx,07FFFh
 jg LClampHigh2
 cmp edx,0FFFF8000h
 jnl LClampDone2
 mov edx,0FFFF8000h
 jmp LClampDone2
LClampHigh2:
 mov edx,07FFFh
LClampDone2:
 shl edx,16
 and eax,0FFFFh
 or edx,eax
 mov ds:dword ptr[-4+edi+ecx*2],edx
 sub ecx,2
 jnz LWLBLoopTop
 pop ebx
 pop edi
 ret
	}
}

#endif

void S_TransferStereo16 (unsigned long *pbuf, int endtime)
{
	int		lpos;
	int		ls_paintedtime;
	
	snd_p = (int *) paintbuffer;
	ls_paintedtime = s_paintedtime;

	while (ls_paintedtime < endtime)
	{
	// handle recirculating buffer issues
		lpos = ls_paintedtime % dma.fullsamples;

		snd_out = (short *) pbuf + (lpos<<1); // lpos * dma.channels

		snd_linear_count = dma.fullsamples - lpos;
		if (ls_paintedtime + snd_linear_count > endtime)
			snd_linear_count = endtime - ls_paintedtime;

		snd_linear_count <<= 1; // snd_linear_count *= dma.channels

	// write a linear blast of samples
		S_WriteLinearBlastStereo16 ();

		snd_p += snd_linear_count;
		ls_paintedtime += (snd_linear_count>>1); // snd_linear_count / dma.channels

		if( CL_VideoRecording( ) )
			CL_WriteAVIAudioFrame( (byte *)snd_out, snd_linear_count << 1 ); // snd_linear_count * (dma.samplebits/8)
	}
}

/*
===================
S_TransferPaintBuffer

===================
*/
void S_TransferPaintBuffer(int endtime)
{
    int out_idx;
    int count;
    int* p;
    int step;
    int val;
    int i;
    unsigned long* pbuf;

    pbuf = (unsigned long*)dma.buffer;

    if (s_testsound->integer)
    {
        // Write a fixed sine wave
        count = (endtime - s_paintedtime);
        for (i = 0; i < count; i++)
            paintbuffer[i].left = paintbuffer[i].right = sin((s_paintedtime + i) * 0.1) * 20000 * 256;
    }

    if (dma.samplebits == 16 && dma.channels == 2)
    {
        // Fully optimized stereo short hardware block path
        S_TransferStereo16(pbuf, endtime);
    }
    else
    {
        p = (int*)paintbuffer;
        count = (endtime - s_paintedtime) * dma.channels;
        out_idx = ((unsigned int)s_paintedtime * dma.channels) % dma.samples;
        step = 3 - MIN(dma.channels, 2);

        int chan_counter = 0; // Eliminates the expensive (i % dma.channels) operation

        if ((dma.isfloat) && (dma.samplebits == 32))
        {
            float* out = (float*)pbuf;
            for (i = 0; i < count; i++)
            {
                // Branch-free stream mapping using an auto-incrementing ring counter
                if (chan_counter >= 2)
                {
                    val = 0;
                }
                else
                {
                    val = *p >> 8;
                    p += step;
                }

                if (val > 0x7fff)
                    val = 0x7fff;
                else if (val < -32767) 
                    val = -32767;

                out[out_idx] = ((float)val) / 32767.0f;
                
                // Optimized linear buffer pointer wrapping bypasses modulo operator
                out_idx++;
                if (out_idx >= dma.samples) out_idx = 0;

                chan_counter++;
                if (chan_counter >= dma.channels) chan_counter = 0;
            }
        }
        else if (dma.samplebits == 16)
        {
            short* out = (short*)pbuf;
            for (i = 0; i < count; i++)
            {
                if (chan_counter >= 2)
                {
                    val = 0;
                }
                else
                {
                    val = *p >> 8;
                    p += step;
                }

                if (val > 0x7fff)
                    val = 0x7fff;
                else if (val < -32768)
                    val = -32768;

                out[out_idx] = val;
                
                out_idx++;
                if (out_idx >= dma.samples) out_idx = 0;

                chan_counter++;
                if (chan_counter >= dma.channels) chan_counter = 0;
            }
        }
        else if (dma.samplebits == 8)
        {
            unsigned char* out = (unsigned char*)pbuf;
            for (i = 0; i < count; i++)
            {
                if (chan_counter >= 2)
                {
                    val = 0;
                }
                else
                {
                    val = *p >> 8;
                    p += step;
                }

                if (val > 0x7fff)
                    val = 0x7fff;
                else if (val < -32768)
                    val = -32768;

                out[out_idx] = (val >> 8) + 128;
                
                out_idx++;
                if (out_idx >= dma.samples) out_idx = 0;

                chan_counter++;
                if (chan_counter >= dma.channels) chan_counter = 0;
            }
        }
    }
}

/*
===============================================================================

LIP SYNCING

===============================================================================
*/

unsigned char s_entityTalkAmplitude[MAX_CLIENTS];

/*
===================
S_SetVoiceAmplitudeFrom16
===================
*/
void S_SetVoiceAmplitudeFrom16( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	chunk = sc->soundData;
	while ( sampleOffset >= SND_CHUNK_SIZE ) {
		chunk = chunk->next;
		sampleOffset -= SND_CHUNK_SIZE;
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}

	sfx_count = 0;
	samples = chunk->sndChunk;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= SND_CHUNK_SIZE ) {
			chunk = chunk->next;
			samples = chunk->sndChunk;
			sampleOffset = 0;
		}
		data  = samples[sampleOffset++];
		if ( abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromADPCM
===================
*/
void S_SetVoiceAmplitudeFromADPCM( sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	i = 0;
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 4 );
		i++;
	}

	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		S_AdpcmGetSamples( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}

	sfx_count = 0;
	samples = sfxScratchBuffer;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= SND_CHUNK_SIZE * 4 ) {
			chunk = chunk->next;
			S_AdpcmGetSamples( chunk, sfxScratchBuffer );
			sampleOffset = 0;
			sfxScratchIndex++;
		}
		data  = samples[sampleOffset++];
		if ( abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromWavelet
===================
*/
void S_SetVoiceAmplitudeFromWavelet( sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	i = 0;
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE_FLOAT * 4 );
		i++;
	}
	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		decodeWavelet( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}
	sfx_count = 0;
	samples = sfxScratchBuffer;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
			chunk = chunk->next;
			decodeWavelet( chunk, sfxScratchBuffer );
			sfxScratchIndex++;
			sampleOffset = 0;
		}
		data = samples[sampleOffset++];
		if ( abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}

	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromMuLaw
===================
*/
void S_SetVoiceAmplitudeFromMuLaw( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	byte *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE * 2 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 2 );
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}
	sfx_count = 0;
	samples = (byte *)chunk->sndChunk + sampleOffset;
	for ( i = 0; i < count; i++ ) {
		if ( chunk && ( samples >= (byte *)chunk->sndChunk + ( SND_CHUNK_SIZE * 2 ) ) ) {
			chunk = chunk->next;
			samples = (byte *)chunk->sndChunk;
		}
		if ( samples ) {
			data  = mulawToShort[*samples];
			if ( abs( data ) > 5000 ) {
				sfx_count += ( data * 255 ) >> 8;
			}
			samples++;
		}
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

static void S_PaintChannelFrom16_scalar(channel_t* ch, const sfx_t* sc, int count, int sampleOffset, int bufferOffset)
{
    int leftvol, rightvol;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    short* samples;

    if (sc->soundChannels <= 0)
    {
        return;
    }

    samp = &paintbuffer[bufferOffset];

    if (ch->doppler)
    {
        sampleOffset = sampleOffset * ch->oldDopplerScale;
    }

    if (sc->soundChannels == 2)
    {
        sampleOffset *= sc->soundChannels;
        if (sampleOffset & 1)
        {
            sampleOffset &= ~1;
        }
    }

    // Advance to the correct starting audio memory chunk
    chunk = sc->soundData;
    while (sampleOffset >= SND_CHUNK_SIZE)
    {
        chunk = chunk->next;
        sampleOffset -= SND_CHUNK_SIZE;
        if (!chunk)
        {
            chunk = sc->soundData;
        }
    }

    if (!ch->doppler || ch->dopplerScale == 1.0f)
    {
        leftvol = ch->leftvol * snd_vol;
        rightvol = ch->rightvol * snd_vol;
        samples = chunk->sndChunk;

#ifndef Q3_VM
        // Set up the volume vector layout: [Right, Left, Right, Left]
        __m128i v_vol = _mm_set_epi32(rightvol, leftvol, rightvol, leftvol);

        while (count > 0)
        {
            // Loop Chunking: Calculate exactly how many samples we can safely mix 
            // before hitting the hard boundary of the current audio chunk pointer
            int chunk_remaining = SND_CHUNK_SIZE - sampleOffset;
            int max_pairs = (sc->soundChannels == 2) ? (chunk_remaining >> 1) : chunk_remaining;
            int to_process = (count < max_pairs) ? count : max_pairs;

            if (to_process <= 0)
            {
                chunk = chunk->next;
                if (!chunk) chunk = sc->soundData;
                samples = chunk->sndChunk;
                sampleOffset = 0;
                continue;
            }

            int j = 0;
            if (sc->soundChannels == 2)
            {
                // Vectorized Stereo Loop: Process 2 stereo pairs (4 samples) per iteration
                for (; j + 1 < to_process; j += 2)
                {
                    // Load 4 shorts (L0, R0, L1, R1) and sign-extend them to 32-bit integers
                    __m128i src = _mm_loadl_epi64((const __m128i*)&samples[sampleOffset]);
                    __m128i src32 = _mm_cvtepi16_epi32(src);

                    // Multiply by channel volumes and shift right to scale audio levels
                    __m128i mixed = _mm_mullo_epi32(src32, v_vol);
                    __m128i scaled = _mm_srai_epi32(mixed, 8);

                    // Accumulate directly into the 32-bit integer paintbuffer
                    __m128i dest = _mm_loadu_si128((const __m128i*)&samp[j]);
                    _mm_storeu_si128((__m128i*)&samp[j], _mm_add_epi32(dest, scaled));

                    sampleOffset += 4;
                }
                // Scalar cleanup for remaining single stereo pair in this chunk slice
                for (; j < to_process; j++)
                {
                    samp[j].left  += (samples[sampleOffset]   * leftvol) >> 8;
                    samp[j].right += (samples[sampleOffset+1] * rightvol) >> 8;
                    sampleOffset += 2;
                }
            }
            else
            {
                // Vectorized Mono Loop: Process 4 mono samples (filling 4 stereo pairs) per iteration
                for (; j + 3 < to_process; j += 4)
                {
                    // Load 4 mono shorts and sign-extend to 32-bit integers
                    __m128i src = _mm_loadl_epi64((const __m128i*)&samples[sampleOffset]);
                    __m128i src32 = _mm_cvtepi16_epi32(src);

                    // Unpack elements to duplicate each mono sample across Left and Right slots
                    __m128i src_lo = _mm_unpacklo_epi32(src32, src32); // [M1, M1, M0, M0]
                    __m128i src_hi = _mm_unpackhi_epi32(src32, src32); // [M3, M3, M2, M2]

                    // Multiply and scale audio streams
                    __m128i mixed_lo = _mm_mullo_epi32(src_lo, v_vol);
                    __m128i mixed_hi = _mm_mullo_epi32(src_hi, v_vol);
                    __m128i scaled_lo = _mm_srai_epi32(mixed_lo, 8);
                    __m128i scaled_hi = _mm_srai_epi32(mixed_hi, 8);

                    // Accumulate 4 stereo pairs back to paintbuffer
                    __m128i dest_lo = _mm_loadu_si128((const __m128i*)&samp[j]);
                    __m128i dest_hi = _mm_loadu_si128((const __m128i*)&samp[j + 2]);
                    _mm_storeu_si128((__m128i*)&samp[j],     _mm_add_epi32(dest_lo, scaled_lo));
                    _mm_storeu_si128((__m128i*)&samp[j + 2], _mm_add_epi32(dest_hi, scaled_hi));

                    sampleOffset += 4;
                }
                // Scalar cleanup for remaining mono samples in this chunk slice
                for (; j < to_process; j++)
                {
                    int data = samples[sampleOffset++];
                    samp[j].left  += (data * leftvol) >> 8;
                    samp[j].right += (data * rightvol) >> 8;
                }
            }

            samp += to_process;
            count -= to_process;

            if (sampleOffset >= SND_CHUNK_SIZE)
            {
                chunk = chunk->next;
                if (!chunk) chunk = sc->soundData;
                samples = chunk->sndChunk;
                sampleOffset = 0;
            }
        }
#else
        // Legacy QVM fallback path
        for (int i = 0; i < count; i++)
        {
            int data = samples[sampleOffset++];
            samp[i].left += (data * leftvol) >> 8;

            if (sc->soundChannels == 2)
            {
                data = samples[sampleOffset++];
            }
            samp[i].right += (data * rightvol) >> 8;

            if (sampleOffset == SND_CHUNK_SIZE)
            {
                chunk = chunk->next;
                samples = chunk->sndChunk;
                sampleOffset = 0;
            }
        }
#endif
    }
    else
    {
        // Keep the legacy doppler handling loop intact as fallback
        float ooff, fdata[2], fdiv, fleftvol, frightvol;
        int aoff, boff, j;

        fleftvol = ch->leftvol * snd_vol;
        frightvol = ch->rightvol * snd_vol;

        ooff = sampleOffset;
        samples = chunk->sndChunk;

        for (int i = 0; i < count; i++)
        {
            aoff = ooff;
            ooff = ooff + ch->dopplerScale * sc->soundChannels;
            boff = ooff;
            fdata[0] = fdata[1] = 0;
            for (j = aoff; j < boff; j += sc->soundChannels)
            {
                if (j == SND_CHUNK_SIZE)
                {
                    chunk = chunk->next;
                    if (!chunk)
                    {
                        chunk = sc->soundData;
                    }
                    samples = chunk->sndChunk;
                    ooff -= SND_CHUNK_SIZE;
                }
                if (sc->soundChannels == 2)
                {
                    fdata[0] += samples[j & (SND_CHUNK_SIZE - 1)];
                    fdata[1] += samples[(j + 1) & (SND_CHUNK_SIZE - 1)];
                }
                else
                {
                    fdata[0] += samples[j & (SND_CHUNK_SIZE - 1)];
                    fdata[1] += samples[j & (SND_CHUNK_SIZE - 1)];
                }
            }
            fdiv = 256 * (boff - aoff) / sc->soundChannels;
            samp[i].left += (fdata[0] * fleftvol) / fdiv;
            samp[i].right += (fdata[1] * frightvol) / fdiv;
        }
    }
}

static void S_PaintChannelFrom16( channel_t *ch, const sfx_t *sc, int count, int sampleOffset, int bufferOffset ) {
#if idppc_altivec
	if (com_altivec->integer) {
		// must be in a separate translation unit or G3 systems will crash.
		S_PaintChannelFrom16_altivec( paintbuffer, snd_vol, ch, sc, count, sampleOffset, bufferOffset );
		return;
	}
#endif
	S_PaintChannelFrom16_scalar( ch, sc, count, sampleOffset, bufferOffset );
}

void S_PaintChannelFromWavelet(channel_t* ch, sfx_t* sc, int count, int sampleOffset, int bufferOffset)
{
    int data;
    int leftvol, rightvol;
    int i;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    short* samples;

    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;

    i = 0;
    samp = &paintbuffer[bufferOffset];
    chunk = sc->soundData;
    while (sampleOffset >= (SND_CHUNK_SIZE_FLOAT * 4))
    {
        chunk = chunk->next;
        sampleOffset -= (SND_CHUNK_SIZE_FLOAT * 4);
        i++;
    }

    if (i != sfxScratchIndex || sfxScratchPointer != sc)
    {
        decodeWavelet(chunk, sfxScratchBuffer);
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }

    samples = sfxScratchBuffer;

#ifndef Q3_VM
    // Set up parallel stereo volume vector layout: [Right, Left, Right, Left]
    __m128i v_vol = _mm_set_epi32(rightvol, leftvol, rightvol, leftvol);

    while (count > 0)
    {
        // Loop Chunking: Calculate space remaining in the current wavelet scratch block
        int chunk_remaining = (SND_CHUNK_SIZE * 2) - sampleOffset;
        int to_process = (count < chunk_remaining) ? count : chunk_remaining;

        if (to_process <= 0)
        {
            chunk = chunk->next;
            decodeWavelet(chunk, sfxScratchBuffer);
            sampleOffset = 0;
            sfxScratchIndex++;
            samples = sfxScratchBuffer;
            continue;
        }

        int j = 0;
        // Vectorized Mono Loop: Process 4 wavelet frames per iteration
        for (; j + 3 < to_process; j += 4)
        {
            // Load 4 mono samples and sign-extend to 32-bit registers
            __m128i src = _mm_loadl_epi64((const __m128i*)&samples[sampleOffset]);
            __m128i src32 = _mm_cvtepi16_epi32(src);

            // Duplicate mono samples across left/right channels
            __m128i src_lo = _mm_unpacklo_epi32(src32, src32); // [M1, M1, M0, M0]
            __m128i src_hi = _mm_unpackhi_epi32(src32, src32); // [M3, M3, M2, M2]

            // Apply scaling multiplications and shifts
            __m128i mixed_lo = _mm_mullo_epi32(src_lo, v_vol);
            __m128i mixed_hi = _mm_mullo_epi32(src_hi, v_vol);
            __m128i scaled_lo = _mm_srai_epi32(mixed_lo, 8);
            __m128i scaled_hi = _mm_srai_epi32(mixed_hi, 8);

            // Accumulate into the 32-bit integer paintbuffer
            __m128i dest_lo = _mm_loadu_si128((const __m128i*)&samp[j]);
            __m128i dest_hi = _mm_loadu_si128((const __m128i*)&samp[j + 2]);
            _mm_storeu_si128((__m128i*)&samp[j],     _mm_add_epi32(dest_lo, scaled_lo));
            _mm_storeu_si128((__m128i*)&samp[j + 2], _mm_add_epi32(dest_hi, scaled_hi));

            sampleOffset += 4;
        }

        // Clean scalar fallback for block tails
        for (; j < to_process; j++)
        {
            int data = samples[sampleOffset++];
            samp[j].left  += (data * leftvol) >> 8;
            samp[j].right += (data * rightvol) >> 8;
        }

        samp += to_process;
        count -= to_process;

        if (sampleOffset >= (SND_CHUNK_SIZE * 2))
        {
            chunk = chunk->next;
            decodeWavelet(chunk, sfxScratchBuffer);
            sampleOffset = 0;
            sfxScratchIndex++;
            samples = sfxScratchBuffer;
        }
    }
#else
    // Legacy QVM fallback path
    for (i = 0; i < count; i++)
    {
        data = samples[sampleOffset++];
        samp[i].left += (data * leftvol) >> 8;
        samp[i].right += (data * rightvol) >> 8;

        if (sampleOffset == SND_CHUNK_SIZE * 2)
        {
            chunk = chunk->next;
            decodeWavelet(chunk, sfxScratchBuffer);
            sfxScratchIndex++;
            sampleOffset = 0;
        }
    }
#endif
}

void S_PaintChannelFromADPCM(channel_t* ch, sfx_t* sc, int count, int sampleOffset, int bufferOffset)
{
    int data;
    int leftvol, rightvol;
    int i;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    short* samples;

    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;

    i = 0;
    samp = &paintbuffer[bufferOffset];
    chunk = sc->soundData;

    if (ch->doppler)
    {
        sampleOffset = sampleOffset * ch->oldDopplerScale;
    }

    while (sampleOffset >= (SND_CHUNK_SIZE * 4))
    {
        chunk = chunk->next;
        sampleOffset -= (SND_CHUNK_SIZE * 4);
        i++;
    }

    if (i != sfxScratchIndex || sfxScratchPointer != sc)
    {
        S_AdpcmGetSamples(chunk, sfxScratchBuffer);
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }

    samples = sfxScratchBuffer;

#ifndef Q3_VM
    // Set up parallel stereo volume vector layout: [Right, Left, Right, Left]
    __m128i v_vol = _mm_set_epi32(rightvol, leftvol, rightvol, leftvol);

    while (count > 0)
    {
        // Loop Chunking: Calculate the remaining sample bounds inside the current scratch block
        int chunk_remaining = (SND_CHUNK_SIZE * 4) - sampleOffset;
        int to_process = (count < chunk_remaining) ? count : chunk_remaining;

        if (to_process <= 0)
        {
            chunk = chunk->next;
            S_AdpcmGetSamples(chunk, sfxScratchBuffer);
            sampleOffset = 0;
            sfxScratchIndex++;
            samples = sfxScratchBuffer;
            continue;
        }

        int j = 0;
        // Vectorized Mono-to-Stereo Loop: Mix 4 audio frames per iteration
        for (; j + 3 < to_process; j += 4)
        {
            // Load 4 mono short samples and sign-extend to 32-bit registers
            __m128i src = _mm_loadl_epi64((const __m128i*)&samples[sampleOffset]);
            __m128i src32 = _mm_cvtepi16_epi32(src);

            // Duplicate mono samples across left/right channels via interleave unpacking
            __m128i src_lo = _mm_unpacklo_epi32(src32, src32); // [M1, M1, M0, M0]
            __m128i src_hi = _mm_unpackhi_epi32(src32, src32); // [M3, M3, M2, M2]

            // Apply scaling multiplications and shifts
            __m128i mixed_lo = _mm_mullo_epi32(src_lo, v_vol);
            __m128i mixed_hi = _mm_mullo_epi32(src_hi, v_vol);
            __m128i scaled_lo = _mm_srai_epi32(mixed_lo, 8);
            __m128i scaled_hi = _mm_srai_epi32(mixed_hi, 8);

            // Accumulate directly into the 32-bit mixing paintbuffer
            __m128i dest_lo = _mm_loadu_si128((const __m128i*)&samp[j]);
            __m128i dest_hi = _mm_loadu_si128((const __m128i*)&samp[j + 2]);
            _mm_storeu_si128((__m128i*)&samp[j],     _mm_add_epi32(dest_lo, scaled_lo));
            _mm_storeu_si128((__m128i*)&samp[j + 2], _mm_add_epi32(dest_hi, scaled_hi));

            sampleOffset += 4;
        }

        // Clean scalar fallback for block tail ends
        for (; j < to_process; j++)
        {
            int data = samples[sampleOffset++];
            samp[j].left  += (data * leftvol) >> 8;
            samp[j].right += (data * rightvol) >> 8;
        }

        samp += to_process;
        count -= to_process;

        if (sampleOffset >= (SND_CHUNK_SIZE * 4))
        {
            chunk = chunk->next;
            S_AdpcmGetSamples(chunk, sfxScratchBuffer);
            sampleOffset = 0;
            sfxScratchIndex++;
            samples = sfxScratchBuffer;
        }
    }
#else
    // Legacy QVM fallback path
    for (i = 0; i < count; i++)
    {
        data = samples[sampleOffset++];
        samp[i].left += (data * leftvol) >> 8;
        samp[i].right += (data * rightvol) >> 8;

        if (sampleOffset == SND_CHUNK_SIZE * 4)
        {
            chunk = chunk->next;
            S_AdpcmGetSamples(chunk, sfxScratchBuffer);
            sampleOffset = 0;
            sfxScratchIndex++;
        }
    }
#endif
}

void S_PaintChannelFromMuLaw(channel_t* ch, sfx_t* sc, int count, int sampleOffset, int bufferOffset)
{
    int data;
    int leftvol, rightvol;
    int i;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    byte* samples;

    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;

    samp = &paintbuffer[bufferOffset];
    chunk = sc->soundData;
    while (sampleOffset >= (SND_CHUNK_SIZE * 2))
    {
        chunk = chunk->next;
        sampleOffset -= (SND_CHUNK_SIZE * 2);
        if (!chunk)
        {
            chunk = sc->soundData;
        }
    }

    if (!ch->doppler)
    {
        samples = (byte*)chunk->sndChunk + sampleOffset;

#ifndef Q3_VM
        // Parallel stereo volume setup: [Right, Left, Right, Left]
        __m128i v_vol = _mm_set_epi32(rightvol, leftvol, rightvol, leftvol);

        while (count > 0)
        {
            // Loop Chunking: Isolate memory boundaries before mixing
            int chunk_remaining = (SND_CHUNK_SIZE * 2) - sampleOffset;
            int to_process = (count < chunk_remaining) ? count : chunk_remaining;

            if (to_process <= 0)
            {
                chunk = chunk->next;
                samples = (byte*)chunk->sndChunk;
                sampleOffset = 0;
                continue;
            }

            int j = 0;
            // Vectorized Processing Loop: Mix 4 Mu-Law frames concurrently
            for (; j + 3 < to_process; j += 4)
            {
                // Parallel lookups converted into a single register layout
                int d0 = mulawToShort[samples[0]];
                int d1 = mulawToShort[samples[1]];
                int d2 = mulawToShort[samples[2]];
                int d3 = mulawToShort[samples[3]];
                
                __m128i src32 = _mm_set_epi32(d3, d2, d1, d0);

                // Expand Mono to Stereo channels via register interleaving
                __m128i src_lo = _mm_unpacklo_epi32(src32, src32); // [d1, d1, d0, d0]
                __m128i src_hi = _mm_unpackhi_epi32(src32, src32); // [d3, d3, d2, d2]

                // Scale amplitudes via parallel matrix vector multiply
                __m128i mixed_lo = _mm_mullo_epi32(src_lo, v_vol);
                __m128i mixed_hi = _mm_mullo_epi32(src_hi, v_vol);
                __m128i scaled_lo = _mm_srai_epi32(mixed_lo, 8);
                __m128i scaled_hi = _mm_srai_epi32(mixed_hi, 8);

                // Accumulate results into the 32-bit integer paintbuffer
                __m128i dest_lo = _mm_loadu_si128((const __m128i*)&samp[j]);
                __m128i dest_hi = _mm_loadu_si128((const __m128i*)&samp[j + 2]);
                _mm_storeu_si128((__m128i*)&samp[j],     _mm_add_epi32(dest_lo, scaled_lo));
                _mm_storeu_si128((__m128i*)&samp[j + 2], _mm_add_epi32(dest_hi, scaled_hi));

                samples      += 4;
                sampleOffset += 4;
            }

            // Clean scalar fallback for slice remainders
            for (; j < to_process; j++)
            {
                data = mulawToShort[*samples];
                samp[j].left  += (data * leftvol) >> 8;
                samp[j].right += (data * rightvol) >> 8;
                samples++;
                sampleOffset++;
            }

            samp  += to_process;
            count -= to_process;

            if (sampleOffset >= (SND_CHUNK_SIZE * 2))
            {
                chunk = chunk->next;
                samples = (byte*)chunk->sndChunk;
                sampleOffset = 0;
            }
        }
#else
        // Legacy QVM fallback path
        for (i = 0; i < count; i++)
        {
            data = mulawToShort[*samples];
            samp[i].left += (data * leftvol) >> 8;
            samp[i].right += (data * rightvol) >> 8;
            samples++;
            if (chunk != NULL && samples == (byte*)chunk->sndChunk + (SND_CHUNK_SIZE * 2))
            {
                chunk = chunk->next;
                samples = (byte*)chunk->sndChunk;
            }
        }
#endif
    }
    else
    {
        // Maintain the rare Doppler math loop as standard scalar logic
        float ooff = sampleOffset;
        samples = (byte*)chunk->sndChunk;
        for (i = 0; i < count; i++)
        {
            data = mulawToShort[samples[(int)(ooff)]];
            ooff = ooff + ch->dopplerScale;
            samp[i].left += (data * leftvol) >> 8;
            samp[i].right += (data * rightvol) >> 8;
            if (ooff >= SND_CHUNK_SIZE * 2)
            {
                chunk = chunk->next;
                if (!chunk)
                {
                    chunk = sc->soundData;
                }
                samples = (byte*)chunk->sndChunk;
                ooff = 0.0;
            }
        }
    }
}

#define TALK_FUTURE_SEC 0.01        // go this far into the future (seconds)

/*
===================
S_PaintChannels
===================
*/
void S_PaintChannels(int endtime)
{
    int i;
    int end;
    int stream;
    channel_t* ch;
    sfx_t* sc;
    int ltime, count;
    int sampleOffset;

    if (s_muted->integer)
        snd_vol = 0;
    else
        snd_vol = s_volume->value * 255;

    while (s_paintedtime < endtime)
    {
        end = endtime;
        if (endtime - s_paintedtime > PAINTBUFFER_SIZE)
        {
            end = s_paintedtime + PAINTBUFFER_SIZE;
        }

        // Clear the paint buffer
        Com_Memset(paintbuffer, 0, sizeof(paintbuffer));
        
        // Accumulate raw streaming sound sources
        for (stream = 0; stream < MAX_RAW_STREAMS; stream++)
        {
            if (s_rawend[stream] >= s_paintedtime)
            {
                const portable_samplepair_t* rawsamples = s_rawsamples[stream];
                const int stop = (end < s_rawend[stream]) ? end : s_rawend[stream];
                int paint_idx = 0;
                i = s_paintedtime;

#ifndef Q3_VM
                // Vectorized Raw Mixer: Process 2 stereo pairs (4 audio channels) simultaneously
                // We stop 1 sample early to safely allow 2-element contiguous vector loads
                for (; i + 1 < stop; i += 2)
                {
                    int s0 = i & (MAX_RAW_SAMPLES - 1);
                    int s1 = (i + 1) & (MAX_RAW_SAMPLES - 1);

                    // If the ring buffer samples are contiguous, we can use a parallel SIMD add
                    if (s1 == s0 + 1)
                    {
                        __m128i v_raw  = _mm_loadu_si128((const __m128i*)&rawsamples[s0]);
                        __m128i v_paint = _mm_loadu_si128((const __m128i*)&paintbuffer[paint_idx]);
                        _mm_storeu_si128((__m128i*)&paintbuffer[paint_idx], _mm_add_epi32(v_paint, v_raw));
                        paint_idx += 2;
                    }
                    else
                    {
                        // Handle unaligned ring-buffer wrapping boundaries gracefully
                        paintbuffer[paint_idx].left   += rawsamples[s0].left;
                        paintbuffer[paint_idx].right  += rawsamples[s0].right;
                        paintbuffer[paint_idx+1].left += rawsamples[s1].left;
                        paintbuffer[paint_idx+1].right += rawsamples[s1].right;
                        paint_idx += 2;
                    }
                }
#endif
                // Scalar tail cleanup
                for (; i < stop; i++)
                {
                    const int s = i & (MAX_RAW_SAMPLES - 1);
                    paintbuffer[paint_idx].left  += rawsamples[s].left;
                    paintbuffer[paint_idx].right += rawsamples[s].right;
                    paint_idx++;
                }
            }
        }

        // Paint in the standard 3D active positional channels
        ch = s_channels;
        for (i = 0; i < MAX_CHANNELS; i++, ch++)
        {
            // Branch Hint: Most channels are inactive on any given frame; optimize the skip path
#if defined(__GNUC__) || defined(__clang__)
            if (__builtin_expect(!ch->thesfx || (ch->leftvol < 0.25f && ch->rightvol < 0.25f), 1))
#else
            if (!ch->thesfx || (ch->leftvol < 0.25f && ch->rightvol < 0.25f))
#endif
            {
                continue;
            }

            ltime = s_paintedtime;
            sc = ch->thesfx;

            if (sc->soundData == NULL || sc->soundLength == 0)
            {
                continue;
            }

            sampleOffset = ltime - ch->startSample;
            count = end - ltime;
            if (sampleOffset + count > sc->soundLength)
            {
                count = sc->soundLength - sampleOffset;
            }

            if (count > 0)
            {
                if (sc->soundCompressionMethod == 1)
                {
                    S_PaintChannelFromADPCM(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                }
                else if (sc->soundCompressionMethod == 2)
                {
                    S_PaintChannelFromWavelet(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                }
                else if (sc->soundCompressionMethod == 3)
                {
                    S_PaintChannelFromMuLaw(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                }
                else
                {
                    S_PaintChannelFrom16(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                }
            }
        }

        // Paint in the looped ambient channels
        ch = loop_channels;
        for (i = 0; i < numLoopChannels; i++, ch++)
        {
#if defined(__GNUC__) || defined(__clang__)
            if (__builtin_expect(!ch->thesfx || (!ch->leftvol && !ch->rightvol), 1))
#else
            if (!ch->thesfx || (!ch->leftvol && !ch->rightvol))
#endif
            {
                continue;
            }

            ltime = s_paintedtime;
            sc = ch->thesfx;

            if (sc->soundData == NULL || sc->soundLength == 0)
            {
                continue;
            }

            do
            {
                sampleOffset = (ltime % sc->soundLength);
                count = end - ltime;
                if (sampleOffset + count > sc->soundLength)
                {
                    count = sc->soundLength - sampleOffset;
                }

                if (count > 0)
                {
                    if (sc->soundCompressionMethod == 1)
                    {
                        S_PaintChannelFromADPCM(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                    }
                    else if (sc->soundCompressionMethod == 2)
                    {
                        S_PaintChannelFromWavelet(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                    }
                    else if (sc->soundCompressionMethod == 3)
                    {
                        S_PaintChannelFromMuLaw(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                    }
                    else
                    {
                        S_PaintChannelFrom16(ch, sc, count, sampleOffset, ltime - s_paintedtime);
                    }
                    ltime += count;
                }
            }
            while (ltime < end);
        }

        // Transfer local paint buffer values out directly to the audio hardware DMA system
        S_TransferPaintBuffer(end);
        s_paintedtime = end;
    }
}
