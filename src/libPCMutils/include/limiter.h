/*-----------------------------------------------------------------------------
Software License for The Fraunhofer FDK MPEG-H Software

Copyright (c) 2018 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. and Contributors
All rights reserved.

1. INTRODUCTION

The "Fraunhofer FDK MPEG-H Software" is software that implements the ISO/MPEG
MPEG-H 3D Audio standard for digital audio or related system features. Patent
licenses for necessary patent claims for the Fraunhofer FDK MPEG-H Software
(including those of Fraunhofer), for the use in commercial products and
services, may be obtained from the respective patent owners individually and/or
from Via LA (www.via-la.com).

Fraunhofer supports the development of MPEG-H products and services by offering
additional software, documentation, and technical advice. In addition, it
operates the MPEG-H Trademark Program to ease interoperability testing of end-
products. Please visit www.mpegh.com for more information.

2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

* You must retain the complete text of this software license in redistributions
of the Fraunhofer FDK MPEG-H Software or your modifications thereto in source
code form.

* You must retain the complete text of this software license in the
documentation and/or other materials provided with redistributions of
the Fraunhofer FDK MPEG-H Software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of
the Fraunhofer FDK MPEG-H Software and your modifications thereto to recipients
of copies in binary form.

* The name of Fraunhofer may not be used to endorse or promote products derived
from the Fraunhofer FDK MPEG-H Software without prior written permission.

* You may not charge copyright license fees for anyone to use, copy or
distribute the Fraunhofer FDK MPEG-H Software or your modifications thereto.

* Your modified versions of the Fraunhofer FDK MPEG-H Software must carry
prominent notices stating that you changed the software and the date of any
change. For modified versions of the Fraunhofer FDK MPEG-H Software, the term
"Fraunhofer FDK MPEG-H Software" must be replaced by the term "Third-Party
Modified Version of the Fraunhofer FDK MPEG-H Software".

3. No PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software. You may use this Fraunhofer FDK MPEG-H Software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.

4. DISCLAIMER

This Fraunhofer FDK MPEG-H Software is provided by Fraunhofer on behalf of the
copyright holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, including but not limited to the implied warranties of
merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE for any direct, indirect,
incidental, special, exemplary, or consequential damages, including but not
limited to procurement of substitute goods or services; loss of use, data, or
profits, or business interruption, however caused and on any theory of
liability, whether in contract, strict liability, or tort (including
negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.

5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Audio and Media Technologies - MPEG-H FDK
Am Wolfsmantel 33
91058 Erlangen, Germany
www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
-----------------------------------------------------------------------------*/

/**************************** PCM utility library ******************************

   Author(s):   Matthias Neusinger

   Description: Hard limiter for clipping prevention

*******************************************************************************/

#ifndef LIMITER_H
#define LIMITER_H

#include "common_fix.h"
#include "FDK_audio.h"

#define TDL_ATTACK_DEFAULT_MS (15)  /* default attack  time in ms */
#define TDL_RELEASE_DEFAULT_MS (50) /* default release time in ms */
#define TDLIMIT_FALLBACK_FACTOR \
  8 /* factor on release time to let the gain fall back to 1.0 in float precision */

