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

#include "drcDec_reader.h"
#include "drcDec_gainDecoder.h"
#include "FDK_drcDecLib.h"

#include "drcDec_selectionProcess.h"
#include "drcDec_tools.h"

/* Decoder library info */
#define DRCDEC_LIB_VL0 2
#define DRCDEC_LIB_VL1 8
#define DRCDEC_LIB_VL2 0
#define DRCDEC_LIB_TITLE "MPEG-D DRC Decoder Lib"
#ifdef __ANDROID__
#define DRCDEC_LIB_BUILD_DATE ""
#define DRCDEC_LIB_BUILD_TIME ""
#else
#define DRCDEC_LIB_BUILD_DATE __DATE__
#define DRCDEC_LIB_BUILD_TIME __TIME__
#endif

typedef enum {
  DRC_DEC_NOT_INITIALIZED = 0,
  DRC_DEC_INITIALIZED,
  DRC_DEC_NEW_GAIN_PAYLOAD,
  DRC_DEC_INTERPOLATION_PREPARED,
  DRCDEC_INCOMPLETE_UNIDRC
} DRC_DEC_STATUS;

struct s_drc_decoder {
  DRC_DEC_CODEC_MODE codecMode;
  DRC_DEC_FUNCTIONAL_RANGE functionalRange;
  DRC_DEC_STATUS status;

  /* handles of submodules */
  HANDLE_DRC_GAIN_DECODER hGainDec;
  HANDLE_DRC_SELECTION_PROCESS hSelectionProc;
  int selProcInputDiff;

  /* data structs */
  UNI_DRC_CONFIG uniDrcConfig;
  LOUDNESS_INFO_SET loudnessInfoSet;
  UNI_DRC_GAIN uniDrcGain;

  SEL_PROC_OUTPUT selProcOutput;
};

/* compare and assign */
static inline int _compAssign(UCHAR* dest, const UCHAR src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static int _getGainStatus(HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  return hUniDrcGain->status;
}

static int isResetNeeded(HANDLE_DRC_DECODER hDrcDec, const SEL_PROC_OUTPUT* oldSelProcOutput) {
  int i, resetNeeded = 0;

  if (hDrcDec->selProcOutput.numSelectedDrcSets != oldSelProcOutput->numSelectedDrcSets) {
    resetNeeded = 1;
  } else {
    for (i = 0; i < hDrcDec->selProcOutput.numSelectedDrcSets; i++) {
      if (hDrcDec->selProcOutput.selectedDrcSetIds[i] != oldSelProcOutput->selectedDrcSetIds[i])
        resetNeeded = 1;
      if (hDrcDec->selProcOutput.selectedDownmixIds[i] != oldSelProcOutput->selectedDownmixIds[i])
        resetNeeded = 1;
    }
  }

  if (hDrcDec->selProcOutput.boost != oldSelProcOutput->boost) resetNeeded = 1;
  if (hDrcDec->selProcOutput.compress != oldSelProcOutput->compress) resetNeeded = 1;

  /* Note: Changes in downmix matrix are not caught, as they don't affect the DRC gain decoder */

  return resetNeeded;
}

static void startSelectionProcess(HANDLE_DRC_DECODER hDrcDec) {
  int uniDrcConfigHasChanged = 0;

  if (!hDrcDec->status) return;

  C_ALLOC_SCRATCH_START(oldSelProcOutput, SEL_PROC_OUTPUT, 1);
  FDKmemcpy(oldSelProcOutput, &hDrcDec->selProcOutput, sizeof(SEL_PROC_OUTPUT));

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    uniDrcConfigHasChanged = hDrcDec->uniDrcConfig.diff;
    if (hDrcDec->uniDrcConfig.diff || hDrcDec->loudnessInfoSet.diff || hDrcDec->selProcInputDiff) {
      /* in case of an error, signal that selection process was not successful */
      hDrcDec->selProcOutput.numSelectedDrcSets = 0;

      drcDec_SelectionProcess_Process(hDrcDec->hSelectionProc, &(hDrcDec->uniDrcConfig),
                                      &(hDrcDec->loudnessInfoSet), &(hDrcDec->selProcOutput));

      hDrcDec->selProcInputDiff = 0;
      hDrcDec->uniDrcConfig.diff = 0;
      hDrcDec->loudnessInfoSet.diff = 0;
    }
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    if (isResetNeeded(hDrcDec, oldSelProcOutput) || uniDrcConfigHasChanged) {
      drcDec_GainDecoder_Config(
          hDrcDec->hGainDec, &(hDrcDec->uniDrcConfig), hDrcDec->selProcOutput.numSelectedDrcSets,
          hDrcDec->selProcOutput.selectedDrcSetIds, hDrcDec->selProcOutput.selectedDownmixIds);
    }
  }
  C_ALLOC_SCRATCH_END(oldSelProcOutput, SEL_PROC_OUTPUT, 1);
}

