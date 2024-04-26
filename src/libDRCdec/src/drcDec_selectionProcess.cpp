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

   Author(s):   Andreas Hoelzer

   Description: DRC Set Selection

*******************************************************************************/

#include "drcDec_selectionProcess.h"
#include "drcDec_tools.h"

typedef enum {
  DETR_NONE = 0,
  DETR_NIGHT = 1,
  DETR_NOISY = 2,
  DETR_LIMITED = 3,
  DETR_LOWLEVEL = 4,
  DETR_DIALOG = 5,
  DETR_GENERAL_COMPR = 6,
  DETR_EXPAND = 7,
  DETR_ARTISTIC = 8,
  DETR_COUNT
} DRC_EFFECT_TYPE_REQUEST;

typedef enum {
  DFRT_EFFECT_TYPE,
  DFRT_DYNAMIC_RANGE,
  DFRT_DRC_CHARACTERISTIC
} DRC_FEATURE_REQUEST_TYPE;

typedef enum {
  MDR_DEFAULT = 0,
  MDR_PROGRAM_LOUDNESS = 1,
  MDR_ANCHOR_LOUDNESS = 2
} METHOD_DEFINITION_REQUEST;

typedef enum {
  MSR_DEFAULT = 0,
  MSR_BS_1770_4 = 1,
  MSR_USER = 2,
  MSR_EXPERT_PANEL = 3,
  MSR_RESERVED_A = 4,
  MSR_RESERVED_B = 5,
  MSR_RESERVED_C = 6,
  MSR_RESERVED_D = 7,
  MSR_RESERVED_E = 8
} MEASUREMENT_SYSTEM_REQUEST;

typedef enum { LPR_DEFAULT = 0, LPR_OFF = 1, LPR_HIGHPASS = 2 } LOUDNESS_PREPROCESSING_REQUEST;

typedef enum {
  DRMRT_SHORT_TERM_LOUDNESS_TO_AVG = 0,
  DRMRT_MOMENTARY_LOUDNESS_TO_AVG = 1,
  DRMRT_TOP_OF_LOUDNESS_RANGE_TO_AVG = 2
} DYN_RANGE_MEASUREMENT_REQUEST_TYPE;

typedef enum {
  TCRT_DOWNMIX_ID = 0,
  TCRT_TARGET_LAYOUT = 1,
  TCRT_TARGET_CHANNEL_COUNT = 2
} TARGET_CONFIG_REQUEST_TYPE;

typedef shouldBeUnion {
  struct {
    UCHAR numRequests;
    UCHAR numRequestsDesired;
    DRC_EFFECT_TYPE_REQUEST request[MAX_REQUESTS_DRC_EFFECT_TYPE];
  } drcEffectType;
}
DRC_FEATURE_REQUEST;

typedef struct {
  UCHAR numGroupIdsRequested;
  UCHAR groupIdRequested[MAX_REQUESTS_GROUP_ID];
  UCHAR numGroupPresetIdsRequested;
  UCHAR groupPresetIdRequested[MAX_REQUESTS_GROUP_PRESET_ID];
  UCHAR numMembersGroupPresetIdsRequested[MAX_REQUESTS_GROUP_PRESET_ID];
  SCHAR groupPresetIdRequestedPreference;
} MPEGH_REQUEST;

typedef struct {
  /* system parameters */
  SCHAR baseChannelCount;
  SCHAR baseLayout; /* not supported */
  TARGET_CONFIG_REQUEST_TYPE targetConfigRequestType;
  UCHAR numDownmixIdRequests;
  UCHAR downmixIdRequested[MAX_REQUESTS_DOWNMIX_ID];
  UCHAR targetLayoutRequested;
  UCHAR targetChannelCountRequested;
  LONG audioSampleRate; /* needed for complexity estimation, currently not supported */

  /* loudness normalization parameters */
  UCHAR loudnessNormalizationOn;
  FIXP_DBL targetLoudness; /* e = 7 */
  UCHAR albumMode;
  UCHAR peakLimiterPresent;
  UCHAR loudnessDeviationMax;                       /* resolution: 1 dB */
  FIXP_DBL loudnessNormalizationGainDbMax;          /* e = 7 */
  FIXP_DBL loudnessNormalizationGainModificationDb; /* e = 7 */
  FIXP_DBL outputPeakLevelMax;                      /* e = 7 */

  /* dynamic range control parameters */
  UCHAR dynamicRangeControlOn;
  UCHAR numDrcFeatureRequests;
  DRC_FEATURE_REQUEST_TYPE drcFeatureRequestType[MAX_REQUESTS_DRC_FEATURE];
  DRC_FEATURE_REQUEST drcFeatureRequest[MAX_REQUESTS_DRC_FEATURE];

  /* MPEG-H parameters */
  MPEGH_REQUEST mpeghRequest;

  /* other */
  FIXP_SGL boost;                /* e = 1 */
  FIXP_SGL compress;             /* e = 1 */
  UCHAR drcCharacteristicTarget; /* not supported */
} SEL_PROC_INPUT, *HANDLE_SEL_PROC_INPUT;

/* Table E.1 of ISO/IEC DIS 23003-4: Recommended order of fallback effect type requests */
static const DRC_EFFECT_TYPE_REQUEST fallbackEffectTypeRequests[6][5] = {
    /* Night */ {DETR_GENERAL_COMPR, DETR_NOISY, DETR_LIMITED, DETR_LOWLEVEL, DETR_DIALOG},
    /* Noisy */ {DETR_GENERAL_COMPR, DETR_NIGHT, DETR_LIMITED, DETR_LOWLEVEL, DETR_DIALOG},
    /* Limited */ {DETR_GENERAL_COMPR, DETR_NIGHT, DETR_NOISY, DETR_LOWLEVEL, DETR_DIALOG},
    /* LowLevel */ {DETR_GENERAL_COMPR, DETR_NOISY, DETR_NIGHT, DETR_LIMITED, DETR_DIALOG},
    /* Dialog */ {DETR_GENERAL_COMPR, DETR_NIGHT, DETR_NOISY, DETR_LIMITED, DETR_LOWLEVEL},
    /* General */ {DETR_NIGHT, DETR_NOISY, DETR_LIMITED, DETR_LOWLEVEL, DETR_DIALOG}};

/*******************************************/
typedef struct {
  UCHAR selectionFlag;
  UCHAR downmixIdRequestIndex;
  FIXP_DBL outputPeakLevel;                     /* e = 7 */
  FIXP_DBL loudnessNormalizationGainDbAdjusted; /* e = 7 */
  FIXP_DBL outputLoudness;                      /* e = 7 */
  DRC_INSTRUCTIONS_UNI_DRC* pInst;

} DRCDEC_SELECTION_DATA;

typedef struct {
  UCHAR numData;
  DRCDEC_SELECTION_DATA data[(32 + 1)];

} DRCDEC_SELECTION;

/*******************************************/
/* helper functions                        */
/*******************************************/

static int _isError(int x) {
  if (x < DRCDEC_SELECTION_PROCESS_WARNING) {
    return 1;
  }

  return 0;
}