#ifdef __cplusplus
extern "C" {
#endif

struct TDLimiter {
  unsigned int attack;
  FIXP_DBL attackConst, releaseConst;
  unsigned int attackMs, releaseMs, maxAttackMs;
  FIXP_DBL threshold;
  unsigned int channels, maxChannels;
  UINT sampleRate, maxSampleRate;
  FIXP_DBL cor, max;
  FIXP_DBL* maxBuf;
  FIXP_DBL* delayBuf;
  unsigned int maxBufIdx, delayBufIdx;
  FIXP_DBL smoothState0;
  FIXP_DBL minGain;
  UINT cleanSamples;
  UINT release;
  UCHAR previous_mode;
  INT scaling;
};

typedef enum {
  TDLIMIT_OK = 0,
  TDLIMIT_UNKNOWN = -1,

  __error_codes_start = -100,

  TDLIMIT_INVALID_HANDLE,
  TDLIMIT_INVALID_PARAMETER,

  __error_codes_end
} TDLIMITER_ERROR;

struct TDLimiter;
typedef struct TDLimiter* TDLimiterPtr;

#define PCM_LIM LONG
#define FIXP_DBL2PCM_LIM(x) (x)
#define PCM_LIM2FIXP_DBL(x) (x)
#define PCM_LIM_BITS 32
#define FIXP_PCM_LIM FIXP_DBL

#define SAMPLE_BITS_LIM DFRACT_BITS

/******************************************************************************
 * pcmLimiter_Reset                                                            *
 * limiter: limiter handle                                                     *
 * returns: error code                                                         *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Reset(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_Destroy                                                          *
 * limiter: limiter handle                                                     *
 * returns: error code                                                         *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Destroy(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_GetDelay                                                         *
 * limiter: limiter handle                                                     *
 * returns: exact delay caused by the limiter in samples per channel           *
 ******************************************************************************/
unsigned int pcmLimiter_GetDelay(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_GetMaxGainReduction                                              *
 * limiter: limiter handle                                                     *
 * returns: maximum gain reduction in last processed block in dB               *
 ******************************************************************************/
INT pcmLimiter_GetMaxGainReduction(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_SetNChannels                                                     *
 * limiter:   limiter handle                                                   *
 * nChannels: number of channels ( <= maxChannels specified on create)         *
 * returns:   error code                                                       *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetNChannels(TDLimiterPtr limiter, unsigned int nChannels);

/******************************************************************************
 * pcmLimiter_SetSampleRate                                                    *
 * limiter:    limiter handle                                                  *
 * sampleRate: sampling rate in Hz ( <= maxSampleRate specified on create)     *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetSampleRate(TDLimiterPtr limiter, UINT sampleRate);

/******************************************************************************
 * pcmLimiter_SetAttack                                                        *
 * limiter:    limiter handle                                                  *
 * attackMs:   attack time in ms ( <= maxAttackMs specified on create)         *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetAttack(TDLimiterPtr limiter, unsigned int attackMs);

/******************************************************************************
 * pcmLimiter_SetRelease                                                       *
 * limiter:    limiter handle                                                  *
 * releaseMs:  release time in ms                                              *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetRelease(TDLimiterPtr limiter, unsigned int releaseMs);

/******************************************************************************
 * pcmLimiter_Create                                                           *
 * maxAttackMs:   maximum and initial attack/lookahead time in milliseconds    *
 * releaseMs:     release time in milliseconds (90% time constant)             *
 * threshold:     limiting threshold                                           *
 * maxChannels:   maximum and initial number of channels                       *
 * maxSampleRate: maximum and initial sampling rate in Hz                      *
 * returns:       limiter handle                                               *
 ******************************************************************************/
TDLimiterPtr pcmLimiter_Create(unsigned int maxAttackMs, unsigned int releaseMs, FIXP_DBL threshold,
                               unsigned int maxChannels, UINT maxSampleRate);

/******************************************************************************
 * pcmLimiter_SetThreshold                                                     *
 * limiter:    limiter handle                                                  *
 * threshold:  limiter threshold                                               *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetThreshold(TDLimiterPtr limiter, FIXP_DBL threshold);

/******************************************************************************
 * pcmLimiter_Apply                                                            *
 * limiter:        limiter handle                                              *
 * samplesIn:      pointer to input buffer containing interleaved samples      *
 * samplesOut:     pointer to output buffer containing interleaved samples     *
 *                 in-place processing is not supported                        *
 * workBuf:        pointer to work buffer of size workBuf[nSamples]            *
 *                 if sizeof(PCM_LIM) == sizeof(INT_PCM), then                 *
 *                 workBuf may be samplesOut + (nChannels-1)*nSamples          *
 * pGainPerSample: pointer to gains for each sample, or NULL if not needed     *
 * scaling:        scaling of output samples                                   *
 * nSamples:       number of samples per channel                               *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Apply(TDLimiterPtr limiter, PCM_LIM* samplesIn, INT_PCM* samplesOut,
                                 PCM_LIM* workBuf, FIXP_DBL* pGainPerSample, const INT scaling,
                                 const UINT nSamples);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef LIMITER_H */