static UINT getScratchBufferSize() {
  /* scratchBufferSize is the necessary size of p_scratch buffer in bytes */
  UINT scratchBufferSize = 0;

  scratchBufferSize = fMax(scratchBufferSize, sizeof(DRC_INSTRUCTIONS_UNI_DRC));
  scratchBufferSize = fMax(scratchBufferSize, sizeof(DRC_COEFFICIENTS_UNI_DRC));
  scratchBufferSize = fMax(scratchBufferSize, sizeof(DOWNMIX_INSTRUCTIONS));

  return scratchBufferSize;
}

DRC_DEC_ERROR
FDK_drcDec_Open(HANDLE_DRC_DECODER* phDrcDec, const DRC_DEC_FUNCTIONAL_RANGE functionalRange) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  HANDLE_DRC_DECODER hDrcDec;

  *phDrcDec = (HANDLE_DRC_DECODER)FDKcalloc(1, sizeof(struct s_drc_decoder));
  if (!*phDrcDec) return DRC_DEC_OUT_OF_MEMORY;
  hDrcDec = *phDrcDec;

  hDrcDec->uniDrcConfig.p_scratch = (UINT*)FDKcalloc(1, getScratchBufferSize());
  if (!hDrcDec->uniDrcConfig.p_scratch) return DRC_DEC_OUT_OF_MEMORY;

  hDrcDec->functionalRange = functionalRange;

  hDrcDec->status = DRC_DEC_NOT_INITIALIZED;
  hDrcDec->codecMode = DRC_DEC_CODEC_MODE_UNDEFINED;

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    sErr = drcDec_SelectionProcess_Create(&(hDrcDec->hSelectionProc));
    if (sErr) return DRC_DEC_OUT_OF_MEMORY;
    sErr = drcDec_SelectionProcess_Init(hDrcDec->hSelectionProc);
    if (sErr) return DRC_DEC_NOT_OK;
    hDrcDec->selProcOutput.outputLoudness = UNDEFINED_LOUDNESS_VALUE;
    hDrcDec->selProcInputDiff = 1;
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    dErr = drcDec_GainDecoder_Open(&(hDrcDec->hGainDec));
    if (dErr) return DRC_DEC_OUT_OF_MEMORY;
  }

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_SetCodecMode(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_CODEC_MODE codecMode) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->codecMode ==
      DRC_DEC_CODEC_MODE_UNDEFINED) { /* Set codec mode, if it is set for the first time */
    hDrcDec->codecMode = codecMode;

    if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
      sErr = drcDec_SelectionProcess_SetCodecMode(hDrcDec->hSelectionProc,
                                                  (SEL_PROC_CODEC_MODE)codecMode);
      if (sErr) return DRC_DEC_NOT_OK;
      hDrcDec->selProcInputDiff = 1;
    }

    if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
      DELAY_MODE delayMode;
      int timeDomainSupported;
      SUBBAND_DOMAIN_MODE subbandDomainSupported;

      switch (hDrcDec->codecMode) {
        case DRC_DEC_MPEG_4_AAC:
        case DRC_DEC_MPEG_D_USAC:
        case DRC_DEC_MPEG_H_3DA:
        default:
          delayMode = DM_REGULAR_DELAY;
      }

      switch (hDrcDec->codecMode) {
        case DRC_DEC_MPEG_4_AAC:
        case DRC_DEC_MPEG_D_USAC:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_OFF;
          break;
        case DRC_DEC_MPEG_H_3DA:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_STFT256;
          break;

        case DRC_DEC_TEST_TIME_DOMAIN:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_OFF;
          break;
        case DRC_DEC_TEST_QMF_DOMAIN:
          timeDomainSupported = 0;
          subbandDomainSupported = SDM_QMF64;
          break;
        case DRC_DEC_TEST_STFT_DOMAIN:
          timeDomainSupported = 0;
          subbandDomainSupported = SDM_STFT256;
          break;

        default:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_OFF;
      }

      dErr = drcDec_GainDecoder_SetCodecDependentParameters(
          hDrcDec->hGainDec, delayMode, timeDomainSupported, subbandDomainSupported);
      if (dErr) return DRC_DEC_NOT_OK;
    }
  }

  /* Don't allow changing codecMode if it has already been set. */
  if (hDrcDec->codecMode != codecMode) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_Init(HANDLE_DRC_DECODER hDrcDec, const int frameSize, const int sampleRate,
                const int baseChannelCount) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hDrcDec == NULL || frameSize == 0 || sampleRate == 0 || baseChannelCount == 0)
    return DRC_DEC_OK; /* return without doing anything */

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    sErr =
        drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_BASE_CHANNEL_COUNT,
                                         (FIXP_DBL)baseChannelCount, &(hDrcDec->selProcInputDiff));
    if (sErr) return DRC_DEC_NOT_OK;
    sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_SAMPLE_RATE,
                                            (FIXP_DBL)sampleRate, &(hDrcDec->selProcInputDiff));
    if (sErr) return DRC_DEC_NOT_OK;
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    dErr = drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_FRAME_SIZE, frameSize);
    if (dErr) return DRC_DEC_NOT_OK;
    dErr = drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_SAMPLE_RATE, sampleRate);
    if (dErr) return DRC_DEC_NOT_OK;
    dErr = drcDec_GainDecoder_Init(hDrcDec->hGainDec);
    if (dErr) return DRC_DEC_NOT_OK;
  }

  hDrcDec->status = DRC_DEC_INITIALIZED;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_Close(HANDLE_DRC_DECODER* phDrcDec) {
  HANDLE_DRC_DECODER hDrcDec;

  if (phDrcDec == NULL) {
    return DRC_DEC_OK;
  }

  hDrcDec = *phDrcDec;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    drcDec_GainDecoder_Close(&(hDrcDec->hGainDec));
  }

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    drcDec_SelectionProcess_Delete(&(hDrcDec->hSelectionProc));
  }

  FDKfree(hDrcDec->uniDrcConfig.p_scratch);
  hDrcDec->uniDrcConfig.p_scratch = NULL;

  FDKfree(*phDrcDec);
  *phDrcDec = NULL;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_SetParam(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_USERPARAM requestType,
                    const FIXP_DBL requestValue) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int invalidParameter = 0;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    switch (requestType) {
      case DRC_DEC_SAMPLE_RATE:
        dErr =
            drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_SAMPLE_RATE, (int)requestValue);
        if (dErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_FRAME_SIZE:
        dErr =
            drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_FRAME_SIZE, (int)requestValue);
        if (dErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      default:
        invalidParameter |= DRC_DEC_GAIN;
    }
  }

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    switch (requestType) {
      case DRC_DEC_BOOST:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_BOOST,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_COMPRESS:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_COMPRESS,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_LOUDNESS_NORMALIZATION_ON:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc,
                                                SEL_PROC_LOUDNESS_NORMALIZATION_ON, requestValue,
                                                &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_TARGET_LOUDNESS:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_TARGET_LOUDNESS,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_EFFECT_TYPE:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_EFFECT_TYPE,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_EFFECT_TYPE_FALLBACK_CODE:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc,
                                                SEL_PROC_EFFECT_TYPE_FALLBACK_CODE, requestValue,
                                                &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_DOWNMIX_ID:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_DOWNMIX_ID,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_ALBUM_MODE:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc, SEL_PROC_ALBUM_MODE,
                                                requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      default:
        invalidParameter |= DRC_DEC_SELECTION;
    }
  }

  if (invalidParameter == hDrcDec->functionalRange) return DRC_DEC_INVALID_PARAM;

  /* All parameters need a new start of the selection process */
  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

