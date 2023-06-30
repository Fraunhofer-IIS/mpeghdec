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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#ifndef DRCDEC_GAINDECODER_H
#define DRCDEC_GAINDECODER_H

#include "drcDecoder.h"
#include "drcGainDec_multiband.h"

/* Definitions common to gainDecoder submodule */

#define NUM_LNB_FRAMES                                                         \
  5 /* previous frame + this frame + one frame for DM_REGULAR_DELAY + (maximum \
       delaySamples)/frameSize */

/* QMF64 */
#define SUBBAND_NUM_BANDS_QMF64 64

#define SUBBAND_ANALYSIS_DELAY_QMF64 320

/* QMF71 (according to ISO/IEC 23003-1:2007) */
#define SUBBAND_NUM_BANDS_QMF71 71

#define SUBBAND_ANALYSIS_DELAY_QMF71 (320 + 384)

/* STFT256 (according to ISO/IEC 23008-3:2015/AMD3) */
#define SUBBAND_NUM_BANDS_STFT256 256

#define SUBBAND_ANALYSIS_DELAY_STFT256 256

typedef enum { GAIN_DEC_DRC1, GAIN_DEC_DRC2_DRC3 } GAIN_DEC_LOCATION;

typedef enum { GAIN_DEC_FRAME_SIZE, GAIN_DEC_SAMPLE_RATE } GAIN_DEC_PARAM;

typedef struct {
  FIXP_DBL gainLin; /* e = 7 */
  SHORT time;
} NODE_LIN;

typedef struct {
  GAIN_INTERPOLATION_TYPE gainInterpolationType;
  int nNodes[NUM_LNB_FRAMES]; /* number of nodes, saturated to 32 */
  NODE_LIN linearNode[NUM_LNB_FRAMES][32];
} LINEAR_NODE_BUFFER;

typedef struct {
  int lnbPointer;
  LINEAR_NODE_BUFFER linearNodeBuffer[48];
  LINEAR_NODE_BUFFER dummyLnb;
} DRC_GAIN_BUFFERS;

typedef struct {
  int activeDrcOffset;
  DRC_INSTRUCTIONS_UNI_DRC drcInst;

  DUCKING_MODIFICATION duckingModificationForChannelGroup[28];
  SCHAR channelGroupForChannel[2 * 28];

  UCHAR bandCountForChannelGroup[28];
  UCHAR gainElementForGroup[28];
  UCHAR channelGroupIsParametricDrc[28];
  UCHAR gainElementCount; /* number of different DRC gains including all DRC bands */
  int lnbIndexForChannel[28][NUM_LNB_FRAMES];
  int subbandGainsReady;
} ACTIVE_DRC;

typedef struct {
  int deltaTminDefault;
  INT frameSize;
  INT sampleRate;
  INT msPerFrame;
  FIXP_DBL loudnessNormalisationGainDb;
  DELAY_MODE delayMode;

  DRC_COEFFICIENTS_UNI_DRC drcCoef;
  int nActiveDrcs[ACTIVE_DRC_LOCATIONS];
  ACTIVE_DRC activeDrc[ACTIVE_DRC_LOCATIONS][MAX_ACTIVE_DRCS];
  int multiBandActiveDrcIndex;   /* relates to location 0: before downmix */
  int channelGainActiveDrcIndex; /* relates to location 0: before downmix */
  FIXP_DBL channelGain[28];      /* e = 8 */
  FIXP_DBL channelGainPrev[28];  /* e = 8 */

  DRC_GAIN_BUFFERS drcGainBuffers;
  OVERLAP_PARAMETERS overlap[28][4];
  FIXP_DBL subbandGains[48][4 * (1024 / 256)];
  FIXP_DBL dummySubbandGains[4 * (1024 / 256)];

  int status;
  int timeDomainSupported;
  int startupMs; /* reverse counter active for time segment 2.5 seconds after startup */
  SUBBAND_DOMAIN_MODE subbandDomainSupported;
} DRC_GAIN_DECODER, *HANDLE_DRC_GAIN_DECODER;

/* init functions */
DRC_ERROR
drcDec_GainDecoder_Open(HANDLE_DRC_GAIN_DECODER* phGainDec);

DRC_ERROR
drcDec_GainDecoder_Init(HANDLE_DRC_GAIN_DECODER hGainDec);

DRC_ERROR
drcDec_GainDecoder_SetParam(HANDLE_DRC_GAIN_DECODER hGainDec, const GAIN_DEC_PARAM paramType,
                            const int paramValue);

DRC_ERROR
drcDec_GainDecoder_SetCodecDependentParameters(HANDLE_DRC_GAIN_DECODER hGainDec,
                                               const DELAY_MODE delayMode,
                                               const int timeDomainSupported,
                                               const SUBBAND_DOMAIN_MODE subbandDomainSupported);

DRC_ERROR
drcDec_GainDecoder_Config(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                          const UCHAR numSelectedDrcSets, const SCHAR* selectedDrcSetIds,
                          const UCHAR* selectedDownmixIds);

/* close functions */
DRC_ERROR
drcDec_GainDecoder_Close(HANDLE_DRC_GAIN_DECODER* phGainDec);

/* process functions */

/* call drcDec_GainDecoder_Preprocess first */
DRC_ERROR
drcDec_GainDecoder_Preprocess(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_GAIN hUniDrcGain,
                              const FIXP_DBL loudnessNormalizationGainDb, const FIXP_SGL boost,
                              const FIXP_SGL compress);

/* Then call one of drcDec_GainDecoder_ProcessTimeDomain or drcDec_GainDecoder_ProcessSubbandDomain
 */
DRC_ERROR
drcDec_GainDecoder_ProcessTimeDomain(HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples,
                                     const GAIN_DEC_LOCATION drcLocation, const int channelOffset,
                                     const int drcChannelOffset, const int numChannelsProcessed,
                                     const int timeDataChannelOffset, FIXP_DBL* audioIOBuffer);

DRC_ERROR
drcDec_GainDecoder_ProcessSubbandDomain(HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples,
                                        GAIN_DEC_LOCATION drcLocation, const int channelOffset,
                                        const int drcChannelOffset, const int numChannelsProcessed,
                                        const int processSingleTimeslot,
                                        FIXP_DBL* audioIOBufferReal[],
                                        FIXP_DBL* audioIOBufferImag[]);

DRC_ERROR
drcDec_GainDecoder_Conceal(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                           HANDLE_UNI_DRC_GAIN hUniDrcGain);

DRC_ERROR
drcDec_GainDecoder_SetLoudnessNormalizationGainDb(HANDLE_DRC_GAIN_DECODER hGainDec,
                                                  FIXP_DBL loudnessNormalizationGainDb);

int drcDec_GainDecoder_GetFrameSize(HANDLE_DRC_GAIN_DECODER hGainDec);

int drcDec_GainDecoder_GetDeltaTminDefault(HANDLE_DRC_GAIN_DECODER hGainDec);

int drcDec_GainDecoder_GetStartupPhase(HANDLE_DRC_GAIN_DECODER hGainDec);

DRC_COEFFICIENTS_UNI_DRC* drcDec_GainDecoder_GetDrcCoefficients(HANDLE_DRC_GAIN_DECODER hGainDec);

void drcDec_GainDecoder_SetChannelGains(HANDLE_DRC_GAIN_DECODER hGainDec, const int numChannels,
                                        const int frameSize, const FIXP_DBL* channelGainDb,
                                        const int audioBufferChannelOffset, FIXP_DBL* audioBuffer);

#endif