/* compare and assign */
static inline int _compAssign(UCHAR* dest, const UCHAR src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int _compAssign(SCHAR* dest, const SCHAR src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int _compAssign(FIXP_DBL* dest, const FIXP_DBL src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int _compAssign(FIXP_SGL* dest, const FIXP_SGL src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int _compAssign(TARGET_CONFIG_REQUEST_TYPE* dest, const int src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = (TARGET_CONFIG_REQUEST_TYPE)src;
  return diff;
}

static inline int _compAssign(DRC_FEATURE_REQUEST_TYPE* dest, const int src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = (DRC_FEATURE_REQUEST_TYPE)src;
  return diff;
}

static inline int _compAssign(DRC_EFFECT_TYPE_REQUEST* dest, const int src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = (DRC_EFFECT_TYPE_REQUEST)src;
  return diff;
}

static DRCDEC_SELECTION_DATA* _drcdec_selection_addNew(DRCDEC_SELECTION* pSelection);

static DRCDEC_SELECTION_DATA* _drcdec_selection_add(DRCDEC_SELECTION* pSelection,
                                                    DRCDEC_SELECTION_DATA* pDataIn);

static int _drcdec_selection_clear(DRCDEC_SELECTION* pSelection);

static int _drcdec_selection_getNumber(DRCDEC_SELECTION* pSelection);

static int _drcdec_selection_setNumber(DRCDEC_SELECTION* pSelection, int num);

static DRCDEC_SELECTION_DATA* _drcdec_selection_getAt(DRCDEC_SELECTION* pSelection, int at);

static int _swapSelectionAndClear(DRCDEC_SELECTION** ppCandidatesPotential,
                                  DRCDEC_SELECTION** ppCandidatesSelected);

static int _swapSelection(DRCDEC_SELECTION** ppCandidatesPotential,
                          DRCDEC_SELECTION** ppCandidatesSelected);

/*******************************************/
/* declarations of static functions        */
/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _initDefaultParams(HANDLE_SEL_PROC_INPUT hSelProcInput);

static DRCDEC_SELECTION_PROCESS_RETURN _initCodecModeParams(HANDLE_SEL_PROC_INPUT hSelProcInput,
                                                            const SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetPreSelection(
    SEL_PROC_INPUT* hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected, SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_peakValue0(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected);

static DRCDEC_SELECTION_PROCESS_RETURN _channelLayoutToDownmixIdMapping(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig);

static DRCDEC_SELECTION_PROCESS_RETURN _generateVirtualDrcSets(HANDLE_SEL_PROC_INPUT hSelProcInput,
                                                               HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                               SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetRequestSelection(
    SEL_PROC_INPUT* hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected);

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection(
    HANDLE_SEL_PROC_INPUT hSelProcInput, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected, SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _generateOutputInfo(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_SEL_PROC_OUTPUT hSelProcOutput,
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
    DRCDEC_SELECTION_DATA* pSelectionData, SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _selectDownmixMatrix(HANDLE_SEL_PROC_OUTPUT hSelProcOutput,
                                                            HANDLE_UNI_DRC_CONFIG hUniDrcConfig);

static DRCDEC_SELECTION_PROCESS_RETURN _getLoudness(
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, int albumMode,
    METHOD_DEFINITION_REQUEST measurementMethodRequested,
    MEASUREMENT_SYSTEM_REQUEST measurementSystemRequested, FIXP_DBL targetLoudness, int drcSetId,
    int downmixIdRequested, SEL_PROC_CODEC_MODE codecMode, int groupIdRequested,
    int groupPresetIdRequested, MPEGH_REQUEST* pMpeghRequest, FIXP_DBL* pLoudnessNormalizationGain,
    FIXP_DBL* pLoudness);

static DRCDEC_SELECTION_PROCESS_RETURN _getMixingLevel(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                       int downmixIdRequested,
                                                       int drcSetIdRequested, int albumMode,
                                                       FIXP_DBL* pMixingLevel);

static DRCDEC_SELECTION_PROCESS_RETURN _getSignalPeakLevel(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRC_INSTRUCTIONS_UNI_DRC* pInst,
    int downmixIdRequested, int* explicitPeakInformationPresent,
    FIXP_DBL* signalPeakLevelOut, /* e = 7 */
    SEL_PROC_CODEC_MODE codecMode);

static DRCDEC_SELECTION_PROCESS_RETURN _selectAlbumLoudness(
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION* pCandidatesPotential,
    DRCDEC_SELECTION* pCandidatesSelected);

static int _findMethodDefinition(LOUDNESS_INFO* pLoudnessInfo, int methodDefinition,
                                 int startIndex);

static DRCDEC_SELECTION_PROCESS_RETURN _getGroupLoudness(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                         HANDLE_SEL_PROC_OUTPUT hSelProcOutput);

/*******************************************/
/* public functions                        */
/*******************************************/

struct s_drcdec_selection_process {
  SEL_PROC_CODEC_MODE codecMode;
  SEL_PROC_INPUT selProcInput;
  DRCDEC_SELECTION selectionData[2]; /* 2 instances, one before and one after selection */
};

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Create(HANDLE_DRC_SELECTION_PROCESS* phInstance) {
  HANDLE_DRC_SELECTION_PROCESS hInstance;
  hInstance = (HANDLE_DRC_SELECTION_PROCESS)FDKcalloc(1, sizeof(struct s_drcdec_selection_process));

  if (!hInstance) return DRCDEC_SELECTION_PROCESS_OUTOFMEMORY;

  hInstance->codecMode = SEL_PROC_CODEC_MODE_UNDEFINED;

  *phInstance = hInstance;
  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Init(HANDLE_DRC_SELECTION_PROCESS hInstance) {
  if (!hInstance) return DRCDEC_SELECTION_PROCESS_NOT_OK;

  _initDefaultParams(&hInstance->selProcInput);
  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetCodecMode(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                     const SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (!hInstance) return DRCDEC_SELECTION_PROCESS_NOT_OK;

  switch (codecMode) {
    case SEL_PROC_MPEG_H_3DA:
    case SEL_PROC_TEST_TIME_DOMAIN:
    case SEL_PROC_TEST_QMF_DOMAIN:
    case SEL_PROC_TEST_STFT_DOMAIN:
      hInstance->codecMode = codecMode;
      break;

    case SEL_PROC_CODEC_MODE_UNDEFINED:
    default:
      return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  retVal = _initCodecModeParams(&(hInstance->selProcInput), hInstance->codecMode = codecMode);

  return retVal;
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetParam(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                 const SEL_PROC_USER_PARAM requestType, FIXP_DBL requestValue,
                                 int* pDiff) {
  INT requestValueInt = (INT)requestValue;
  ULONG requestValueUlong = (ULONG)(LONG)requestValue;
  int i, diff = 0;
  SEL_PROC_INPUT* pSelProcInput = &(hInstance->selProcInput);

  switch (requestType) {
    case SEL_PROC_LOUDNESS_NORMALIZATION_ON:
      if ((requestValueInt != 0) && (requestValueInt != 1))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->loudnessNormalizationOn, requestValueInt);
      break;
    case SEL_PROC_TARGET_LOUDNESS:
      /* Lower boundary: drcSetTargetLoudnessValueLower default value.
         Upper boundary: drcSetTargetLoudnessValueUpper default value */
      if ((requestValue < FL2FXCONST_DBL(-63.0f / (float)(1 << 7))) || (requestValue > (FIXP_DBL)0))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      if (requestValue > FL2FXCONST_DBL(-10.0f / (float)(1 << 7))) /* recommended maximum value */
        requestValue = FL2FXCONST_DBL(-10.0f / (float)(1 << 7));
      diff |= _compAssign(&pSelProcInput->targetLoudness, requestValue);
      break;
    case SEL_PROC_EFFECT_TYPE:
      if ((requestValueInt < -1) || (requestValueInt >= DETR_COUNT))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      /* Caution. This overrides all drcFeatureRequests requested so far! */
      if (requestValueInt == -1) {
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 0);
      } else if (requestValueInt == DETR_NONE) {
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 1);
        diff |= _compAssign(&pSelProcInput->numDrcFeatureRequests, 0);
      } else {
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 1);
        diff |= _compAssign(&pSelProcInput->numDrcFeatureRequests, 1);
        diff |= _compAssign(&pSelProcInput->drcFeatureRequestType[0], DFRT_EFFECT_TYPE);
        diff |=
            _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.numRequestsDesired, 1);
        diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.request[0],
                            requestValueInt);
        if ((requestValueInt > DETR_NONE) && (requestValueInt <= DETR_GENERAL_COMPR)) {
          /* use fallback effect type requests */
          for (i = 0; i < 5; i++) {
            diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.request[i + 1],
                                fallbackEffectTypeRequests[requestValueInt - 1][i]);
          }
          diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.numRequests, 6);
        } else {
          diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.numRequests, 1);
        }
      }
      break;
    case SEL_PROC_EFFECT_TYPE_FALLBACK_CODE:
      /* Caution. This overrides all drcFeatureRequests requested so far! */
      if (requestValueUlong == 0xFFFFFFFF) { /* DRC off */
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 0);
      } else if (requestValueUlong == 0) /* DETR_NONE without fallbacks */
      {
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 1);
        diff |= _compAssign(&pSelProcInput->numDrcFeatureRequests, 0);
      } else {
        UCHAR numRequests = 1;
        for (i = 0; i < 8; i++) {
          ULONG fallbackEffectType = (requestValueUlong >> (4 * i)) & 0xF;
          if (fallbackEffectType >= DETR_COUNT) return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
        }
        diff |= _compAssign(&pSelProcInput->dynamicRangeControlOn, 1);
        diff |= _compAssign(&pSelProcInput->numDrcFeatureRequests, 1);
        diff |= _compAssign(&pSelProcInput->drcFeatureRequestType[0], DFRT_EFFECT_TYPE);
        diff |=
            _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.numRequestsDesired, 1);
        /* selected DRC effect type is located at the 4 LSB */
        diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.request[0],
                            (DRC_EFFECT_TYPE_REQUEST)(requestValueUlong & 0xF));
        /* decode fallback effect type requests */
        for (i = 1; i < 8; i++) {
          /* fallback DRC effect types are sorted from right to left, 4 bits each. */
          ULONG fallbackEffectType = (requestValueUlong >> (4 * i)) & 0xF;
          if (fallbackEffectType == 0) break;
          diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.request[i],
                              (DRC_EFFECT_TYPE_REQUEST)(fallbackEffectType));
          numRequests++;
        }
        diff |= _compAssign(&pSelProcInput->drcFeatureRequest[0].drcEffectType.numRequests,
                            numRequests);
      }
      break;
    case SEL_PROC_ALBUM_MODE:
      if ((requestValueInt < 0) || (requestValueInt > 1))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->albumMode, requestValueInt);
      break;
    case SEL_PROC_DOWNMIX_ID:
      diff |= _compAssign(&pSelProcInput->targetConfigRequestType, TCRT_DOWNMIX_ID);
      if (requestValueInt < 0) { /* negative requests signal no downmixId */
        diff |= _compAssign(&pSelProcInput->numDownmixIdRequests, 0);
      } else {
        diff |= _compAssign(&pSelProcInput->numDownmixIdRequests, 1);
        diff |= _compAssign(&pSelProcInput->downmixIdRequested[0], requestValueInt);
      }
      break;
    case SEL_PROC_TARGET_LAYOUT:
      /* Request target layout according to ChannelConfiguration in ISO/IEC 23001-8 (CICP) */
      if ((requestValueInt < 1) || (requestValueInt > 63))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->targetConfigRequestType, TCRT_TARGET_LAYOUT);
      diff |= _compAssign(&pSelProcInput->targetLayoutRequested, requestValueInt);
      break;
    case SEL_PROC_TARGET_CHANNEL_COUNT:
      if (requestValueInt < 1) return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->targetConfigRequestType, TCRT_TARGET_CHANNEL_COUNT);
      diff |= _compAssign(&pSelProcInput->targetChannelCountRequested, requestValueInt);
      break;
    case SEL_PROC_BASE_CHANNEL_COUNT:
      if (requestValueInt < 0) return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->baseChannelCount, requestValueInt);
      break;
    case SEL_PROC_SAMPLE_RATE:
      if (requestValueInt < 0) return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |= _compAssign(&pSelProcInput->audioSampleRate, requestValueInt);
      break;
    case SEL_PROC_BOOST:
      if ((requestValue < (FIXP_DBL)0) || (requestValue > FL2FXCONST_DBL(1.0f / (float)(1 << 1))))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |=
          _compAssign(&pSelProcInput->boost,
                      FX_DBL2FX_SGL(requestValue +
                                    (FIXP_DBL)(1 << 15))); /* convert to FIXP_SGL with rounding */
      break;
    case SEL_PROC_COMPRESS:
      if ((requestValue < (FIXP_DBL)0) || (requestValue > FL2FXCONST_DBL(1.0f / (float)(1 << 1))))
        return DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE;
      diff |=
          _compAssign(&pSelProcInput->compress,
                      FX_DBL2FX_SGL(requestValue +
                                    (FIXP_DBL)(1 << 15))); /* convert to FIXP_SGL with rounding */
      break;
    default:
      return DRCDEC_SELECTION_PROCESS_INVALID_PARAM;
  }

  if (pDiff != NULL) {
    *pDiff |= diff;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

FIXP_DBL
drcDec_SelectionProcess_GetParam(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                 const SEL_PROC_USER_PARAM requestType) {
  SEL_PROC_INPUT* pSelProcInput = &(hInstance->selProcInput);

  switch (requestType) {
    case SEL_PROC_LOUDNESS_NORMALIZATION_ON:
      return (FIXP_DBL)pSelProcInput->loudnessNormalizationOn;
    case SEL_PROC_DYNAMIC_RANGE_CONTROL_ON:
      return (FIXP_DBL)pSelProcInput->dynamicRangeControlOn;
    default:
      return (FIXP_DBL)0;
  }
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetMpeghParams(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                       const int numGroupIdsRequested, const int* groupIdRequested,
                                       const int numGroupPresetIdsRequested,
                                       const int* groupPresetIdRequested,
                                       const int* numMembersGroupPresetIdsRequested,
                                       const int groupPresetIdRequestedPreference, int* pDiff) {
  int i;
  MPEGH_REQUEST* pMpeghRequest = &hInstance->selProcInput.mpeghRequest;
  int diff = 0;

  /* groupId request */
  diff |= _compAssign(&pMpeghRequest->numGroupIdsRequested, numGroupIdsRequested);
  if (numGroupIdsRequested > MAX_REQUESTS_GROUP_ID) return DRCDEC_SELECTION_PROCESS_NOT_OK;
  for (i = 0; i < pMpeghRequest->numGroupIdsRequested; i++) {
    diff |= _compAssign(&pMpeghRequest->groupIdRequested[i], groupIdRequested[i]);
  }

  /* groupPresetId request */
  diff |= _compAssign(&pMpeghRequest->numGroupPresetIdsRequested, numGroupPresetIdsRequested);
  if (numGroupPresetIdsRequested > MAX_REQUESTS_GROUP_PRESET_ID)
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  for (i = 0; i < pMpeghRequest->numGroupPresetIdsRequested; i++) {
    diff |= _compAssign(&pMpeghRequest->groupPresetIdRequested[i], groupPresetIdRequested[i]);
    diff |= _compAssign(&pMpeghRequest->numMembersGroupPresetIdsRequested[i],
                        numMembersGroupPresetIdsRequested[i]);
  }
  diff |= _compAssign(&pMpeghRequest->groupPresetIdRequestedPreference,
                      groupPresetIdRequestedPreference);

  if (pDiff != NULL) {
    *pDiff |= diff;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Delete(HANDLE_DRC_SELECTION_PROCESS* phInstance) {
  if (phInstance == NULL || *phInstance == NULL) return DRCDEC_SELECTION_PROCESS_INVALID_HANDLE;

  FDKfree(*phInstance);
  *phInstance = NULL;
  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Process(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                HANDLE_SEL_PROC_OUTPUT hSelProcOutput) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  DRCDEC_SELECTION* pCandidatesSelected;
  DRCDEC_SELECTION* pCandidatesPotential;

  if (hInstance == NULL) return DRCDEC_SELECTION_PROCESS_INVALID_HANDLE;

  pCandidatesSelected = &(hInstance->selectionData[0]);
  pCandidatesPotential = &(hInstance->selectionData[1]);
  _drcdec_selection_setNumber(pCandidatesSelected, 0);
  _drcdec_selection_setNumber(pCandidatesPotential, 0);

  retVal = _generateVirtualDrcSets(&(hInstance->selProcInput), hUniDrcConfig, hInstance->codecMode);
  if (retVal) return (retVal);

  if (hInstance->selProcInput.baseChannelCount != hUniDrcConfig->channelLayout.baseChannelCount) {
    hInstance->selProcInput.baseChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  }

  if ((hInstance->selProcInput.targetConfigRequestType != 0) ||
      (hInstance->selProcInput.targetConfigRequestType == 0 &&
       hInstance->selProcInput.numDownmixIdRequests == 0)) {
    retVal = _channelLayoutToDownmixIdMapping(&(hInstance->selProcInput), hUniDrcConfig);

    if (_isError(retVal)) return (retVal);
  }

  retVal = _drcSetPreSelection(&(hInstance->selProcInput), hUniDrcConfig, hLoudnessInfoSet,
                               &pCandidatesPotential, &pCandidatesSelected, hInstance->codecMode);
  if (retVal) return (retVal);

  if (hInstance->selProcInput.albumMode) {
    _swapSelectionAndClear(&pCandidatesPotential, &pCandidatesSelected);

    retVal = _selectAlbumLoudness(hLoudnessInfoSet, pCandidatesPotential, pCandidatesSelected);
    if (retVal) return (retVal);

    if (_drcdec_selection_getNumber(pCandidatesSelected) == 0) {
      _swapSelection(&pCandidatesPotential, &pCandidatesSelected);
    }
  }

  _swapSelectionAndClear(&pCandidatesPotential, &pCandidatesSelected);

  retVal = _drcSetRequestSelection(&(hInstance->selProcInput), hUniDrcConfig, hLoudnessInfoSet,
                                   &pCandidatesPotential, &pCandidatesSelected);
  if (retVal) return (retVal);

  retVal = _drcSetFinalSelection(&(hInstance->selProcInput), &pCandidatesPotential,
                                 &pCandidatesSelected, hInstance->codecMode);
  if (retVal) return (retVal);

  retVal =
      _generateOutputInfo(&(hInstance->selProcInput), hSelProcOutput, hUniDrcConfig,
                          hLoudnessInfoSet, &(pCandidatesSelected->data[0]), hInstance->codecMode);
  if (_isError(retVal)) return (retVal);

  retVal = _selectDownmixMatrix(hSelProcOutput, hUniDrcConfig);
  if (retVal) return (retVal);

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/
/* static functions                        */
/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _initDefaultParams(HANDLE_SEL_PROC_INPUT hSelProcInput) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hSelProcInput == NULL) return DRCDEC_SELECTION_PROCESS_INVALID_HANDLE;

  /* system parameters */
  hSelProcInput->baseChannelCount = -1;
  hSelProcInput->baseLayout = -1;
  hSelProcInput->targetConfigRequestType = TCRT_DOWNMIX_ID;
  hSelProcInput->numDownmixIdRequests = 0;

  /* loudness normalization parameters */
  hSelProcInput->albumMode = 0;
  hSelProcInput->peakLimiterPresent = 0;
  hSelProcInput->loudnessNormalizationOn = 1;
  hSelProcInput->targetLoudness = FL2FXCONST_DBL(-24.0f / (float)(1 << 7));
  hSelProcInput->loudnessDeviationMax = DEFAULT_LOUDNESS_DEVIATION_MAX;
  hSelProcInput->loudnessNormalizationGainDbMax = (FIXP_DBL)MAXVAL_DBL; /* infinity as default */
  hSelProcInput->loudnessNormalizationGainModificationDb = (FIXP_DBL)0;
  hSelProcInput->outputPeakLevelMax = (FIXP_DBL)0;
  if (hSelProcInput->peakLimiterPresent == 1) {
    hSelProcInput->outputPeakLevelMax = FL2FXCONST_DBL(6.0f / (float)(1 << 7));
  }

  /* dynamic range control parameters */
  hSelProcInput->dynamicRangeControlOn = 1;

  hSelProcInput->numDrcFeatureRequests = 0;

  /* other parameters */
  hSelProcInput->boost = FL2FXCONST_SGL(1.f / (float)(1 << 1));
  hSelProcInput->compress = FL2FXCONST_SGL(1.f / (float)(1 << 1));
  hSelProcInput->drcCharacteristicTarget = 0;

  /* MPEG-H parameters */
  hSelProcInput->mpeghRequest.numGroupIdsRequested = 0;
  hSelProcInput->mpeghRequest.numGroupPresetIdsRequested = 0;

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _initCodecModeParams(HANDLE_SEL_PROC_INPUT hSelProcInput,
                                                            const SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hSelProcInput == NULL) return DRCDEC_SELECTION_PROCESS_INVALID_HANDLE;

  switch (codecMode) {
    case SEL_PROC_MPEG_H_3DA:
      hSelProcInput->loudnessDeviationMax = 0;
      hSelProcInput->peakLimiterPresent = 1; /* peak limiter is mandatory */
      /* The peak limiter also has to catch overshoots due to user interactivity, downmixing etc.
      Therefore the maximum output peak level is reduced to 0 dB. */
      hSelProcInput->outputPeakLevelMax = (FIXP_DBL)0;
      break;
    case SEL_PROC_MPEG_4_AAC:
    case SEL_PROC_MPEG_D_USAC:
      hSelProcInput->loudnessDeviationMax = DEFAULT_LOUDNESS_DEVIATION_MAX;
      hSelProcInput->peakLimiterPresent = 1;
      /* A peak limiter is present at the end of the decoder, therefore we can allow for a maximum
       * output peak level greater than full scale
       */
      hSelProcInput->outputPeakLevelMax = FL2FXCONST_DBL(6.0f / (float)(1 << 7));
      break;
    case SEL_PROC_TEST_TIME_DOMAIN:
    case SEL_PROC_TEST_QMF_DOMAIN:
    case SEL_PROC_TEST_STFT_DOMAIN:
      /* for testing, adapt to default settings in reference software */
      hSelProcInput->loudnessNormalizationOn = 0;
      hSelProcInput->dynamicRangeControlOn = 0;
      break;
    case SEL_PROC_CODEC_MODE_UNDEFINED:
    default:
      hSelProcInput->loudnessDeviationMax = DEFAULT_LOUDNESS_DEVIATION_MAX;
      hSelProcInput->peakLimiterPresent = 0;
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _channelLayoutToDownmixIdMapping(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  DOWNMIX_INSTRUCTIONS* pDown = NULL;

  int i;

  hSelProcInput->numDownmixIdRequests = 0;

  switch (hSelProcInput->targetConfigRequestType) {
    case TCRT_DOWNMIX_ID: {
      hSelProcInput->downmixIdRequested[0] = 0;
      hSelProcInput->numDownmixIdRequests = 1;
    }

    break;

    case TCRT_TARGET_LAYOUT:
      if (hSelProcInput->targetLayoutRequested == hSelProcInput->baseLayout) {
        hSelProcInput->downmixIdRequested[0] = 0;
        hSelProcInput->numDownmixIdRequests = 1;
      }

      if (hSelProcInput->numDownmixIdRequests == 0) {
        for (i = 0; i < hUniDrcConfig->downmixInstructionsCount; i++) {
          pDown = &(hUniDrcConfig->downmixInstructions[i]);

          if (hSelProcInput->targetLayoutRequested == pDown->targetLayout) {
            hSelProcInput->downmixIdRequested[hSelProcInput->numDownmixIdRequests] =
                pDown->downmixId;
            hSelProcInput->numDownmixIdRequests++;
          }
        }
      }

      if (hSelProcInput->baseLayout == -1) {
        retVal = DRCDEC_SELECTION_PROCESS_WARNING;
      }

      if (hSelProcInput->numDownmixIdRequests == 0) {
        hSelProcInput->downmixIdRequested[0] = 0;
        hSelProcInput->numDownmixIdRequests = 1;
        retVal = DRCDEC_SELECTION_PROCESS_WARNING;
      }

      break;

    case TCRT_TARGET_CHANNEL_COUNT:
      if (hSelProcInput->targetChannelCountRequested == hSelProcInput->baseChannelCount) {
        hSelProcInput->downmixIdRequested[0] = 0;
        hSelProcInput->numDownmixIdRequests = 1;
      }

      if (hSelProcInput->numDownmixIdRequests == 0) {
        for (i = 0; i < hUniDrcConfig->downmixInstructionsCount; i++) {
          pDown = &(hUniDrcConfig->downmixInstructions[i]);

          if (hSelProcInput->targetChannelCountRequested == pDown->targetChannelCount) {
            hSelProcInput->downmixIdRequested[hSelProcInput->numDownmixIdRequests] =
                pDown->downmixId;
            hSelProcInput->numDownmixIdRequests++;
          }
        }
      }

      if (hSelProcInput->baseChannelCount == -1) {
        retVal = DRCDEC_SELECTION_PROCESS_WARNING;
      }

      if (hSelProcInput->numDownmixIdRequests == 0) {
        retVal = DRCDEC_SELECTION_PROCESS_WARNING;
        hSelProcInput->downmixIdRequested[0] = 0;
        hSelProcInput->numDownmixIdRequests = 1;
      }

      break;

    default:
      return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  return retVal;
}

/*******************************************/

/* Note: Numbering of DRC pre-selection steps according to MPEG-D Part-4 DRC Amd1 */

/* #1: DownmixId of DRC set matches the requested downmixId.
   #2: Output channel layout of DRC set matches the requested layout.
   #3: Channel count of DRC set matches the requested channel count. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement123(
    int nRequestedDownmixId, DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc, int* pMatchFound) {
  int i;
  *pMatchFound = 0;

  for (i = 0; i < pDrcInstructionUniDrc->downmixIdCount; i++) {
    if ((pDrcInstructionUniDrc->downmixId[i] == nRequestedDownmixId) ||
        (pDrcInstructionUniDrc->downmixId[i] == DOWNMIX_ID_ANY_DOWNMIX) ||
        ((pDrcInstructionUniDrc->downmixId[i] == DOWNMIX_ID_BASE_LAYOUT) &&
         (pDrcInstructionUniDrc->drcSetId > 0))) {
      *pMatchFound = 1;
      break;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/* #4: The DRC set is not a "Fade-" or "Ducking-" only DRC set. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement4(
    DRC_INSTRUCTIONS_UNI_DRC* pDrcInstruction, int nDynamicRangeControlOn, int* pMatchFound) {
  *pMatchFound = 0;

  if (nDynamicRangeControlOn == 1) {
    if ((pDrcInstruction->drcSetEffect != EB_FADE) &&
        (pDrcInstruction->drcSetEffect != EB_DUCK_OTHER) &&
        (pDrcInstruction->drcSetEffect != EB_DUCK_SELF) &&
        (pDrcInstruction->drcSetEffect != 0 || pDrcInstruction->drcSetId < 0)) {
      *pMatchFound = 1;
    }
  } else {
    *pMatchFound = 1;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/* #5: The number of DRC bands is supported. Moreover, gainSetIndex and gainSequenceIndex are within
 * the allowed range. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement5(
    DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc, DRC_COEFFICIENTS_UNI_DRC* pCoef,
    int* pMatchFound) {
  int b, i;

  *pMatchFound = 1;

  if (pDrcInstructionUniDrc->drcSetId < 0) /* virtual DRC sets are okay */
  {
    return DRCDEC_SELECTION_PROCESS_NO_ERROR;
  }

  if (pCoef == NULL) /* check for parametricDRC */
  {
    *pMatchFound = 0; /* parametricDRC not supported */
    return DRCDEC_SELECTION_PROCESS_NO_ERROR;
  }

  if (pCoef->drcLocation !=
      pDrcInstructionUniDrc->drcLocation) /* drcLocation must be LOCATION_SELECTED */
  {
    *pMatchFound = 0;
    return DRCDEC_SELECTION_PROCESS_NO_ERROR;
  }

  for (i = 0; i < pDrcInstructionUniDrc->nDrcChannelGroups; i++) {
    int indexDrcCoeff = pDrcInstructionUniDrc->gainSetIndexForChannelGroup[i];
    int bandCount = 0;

    if (indexDrcCoeff >= 48) {
      *pMatchFound = 0;
      return DRCDEC_SELECTION_PROCESS_NO_ERROR;
    }

    if (indexDrcCoeff > pCoef->gainSetCount - 1) /* check for parametricDRC */
    {
      continue;
    }

    GAIN_SET* gainSet = &(pCoef->gainSet[indexDrcCoeff]);
    bandCount = gainSet->bandCount;

    if (bandCount > 4) {
      *pMatchFound = 0;
    }

    for (b = 0; b < bandCount; b++) {
      if ((gainSet->gainSequenceIndex[b] >= 48) ||
          (gainSet->gainSequenceIndex[b] >= pCoef->gainSequenceCount)) {
        *pMatchFound = 0;
        return DRCDEC_SELECTION_PROCESS_NO_ERROR;
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/* #6: Independent use of DRC set is permitted.*/
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement6(
    DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc, int* pMatchFound) {
  *pMatchFound = 0;

  if (((pDrcInstructionUniDrc->dependsOnDrcSetPresent == 0) &&
       (pDrcInstructionUniDrc->noIndependentUse == 0)) ||
      (pDrcInstructionUniDrc->dependsOnDrcSetPresent == 1)) {
    *pMatchFound = 1;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/* #7: DRC sets that require EQ are only permitted if EQ is supported. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement7(
    DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc, int* pMatchFound) {
  *pMatchFound = 1;

  if (pDrcInstructionUniDrc->requiresEq) {
    /* EQ is not supported */
    *pMatchFound = 0;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static void _setSelectionDataInfo(DRCDEC_SELECTION_DATA* pData, FIXP_DBL loudness, /* e = 7 */
                                  FIXP_DBL loudnessNormalizationGainDb,            /* e = 7 */
                                  FIXP_DBL loudnessNormalizationGainDbMax,         /* e = 7 */
                                  FIXP_DBL loudnessDeviationMax,                   /* e = 7 */
                                  FIXP_DBL signalPeakLevel,                        /* e = 7 */
                                  FIXP_DBL outputPeakLevelMax,                     /* e = 7 */
                                  int applyAdjustment) {
  FIXP_DBL adjustment = 0; /* e = 8 */

  /* use e = 8 for all function parameters to prevent overflow */
  loudness >>= 1;
  loudnessNormalizationGainDb >>= 1;
  loudnessNormalizationGainDbMax >>= 1;
  loudnessDeviationMax >>= 1;
  signalPeakLevel >>= 1;
  outputPeakLevelMax >>= 1;

  if (applyAdjustment) {
    adjustment =
        fMax((FIXP_DBL)0, signalPeakLevel + loudnessNormalizationGainDb - outputPeakLevelMax);
    adjustment = fMin(adjustment, fMax((FIXP_DBL)0, loudnessDeviationMax));
  }

  pData->loudnessNormalizationGainDbAdjusted =
      fMin(loudnessNormalizationGainDb - adjustment, loudnessNormalizationGainDbMax);
  pData->outputLoudness = loudness + pData->loudnessNormalizationGainDbAdjusted;
  pData->outputPeakLevel = signalPeakLevel + pData->loudnessNormalizationGainDbAdjusted;

  /* shift back to e = 7 using saturation */
  pData->loudnessNormalizationGainDbAdjusted =
      SATURATE_LEFT_SHIFT(pData->loudnessNormalizationGainDbAdjusted, 1, DFRACT_BITS);
  pData->outputLoudness = SATURATE_LEFT_SHIFT(pData->outputLoudness, 1, DFRACT_BITS);
  pData->outputPeakLevel = SATURATE_LEFT_SHIFT(pData->outputPeakLevel, 1, DFRACT_BITS);
}

static int _targetLoudnessInRange(DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc,
                                  FIXP_DBL targetLoudness) {
  int retVal = 0;

  FIXP_DBL drcSetTargetLoudnessValueUpper =
      ((FIXP_DBL)pDrcInstructionUniDrc->drcSetTargetLoudnessValueUpper) << (DFRACT_BITS - 1 - 7);
  FIXP_DBL drcSetTargetLoudnessValueLower =
      ((FIXP_DBL)pDrcInstructionUniDrc->drcSetTargetLoudnessValueLower) << (DFRACT_BITS - 1 - 7);

  if (pDrcInstructionUniDrc->drcSetTargetLoudnessPresent &&
      drcSetTargetLoudnessValueUpper >= targetLoudness &&
      drcSetTargetLoudnessValueLower < targetLoudness) {
    retVal = 1;
  }

  return retVal;
}

static int _drcSetIsUsable(HANDLE_UNI_DRC_CONFIG hUniDrcConfig, DRC_INSTRUCTIONS_UNI_DRC* pInst) {
  int usable = 0;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);

  /* check if ID is unique */
  if (selectDrcInstructions(hUniDrcConfig, pInst->drcSetId) != pInst) return 0;
  /* sanity check on drcInstructions */
  _preSelectionRequirement5(pInst, pCoef, &usable);
  return usable;
}

/* #8: The range of the target loudness specified for a DRC set has to include the requested decoder
 * target loudness. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement8(
    SEL_PROC_INPUT* hSelProcInput, int downmixIdIndex, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc,
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected,
    SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int explicitPeakInformationPresent;
  FIXP_DBL signalPeakLevel;
  int addToCandidate = 0;

  FIXP_DBL loudnessNormalizationGainDb;
  FIXP_DBL loudness;

  FIXP_DBL loudnessDeviationMax = ((FIXP_DBL)hSelProcInput->loudnessDeviationMax)
                                  << (DFRACT_BITS - 1 - 7);

  {
    int groupIdRequested = -1, groupPresetIdRequested = -1;

    if (pDrcInstructionUniDrc->drcInstructionsType == 3) {
      groupPresetIdRequested = pDrcInstructionUniDrc->mae_groupPresetID;
    } else if (pDrcInstructionUniDrc->drcInstructionsType == 2) {
      groupIdRequested = pDrcInstructionUniDrc->mae_groupID;
    }

    retVal = _getLoudness(hLoudnessInfoSet, hSelProcInput->albumMode, MDR_ANCHOR_LOUDNESS,
                          MSR_EXPERT_PANEL, hSelProcInput->targetLoudness,
                          pDrcInstructionUniDrc->drcSetId,
                          hSelProcInput->downmixIdRequested[downmixIdIndex], codecMode,
                          groupIdRequested, groupPresetIdRequested, &hSelProcInput->mpeghRequest,
                          &loudnessNormalizationGainDb, &loudness);
    if (retVal) return (retVal);
  }

  if (!hSelProcInput->loudnessNormalizationOn) {
    loudnessNormalizationGainDb = (FIXP_DBL)0;
  }

  retVal =
      _getSignalPeakLevel(hSelProcInput, hUniDrcConfig, hLoudnessInfoSet, pDrcInstructionUniDrc,
                          hSelProcInput->downmixIdRequested[downmixIdIndex],
                          &explicitPeakInformationPresent, &signalPeakLevel, codecMode

      );
  if (retVal) return (retVal);

  if (hSelProcInput->dynamicRangeControlOn) {
    if (explicitPeakInformationPresent == 0) {
      if (pDrcInstructionUniDrc->drcSetTargetLoudnessPresent &&
          ((hSelProcInput->loudnessNormalizationOn &&
            _targetLoudnessInRange(pDrcInstructionUniDrc, hSelProcInput->targetLoudness)) ||
           !hSelProcInput->loudnessNormalizationOn)) {
        DRCDEC_SELECTION_DATA* pData = _drcdec_selection_addNew(pCandidatesSelected);
        if (pData == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

        _setSelectionDataInfo(pData, loudness, loudnessNormalizationGainDb,
                              hSelProcInput->loudnessNormalizationGainDbMax, loudnessDeviationMax,
                              signalPeakLevel, hSelProcInput->outputPeakLevelMax, 0);
        pData->downmixIdRequestIndex = downmixIdIndex;
        pData->pInst = pDrcInstructionUniDrc;
        pData->selectionFlag = 1; /* signal pre-selection step dealing with drcSetTargetLoudness */

        if (hSelProcInput->loudnessNormalizationOn) {
          pData->outputPeakLevel =
              hSelProcInput->targetLoudness -
              (((FIXP_DBL)pData->pInst->drcSetTargetLoudnessValueUpper) << (DFRACT_BITS - 1 - 7));
        } else {
          pData->outputPeakLevel = (FIXP_DBL)0;
        }
      } else {
        if ((!hSelProcInput->loudnessNormalizationOn) ||
            (!pDrcInstructionUniDrc->drcSetTargetLoudnessPresent) ||
            (hSelProcInput->loudnessNormalizationOn &&
             _targetLoudnessInRange(pDrcInstructionUniDrc, hSelProcInput->targetLoudness))) {
          addToCandidate = 1;
        }
      }
    } else {
      addToCandidate = 1;
    }

    if (addToCandidate) {
      DRCDEC_SELECTION_DATA* pData = _drcdec_selection_addNew(pCandidatesPotential);
      if (pData == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      _setSelectionDataInfo(pData, loudness, loudnessNormalizationGainDb,
                            hSelProcInput->loudnessNormalizationGainDbMax, loudnessDeviationMax,
                            signalPeakLevel, hSelProcInput->outputPeakLevelMax, 0);
      pData->downmixIdRequestIndex = downmixIdIndex;
      pData->pInst = pDrcInstructionUniDrc;
      pData->selectionFlag = 0;
    }
  } else {
    if (pDrcInstructionUniDrc->drcSetId < 0) {
      DRCDEC_SELECTION_DATA* pData = _drcdec_selection_addNew(pCandidatesSelected);
      if (pData == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      _setSelectionDataInfo(pData, loudness, loudnessNormalizationGainDb,
                            hSelProcInput->loudnessNormalizationGainDbMax, loudnessDeviationMax,
                            signalPeakLevel, hSelProcInput->outputPeakLevelMax, 1);

      pData->downmixIdRequestIndex = downmixIdIndex;
      pData->pInst = pDrcInstructionUniDrc;
      pData->selectionFlag = 0;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/* #9: Clipping is minimized. */
static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement9(
    SEL_PROC_INPUT* hSelProcInput, DRCDEC_SELECTION* pCandidatesPotential,
    DRCDEC_SELECTION* pCandidatesSelected) {
  int i;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if (pCandidate->outputPeakLevel <= hSelProcInput->outputPeakLevelMax) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement10(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected,
    UCHAR numGroupPresetIdsRequested, UCHAR* pGroupPresetIdRequested,
    int* pOmitPreSelectionBasedOnRequestedGroupId) {
  int i, j;

  *pOmitPreSelectionBasedOnRequestedGroupId = 0;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if (pCandidate->pInst->drcInstructionsType == 0 ||
        pCandidate->pInst->drcInstructionsType == 2) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    } else if (pCandidate->pInst->drcInstructionsType == 3) {
      for (j = 0; j < numGroupPresetIdsRequested; j++) {
        if (pCandidate->pInst->mae_groupPresetID == pGroupPresetIdRequested[j]) {
          if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
            return DRCDEC_SELECTION_PROCESS_NOT_OK;

          *pOmitPreSelectionBasedOnRequestedGroupId = 1;
        }
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _preSelectionRequirement11(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected,
    UCHAR numGroupIdsRequested, UCHAR* pGroupIdRequested,
    int omitPreSelectionBasedOnRequestedGroupId) {
  int i, j;

  if (omitPreSelectionBasedOnRequestedGroupId == 1) {
    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      if (pCandidate->pInst->drcInstructionsType == 0 ||
          pCandidate->pInst->drcInstructionsType == 3) {
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }
  } else {
    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      if (pCandidate->pInst->drcInstructionsType == 0) {
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      } else if (pCandidate->pInst->drcInstructionsType == 2) {
        for (j = 0; j < numGroupIdsRequested; j++) {
          if (pCandidate->pInst->mae_groupID == pGroupIdRequested[j]) {
            if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
              return DRCDEC_SELECTION_PROCESS_NOT_OK;
          }
        }
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetPreSelectionSingleInstruction(
    SEL_PROC_INPUT* hSelProcInput, int downmixIdIndex, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc,
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected,
    SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int matchFound = 0;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);

  retVal = _preSelectionRequirement123(hSelProcInput->downmixIdRequested[downmixIdIndex],
                                       pDrcInstructionUniDrc, &matchFound);

  if (!retVal && matchFound)
    retVal = _preSelectionRequirement4(pDrcInstructionUniDrc, hSelProcInput->dynamicRangeControlOn,
                                       &matchFound);

  if (!retVal && matchFound)
    retVal = _preSelectionRequirement5(pDrcInstructionUniDrc, pCoef, &matchFound);

  if (!retVal && matchFound) retVal = _preSelectionRequirement6(pDrcInstructionUniDrc, &matchFound);

  if (!retVal && matchFound) retVal = _preSelectionRequirement7(pDrcInstructionUniDrc, &matchFound);

  if (!retVal && matchFound)
    retVal = _preSelectionRequirement8(hSelProcInput, downmixIdIndex, hUniDrcConfig,
                                       hLoudnessInfoSet, pDrcInstructionUniDrc,
                                       pCandidatesPotential, pCandidatesSelected, codecMode);

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetSelectionAddCandidates(
    SEL_PROC_INPUT* hSelProcInput, DRCDEC_SELECTION* pCandidatesPotential,
    DRCDEC_SELECTION* pCandidatesSelected) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int nHitCount = 0;
  int i;

  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    pDrcInstructionUniDrc = pCandidate->pInst;

    if (_targetLoudnessInRange(pDrcInstructionUniDrc, hSelProcInput->targetLoudness)) {
      nHitCount++;
    }
  }

  if (nHitCount != 0) {
    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      pDrcInstructionUniDrc = pCandidate->pInst;

      if (_targetLoudnessInRange(pDrcInstructionUniDrc, hSelProcInput->targetLoudness)) {
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }
  } else {
    FIXP_DBL lowestPeakLevel =
        FL2FXCONST_DBL(126.9f / (float)(1 << 7)); /* Initialize with very high value. e = 7 */
    FIXP_DBL peakLevel = 0;                       /* e = 7 */

    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      peakLevel = pCandidate->outputPeakLevel;

      if (peakLevel < lowestPeakLevel) {
        int j;

        if (pCandidate->pInst->drcInstructionsType == 3) {
          for (j = 0; j < hSelProcInput->mpeghRequest.numGroupPresetIdsRequested; j++) {
            if (pCandidate->pInst->mae_groupPresetID ==
                hSelProcInput->mpeghRequest.groupPresetIdRequested[j]) {
              lowestPeakLevel = peakLevel;
              break;
            }
          }
        } else if (pCandidate->pInst->drcInstructionsType == 2) {
          for (j = 0; j < hSelProcInput->mpeghRequest.numGroupIdsRequested; j++) {
            if (pCandidate->pInst->mae_groupID == hSelProcInput->mpeghRequest.groupIdRequested[j]) {
              lowestPeakLevel = peakLevel;
              break;
            }
          }
        } else {
          lowestPeakLevel = peakLevel;
        }
      }
    }

    /* add all with lowest peak level or max 1dB above */
    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      FIXP_DBL loudnessDeviationMax = ((FIXP_DBL)hSelProcInput->loudnessDeviationMax)
                                      << (DFRACT_BITS - 1 - 7); /* e = 7 */

      pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      peakLevel = pCandidate->outputPeakLevel;

      if (peakLevel == lowestPeakLevel ||
          peakLevel <= lowestPeakLevel + FL2FXCONST_DBL(1.0f / (float)(1 << 7))) {
        FIXP_DBL adjustment = fMax((FIXP_DBL)0, peakLevel - hSelProcInput->outputPeakLevelMax);
        adjustment = fMin(adjustment, fMax((FIXP_DBL)0, loudnessDeviationMax));

        pCandidate->loudnessNormalizationGainDbAdjusted -= adjustment;
        pCandidate->outputPeakLevel -= adjustment;
        pCandidate->outputLoudness -= adjustment;
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _dependentDrcInstruction(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, DRC_INSTRUCTIONS_UNI_DRC* pInst,
    DRC_INSTRUCTIONS_UNI_DRC** ppDrcInstructionsDependent) {
  int i;
  DRC_INSTRUCTIONS_UNI_DRC* pDependentDrc = NULL;

  for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
    pDependentDrc = (DRC_INSTRUCTIONS_UNI_DRC*)&(hUniDrcConfig->drcInstructionsUniDrc[i]);

    if (pDependentDrc->drcSetId == pInst->dependsOnDrcSet) {
      break;
    }
  }

  if (i == hUniDrcConfig->drcInstructionsUniDrcCount) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  if (pDependentDrc == NULL || pDependentDrc->dependsOnDrcSetPresent == 1) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  *ppDrcInstructionsDependent = pDependentDrc;

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _selectDrcSetEffectNone(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if ((pCandidate->pInst->drcSetEffect & 0xff) == 0) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _selectSingleEffectType(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, DRC_EFFECT_TYPE_REQUEST effectType,
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  DRC_INSTRUCTIONS_UNI_DRC* pInst;
  DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionsDependent;

  if (effectType == DETR_NONE) {
    retVal = _selectDrcSetEffectNone(pCandidatesPotential, pCandidatesSelected);
    if (retVal) return (retVal);
  } else {
    int effectBitPosition = 1 << (effectType - 1);

    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      pInst = pCandidate->pInst;

      if (!pInst->dependsOnDrcSetPresent) {
        if ((pInst->drcSetEffect & effectBitPosition)) {
          if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
            return DRCDEC_SELECTION_PROCESS_NOT_OK;
        }
      } else {
        retVal = _dependentDrcInstruction(hUniDrcConfig, pInst, &pDrcInstructionsDependent);
        if (retVal) return (retVal);

        if (((pInst->drcSetEffect & effectBitPosition)) ||
            ((pDrcInstructionsDependent->drcSetEffect & effectBitPosition))) {
          if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
            return DRCDEC_SELECTION_PROCESS_NOT_OK;
        }
      }
    }
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _selectEffectTypeFeature(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, DRC_FEATURE_REQUEST drcFeatureRequest,
    DRCDEC_SELECTION** ppCandidatesPotential, DRCDEC_SELECTION** ppCandidatesSelected) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int i;
  int desiredEffectTypeFound = 0;

  for (i = 0; i < drcFeatureRequest.drcEffectType.numRequestsDesired; i++) {
    retVal = _selectSingleEffectType(hUniDrcConfig, drcFeatureRequest.drcEffectType.request[i],
                                     *ppCandidatesPotential, *ppCandidatesSelected);
    if (retVal) return (retVal);

    if (_drcdec_selection_getNumber(*ppCandidatesSelected)) {
      desiredEffectTypeFound = 1;
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
    }
  }

  if (!desiredEffectTypeFound) {
    for (i = drcFeatureRequest.drcEffectType.numRequestsDesired;
         i < drcFeatureRequest.drcEffectType.numRequests; i++) {
      retVal = _selectSingleEffectType(hUniDrcConfig, drcFeatureRequest.drcEffectType.request[i],
                                       *ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);

      if (_drcdec_selection_getNumber(*ppCandidatesSelected)) {
        _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
        break;
      }
    }
  }

  _swapSelection(ppCandidatesPotential, ppCandidatesSelected);

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_peakValue0(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;

  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  FIXP_DBL lowestPeakLevel = MAXVAL_DBL;
  int indexLowestPeakLevel = 0;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if (pCandidate->outputPeakLevel <= (FIXP_DBL)0) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }

    if (pCandidate->outputPeakLevel < lowestPeakLevel) {
      indexLowestPeakLevel = i;
      lowestPeakLevel = pCandidate->outputPeakLevel;
    }
  }

  if (_drcdec_selection_getNumber(pCandidatesSelected) == 0) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, indexLowestPeakLevel);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;
    if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
      return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_presetID_groupID(
    HANDLE_SEL_PROC_INPUT hSelProcInput, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected) {
  int i, j;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;

  int numMembersGroupPresetId = 0;
  int maxNumMembersGroupPresetIds = 0;
  int mae_groupPresetID_lowest = 1000;
  int instructionsType3Found = 0;

  /* final MPEG-H selection based on groupPresetId and groupId */

  for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    pInst = pCandidate->pInst;

    if (pInst->drcInstructionsType == 3) {
      instructionsType3Found = 1;
    }
  }

  if (instructionsType3Found) {
    for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
      pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      pInst = pCandidate->pInst;

      if (pInst->drcInstructionsType == 3) {
        if (hSelProcInput->mpeghRequest.groupPresetIdRequestedPreference ==
            pInst->mae_groupPresetID) {
          if (_drcdec_selection_add(*ppCandidatesSelected, pCandidate) == NULL)
            return DRCDEC_SELECTION_PROCESS_NOT_OK;

          break;
        }
      }
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
      for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
        pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
        if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

        pInst = pCandidate->pInst;

        if (pInst->drcInstructionsType == 3) {
          for (j = 0; j < hSelProcInput->mpeghRequest.numGroupPresetIdsRequested; j++) {
            if (hSelProcInput->mpeghRequest.groupPresetIdRequested[j] == pInst->mae_groupPresetID) {
              numMembersGroupPresetId =
                  hSelProcInput->mpeghRequest.numMembersGroupPresetIdsRequested[j];
              break;
            }
          }

          if (maxNumMembersGroupPresetIds <= numMembersGroupPresetId) {
            if (maxNumMembersGroupPresetIds < numMembersGroupPresetId) {
              maxNumMembersGroupPresetIds = numMembersGroupPresetId;

              _drcdec_selection_setNumber(*ppCandidatesSelected, 0);
            }
            if (_drcdec_selection_add(*ppCandidatesSelected, pCandidate) == NULL)
              return DRCDEC_SELECTION_PROCESS_NOT_OK;
          }
        }
      }
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);

      for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
        pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
        if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

        pInst = pCandidate->pInst;

        if (mae_groupPresetID_lowest >= pInst->mae_groupPresetID) {
          if (mae_groupPresetID_lowest > pInst->mae_groupPresetID) {
            mae_groupPresetID_lowest = pInst->mae_groupPresetID;

            _drcdec_selection_setNumber(*ppCandidatesSelected, 0);
          }
          if (_drcdec_selection_add(*ppCandidatesSelected, pCandidate) == NULL)
            return DRCDEC_SELECTION_PROCESS_NOT_OK;
        }
      }
    }
  } else {
    for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
      pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      pInst = pCandidate->pInst;

      if (pInst->drcInstructionsType == 2) {
        if (_drcdec_selection_add(*ppCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
      _swapSelection(ppCandidatesPotential, ppCandidatesSelected);
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_downmixId(
    HANDLE_SEL_PROC_INPUT hSelProcInput, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected) {
  int i, j;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(*ppCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(*ppCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    pInst = pCandidate->pInst;

    for (j = 0; j < pInst->downmixIdCount; j++) {
      if (DOWNMIX_ID_BASE_LAYOUT != pInst->downmixId[j] &&
          DOWNMIX_ID_ANY_DOWNMIX != pInst->downmixId[j] &&
          hSelProcInput->downmixIdRequested[pCandidate->downmixIdRequestIndex] ==
              pInst->downmixId[j]) {
        if (_drcdec_selection_add(*ppCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }
  }

  if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
    _swapSelection(ppCandidatesPotential, ppCandidatesSelected);
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static int _crossSum(int value) {
  int sum = 0;

  while (value != 0) {
    if ((value & 1) == 1) {
      sum++;
    }

    value >>= 1;
  }

  return sum;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_effectTypes(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;
  int minNumEffects = 1000;
  int numEffects = 0;
  int effects = 0;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    pInst = pCandidate->pInst;

    effects = pInst->drcSetEffect;
    effects &= 0xffff ^ (EB_GENERAL_COMPR);
    numEffects = _crossSum(effects);

    if (numEffects < minNumEffects) {
      minNumEffects = numEffects;
    }
  }

  /* add all with minimum number of effects */
  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    pInst = pCandidate->pInst;

    effects = pInst->drcSetEffect;
    effects &= 0xffff ^ (EB_GENERAL_COMPR);
    numEffects = _crossSum(effects);

    if (numEffects == minNumEffects) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _selectSmallestTargetLoudnessValueUpper(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;
  SCHAR minVal = 0x7F;
  SCHAR val = 0;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    val = pCandidate->pInst->drcSetTargetLoudnessValueUpper;

    if (val < minVal) {
      minVal = val;
    }
  }

  /* add all with same smallest drcSetTargetLoudnessValueUpper */
  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    val = pCandidate->pInst->drcSetTargetLoudnessValueUpper;

    if (val == minVal) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_targetLoudness(
    FIXP_DBL targetLoudness, DRCDEC_SELECTION* pCandidatesPotential,
    DRCDEC_SELECTION* pCandidatesSelected) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int i;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if (pCandidate->selectionFlag == 0) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  if (_drcdec_selection_getNumber(pCandidatesSelected) == 0) {
    retVal = _selectSmallestTargetLoudnessValueUpper(pCandidatesPotential, pCandidatesSelected);
    if (retVal) return (retVal);
  }

  if (_drcdec_selection_getNumber(pCandidatesSelected) > 1) {
    DRC_INSTRUCTIONS_UNI_DRC* pDrcInstructionUniDrc = NULL;

    _swapSelectionAndClear(&pCandidatesPotential, &pCandidatesSelected);

    for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
      pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
      if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

      pDrcInstructionUniDrc = pCandidate->pInst;

      if (_targetLoudnessInRange(pDrcInstructionUniDrc, targetLoudness)) {
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }

    if (_drcdec_selection_getNumber(pCandidatesSelected) > 1) {
      _swapSelectionAndClear(&pCandidatesPotential, &pCandidatesSelected);

      retVal = _selectSmallestTargetLoudnessValueUpper(pCandidatesPotential, pCandidatesSelected);
      if (retVal) return (retVal);
    }
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_peakValueLargest(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;
  FIXP_DBL largestPeakLevel = MINVAL_DBL;
  FIXP_DBL peakLevel = 0;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    peakLevel = pCandidate->outputPeakLevel;

    if (peakLevel > largestPeakLevel) {
      largestPeakLevel = peakLevel;
    }
  }

  /* add all with same largest peak level */
  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    peakLevel = pCandidate->outputPeakLevel;

    if (peakLevel == largestPeakLevel) {
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection_drcSetId(
    DRCDEC_SELECTION* pCandidatesPotential, DRCDEC_SELECTION* pCandidatesSelected) {
  int i;
  int largestId = -1000;
  int id = 0;
  DRCDEC_SELECTION_DATA* pCandidate = NULL;
  DRCDEC_SELECTION_DATA* pCandidateSelected = NULL;

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    id = pCandidate->pInst->drcSetId;

    if (id > largestId) {
      largestId = id;
      pCandidateSelected = pCandidate;
    }
  }

  if (pCandidateSelected != NULL) {
    if (_drcdec_selection_add(pCandidatesSelected, pCandidateSelected) == NULL)
      return DRCDEC_SELECTION_PROCESS_NOT_OK;
  } else {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetFinalSelection(
    HANDLE_SEL_PROC_INPUT hSelProcInput, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected, SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (_drcdec_selection_getNumber(*ppCandidatesPotential) == 0) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  } else if (_drcdec_selection_getNumber(*ppCandidatesPotential) == 1) {
    _swapSelection(ppCandidatesPotential, ppCandidatesSelected);
    /* finished */
  } else /* > 1 */
  {
    retVal = _drcSetFinalSelection_peakValue0(*ppCandidatesPotential, *ppCandidatesSelected);
    if (retVal) return (retVal);

    if (codecMode == SEL_PROC_MPEG_H_3DA) {
      if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
        _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);

        retVal = _drcSetFinalSelection_presetID_groupID(hSelProcInput, ppCandidatesPotential,
                                                        ppCandidatesSelected);
        if (retVal) return (retVal);
      }
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
      retVal = _drcSetFinalSelection_downmixId(hSelProcInput, ppCandidatesPotential,
                                               ppCandidatesSelected);
      if (retVal) return (retVal);
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
      retVal = _drcSetFinalSelection_effectTypes(*ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
      retVal = _drcSetFinalSelection_targetLoudness(hSelProcInput->targetLoudness,
                                                    *ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
      retVal =
          _drcSetFinalSelection_peakValueLargest(*ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);
    }

    if (_drcdec_selection_getNumber(*ppCandidatesSelected) > 1) {
      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
      retVal = _drcSetFinalSelection_drcSetId(*ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);
    }
  }

  if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _generateVirtualDrcSets(HANDLE_SEL_PROC_INPUT hSelProcInput,
                                                               HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                               SEL_PROC_CODEC_MODE codecMode) {
  int i;
  int nMixes = hUniDrcConfig->downmixInstructionsCount + 1;
  int index = hUniDrcConfig->drcInstructionsUniDrcCount;
  int indexVirtual = -1;
  DRC_INSTRUCTIONS_UNI_DRC* pDrcInstruction = &(hUniDrcConfig->drcInstructionsUniDrc[index]);

  if (codecMode == SEL_PROC_MPEG_H_3DA) {
    nMixes = 1;
  }

  if ((index + nMixes) > (32 + 1)) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  FDKmemset(pDrcInstruction, 0, sizeof(DRC_INSTRUCTIONS_UNI_DRC));

  pDrcInstruction->drcSetId = indexVirtual;
  index++;
  indexVirtual--;
  pDrcInstruction->downmixIdCount = 1;

  if ((codecMode == SEL_PROC_MPEG_H_3DA) && (hSelProcInput->numDownmixIdRequests)) {
    pDrcInstruction->downmixId[0] = hSelProcInput->downmixIdRequested[0];
  } else {
    pDrcInstruction->downmixId[0] = DOWNMIX_ID_BASE_LAYOUT;
  }

  for (i = 1; i < nMixes; i++) {
    pDrcInstruction = &(hUniDrcConfig->drcInstructionsUniDrc[index]);
    FDKmemset(pDrcInstruction, 0, sizeof(DRC_INSTRUCTIONS_UNI_DRC));
    pDrcInstruction->drcSetId = indexVirtual;
    pDrcInstruction->downmixId[0] = hUniDrcConfig->downmixInstructions[i - 1].downmixId;
    pDrcInstruction->downmixIdCount = 1;
    index++;
    indexVirtual--;
  }

  hUniDrcConfig->drcInstructionsCountInclVirtual =
      hUniDrcConfig->drcInstructionsUniDrcCount + nMixes;

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _generateOutputInfo(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_SEL_PROC_OUTPUT hSelProcOutput,
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
    DRCDEC_SELECTION_DATA* pSelectionData, SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  int i, j;
  int hasDependend = 0;
  int hasFading = 0;
  int hasDucking = 0;
  int selectedDrcSetIds;
  int selectedDownmixIds;
  FIXP_DBL mixingLevel = 0;
  int albumMode = hSelProcInput->albumMode;
  UCHAR* pDownmixIdRequested = hSelProcInput->downmixIdRequested;
  FIXP_SGL boost = hSelProcInput->boost;
  FIXP_SGL compress = hSelProcInput->compress;

  hSelProcOutput->numSelectedDrcSets = 1;
  hSelProcOutput->selectedDrcSetIds[0] = pSelectionData->pInst->drcSetId;
  hSelProcOutput->selectedDownmixIds[0] =
      pSelectionData->pInst->drcApplyToDownmix == 1 ? pSelectionData->pInst->downmixId[0] : 0;
  hSelProcOutput->loudnessNormalizationGainDb =
      pSelectionData->loudnessNormalizationGainDbAdjusted +
      hSelProcInput->loudnessNormalizationGainModificationDb;
  hSelProcOutput->outputPeakLevelDb = pSelectionData->outputPeakLevel;
  hSelProcOutput->outputLoudness = pSelectionData->outputLoudness;

  hSelProcOutput->boost = boost;
  hSelProcOutput->compress = compress;
  hSelProcOutput->baseChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  hSelProcOutput->targetChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  hSelProcOutput->activeDownmixId = pDownmixIdRequested[pSelectionData->downmixIdRequestIndex];

  _getMixingLevel(hLoudnessInfoSet, *pDownmixIdRequested, hSelProcOutput->selectedDrcSetIds[0],
                  albumMode, &mixingLevel);
  hSelProcOutput->mixingLevel = mixingLevel;

  _getGroupLoudness(hLoudnessInfoSet, hSelProcOutput);

  /*dependent*/
  if (pSelectionData->pInst->dependsOnDrcSetPresent) {
    int dependsOnDrcSetID = pSelectionData->pInst->dependsOnDrcSet;

    for (i = 0; i < hUniDrcConfig->drcInstructionsCountInclVirtual; i++) {
      DRC_INSTRUCTIONS_UNI_DRC* pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
      if (!_drcSetIsUsable(hUniDrcConfig, pInst)) continue;

      if (pInst->drcSetId == dependsOnDrcSetID) {
        hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
            hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
        hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] =
            hUniDrcConfig->drcInstructionsUniDrc[i].drcApplyToDownmix == 1
                ? hUniDrcConfig->drcInstructionsUniDrc[i].downmixId[0]
                : 0;
        hSelProcOutput->numSelectedDrcSets++;
        hasDependend = 1;
        break;
      }
    }
  }

  /* fading */
  if (hSelProcInput->albumMode == 0) {
    for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
      DRC_INSTRUCTIONS_UNI_DRC* pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
      if (!_drcSetIsUsable(hUniDrcConfig, pInst)) continue;

      if (pInst->drcSetEffect & EB_FADE) {
        if (pInst->downmixId[0] == DOWNMIX_ID_ANY_DOWNMIX) {
          if (pInst->drcInstructionsType == 0) {
            hasFading = 1;
            hSelProcOutput->numSelectedDrcSets = hasDependend + 1;
            hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
                hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
            hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] =
                hUniDrcConfig->drcInstructionsUniDrc[i].drcApplyToDownmix == 1
                    ? hUniDrcConfig->drcInstructionsUniDrc[i].downmixId[0]
                    : 0;
            hSelProcOutput->numSelectedDrcSets++;
          } else if (pInst->drcInstructionsType == 2) {
            int m;

            for (m = 0; m < hSelProcInput->mpeghRequest.numGroupIdsRequested; m++) {
              if (hSelProcInput->mpeghRequest.groupIdRequested[m] == pInst->mae_groupID) {
                hasFading = 1;
                hSelProcOutput->numSelectedDrcSets = hasDependend + 1;
                hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
                    hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
                hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] =
                    hUniDrcConfig->drcInstructionsUniDrc[i].drcApplyToDownmix == 1
                        ? hUniDrcConfig->drcInstructionsUniDrc[i].downmixId[0]
                        : 0;
                hSelProcOutput->numSelectedDrcSets++;
                break;
              }
            }
          } else if (pInst->drcInstructionsType == 3) {
            int m;

            for (m = 0; m < hSelProcInput->mpeghRequest.numGroupPresetIdsRequested; m++) {
              if (hSelProcInput->mpeghRequest.groupPresetIdRequested[m] ==
                  pInst->mae_groupPresetID) {
                hasFading = 1;
                hSelProcOutput->numSelectedDrcSets = hasDependend + 1;
                hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
                    hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
                hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] =
                    hUniDrcConfig->drcInstructionsUniDrc[i].drcApplyToDownmix == 1
                        ? hUniDrcConfig->drcInstructionsUniDrc[i].downmixId[0]
                        : 0;
                hSelProcOutput->numSelectedDrcSets++;
                break;
              }
            }
          } else {
            return DRCDEC_SELECTION_PROCESS_NOT_OK;
          }
        } else {
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
        }
      }
    }
  }

  /* ducking */
  for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
    DRC_INSTRUCTIONS_UNI_DRC* pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
    if (!_drcSetIsUsable(hUniDrcConfig, pInst)) continue;

    if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
      for (j = 0; j < pInst->downmixIdCount; j++) {
        if (pInst->downmixId[j] == hSelProcOutput->activeDownmixId) {
          hSelProcOutput->numSelectedDrcSets = hasDependend + 1; /* ducking overrides fading */

          hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
              hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
          /* force ducking DRC set to be processed on base layout */
          hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] = 0;
          hSelProcOutput->numSelectedDrcSets++;
          hasDucking = 1;
        }
      }
    }
  }

  /* repeat for DOWNMIX_ID_BASE_LAYOUT if no ducking found*/

  if (codecMode == SEL_PROC_MPEG_H_3DA) {
    if (hasDucking) {
      /* remove ducking */
      hasDucking = 0;
      hSelProcOutput->numSelectedDrcSets = hasDependend + hasFading + 1;
    }
  }

  if (!hasDucking) {
    for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
      DRC_INSTRUCTIONS_UNI_DRC* pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
      if (!_drcSetIsUsable(hUniDrcConfig, pInst)) continue;

      if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
        for (j = 0; j < pInst->downmixIdCount; j++) {
          if (pInst->downmixId[j] == DOWNMIX_ID_BASE_LAYOUT) {
            hasDucking = 0;

            if (pInst->drcInstructionsType == 0) {
              hasDucking = 1;
            } else if (pInst->drcInstructionsType == 2) {
              int m;

              for (m = 0; m < hSelProcInput->mpeghRequest.numGroupIdsRequested; m++) {
                if (hSelProcInput->mpeghRequest.groupIdRequested[m] == pInst->mae_groupID) {
                  hasDucking = 1;
                  break;
                }
              }
            } else if (pInst->drcInstructionsType == 3) {
              int m;

              for (m = 0; m < hSelProcInput->mpeghRequest.numGroupPresetIdsRequested; m++) {
                if (hSelProcInput->mpeghRequest.groupPresetIdRequested[m] ==
                    pInst->mae_groupPresetID) {
                  hasDucking = 1;
                  break;
                }
              }
            } else {
              retVal = DRCDEC_SELECTION_PROCESS_WARNING;
            }

            if (hasDucking) {
              hSelProcOutput->numSelectedDrcSets = hasDependend + 1; /* ducking overrides fading */
              hSelProcOutput->selectedDrcSetIds[hSelProcOutput->numSelectedDrcSets] =
                  hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId;
              /* force ducking DRC set to be processed on base layout */
              hSelProcOutput->selectedDownmixIds[hSelProcOutput->numSelectedDrcSets] = 0;
              hSelProcOutput->numSelectedDrcSets++;
            }
          }
        }
      }
    }
  }

  if (hSelProcOutput->numSelectedDrcSets > 3) {
    /* maximum permitted number of applied DRC sets is 3, see section 6.3.5 of ISO/IEC 23003-4 */
    hSelProcOutput->numSelectedDrcSets = 0;
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  /* sorting: Ducking/Fading -> Dependent -> Selected */
  if (hSelProcOutput->numSelectedDrcSets == 3) {
    selectedDrcSetIds = hSelProcOutput->selectedDrcSetIds[0];
    selectedDownmixIds = hSelProcOutput->selectedDownmixIds[0];
    hSelProcOutput->selectedDrcSetIds[0] = hSelProcOutput->selectedDrcSetIds[2];
    hSelProcOutput->selectedDownmixIds[0] = hSelProcOutput->selectedDownmixIds[2];
    hSelProcOutput->selectedDrcSetIds[2] = selectedDrcSetIds;
    hSelProcOutput->selectedDownmixIds[2] = selectedDownmixIds;
  } else if (hSelProcOutput->numSelectedDrcSets == 2) {
    selectedDrcSetIds = hSelProcOutput->selectedDrcSetIds[0];
    selectedDownmixIds = hSelProcOutput->selectedDownmixIds[0];
    hSelProcOutput->selectedDrcSetIds[0] = hSelProcOutput->selectedDrcSetIds[1];
    hSelProcOutput->selectedDownmixIds[0] = hSelProcOutput->selectedDownmixIds[1];
    hSelProcOutput->selectedDrcSetIds[1] = selectedDrcSetIds;
    hSelProcOutput->selectedDownmixIds[1] = selectedDownmixIds;
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _selectDownmixMatrix(HANDLE_SEL_PROC_OUTPUT hSelProcOutput,
                                                            HANDLE_UNI_DRC_CONFIG hUniDrcConfig) {
  int i;
  hSelProcOutput->baseChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  hSelProcOutput->targetChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  hSelProcOutput->targetLayout = -1;
  hSelProcOutput->downmixMatrixPresent = 0;

  if (hSelProcOutput->activeDownmixId != 0) {
    for (i = 0; i < hUniDrcConfig->downmixInstructionsCount; i++) {
      DOWNMIX_INSTRUCTIONS* pDown = &(hUniDrcConfig->downmixInstructions[i]);
      if (pDown->targetChannelCount > 28) {
        continue;
      }

      if (hSelProcOutput->activeDownmixId == pDown->downmixId) {
        hSelProcOutput->targetChannelCount = pDown->targetChannelCount;
        hSelProcOutput->targetLayout = pDown->targetLayout;

        break;
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetPreSelection(
    SEL_PROC_INPUT* hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected, SEL_PROC_CODEC_MODE codecMode) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int i, j;

  for (i = 0; i < hSelProcInput->numDownmixIdRequests; i++) {
    for (j = 0; j < hUniDrcConfig->drcInstructionsCountInclVirtual; j++) {
      DRC_INSTRUCTIONS_UNI_DRC* pDrcInstruction = &(hUniDrcConfig->drcInstructionsUniDrc[j]);
      /* check if ID is unique */
      if (selectDrcInstructions(hUniDrcConfig, pDrcInstruction->drcSetId) != pDrcInstruction)
        continue;

      retVal = _drcSetPreSelectionSingleInstruction(
          hSelProcInput, i, hUniDrcConfig, hLoudnessInfoSet, pDrcInstruction,
          *ppCandidatesPotential, *ppCandidatesSelected, codecMode);
      if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;
    }
  }

  retVal = _preSelectionRequirement9(hSelProcInput, *ppCandidatesPotential, *ppCandidatesSelected);
  if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;

  if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
    retVal =
        _drcSetSelectionAddCandidates(hSelProcInput, *ppCandidatesPotential, *ppCandidatesSelected);
    if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  if (codecMode == SEL_PROC_MPEG_H_3DA) {
    int omitPreSelectionBasedOnRequestedGroupId = 0;
    _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
    retVal = _preSelectionRequirement10(*ppCandidatesPotential, *ppCandidatesSelected,
                                        hSelProcInput->mpeghRequest.numGroupPresetIdsRequested,
                                        hSelProcInput->mpeghRequest.groupPresetIdRequested,
                                        &omitPreSelectionBasedOnRequestedGroupId);
    if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
    retVal = _preSelectionRequirement11(*ppCandidatesPotential, *ppCandidatesSelected,
                                        hSelProcInput->mpeghRequest.numGroupIdsRequested,
                                        hSelProcInput->mpeghRequest.groupIdRequested,
                                        omitPreSelectionBasedOnRequestedGroupId);
    if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  return retVal;
}

static DRCDEC_SELECTION_PROCESS_RETURN _drcSetRequestSelection(
    SEL_PROC_INPUT* hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION** ppCandidatesPotential,
    DRCDEC_SELECTION** ppCandidatesSelected) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal;
  int i;

  if (_drcdec_selection_getNumber(*ppCandidatesPotential) == 0) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  if (hSelProcInput->dynamicRangeControlOn) {
    if (hSelProcInput->numDrcFeatureRequests == 0) {
      retVal = _selectDrcSetEffectNone(*ppCandidatesPotential, *ppCandidatesSelected);
      if (retVal) return (retVal);

      if (_drcdec_selection_getNumber(*ppCandidatesSelected) == 0) {
        DRC_FEATURE_REQUEST fallbackRequest;
        fallbackRequest.drcEffectType.numRequests = 5;
        fallbackRequest.drcEffectType.numRequestsDesired = 5;
        fallbackRequest.drcEffectType.request[0] = DETR_GENERAL_COMPR;
        fallbackRequest.drcEffectType.request[1] = DETR_NIGHT;
        fallbackRequest.drcEffectType.request[2] = DETR_NOISY;
        fallbackRequest.drcEffectType.request[3] = DETR_LIMITED;
        fallbackRequest.drcEffectType.request[4] = DETR_LOWLEVEL;

        retVal = _selectEffectTypeFeature(hUniDrcConfig, fallbackRequest, ppCandidatesPotential,
                                          ppCandidatesSelected);
        if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }

      _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
    } else {
      for (i = 0; i < hSelProcInput->numDrcFeatureRequests; i++) {
        if (hSelProcInput->drcFeatureRequestType[i] == DFRT_EFFECT_TYPE) {
          retVal = _selectEffectTypeFeature(hUniDrcConfig, hSelProcInput->drcFeatureRequest[i],
                                            ppCandidatesPotential, ppCandidatesSelected);

          _swapSelectionAndClear(ppCandidatesPotential, ppCandidatesSelected);
          if (retVal) return DRCDEC_SELECTION_PROCESS_NOT_OK;
        }
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/
/*******************************************/

static DRCDEC_SELECTION_DATA* _drcdec_selection_addNew(DRCDEC_SELECTION* pSelection) {
  if (pSelection->numData < (32 + 1)) {
    DRCDEC_SELECTION_DATA* pData = &(pSelection->data[pSelection->numData]);
    FDKmemset(pData, 0, sizeof(DRCDEC_SELECTION_DATA));
    pSelection->numData++;

    return pData;
  } else {
    return NULL;
  }
}

static DRCDEC_SELECTION_DATA* _drcdec_selection_add(DRCDEC_SELECTION* pSelection,
                                                    DRCDEC_SELECTION_DATA* pDataIn) {
  if (pSelection->numData < (32 + 1)) {
    DRCDEC_SELECTION_DATA* pData = &(pSelection->data[pSelection->numData]);
    FDKmemcpy(pData, pDataIn, sizeof(DRCDEC_SELECTION_DATA));
    pSelection->numData++;
    return pData;
  } else {
    return NULL;
  }
}

static int _drcdec_selection_clear(DRCDEC_SELECTION* pSelection) {
  return pSelection->numData = 0;
}

static int _drcdec_selection_getNumber(DRCDEC_SELECTION* pSelection) {
  return pSelection->numData;
}

static int _drcdec_selection_setNumber(DRCDEC_SELECTION* pSelection, int num) {
  if (num >= 0 && num < pSelection->numData) {
    return pSelection->numData = num;
  } else {
    return pSelection->numData;
  }
}

static DRCDEC_SELECTION_DATA* _drcdec_selection_getAt(DRCDEC_SELECTION* pSelection, int at) {
  if (at >= 0 && at < (32 + 1)) {
    return &(pSelection->data[at]);
  } else {
    return NULL;
  }
}

static int _swapSelectionAndClear(DRCDEC_SELECTION** ppCandidatesPotential,
                                  DRCDEC_SELECTION** ppCandidatesSelected) {
  DRCDEC_SELECTION* pTmp = *ppCandidatesPotential;
  *ppCandidatesPotential = *ppCandidatesSelected;
  *ppCandidatesSelected = pTmp;
  _drcdec_selection_clear(*ppCandidatesSelected);
  return 0;
}

static int _swapSelection(DRCDEC_SELECTION** ppCandidatesPotential,
                          DRCDEC_SELECTION** ppCandidatesSelected) {
  DRCDEC_SELECTION* pTmp = *ppCandidatesPotential;
  *ppCandidatesPotential = *ppCandidatesSelected;
  *ppCandidatesSelected = pTmp;
  return 0;
}

/*******************************************/

static LOUDNESS_INFO* _getLoudnessInfoStructure(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                int drcSetId, int downmixId, int groupIdRequested,
                                                int groupPresetIdRequested,
                                                MPEGH_REQUEST* mpeghRequest,
                                                SEL_PROC_CODEC_MODE codecMode, int albumMode) {
  int i, j;
  int count;

  LOUDNESS_INFO* loudnessInfoMatching[32] = {NULL};
  int infoCount = 0;

  LOUDNESS_INFO* pLoudnessInfo = NULL;

  if (albumMode) {
    count = hLoudnessInfoSet->loudnessInfoAlbumCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfoAlbum;
  } else {
    count = hLoudnessInfoSet->loudnessInfoCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  }

  for (i = 0; i < count; i++) {
    if ((pLoudnessInfo[i].drcSetId == drcSetId) && (pLoudnessInfo[i].downmixId == downmixId)) {
      for (j = 0; j < pLoudnessInfo[i].measurementCount; j++) {
        if ((pLoudnessInfo[i].loudnessMeasurement[j].methodDefinition == 1) ||
            (pLoudnessInfo[i].loudnessMeasurement[j].methodDefinition == 2)) {
          loudnessInfoMatching[infoCount] = &pLoudnessInfo[i];
          infoCount++;
          break;
        }
      }
    }
  }

  if (codecMode == SEL_PROC_MPEG_H_3DA) {
    int n = 0, k = 0, l = 0, m = 0;
    int loudnessInfoType3Match[32];
    int loudnessInfoType3MatchNumMembers[32];
    int loudnessInfoType2Match[32];
    int loudnessInfoType0Match[32];
    int maxNumMembersGroupPresetIds = 0;
    int maxGroupId = 0;
    int largestGroupPresetId = 0;

    /* categorize loudnessInfoMatching dependent on loudnessInfoType */
    for (n = 0; n < infoCount; n++) {
      if (loudnessInfoMatching[n]->loudnessInfoType == 3) {
        for (i = 0; i < mpeghRequest->numGroupPresetIdsRequested; i++) {
          if (mpeghRequest->groupPresetIdRequested[i] ==
              loudnessInfoMatching[n]->mae_groupPresetID) {
            loudnessInfoType3Match[k] = n;
            loudnessInfoType3MatchNumMembers[k] =
                mpeghRequest->numMembersGroupPresetIdsRequested[i];
            k++;
            break;
          }
        }
      } else if (loudnessInfoMatching[n]->loudnessInfoType == 2) {
        for (i = 0; i < mpeghRequest->numGroupIdsRequested; i++) {
          if (mpeghRequest->groupIdRequested[i] == loudnessInfoMatching[n]->mae_groupID) {
            loudnessInfoType2Match[l] = n;
            l++;
            break;
          }
        }
      } else if (loudnessInfoMatching[n]->loudnessInfoType == 0) {
        loudnessInfoType0Match[m] = n;
        m++;
      }
    }

    infoCount = 0;

    /* groupPresetId match */
    if (k != 0) {
      /* full match */
      if (groupPresetIdRequested != -1) {
        for (n = 0; n < k; n++) {
          if (groupPresetIdRequested ==
              loudnessInfoMatching[loudnessInfoType3Match[n]]->mae_groupPresetID) {
            loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType3Match[n]];
            infoCount++;
          }
        }
      }

      /* preference match */
      if (infoCount == 0 && mpeghRequest->groupPresetIdRequestedPreference != -1) {
        for (n = 0; n < k; n++) {
          if (mpeghRequest->groupPresetIdRequestedPreference ==
              loudnessInfoMatching[loudnessInfoType3Match[n]]->mae_groupPresetID) {
            loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType3Match[n]];
            infoCount++;
          }
        }
      }

      /* other match */
      if (infoCount == 0) {
        for (n = 0; n < k; n++) {
          /* choose groupPreset with largest number of members */
          if (maxNumMembersGroupPresetIds <= loudnessInfoType3MatchNumMembers[n]) {
            if (maxNumMembersGroupPresetIds < loudnessInfoType3MatchNumMembers[n]) {
              maxNumMembersGroupPresetIds = loudnessInfoType3MatchNumMembers[n];
              infoCount = 0;
            }

            /* choose largest groupPresetId */
            if (largestGroupPresetId <=
                loudnessInfoMatching[loudnessInfoType3Match[n]]->mae_groupPresetID) {
              if (largestGroupPresetId <
                  loudnessInfoMatching[loudnessInfoType3Match[n]]->mae_groupPresetID) {
                largestGroupPresetId =
                    loudnessInfoMatching[loudnessInfoType3Match[n]]->mae_groupPresetID;
                infoCount = 0;
              }

              loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType3Match[n]];
              infoCount++;
            }
          }
        }
      }
    }

    /* groupId match */
    if (infoCount == 0 && l != 0) {
      /* full match */
      if (groupIdRequested != -1) {
        for (n = 0; n < l; n++) {
          if (groupIdRequested == loudnessInfoMatching[loudnessInfoType2Match[n]]->mae_groupID) {
            loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType2Match[n]];
            infoCount++;
          }
        }
      }

      /* other match */
      if (infoCount == 0) {
        for (n = 0; n < l; n++) {
          if (maxGroupId <= loudnessInfoMatching[loudnessInfoType2Match[n]]->mae_groupID) {
            if (maxGroupId < loudnessInfoMatching[loudnessInfoType2Match[n]]->mae_groupID) {
              maxGroupId = loudnessInfoMatching[loudnessInfoType2Match[n]]->mae_groupID;
              infoCount = 0;
            }

            loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType2Match[n]];
            infoCount++;
          }
        }
      }
    }

    /* other match without groupId and groupPresetId */
    if (infoCount == 0 && m != 0) {
      for (n = 0; n < m; n++) {
        loudnessInfoMatching[infoCount] = loudnessInfoMatching[loudnessInfoType0Match[n]];
        infoCount++;
      }
    }
  }

  if (infoCount > 0) {
    return loudnessInfoMatching[0];
  }

  return NULL;
}

static LOUDNESS_INFO* _getApplicableLoudnessInfoStructure(
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, int drcSetId, int downmixIdRequested,
    int groupIdRequested, int groupPresetIdRequested, MPEGH_REQUEST* pMpeghRequest,
    SEL_PROC_CODEC_MODE codecMode, int albumMode) {
  LOUDNESS_INFO* pLoudnessInfo = NULL;

  /* default value */
  pLoudnessInfo =
      _getLoudnessInfoStructure(hLoudnessInfoSet, drcSetId, downmixIdRequested, groupIdRequested,
                                groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);

  /* fallback values */
  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, drcSetId, 0x7F, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0x3F, downmixIdRequested, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0, downmixIdRequested, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0x3F, 0x7F, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0, 0x7F, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, drcSetId, 0, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0x3F, 0, groupIdRequested,
                                  groupPresetIdRequested, pMpeghRequest, codecMode, albumMode);
  }

  if (pLoudnessInfo == NULL) {
    pLoudnessInfo =
        _getLoudnessInfoStructure(hLoudnessInfoSet, 0, 0, groupIdRequested, groupPresetIdRequested,
                                  pMpeghRequest, codecMode, albumMode);
  }

  return pLoudnessInfo;
}

/*******************************************/

typedef struct {
  FIXP_DBL value;
  int order;
} VALUE_ORDER;

static void _initValueOrder(VALUE_ORDER* pValue) {
  pValue->value = (FIXP_DBL)0;
  pValue->order = -1;
}

enum {
  MS_BONUS0 = 0,
  MS_BONUS1770,
  MS_BONUSUSER,
  MS_BONUSEXPERT,
  MS_RESA,
  MS_RESB,
  MS_RESC,
  MS_RESD,
  MS_RESE,
  MS_PROGRAMLOUDNESS,
  MS_PEAKLOUDNESS
};

static DRCDEC_SELECTION_PROCESS_RETURN _getMethodValue(VALUE_ORDER* pValueOrder, FIXP_DBL value,
                                                       int measurementSystem,
                                                       int measurementSystemRequested) {
  const int rows = 11;
  const int columns = 12;
  const int pOrdering[rows][columns] = {
      {0, 0, 8, 0, 1, 3, 0, 5, 6, 7, 4, 2}, /* default = bonus1770 */
      {0, 0, 8, 0, 1, 3, 0, 5, 6, 7, 4, 2}, /* bonus1770 */
      {0, 0, 1, 0, 8, 5, 0, 2, 3, 4, 6, 7}, /* bonusUser */
      {0, 0, 3, 0, 1, 8, 0, 4, 5, 6, 7, 2}, /* bonusExpert */
      {0, 0, 5, 0, 1, 3, 0, 8, 6, 7, 4, 2}, /* ResA */
      {0, 0, 5, 0, 1, 3, 0, 6, 8, 7, 4, 2}, /* ResB */
      {0, 0, 5, 0, 1, 3, 0, 6, 7, 8, 4, 2}, /* ResC */
      {0, 0, 3, 0, 1, 7, 0, 4, 5, 6, 8, 2}, /* ResD */
      {0, 0, 1, 0, 7, 5, 0, 2, 3, 4, 6, 8}, /* ResE */
      {0, 0, 1, 0, 0, 0, 0, 2, 3, 4, 0, 0}, /* ProgramLoudness */
      {0, 7, 0, 0, 0, 0, 6, 5, 4, 3, 2, 1}  /* PeakLoudness */
  };

  if (measurementSystemRequested < 0 || measurementSystemRequested >= rows ||
      measurementSystem < 0 || measurementSystem >= columns) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  if (pOrdering[measurementSystemRequested][measurementSystem] > pValueOrder->order) {
    pValueOrder->order = pOrdering[measurementSystemRequested][measurementSystem];
    pValueOrder->value = value;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _getLoudness(
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, int albumMode,
    METHOD_DEFINITION_REQUEST measurementMethodRequested,
    MEASUREMENT_SYSTEM_REQUEST measurementSystemRequested, FIXP_DBL targetLoudness, /* e = 7 */
    int drcSetId, int downmixIdRequested, SEL_PROC_CODEC_MODE codecMode, int groupIdRequested,
    int groupPresetIdRequested, MPEGH_REQUEST* pMpeghRequest,
    FIXP_DBL* pLoudnessNormalizationGain, /* e = 7 */
    FIXP_DBL* pLoudness)                  /* e = 7 */
{
  int index;

  LOUDNESS_INFO* pLoudnessInfo = NULL;
  VALUE_ORDER valueOrder;

  /* map MDR_DEFAULT to MDR_PROGRAM_LOUDNESS */
  METHOD_DEFINITION_REQUEST requestedMethodDefinition =
      measurementMethodRequested < MDR_ANCHOR_LOUDNESS ? MDR_PROGRAM_LOUDNESS : MDR_ANCHOR_LOUDNESS;

  if (measurementMethodRequested > MDR_ANCHOR_LOUDNESS) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  }

  _initValueOrder(&valueOrder);

  *pLoudness = UNDEFINED_LOUDNESS_VALUE;
  *pLoudnessNormalizationGain = (FIXP_DBL)0;

  if (drcSetId < 0) {
    drcSetId = 0;
  }

  pLoudnessInfo = _getApplicableLoudnessInfoStructure(
      hLoudnessInfoSet, drcSetId, downmixIdRequested, groupIdRequested, groupPresetIdRequested,
      pMpeghRequest, codecMode, albumMode);

  if (albumMode && (pLoudnessInfo == NULL)) {
    pLoudnessInfo = _getApplicableLoudnessInfoStructure(
        hLoudnessInfoSet, drcSetId, downmixIdRequested, groupIdRequested, groupPresetIdRequested,
        pMpeghRequest, codecMode, 0);
  }

  if (pLoudnessInfo == NULL) {
    return DRCDEC_SELECTION_PROCESS_NO_ERROR;
  }

  index = -1;

  do {
    index = _findMethodDefinition(pLoudnessInfo, requestedMethodDefinition, index + 1);

    if (index >= 0) {
      _getMethodValue(&valueOrder, pLoudnessInfo->loudnessMeasurement[index].methodValue,
                      pLoudnessInfo->loudnessMeasurement[index].measurementSystem,
                      measurementSystemRequested);
    }
  } while (index >= 0);

  /* repeat with other method definition */
  if (valueOrder.order == -1) {
    index = -1;

    do {
      index = _findMethodDefinition(pLoudnessInfo,
                                    requestedMethodDefinition == MDR_PROGRAM_LOUDNESS
                                        ? MDR_ANCHOR_LOUDNESS
                                        : MDR_PROGRAM_LOUDNESS,
                                    index + 1);

      if (index >= 0) {
        _getMethodValue(&valueOrder, pLoudnessInfo->loudnessMeasurement[index].methodValue,
                        pLoudnessInfo->loudnessMeasurement[index].measurementSystem,
                        measurementSystemRequested);
      }
    } while (index >= 0);
  }

  if (valueOrder.order == -1) {
    return DRCDEC_SELECTION_PROCESS_NOT_OK;
  } else {
    *pLoudnessNormalizationGain = targetLoudness - valueOrder.value;
    *pLoudness = valueOrder.value;
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/

static int _getTruePeakLevel(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, int drcSetId, int downmixId,
                             int albumMode, FIXP_DBL* pTruePeakLevel, SEL_PROC_CODEC_MODE codecMode,
                             int drcInstructionsType, int groupPresetID, int groupID) {
  int i;
  int count;
  LOUDNESS_INFO* pLoudnessInfo = NULL;

  if (albumMode) {
    count = hLoudnessInfoSet->loudnessInfoAlbumCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfoAlbum;
  } else {
    count = hLoudnessInfoSet->loudnessInfoCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  }

  for (i = 0; i < count; i++) {
    if ((pLoudnessInfo[i].drcSetId == drcSetId) && (pLoudnessInfo[i].downmixId == downmixId)) {
      if ((codecMode != SEL_PROC_MPEG_H_3DA) ||
          ((drcInstructionsType == 3 && pLoudnessInfo[i].loudnessInfoType == 3 &&
            groupPresetID == pLoudnessInfo[i].mae_groupPresetID) ||
           (drcInstructionsType == 2 && pLoudnessInfo[i].loudnessInfoType == 2 &&
            groupID == pLoudnessInfo[i].mae_groupID) ||
           (drcInstructionsType == 0 && pLoudnessInfo[i].loudnessInfoType == 0)))

        if (pLoudnessInfo[i].truePeakLevelPresent) {
          *pTruePeakLevel = pLoudnessInfo[i].truePeakLevel;
          return 1; /* value found */
        }
    }
  }

  return 0; /* no value found */
}

static int _getSamplePeakLevel(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, int drcSetId,
                               int downmixId, int albumMode, FIXP_DBL* pSamplePeakLevel /* e = 7 */
                               ,
                               SEL_PROC_CODEC_MODE codecMode, int drcInstructionsType,
                               int groupPresetID, int groupID) {
  int i;
  int count;
  LOUDNESS_INFO* pLoudnessInfo = NULL;

  if (albumMode) {
    count = hLoudnessInfoSet->loudnessInfoAlbumCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfoAlbum;
  } else {
    count = hLoudnessInfoSet->loudnessInfoCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  }

  for (i = 0; i < count; i++) {
    if ((pLoudnessInfo[i].drcSetId == drcSetId) && (pLoudnessInfo[i].downmixId == downmixId)) {
      if ((codecMode != SEL_PROC_MPEG_H_3DA) ||
          ((drcInstructionsType == 3 && pLoudnessInfo[i].loudnessInfoType == 3 &&
            groupPresetID == pLoudnessInfo[i].mae_groupPresetID) ||
           (drcInstructionsType == 2 && pLoudnessInfo[i].loudnessInfoType == 2 &&
            groupID == pLoudnessInfo[i].mae_groupID) ||
           (drcInstructionsType == 0 && pLoudnessInfo[i].loudnessInfoType == 0)))

        if (pLoudnessInfo[i].samplePeakLevelPresent) {
          *pSamplePeakLevel = pLoudnessInfo[i].samplePeakLevel;
          return 1; /* value found */
        }
    }
  }

  return 0; /* no value found */
}

static int _getLimiterPeakTarget(DRC_INSTRUCTIONS_UNI_DRC* pDrcInstruction, int downmixId,
                                 FIXP_DBL* pLimiterPeakTarget) {
  int i;

  if (pDrcInstruction->limiterPeakTargetPresent) {
    if ((pDrcInstruction->downmixId[0] == downmixId) || (pDrcInstruction->downmixId[0] == 0x7F)) {
      *pLimiterPeakTarget = ((FX_SGL2FX_DBL(pDrcInstruction->limiterPeakTarget) >> 2));
      return 1; /* value found */
    }

    for (i = 0; i < pDrcInstruction->downmixIdCount; i++) {
      if (pDrcInstruction->downmixId[i] == downmixId) {
        *pLimiterPeakTarget = ((FX_SGL2FX_DBL(pDrcInstruction->limiterPeakTarget) >> 2));
        return 1; /* value found */
      }
    }
  }

  return 0; /* no value found */
}

static int _getPeakVirtualDrcFromLoudnessInfo(LOUDNESS_INFO* loudnessInfo,
                                              MPEGH_REQUEST* pMpeghRequest,
                                              FIXP_DBL* signalPeakLevel,
                                              int* explicitPeakInformationPresent) {
  int n;

  if (loudnessInfo->truePeakLevelPresent) {
    *signalPeakLevel = loudnessInfo->truePeakLevel;

    if (explicitPeakInformationPresent != NULL) *explicitPeakInformationPresent = 1;

    if (loudnessInfo->loudnessInfoType == 3 &&
        pMpeghRequest->groupPresetIdRequestedPreference == loudnessInfo->mae_groupPresetID) {
      return 1;
    }

    if (loudnessInfo->loudnessInfoType == 3) {
      for (n = 0; n < pMpeghRequest->numGroupPresetIdsRequested; n++) {
        if (pMpeghRequest->groupPresetIdRequested[n] == loudnessInfo->mae_groupPresetID) {
          return 1;
        }
      }
    }

    if (loudnessInfo->loudnessInfoType == 2) {
      for (n = 0; n < pMpeghRequest->numGroupIdsRequested; n++) {
        if (pMpeghRequest->groupIdRequested[n] == loudnessInfo->mae_groupID) {
          return 1;
        }
      }
    }
  }

  if (loudnessInfo->samplePeakLevelPresent) {
    *signalPeakLevel = loudnessInfo->samplePeakLevel;

    if (explicitPeakInformationPresent != NULL) *explicitPeakInformationPresent = 1;

    if (loudnessInfo->loudnessInfoType == 3 &&
        pMpeghRequest->groupPresetIdRequestedPreference == loudnessInfo->mae_groupPresetID) {
      return 1;
    }

    if (loudnessInfo->loudnessInfoType == 3) {
      for (n = 0; n < pMpeghRequest->numGroupPresetIdsRequested; n++) {
        if (pMpeghRequest->groupPresetIdRequested[n] == loudnessInfo->mae_groupPresetID) {
          return 1;
        }
      }
    }

    if (loudnessInfo->loudnessInfoType == 2) {
      for (n = 0; n < pMpeghRequest->numGroupIdsRequested; n++) {
        if (pMpeghRequest->groupIdRequested[n] == loudnessInfo->mae_groupID) {
          return 1;
        }
      }
    }
  }

  return 0;
}

static int _getPeakVirtualDrcFromLoudnessInfoSet(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                 int albumMode, int drcSetIdRequested,
                                                 int downmixIdRequested,
                                                 MPEGH_REQUEST* pMpeghRequest,
                                                 FIXP_DBL* signalPeakLevel,
                                                 int* explicitPeakInformationPresent) {
  int i;
  int matchFound = 0;
  int count;
  LOUDNESS_INFO* pLoudnessInfo = NULL;

  if (albumMode) {
    count = hLoudnessInfoSet->loudnessInfoAlbumCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfoAlbum;
  } else {
    count = hLoudnessInfoSet->loudnessInfoCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  }

  for (i = 0; i < count; i++) {
    if (drcSetIdRequested == pLoudnessInfo[i].drcSetId &&
        downmixIdRequested == pLoudnessInfo[i].downmixId) {
      matchFound = _getPeakVirtualDrcFromLoudnessInfo(
          &(pLoudnessInfo[i]), pMpeghRequest, signalPeakLevel, explicitPeakInformationPresent);

      if (matchFound) break;
    }
  }

  return matchFound;
}

static DRCDEC_SELECTION_PROCESS_RETURN _getSignalPeakLevel(
    HANDLE_SEL_PROC_INPUT hSelProcInput, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRC_INSTRUCTIONS_UNI_DRC* pInst,
    int downmixIdRequested, int* explicitPeakInformationPresent,
    FIXP_DBL* signalPeakLevelOut, /* e = 7 */
    SEL_PROC_CODEC_MODE codecMode

) {
  DRCDEC_SELECTION_PROCESS_RETURN retVal = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  int albumMode = hSelProcInput->albumMode;

  FIXP_DBL signalPeakLevelTmp = (FIXP_DBL)0;
  FIXP_DBL signalPeakLevel = (FIXP_DBL)0;

  int matchFound = 0;
  int dmxId = downmixIdRequested;
  int drcSetId = pInst->drcSetId;

  if (drcSetId < 0) {
    drcSetId = 0;
  }

  *explicitPeakInformationPresent = 1;

  if (codecMode == SEL_PROC_MPEG_H_3DA) {
    if (drcSetId == 0) {
      matchFound = _getPeakVirtualDrcFromLoudnessInfoSet(
          hLoudnessInfoSet, albumMode, drcSetId, dmxId, &hSelProcInput->mpeghRequest,
          signalPeakLevelOut, explicitPeakInformationPresent);
    }

    if (matchFound) {
      return retVal; /* finished */
    }
  }

  {
    matchFound =
        _getTruePeakLevel(hLoudnessInfoSet, drcSetId, dmxId, albumMode, &signalPeakLevel, codecMode,
                          pInst->drcInstructionsType, pInst->mae_groupPresetID, pInst->mae_groupID);
  }
  if (!matchFound) {
    matchFound = _getSamplePeakLevel(hLoudnessInfoSet, drcSetId, dmxId, albumMode, &signalPeakLevel,
                                     codecMode, pInst->drcInstructionsType,
                                     pInst->mae_groupPresetID, pInst->mae_groupID);
  }
  if (!matchFound) {
    matchFound =
        _getTruePeakLevel(hLoudnessInfoSet, 0x3F, dmxId, albumMode, &signalPeakLevel, codecMode,
                          pInst->drcInstructionsType, pInst->mae_groupPresetID, pInst->mae_groupID);
  }
  if (!matchFound) {
    matchFound = _getSamplePeakLevel(hLoudnessInfoSet, 0x3F, dmxId, albumMode, &signalPeakLevel,
                                     codecMode, pInst->drcInstructionsType,
                                     pInst->mae_groupPresetID, pInst->mae_groupID);
  }
  if (!matchFound) {
    matchFound = _getLimiterPeakTarget(pInst, dmxId, &signalPeakLevel);
  }
  if (!matchFound && (dmxId != 0)) {
    FIXP_DBL downmixPeakLevelDB = 0;

    *explicitPeakInformationPresent = 0;

    signalPeakLevelTmp = (FIXP_DBL)0;

    if (codecMode == SEL_PROC_MPEG_H_3DA) {
      if (drcSetId == 0) {
        matchFound = _getPeakVirtualDrcFromLoudnessInfoSet(
            hLoudnessInfoSet, albumMode, drcSetId, dmxId, &hSelProcInput->mpeghRequest,
            &signalPeakLevelTmp, explicitPeakInformationPresent);
      }

      if (matchFound) {
        *signalPeakLevelOut = signalPeakLevelTmp + downmixPeakLevelDB;
        return retVal; /* finished */
      }
    }

    if (!matchFound) {
      matchFound = _getTruePeakLevel(hLoudnessInfoSet, drcSetId, 0, albumMode, &signalPeakLevelTmp,
                                     codecMode, pInst->drcInstructionsType,
                                     pInst->mae_groupPresetID, pInst->mae_groupID);
    }
    if (!matchFound) {
      matchFound = _getSamplePeakLevel(hLoudnessInfoSet, drcSetId, 0, albumMode,
                                       &signalPeakLevelTmp, codecMode, pInst->drcInstructionsType,
                                       pInst->mae_groupPresetID, pInst->mae_groupID);
    }
    if (!matchFound) {
      matchFound = _getTruePeakLevel(hLoudnessInfoSet, 0x3F, 0, albumMode, &signalPeakLevelTmp,
                                     codecMode, pInst->drcInstructionsType,
                                     pInst->mae_groupPresetID, pInst->mae_groupID);
    }
    if (!matchFound) {
      matchFound = _getSamplePeakLevel(hLoudnessInfoSet, 0x3F, 0, albumMode, &signalPeakLevelTmp,
                                       codecMode, pInst->drcInstructionsType,
                                       pInst->mae_groupPresetID, pInst->mae_groupID);
    }
    if (!matchFound) {
      matchFound = _getLimiterPeakTarget(pInst, 0, &signalPeakLevelTmp);
    }

    signalPeakLevel = signalPeakLevelTmp + downmixPeakLevelDB;
  } else if (!matchFound) {
    signalPeakLevel = (FIXP_DBL)0; /* worst case estimate */
    *explicitPeakInformationPresent = (FIXP_DBL)0;
  }

  *signalPeakLevelOut = signalPeakLevel;

  return retVal;
}

/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _selectAlbumLoudness(
    HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet, DRCDEC_SELECTION* pCandidatesPotential,
    DRCDEC_SELECTION* pCandidatesSelected) {
  int i, j;
  int v = 0; /* number of selected virtual DRC sets */

  for (i = 0; i < _drcdec_selection_getNumber(pCandidatesPotential); i++) {
    DRCDEC_SELECTION_DATA* pCandidate = _drcdec_selection_getAt(pCandidatesPotential, i);
    if (pCandidate == NULL) return DRCDEC_SELECTION_PROCESS_NOT_OK;

    if (pCandidate->pInst->drcSetId < 0) {
      /* keep virtual DRC sets */
      v++;
      if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
        return DRCDEC_SELECTION_PROCESS_NOT_OK;
      continue;
    }
    for (j = 0; j < hLoudnessInfoSet->loudnessInfoAlbumCount; j++) {
      if (pCandidate->pInst->drcSetId == hLoudnessInfoSet->loudnessInfoAlbum[j].drcSetId) {
        if (_drcdec_selection_add(pCandidatesSelected, pCandidate) == NULL)
          return DRCDEC_SELECTION_PROCESS_NOT_OK;
      }
    }
  }

  if (v == _drcdec_selection_getNumber(pCandidatesSelected)) {
    /* we have only virtual DRC sets. Dismiss the selection */
    _drcdec_selection_clear(pCandidatesSelected);
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/

static int _findMethodDefinition(LOUDNESS_INFO* pLoudnessInfo, int methodDefinition,
                                 int startIndex) {
  int i;
  int index = -1;

  for (i = startIndex; i < pLoudnessInfo->measurementCount; i++) {
    if (pLoudnessInfo->loudnessMeasurement[i].methodDefinition == methodDefinition) {
      index = i;
      break;
    }
  }

  return index;
}

/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _getMixingLevel(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                       int downmixIdRequested,
                                                       int drcSetIdRequested, int albumMode,
                                                       FIXP_DBL* pMixingLevel) {
  const FIXP_DBL mixingLevelDefault = FL2FXCONST_DBL(85.0f / (float)(1 << 7));

  int i;
  int count;

  LOUDNESS_INFO* pLoudnessInfo = NULL;

  *pMixingLevel = mixingLevelDefault;

  if (drcSetIdRequested < 0) {
    drcSetIdRequested = 0;
  }

  if (albumMode) {
    count = hLoudnessInfoSet->loudnessInfoAlbumCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfoAlbum;
  } else {
    count = hLoudnessInfoSet->loudnessInfoCount;
    pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  }

  for (i = 0; i < count; i++) {
    if ((drcSetIdRequested == pLoudnessInfo[i].drcSetId) &&
        ((downmixIdRequested == pLoudnessInfo[i].downmixId) ||
         (DOWNMIX_ID_ANY_DOWNMIX == pLoudnessInfo[i].downmixId))) {
      int index = _findMethodDefinition(&pLoudnessInfo[i], MD_MIXING_LEVEL, 0);

      if (index >= 0) {
        *pMixingLevel = pLoudnessInfo[i].loudnessMeasurement[index].methodValue;
        break;
      }
    }
  }

  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/

static DRCDEC_SELECTION_PROCESS_RETURN _getGroupLoudness(HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                                         HANDLE_SEL_PROC_OUTPUT hSelProcOutput) {
  /* collect group loudness infos from loudnessInfoSet into selection process output */
  int m, i;
  LOUDNESS_INFO* pLoudnessInfo = hLoudnessInfoSet->loudnessInfo;
  hSelProcOutput->groupIdLoudnessCount = 0;
  for (i = 0; i < hLoudnessInfoSet->loudnessInfoCount; i++) {
    if (pLoudnessInfo[i].loudnessInfoType == 1) /* group loudness */
    {
      if (pLoudnessInfo[i].downmixId != DOWNMIX_ID_BASE_LAYOUT) continue;
      if (pLoudnessInfo[i].drcSetId != DRC_SET_ID_NO_DRC) continue;

      for (m = 0; m < pLoudnessInfo[i].measurementCount; m++) {
        /* assumption: program and anchor loudness are equivalent for groups */
        if ((pLoudnessInfo[i].loudnessMeasurement[m].methodDefinition == MD_PROGRAM_LOUDNESS) ||
            (pLoudnessInfo[i].loudnessMeasurement[m].methodDefinition == MD_ANCHOR_LOUDNESS)) {
          hSelProcOutput->groupId[hSelProcOutput->groupIdLoudnessCount] =
              pLoudnessInfo[i].mae_groupID;
          hSelProcOutput->groupIdLoudness[hSelProcOutput->groupIdLoudnessCount] =
              pLoudnessInfo[i].loudnessMeasurement[m].methodValue;
          hSelProcOutput->groupIdLoudnessCount += 1;
          break;
        }
      }
    }
  }
  return DRCDEC_SELECTION_PROCESS_NO_ERROR;
}

/*******************************************/