LONG FDK_drcDec_GetParam(HANDLE_DRC_DECODER hDrcDec, const DRC_DEC_USERPARAM requestType) {
  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  switch (requestType) {
    case DRC_DEC_IS_MULTIBAND_DRC_1:
      return (LONG)bitstreamContainsMultibandDrc(&hDrcDec->uniDrcConfig, 0);
    case DRC_DEC_IS_ACTIVE: {
      /* MPEG-D DRC overrides MPEG-4 DRC, if uniDrc payload is present (loudnessInfoSet and/or
       * uniDrcConfig) */
      int uniDrcPayloadPresent = (hDrcDec->loudnessInfoSet.loudnessInfoCount > 0);
      uniDrcPayloadPresent |= (hDrcDec->loudnessInfoSet.loudnessInfoAlbumCount > 0);
      uniDrcPayloadPresent |= (hDrcDec->uniDrcConfig.drcInstructionsUniDrcCount > 0);
      uniDrcPayloadPresent |= (hDrcDec->uniDrcConfig.downmixInstructionsCount > 0);

      return (LONG)(uniDrcPayloadPresent);
    }
    case DRC_DEC_TARGET_LAYOUT_SELECTED:
      return (LONG)hDrcDec->selProcOutput.targetLayout;
    case DRC_DEC_TARGET_CHANNEL_COUNT_SELECTED:
      return (LONG)hDrcDec->selProcOutput.targetChannelCount;
    case DRC_DEC_OUTPUT_LOUDNESS:
      return (LONG)hDrcDec->selProcOutput.outputLoudness;
    case DRC_DEC_IS_MUTE: {
      /* If uniDrc() payload is present, but no loudnessInfoSet contained so far,
         DRC decoder output shall be mute for 2.5 seconds after startup. */
      if (hDrcDec->status == DRCDEC_INCOMPLETE_UNIDRC) {
        return (LONG)(drcDec_GainDecoder_GetStartupPhase(hDrcDec->hGainDec));
      } else {
        return 0;
      }
    }
    default:
      return 0;
  }
}

