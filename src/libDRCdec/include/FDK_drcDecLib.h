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

   Author(s):   Bernhard Neugebauer

   Description: MPEG-D DRC Decoder

*******************************************************************************/

#ifndef FDK_DRCDECLIB_H
#define FDK_DRCDECLIB_H

#include "FDK_bitstream.h"
#include "FDK_audio.h"
#include "common_fix.h"

/* DRC decoder according to ISO/IEC 23003-4 (MPEG-D DRC) */
/* including ISO/IEC 23003-4/AMD1 (Amendment 1) */
/* and including section 6 of ISO/IEC 23008-3 (MPEG-H 3D Audio) */

#ifdef __cplusplus
extern "C" {
#endif

#define DRC_DEC_LOUDNESS_NOT_PRESENT (LONG)0x7FFFFFFE

typedef struct s_drc_decoder* HANDLE_DRC_DECODER;
typedef struct s_uni_drc_interface* HANDLE_UNI_DRC_INTERFACE;
typedef struct s_selection_process_output* HANDLE_SEL_PROC_OUTPUT;

typedef enum {
  DRC_DEC_SELECTION = 0x1, /* DRC decoder instance for DRC set selection only */
  DRC_DEC_GAIN = 0x2,      /* DRC decoder instance for applying DRC only */
  DRC_DEC_ALL = 0x3        /* DRC decoder with full functionality */
} DRC_DEC_FUNCTIONAL_RANGE;

typedef enum {
  /* get and set userparams */
  DRC_DEC_BOOST,
  DRC_DEC_COMPRESS,
  /* set only userparams */
  DRC_DEC_LOUDNESS_NORMALIZATION_ON,
  DRC_DEC_TARGET_LOUDNESS, /**< target loudness in dB, with exponent e = 7 */
  DRC_DEC_EFFECT_TYPE,
  DRC_DEC_EFFECT_TYPE_FALLBACK_CODE,
  DRC_DEC_LOUDNESS_MEASUREMENT_METHOD,
  DRC_DEC_ALBUM_MODE,
  /* set only system (not user) parameters */
  DRC_DEC_DOWNMIX_ID,
  DRC_DEC_TARGET_LAYOUT_REQUESTED, /**< CICP target layout notified to FDK_drcDecLib for choosing an
                                      appropriate downmixInstruction */
  DRC_DEC_TARGET_CHANNEL_COUNT_REQUESTED, /**< number of output channels notified to FDK_drcDecLib
                                             for choosing an appropriate downmixInstruction */
  DRC_DEC_BASE_CHANNEL_COUNT,
  DRC_DEC_FRAME_SIZE,
  DRC_DEC_SAMPLE_RATE,
  /* get only system parameters */
  DRC_DEC_IS_MULTIBAND_DRC_1,
  DRC_DEC_IS_MULTIBAND_DRC_2,
  DRC_DEC_IS_ACTIVE,              /**< MPEG-D DRC payload is present */
  DRC_DEC_TARGET_LAYOUT_SELECTED, /**< selected CICP target layout if appropriate downmixInstruction
                                     exists, or -1 otherwise */
  DRC_DEC_TARGET_CHANNEL_COUNT_SELECTED, /**< number of output channels if appropriate
                                            downmixInstruction exists */
  DRC_DEC_OUTPUT_LOUDNESS,               /**< output loudness in dB, with exponent e = 7, or
                                            DRC_DEC_LOUDNESS_NOT_PRESENT if no loudness is contained in the
                                            bitstream */
  DRC_DEC_IS_MUTE /**< audio output is mute, because no loudnessInfoSet is present yet in uniDrc()
                     payload. Only for codec mode DRC_DEC_MPEG_4_AAC */
} DRC_DEC_USERPARAM;

typedef enum {
  DRC_DEC_OK = 0,

  DRC_DEC_NOT_OK = -10000,
  DRC_DEC_OUT_OF_MEMORY,
  DRC_DEC_NOT_OPENED,
  DRC_DEC_NOT_READY,
  DRC_DEC_PARAM_OUT_OF_RANGE,
  DRC_DEC_INVALID_PARAM,
  DRC_DEC_UNSUPPORTED_FUNCTION,
  DRC_DEC_PARSE_ERROR
} DRC_DEC_ERROR;

typedef enum {
  DRC_DEC_TEST_TIME_DOMAIN = -100,
  DRC_DEC_TEST_QMF_DOMAIN,
  DRC_DEC_TEST_STFT_DOMAIN,
  DRC_DEC_CODEC_MODE_UNDEFINED = -1,
  DRC_DEC_MPEG_4_AAC,
  DRC_DEC_MPEG_D_USAC,
  DRC_DEC_MPEG_H_3DA
} DRC_DEC_CODEC_MODE;

/* Apply only DRC sets dedicated to processing location.
   DRC1: before downmix
   DRC2/DRC3: after downmix */
typedef enum { DRC_DEC_DRC1, DRC_DEC_DRC2_DRC3 } DRC_DEC_LOCATION;

DRC_DEC_ERROR
FDK_drcDec_Open(HANDLE_DRC_DECODER* phDrcDec, const DRC_DEC_FUNCTIONAL_RANGE functionalRange);

DRC_DEC_ERROR
FDK_drcDec_SetCodecMode(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_CODEC_MODE codecMode);

DRC_DEC_ERROR
FDK_drcDec_Init(HANDLE_DRC_DECODER hDrcDec, const int frameSize, const int sampleRate,
                const int baseChannelCount);

DRC_DEC_ERROR
FDK_drcDec_Close(HANDLE_DRC_DECODER* phDrcDec);

/* set single user request */
DRC_DEC_ERROR
FDK_drcDec_SetParam(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_USERPARAM requestType,
                    const FIXP_DBL requestValue);

LONG FDK_drcDec_GetParam(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_USERPARAM requestType);

DRC_DEC_ERROR
FDK_drcDec_SetInterfaceParameters(HANDLE_DRC_DECODER hDrcDec,
                                  HANDLE_UNI_DRC_INTERFACE uniDrcInterface);

DRC_DEC_ERROR
FDK_drcDec_SetSelectionProcessMpeghParameters(
    HANDLE_DRC_DECODER hDrcDec, const int numGroupIdsRequested, const int* groupIdsRequested,
    const int numGroupPresetIdsRequested, const int* groupPresetIdsRequested,
    const int* numMembersGroupPresetIdsRequested, const int groupPresetIdRequestedPreference);

DRC_DEC_ERROR
FDK_drcDec_SetSelectionProcessMpeghParameters_simple(HANDLE_DRC_DECODER hDrcDec,
                                                     const int groupPresetIdRequested,
                                                     const int numGroupIdsRequested,
                                                     const int* groupIdsRequested);

DRC_DEC_ERROR
FDK_drcDec_SetDownmixInstructions(HANDLE_DRC_DECODER hDrcDec, const int numDowmixId,
                                  const int* downmixId, const int* targetLayout,
                                  const int* targetChannelCount);

void FDK_drcDec_SetSelectionProcessOutput(HANDLE_DRC_DECODER hDrcDec,
                                          HANDLE_SEL_PROC_OUTPUT hSelProcOutput);

HANDLE_SEL_PROC_OUTPUT
FDK_drcDec_GetSelectionProcessOutput(HANDLE_DRC_DECODER hDrcDec);

LONG /* FIXP_DBL, e = 7 */
FDK_drcDec_GetGroupLoudness(HANDLE_SEL_PROC_OUTPUT hSelProcOutput, const int groupID,
                            int* groupLoudnessAvailable);

void FDK_drcDec_SetChannelGains(HANDLE_DRC_DECODER hDrcDec, const int updateLNgain,
                                const int numChannels, const int frameSize, FIXP_DBL* channelGainDb,
                                FIXP_DBL* audioBuffer, const int audioBufferChannelOffset);

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcConfig(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                            const INT subStreamIndex);

DRC_DEC_ERROR
FDK_drcDec_ReadLoudnessInfoSet(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                               const INT subStreamIndex);

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcGain(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                          const INT subStreamIndex);

/* either call FDK_drcDec_ReadUniDrcConfig, FDK_drcDec_ReadLoudnessInfoSet and
   FDK_drcDec_ReadUniDrcGain separately, or call FDK_drcDec_ReadUniDrc */
DRC_DEC_ERROR
FDK_drcDec_ReadUniDrc(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream);

/* calling sequence:
   FDK_drcDec_Read...()
   FDK_drcDec_SetChannelGains()
   FDK_drcDec_Preprocess()
   FDK_drcDec_Process...() */

DRC_DEC_ERROR
FDK_drcDec_Preprocess(HANDLE_DRC_DECODER hDrcDec);

DRC_DEC_ERROR
FDK_drcDec_ProcessTime(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation, const int channelOffset,
                       const int drcChannelOffset, const int numChannelsProcessed,
                       FIXP_DBL* realBuffer, const int timeDataChannelOffset);

DRC_DEC_ERROR
FDK_drcDec_ProcessFreq(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation, const int channelOffset,
                       const int drcChannelOffset, const int numChannelsProcessed,
                       const int processSingleTimeslot, FIXP_DBL** realBuffer,
                       FIXP_DBL** imagBuffer);

DRC_DEC_ERROR
FDK_drcDec_ApplyDownmix(HANDLE_DRC_DECODER hDrcDec, int* reverseInChannelMap,
                        int* reverseOutChannelMap, FIXP_DBL* realBuffer, int* pNChannels);

#ifdef __cplusplus
}
#endif
#endif