DRC_DEC_ERROR
FDK_drcDec_SetInterfaceParameters(HANDLE_DRC_DECODER hDrcDec,
                                  HANDLE_UNI_DRC_INTERFACE hUniDrcInterface) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}

DRC_DEC_ERROR
FDK_drcDec_SetSelectionProcessMpeghParameters(
    HANDLE_DRC_DECODER hDrcDec, const int numGroupIdsRequested, const int* groupIdsRequested,
    const int numGroupPresetIdsRequested, const int* groupPresetIdsRequested,
    const int* numMembersGroupPresetIdsRequested, const int groupPresetIdRequestedPreference) {
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (hDrcDec->functionalRange == DRC_DEC_GAIN) return DRC_DEC_NOT_OK;

  sErr = drcDec_SelectionProcess_SetMpeghParams(
      hDrcDec->hSelectionProc, numGroupIdsRequested, groupIdsRequested, numGroupPresetIdsRequested,
      groupPresetIdsRequested, numMembersGroupPresetIdsRequested, groupPresetIdRequestedPreference,
      &(hDrcDec->selProcInputDiff));
  if (sErr) return DRC_DEC_NOT_OK;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_SetSelectionProcessMpeghParameters_simple(HANDLE_DRC_DECODER hDrcDec,
                                                     const int groupPresetIdRequested,
                                                     const int numGroupIdsRequested,
                                                     const int* groupIdsRequested) {
  DRC_DEC_ERROR err = DRC_DEC_OK;

  int numGroupPresetIdsRequested = 1, numMembersGroupPresetIdsRequested = 1;

  if (groupPresetIdRequested < 0) numGroupPresetIdsRequested = 0;

  err = FDK_drcDec_SetSelectionProcessMpeghParameters(
      hDrcDec, numGroupIdsRequested, groupIdsRequested, numGroupPresetIdsRequested,
      &groupPresetIdRequested, &numMembersGroupPresetIdsRequested, groupPresetIdRequested);

  return err;
}

DRC_DEC_ERROR
FDK_drcDec_SetDownmixInstructions(HANDLE_DRC_DECODER hDrcDec, const int numDownmixId,
                                  const int* downmixId, const int* targetLayout,
                                  const int* targetChannelCount) {
  int i, diff = 0;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (numDownmixId > 32) return DRC_DEC_NOT_OK;

  diff |= _compAssign(&hDrcDec->uniDrcConfig.downmixInstructionsCount, numDownmixId);

  for (i = 0; i < numDownmixId; i++) {
    diff |= _compAssign(&hDrcDec->uniDrcConfig.downmixInstructions[i].downmixId, downmixId[i]);
    diff |=
        _compAssign(&hDrcDec->uniDrcConfig.downmixInstructions[i].targetLayout, targetLayout[i]);
    diff |= _compAssign(&hDrcDec->uniDrcConfig.downmixInstructions[i].targetChannelCount,
                        targetChannelCount[i]);
    diff |=
        _compAssign(&hDrcDec->uniDrcConfig.downmixInstructions[i].downmixCoefficientsPresent, 0);
  }

  hDrcDec->selProcInputDiff |= diff;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

void FDK_drcDec_SetSelectionProcessOutput(HANDLE_DRC_DECODER hDrcDec,
                                          HANDLE_SEL_PROC_OUTPUT hSelProcOutput) {
  return;
}

HANDLE_SEL_PROC_OUTPUT
FDK_drcDec_GetSelectionProcessOutput(HANDLE_DRC_DECODER hDrcDec) {
  if (hDrcDec == NULL) return NULL;

  return &(hDrcDec->selProcOutput);
}

LONG /* FIXP_DBL, e = 7 */
FDK_drcDec_GetGroupLoudness(HANDLE_SEL_PROC_OUTPUT hSelProcOutput, const int groupID,
                            int* groupLoudnessAvailable) {
  int j;

  *groupLoudnessAvailable = 0;
  if (hSelProcOutput == NULL) return (LONG)0;

  for (j = 0; j < hSelProcOutput->groupIdLoudnessCount; j++) {
    if (hSelProcOutput->groupId[j] == groupID) {
      *groupLoudnessAvailable = 1;
      return hSelProcOutput->groupIdLoudness[j];
    }
  }

  /* groupLoudness not found */
  return (LONG)0;
}

void FDK_drcDec_SetChannelGains(HANDLE_DRC_DECODER hDrcDec, const int updateLNgain,
                                const int numChannels, const int frameSize, FIXP_DBL* channelGainDb,
                                FIXP_DBL* audioBuffer, const int audioBufferChannelOffset) {
  int err;

  if (hDrcDec == NULL) return;

  if (updateLNgain) {
    err = drcDec_GainDecoder_SetLoudnessNormalizationGainDb(
        hDrcDec->hGainDec, hDrcDec->selProcOutput.loudnessNormalizationGainDb);
    if (err) return;
  }

  drcDec_GainDecoder_SetChannelGains(hDrcDec->hGainDec, numChannels, frameSize, channelGainDb,
                                     audioBufferChannelOffset, audioBuffer);
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcConfig(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                            const INT subStreamIndex) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  hDrcDec->uniDrcConfig.loudnessInfoSetPresent = 0; /* set to zero for error handling */
  if (hDrcDec->codecMode == DRC_DEC_MPEG_H_3DA) {
    dErr = drcDec_readMpegh3daUniDrcConfig(hBitstream, &(hDrcDec->uniDrcConfig),
                                           &(hDrcDec->loudnessInfoSet), subStreamIndex);
  } else
    return DRC_DEC_NOT_OK;

  if (dErr) return DRC_DEC_PARSE_ERROR;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadLoudnessInfoSet(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                               const INT subStreamIndex) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->codecMode == DRC_DEC_MPEG_H_3DA) {
    dErr =
        drcDec_readMpegh3daLoudnessInfoSet(hBitstream, &(hDrcDec->loudnessInfoSet), subStreamIndex);
  } else
    return DRC_DEC_NOT_OK;

  if (dErr) return DRC_DEC_PARSE_ERROR;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcGain(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream,
                          const INT subStreamIndex) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!hDrcDec->status) {
    return DRC_DEC_OK;
  }

  dErr = drcDec_readUniDrcGain(hBitstream, drcDec_GainDecoder_GetDrcCoefficients(hDrcDec->hGainDec),
                               drcDec_GainDecoder_GetFrameSize(hDrcDec->hGainDec),
                               drcDec_GainDecoder_GetDeltaTminDefault(hDrcDec->hGainDec),
                               subStreamIndex, &(hDrcDec->uniDrcGain));
  if (dErr) return DRC_DEC_PARSE_ERROR;

  if (_getGainStatus(&(hDrcDec->uniDrcGain))) {
    hDrcDec->status = DRC_DEC_NEW_GAIN_PAYLOAD;
  }

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrc(HANDLE_DRC_DECODER hDrcDec, HANDLE_FDK_BITSTREAM hBitstream) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}

DRC_DEC_ERROR
FDK_drcDec_Preprocess(HANDLE_DRC_DECODER hDrcDec) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!hDrcDec->status) return DRC_DEC_NOT_READY;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;

  if (hDrcDec->status != DRC_DEC_NEW_GAIN_PAYLOAD) {
    /* no new gain payload was read, e.g. during concealment or flushing.
       Generate DRC gains based on the stored DRC gains of last frames */
    drcDec_GainDecoder_Conceal(hDrcDec->hGainDec, &(hDrcDec->uniDrcConfig), &(hDrcDec->uniDrcGain));
  }

  dErr = drcDec_GainDecoder_Preprocess(
      hDrcDec->hGainDec, &(hDrcDec->uniDrcGain), hDrcDec->selProcOutput.loudnessNormalizationGainDb,
      hDrcDec->selProcOutput.boost, hDrcDec->selProcOutput.compress);
  if (dErr) return DRC_DEC_NOT_OK;
  hDrcDec->status = DRC_DEC_INTERPOLATION_PREPARED;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ProcessTime(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation, const int channelOffset,
                       const int drcChannelOffset, const int numChannelsProcessed,
                       FIXP_DBL* realBuffer, const int timeDataChannelOffset) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;
  if (hDrcDec->status != DRC_DEC_INTERPOLATION_PREPARED) return DRC_DEC_NOT_READY;

  dErr = drcDec_GainDecoder_ProcessTimeDomain(
      hDrcDec->hGainDec, delaySamples, (GAIN_DEC_LOCATION)drcLocation, channelOffset,
      drcChannelOffset, numChannelsProcessed, timeDataChannelOffset, realBuffer);
  if (dErr) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ProcessFreq(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation, const int channelOffset,
                       const int drcChannelOffset, const int numChannelsProcessed,
                       const int processSingleTimeslot, FIXP_DBL** realBuffer,
                       FIXP_DBL** imagBuffer) /* if imagBuffer == 0, the signal in realBuffer is
                                                 interleaved (real, imag, real, imag...) */
{
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;
  if (hDrcDec->status != DRC_DEC_INTERPOLATION_PREPARED) return DRC_DEC_NOT_READY;

  dErr = drcDec_GainDecoder_ProcessSubbandDomain(
      hDrcDec->hGainDec, delaySamples, (GAIN_DEC_LOCATION)drcLocation, channelOffset,
      drcChannelOffset, numChannelsProcessed, processSingleTimeslot, realBuffer, imagBuffer);
  if (dErr) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ApplyDownmix(HANDLE_DRC_DECODER hDrcDec, int* reverseInChannelMap,
                        int* reverseOutChannelMap, FIXP_DBL* realBuffer, int* pNChannels) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}
