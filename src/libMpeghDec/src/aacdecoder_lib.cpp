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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Manuel Jander

   Description:

*******************************************************************************/

#include "aacdecoder_lib.h"

#include "aac_ram.h"
#include "aacdecoder.h"
#include "tpdec_lib.h"
#include "FDK_core.h" /* FDK_tools version info */

#include "conceal.h"

#include "pcm_utils.h"

#include "FDK_cicp2geometry.h"

#include "FDK_formatConverter_process.h"
#include "FDK_dmxMatrixParser.h"

#include "ui.h"

/* Decoder library info */
#define AACDECODER_LIB_VL0 4
#define AACDECODER_LIB_VL1 1
#define AACDECODER_LIB_VL2 0
#define AACDECODER_LIB_TITLE "AAC Decoder Lib"
#ifdef __ANDROID__
#define AACDECODER_LIB_BUILD_DATE ""
#define AACDECODER_LIB_BUILD_TIME ""
#else
#define AACDECODER_LIB_BUILD_DATE __DATE__
#define AACDECODER_LIB_BUILD_TIME __TIME__
#endif

static AAC_DECODER_ERROR setConcealMethod(const HANDLE_AACDECODER self, const INT method);

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_GetFreeBytes(const HANDLE_AACDECODER self,
                                                       UINT* pFreeBytes) {
  /* check handle */
  if (!self) return AAC_DEC_INVALID_HANDLE;

  if (pFreeBytes == NULL) return AAC_DEC_INVALID_PARAM;

  /* reset free bytes */
  *pFreeBytes = 0;

  /* return nr of free bytes */
  HANDLE_FDK_BITSTREAM hBs = transportDec_GetBitstream(self->hInput, 0);
  *pFreeBytes = FDKgetFreeBits(hBs) >> 3;

  /* success */
  return AAC_DEC_OK;
}

/**
 * Config Decoder using a CSAudioSpecificConfig struct.
 */
static LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_Config(HANDLE_AACDECODER self,
                                                        const CSAudioSpecificConfig* pAscStruct,
                                                        UCHAR configMode, UCHAR* configChanged) {
  AAC_DECODER_ERROR err;

  if (self->flags[0] & AC_MPEGH3DA) {
    /* Assimilate new UI status with changed signals */
    if ((configMode == AC_CM_ALLOC_MEM) && (updateUiStatus(self) == 0)) {
      if (self->uiSignalChanged) {
        self->uiStatus = self->uiStatusNext;
        self->uiSignalChanged = 0;
        updateOnOffFlags(self);
      }
    }
  }

  /* Initialize AAC core decoder, and update self->streaminfo */
  err = CAacDecoder_Init(self, pAscStruct, configMode, configChanged);

  return err;
}

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_ConfigRaw(HANDLE_AACDECODER self, UCHAR* conf[],
                                                    const UINT length[]) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;

  if (self == NULL) {
    err = AAC_DEC_INVALID_HANDLE;
  } else if ((conf == NULL) || (length == NULL)) {
    err = AAC_DEC_INVALID_PARAM;
  } else {
    TRANSPORTDEC_ERROR errTp;
    UINT layer, nrOfLayers = self->nrOfLayers;
    if (TT_IS_MPEGH(transportDec_GetFormat(self->hInput)) &&
        TT_CFG_IS_INBAND(transportDec_GetFormat(self->hInput))) {
      return AAC_DEC_OK; /* in case of mhm1/mhm2 ignore optional out of band config (mhaC) */
    }
    for (layer = 0; layer < nrOfLayers; layer++) {
      if (length[layer] > 0) {
        errTp = transportDec_OutOfBandConfig(self->hInput, conf[layer], length[layer], layer);
        if (errTp != TRANSPORTDEC_OK) {
          switch (errTp) {
            case TRANSPORTDEC_NEED_TO_RESTART:
              err = AAC_DEC_NEED_TO_RESTART;
              break;
            case TRANSPORTDEC_UNSUPPORTED_FORMAT:
              err = AAC_DEC_UNSUPPORTED_FORMAT;
              break;
            default:
              err = AAC_DEC_UNKNOWN;
              break;
          }
          /* if baselayer is OK we continue decoding */
          if (layer >= 1) {
            self->nrOfLayers = layer;
            err = AAC_DEC_OK;
          }
          break;
        }
      }
    }
  }

  return err;
}

static INT aacDecoder_ConfigCallback(void* handle, const CSAudioSpecificConfig* pAscStruct,
                                     UCHAR configMode, UCHAR* configChanged) {
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)handle;
  AAC_DECODER_ERROR err = AAC_DEC_OK;
  TRANSPORTDEC_ERROR errTp;

  FDK_ASSERT(self != NULL);
  {
    { err = aacDecoder_Config(self, pAscStruct, configMode, configChanged); }
  }

  if (err == AAC_DEC_OK) {
    errTp = TRANSPORTDEC_OK;
  } else {
    if (err == AAC_DEC_NEED_TO_RESTART) {
      errTp = TRANSPORTDEC_NEED_TO_RESTART;
    } else if (IS_INIT_ERROR(err)) {
      errTp = TRANSPORTDEC_UNSUPPORTED_FORMAT;
    } /* Fatal errors */
    else {
      errTp = TRANSPORTDEC_UNKOWN_ERROR;
    }
  }

  if (*configChanged && configMode == AC_CM_DET_CFG_CHANGE) {
    AUDIO_SCENE_INFO* asi = UI_Manager_GetAsiPointer(self->hUiManager);
    asi->activeDmxId = self->downmixId;
  }

  return errTp;
}

static INT aacDecoder_DecodeFrameCallback(void* handle) {
  TRANSPORTDEC_ERROR errTp = TRANSPORTDEC_OK;
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)handle;

  self->frameOK = 1;

  if (CAacDecoder_DecodeFrame(self, 0, (PCM_DEC*)self->pTimeData2,
                              self->timeData2Size / sizeof(PCM_DEC), 0) != AAC_DEC_OK) {
    errTp = TRANSPORTDEC_UNKOWN_ERROR;
  }

  return errTp;
}

static INT aacDecoder_FreeMemCallback(void* handle, const CSAudioSpecificConfig* pAscStruct) {
  TRANSPORTDEC_ERROR errTp = TRANSPORTDEC_OK;
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)handle;

  int subStreamIndex = 0;

  if (pAscStruct->m_aot == AOT_MPEGH3DA) {
    subStreamIndex = pAscStruct->m_sc.m_usacConfig.subStreamIndex;
  }

  FDK_ASSERT(self != NULL);

  if (CAacDecoder_FreeMem(self, subStreamIndex) != AAC_DEC_OK) {
    errTp = TRANSPORTDEC_UNKOWN_ERROR;
  }

  return errTp;
}

static INT aacDecoder_CtrlCFGChangeCallback(void* handle,
                                            const CCtrlCFGChange* pCtrlCFGChangeStruct) {
  TRANSPORTDEC_ERROR errTp = TRANSPORTDEC_OK;
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)handle;

  if (self != NULL) {
    CAacDecoder_CtrlCFGChange(self, pCtrlCFGChangeStruct->flushStatus,
                              pCtrlCFGChangeStruct->flushCnt, pCtrlCFGChangeStruct->buildUpStatus,
                              pCtrlCFGChangeStruct->buildUpCnt);
    if (pCtrlCFGChangeStruct->forceCrossfade) {
      self->applyCrossfade |= AACDEC_CROSSFADE_BITMASK_FORCE; /* demand cross-fade between frames at
                                                                 next config change */
    }
  } else {
    errTp = TRANSPORTDEC_UNKOWN_ERROR;
  }

  return errTp;
}

static INT aacDecoder_TruncationMsgCallback(void* handle, INT nTruncSamples, INT truncType) {
  TRANSPORTDEC_ERROR errTp = TRANSPORTDEC_OK;
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)handle;
  SHORT delay = 3 * 256 + TD_UPSAMPLER_MAX_DELAY;

  if (self != NULL) {
    self->truncateSampleCount = nTruncSamples;
    /* set truncation offset */
    if (truncType == 0) { /* right truncation */
      self->truncateStartOffset = delay - nTruncSamples;
      self->truncateStopOffset = delay;
      self->truncateFromEndFlag = 1;
      if (self->discardSamplesAtStartCnt == -2) self->discardSamplesAtStartCnt = -1;
    } else if (truncType == 1) { /* left truncation */
      self->truncateStopOffset = nTruncSamples + delay;
      if (self->discardSamplesAtStartCnt == -1) self->discardSamplesAtStartCnt = -2;
    } else { /* generate truncation for config change without truncation message */
      self->truncateStartOffset = delay;
      self->truncateStopOffset = delay;
      if (self->discardSamplesAtStartCnt == -2) self->discardSamplesAtStartCnt = -1;
    }
  } else {
    errTp = TRANSPORTDEC_UNKOWN_ERROR;
  }

  return errTp;
}

static INT aacDecoder_UniDrcCallback(void* handle, HANDLE_FDK_BITSTREAM hBs,
                                     const INT fullPayloadLength, const INT payloadType,
                                     const INT subStreamIndex, const INT payloadStart,
                                     const AUDIO_OBJECT_TYPE aot) {
  DRC_DEC_ERROR err = DRC_DEC_OK;
  TRANSPORTDEC_ERROR errTp;
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;
  DRC_DEC_CODEC_MODE drcDecCodecMode = DRC_DEC_CODEC_MODE_UNDEFINED;
  UCHAR dummyBuffer[4] = {0, 0, 0, 0};
  FDK_BITSTREAM dummyBs;
  HANDLE_FDK_BITSTREAM hReadBs;

  if (hBs == NULL) {
    if (payloadType == 2) {
      /* clear downmix information in uniDrcConfig */
      FDK_drcDec_SetDownmixInstructions(hAacDecoder->hUniDrcDecoder, 0, NULL, NULL, NULL);
      return TRANSPORTDEC_OK;
    } else {
      /* use dummy zero payload to clear memory */
      hReadBs = &dummyBs;
      FDKinitBitStream(hReadBs, dummyBuffer, 4, 24);
    }
  } else {
    hReadBs = hBs;
  }

  if (aot == AOT_MPEGH3DA) {
    drcDecCodecMode = DRC_DEC_MPEG_H_3DA;
  } else if (aot == AOT_USAC) {
    drcDecCodecMode = DRC_DEC_MPEG_D_USAC;
  }

  err = FDK_drcDec_SetCodecMode(hAacDecoder->hUniDrcDecoder, drcDecCodecMode);
  if (err) return (INT)TRANSPORTDEC_UNKOWN_ERROR;

  if (payloadType == 0) /* uniDrcConfig */
  {
    err = FDK_drcDec_ReadUniDrcConfig(hAacDecoder->hUniDrcDecoder, hReadBs, subStreamIndex);
  } else /* loudnessInfoSet */
  {
    err = FDK_drcDec_ReadLoudnessInfoSet(hAacDecoder->hUniDrcDecoder, hReadBs, subStreamIndex);
    hAacDecoder->loudnessInfoSetPosition[1] = payloadStart;
    hAacDecoder->loudnessInfoSetPosition[2] = fullPayloadLength;
  }

  if (err == DRC_DEC_OK)
    errTp = TRANSPORTDEC_OK;
  else
    errTp = TRANSPORTDEC_UNKOWN_ERROR;

  return (INT)errTp;
}

static INT aacDecoder_ParseDmxMatrixCallback(void* handle, HANDLE_FDK_BITSTREAM hBs,
                                             CSUsacConfig* usc) {
  int err = 0, matchingTransmittedDmxMatrixFound = 0;
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;
  HANDLE_DRC_DECODER hUniDrcDecoder = NULL;
  hUniDrcDecoder = hAacDecoder->hUniDrcDecoder;

  const int targetLayout = usc->targetLayout;
  const int numSignalGroups = usc->bsNumSignalGroups;
  const int downmixConfigType = usc->downmixConfigType;
  const CSSignalGroup* signalGroupType = usc->m_signalGroupType;

  /* borrow memory from pTimeData2: the data will be read by DecodeDownmixMatrix() during
   * CAacDecoder_InitRenderer() */
  FDK_DOWNMIX_GROUPS_MATRIX_SET* groupsDownmixMatrixSet =
      (FDK_DOWNMIX_GROUPS_MATRIX_SET*)hAacDecoder->pTimeData2;
  FDKmemclear(groupsDownmixMatrixSet, sizeof(FDK_DOWNMIX_GROUPS_MATRIX_SET));

  err = DownmixMatrixSet(hBs, groupsDownmixMatrixSet, targetLayout, downmixConfigType,
                         &(hAacDecoder->downmixId), hUniDrcDecoder);

  if (hAacDecoder->hUiManager && err == 0) {
    AUDIO_SCENE_INFO* asi = UI_Manager_GetAsiPointer(hAacDecoder->hUiManager);
    asi->activeDmxId = hAacDecoder->downmixId & 0xFF; /* Truncation is verified */
  }

  if (err == 0) {
    for (INT i = 0; i < numSignalGroups; i++) {
      if (signalGroupType[i].type == 0) {
        UCHAR dmx_index = groupsDownmixMatrixSet->downmixMatrix[i];
        if (groupsDownmixMatrixSet->downmixMatrixSize[dmx_index] > 0) {
          FDK_ASSERT(usc->m_signalGroupType[i].bUseCustomDownmixMatrix == 0);
          usc->m_signalGroupType[i].bUseCustomDownmixMatrix =
              1;                                 /* downmixMatrix parsed for the signal group i. */
          matchingTransmittedDmxMatrixFound = 1; /* signal transmitted dmx matrix. */
        }
      }
    }

    /* Sanity check for transmitted downmix matrix parsing. Every bUseCustomDownmixMatrix must be 1,
     * if matchingTransmittedDmxMatrixFound = 1 */
    for (INT i = 0; i < numSignalGroups; i++) {
      if ((usc->m_signalGroupType[i].bUseCustomDownmixMatrix == 0) &&
          (signalGroupType[i].type == 0)) {
        /* No custom dmx matrix for this channels signal group i ... */
        if (matchingTransmittedDmxMatrixFound == 1) {
          /* ... but a transmitted dmx matrix was found for one signal group. -> invalid bitstream
           */
          /* All channel signal groups require a downmix matrix if a match was found otherwise the
           * bitstream is invalid. */
          err = TRANSPORTDEC_PARSE_ERROR;
        }
      }
    }
  }

  if (err == 0)
    err = TRANSPORTDEC_OK;
  else
    err = TRANSPORTDEC_UNKOWN_ERROR;

  return (INT)err;
}

static INT aacDecoder_EarconSetBSCallback(void* handle, HANDLE_FDK_BITSTREAM bs) {
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;

  if (hAacDecoder->earconDecoder.earconConfig.EarconFlag) {
    hAacDecoder->earconDecoder.CurrentFrameHasEarcon = 1;

    hAacDecoder->earconDecoder.earconConfig.EarconFlag = 0;

    UINT numPcmSignalsInFrame = FDKreadBits(bs, 7) + 1;  // numPcmSignalsInFrame

    if (numPcmSignalsInFrame > EARCON_MAX_NUM_SIGNALS) {
      return 0;
    }
    hAacDecoder->earconDecoder.numPcmSignalsInFrame = numPcmSignalsInFrame;

    /*Set initial state*/
    if (hAacDecoder->earconDecoder.First_Frame == 1) {
      /*Set attenuation gain to 1.0f*/
      hAacDecoder->earconDecoder.EarconGain = (FIXP_DBL)0x7fffffff;
      hAacDecoder->earconDecoder.EarconShift = PCM_OUT_HEADROOM;
      hAacDecoder->earconDecoder.target_loudness_old = 0;
      hAacDecoder->earconDecoder.m_bsPcmLoudnessValue_old = 0;
      hAacDecoder->earconDecoder.m_bsPcmAttenuationGain_old = 0;
      hAacDecoder->earconDecoder.AttGain_store_high_precision = (FIXP_DBL)0x7fffffff;
      hAacDecoder->earconDecoder.AttGain_exp_store_high_precision = 0;
      hAacDecoder->earconDecoder.AttGain_intermediate = (FIXP_DBL)0x7fffffff;
      hAacDecoder->earconDecoder.AttGainShift_intermediate = 0;
      hAacDecoder->earconDecoder.AttGain_increment_intermediate = (FIXP_DBL)0;
      hAacDecoder->earconDecoder.AttGain_old = (FIXP_DBL)0x7fffffff;
      hAacDecoder->earconDecoder.AttGainShift_old = 0;
      hAacDecoder->earconDecoder.AttGain_increment_old = (FIXP_DBL)0;
      hAacDecoder->earconDecoder.AttGain_new = (FIXP_DBL)0x7fffffff;
      hAacDecoder->earconDecoder.AttGainShift_new = 0;
      hAacDecoder->earconDecoder.AccumulatedFrameSize = 0;
    }

    for (UINT i = 0; i < numPcmSignalsInFrame; i++) FDKreadBits(bs, 7);  // pcmSignal_ID[i]

    if (hAacDecoder->earconDecoder.earconConfig.m_pcmHasAttenuationGain == 2)
      hAacDecoder->earconDecoder.earconConfig.m_bsPcmAttenuationGain =
          FDKreadBits(bs, 8);  // bsPcmAttenuationGain

    /*If the attenuation does not change any more, keep it as it was (but with high precision)*/
    if ((hAacDecoder->earconDecoder.earconConfig.m_pcmHasAttenuationGain == 0) ||
        ((hAacDecoder->earconDecoder.earconConfig.m_pcmHasAttenuationGain != 0) &&
         (hAacDecoder->earconDecoder.earconConfig.m_bsPcmAttenuationGain ==
          hAacDecoder->earconDecoder.m_bsPcmAttenuationGain_old))) {
      hAacDecoder->earconDecoder.AttGain_new =
          hAacDecoder->earconDecoder.AttGain_store_high_precision;
      hAacDecoder->earconDecoder.AttGainShift_new =
          -hAacDecoder->earconDecoder.AttGain_exp_store_high_precision;
    }

    INT frameSize = 0;
    if (hAacDecoder->earconDecoder.earconConfig.m_bsPcmFrameSize_index == 6) {
      frameSize = FDKreadBits(bs, 16);  // variable frame size
    }

    if (hAacDecoder->earconDecoder.earconConfig.sampling_frequency) {
      switch (hAacDecoder->earconDecoder.earconConfig.sampling_frequency) {
        case 48000:
          hAacDecoder->earconDecoder.BaseframeSize = 1024;
          break;
        case 32000:
          hAacDecoder->earconDecoder.BaseframeSize = 1536;
          break;
        case 24000:
          hAacDecoder->earconDecoder.BaseframeSize = 2048;
          break;
        case 16000:
          hAacDecoder->earconDecoder.BaseframeSize = 3096;
          break;
        default:
          /* unsupported sampling frequency */
          return 0;
      }
    }

    /*Sanity check for frameSize*/
    if (frameSize > hAacDecoder->earconDecoder.BaseframeSize) {
      return 0;
    }
    /*Sanity check for buffer fullness*/
    if (((hAacDecoder->earconDecoder.AccumulatedFrameSize +
          hAacDecoder->earconDecoder.BaseframeSize * numPcmSignalsInFrame)) >= EARCON_BUFFER_SIZE) {
      return 0;
    }

    INT WriteIndex = hAacDecoder->earconDecoder.AccumulatedFrameSize;
    FIXP_SGL* EarconDataPointer = &hAacDecoder->earconDecoder.EarconData[WriteIndex];
    for (INT i = 0; i < (frameSize * (INT)numPcmSignalsInFrame); i++) {
      *EarconDataPointer++ = (FIXP_SGL)(SHORT)FDKreadBits(bs, 16);  // pcmSample in Q0.15
    }

    /*Fill with zeros to the end of the MPEGH frame*/
    INT MPEGH_frameSize = hAacDecoder->earconDecoder.BaseframeSize;
    MPEGH_frameSize =
        MPEGH_frameSize - hAacDecoder->earconDecoder.earconConfig.m_bsMHASTruncationLength;
    INT diff = fMax(0, MPEGH_frameSize - frameSize);

    /*Sanity check for buffer fullness*/
    if (((hAacDecoder->earconDecoder.AccumulatedFrameSize + frameSize + diff)) >=
        EARCON_BUFFER_SIZE) {
      return 0;
    }
    if (diff) {
      FDKmemclear(EarconDataPointer, sizeof(FIXP_SGL) * diff * numPcmSignalsInFrame);
    }

    /*Increase the accumulated frame size*/
    hAacDecoder->earconDecoder.AccumulatedFrameSize += (frameSize + diff) * numPcmSignalsInFrame;
    hAacDecoder->earconDecoder.earconConfig.m_bsPcmFrameSize = (frameSize + diff);

  } else if (hAacDecoder->earconDecoder.CurrentFrameHasEarcon == 1) {
    hAacDecoder->earconDecoder.LastFrameHadEarcon = 1;
    hAacDecoder->earconDecoder.CurrentFrameHasEarcon = 0;
    return 0;
  } else if (hAacDecoder->earconDecoder.LastFrameHadEarcon == 1) {
    hAacDecoder->earconDecoder.LastFrameHadEarcon = 0;

    INT WriteIndex = hAacDecoder->earconDecoder.AccumulatedFrameSize;
    /*Fill with zeros to the end of the MPEGH frame*/
    INT FillSize =
        hAacDecoder->earconDecoder.BaseframeSize * hAacDecoder->earconDecoder.numPcmSignalsInFrame;
    /*Sanity check for buffer fullness*/
    if ((WriteIndex + FillSize) >= EARCON_BUFFER_SIZE) {
      return 0;
    }
    FIXP_SGL* EarconDataPointer = &hAacDecoder->earconDecoder.EarconData[WriteIndex];
    FDKmemclear(EarconDataPointer, sizeof(FIXP_SGL) * FillSize);

    /*Increase the accumulated frame size*/
    hAacDecoder->earconDecoder.AccumulatedFrameSize += FillSize;

    hAacDecoder->earconDecoder.earconConfig.m_bsPcmFrameSize =
        hAacDecoder->earconDecoder.BaseframeSize;

    hAacDecoder->earconDecoder.earconConfig.m_pcmHasAttenuationGain = 2;
    hAacDecoder->earconDecoder.earconConfig.m_bsPcmAttenuationGain = 0;

    return 0;
  }

  return 0;
}

static INT aacDecoder_EarconSetConfigCallback(void* handle, EarconConfig* earconConfig) {
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;
  hAacDecoder->earconDecoder.earconConfig = *earconConfig;
  return 0;
}

static INT aacDecoder_EarconSetInfoCallback(void* handle, EarconInfo* earconInfo) {
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;
  hAacDecoder->earconDecoder.earconInfo = *earconInfo;
  return 0;
}

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_AncDataInit(HANDLE_AACDECODER self, UCHAR* buffer,
                                                      int size) {
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;
  if (self == NULL) {
    errorStatus = AAC_DEC_INVALID_HANDLE;
  } else if (buffer == NULL) {
    errorStatus = AAC_DEC_INVALID_PARAM;
  } else {
    errorStatus = CAacDecoder_AncDataInit(&self->ancData, buffer, size);
  }
  return errorStatus;
}

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_AncDataGet(HANDLE_AACDECODER self, int index, UCHAR** ptr,
                                                     int* size) {
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;
  if (self == NULL) {
    errorStatus = AAC_DEC_INVALID_HANDLE;
  } else {
    errorStatus = CAacDecoder_AncDataGet(&self->ancData, index, ptr, size);
  }
  return errorStatus;
}

static AAC_DECODER_ERROR setConcealMethod(
    const HANDLE_AACDECODER self, /*!< Handle of the decoder instance */
    const INT method) {
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;
  CConcealParams* pConcealData = NULL;
  int method_revert = 0;
  CConcealmentMethod backupMethod = ConcealMethodNone;

  /* check decoder handle */
  if (self != NULL) {
    pConcealData = &self->concealCommonData;
    if (self->flags[0] & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA) && method >= 2) {
      /* Interpolation concealment is not implemented for USAC/RSVD50 */
      /* errorStatus = AAC_DEC_SET_PARAM_FAIL;
         goto bail; */
      method_revert = 1;
    }
    if (self->flags[0] & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA) && method >= 2) {
      /* Interpolation concealment is not implemented for USAC/RSVD50 */
      errorStatus = AAC_DEC_SET_PARAM_FAIL;
      goto bail;
    }
  }

  /* Get current method/delay */
  backupMethod = CConcealment_GetMethod(pConcealData);

  /* Be sure to set AAC and SBR concealment method simultaneously! */
  errorStatus =
      CConcealment_SetParams(pConcealData,
                             (method_revert == 0) ? (int)method : (int)1,  // concealMethod
                             AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,           // concealFadeOutSlope
                             AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,           // concealFadeInSlope
                             AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,           // concealMuteRelease
                             AACDEC_CONCEAL_PARAM_NOT_SPECIFIED            // concealComfNoiseLevel
      );
  if ((errorStatus != AAC_DEC_OK) && (errorStatus != AAC_DEC_INVALID_HANDLE)) {
    goto bail;
  }

bail:
  if ((errorStatus != AAC_DEC_OK) && (errorStatus != AAC_DEC_INVALID_HANDLE)) {
    /* Revert to the initial state */
    CConcealment_SetParams(pConcealData, (int)backupMethod, AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,
                           AACDEC_CONCEAL_PARAM_NOT_SPECIFIED, AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,
                           AACDEC_CONCEAL_PARAM_NOT_SPECIFIED);
  }

  return errorStatus;
}

LINKSPEC_CPP AAC_DECODER_ERROR
aacDecoder_SetParam(const HANDLE_AACDECODER self, /*!< Handle of the decoder instance */
                    const AACDEC_PARAM param,     /*!< Parameter to set               */
                    const INT value)              /*!< Parameter valued               */
{
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;
  HANDLE_TRANSPORTDEC hTpDec = NULL;
  TRANSPORTDEC_ERROR errTp = TRANSPORTDEC_OK;
  CConcealParams* pConcealData = NULL;
  TDLimiterPtr hPcmTdl = NULL;
  DRC_DEC_ERROR uniDrcErr = DRC_DEC_OK;

  /* check decoder handle */
  if (self != NULL) {
    hTpDec = self->hInput;
    pConcealData = &self->concealCommonData;
    hPcmTdl = self->hLimiter;
  } else {
    errorStatus = AAC_DEC_INVALID_HANDLE;
    goto bail;
  }

  /* configure the subsystems */
  switch (param) {
    case AAC_PCM_LIMITER_ENABLE:
      if (value < -1 || value > 1) {
        return AAC_DEC_SET_PARAM_FAIL;
      }
      self->limiterEnableUser = value & 0xFF; /* Truncation is verified */
      break;

    case AAC_PCM_LIMITER_ATTACK_TIME:
      if (value <= 0) { /* module function converts value to unsigned */
        return AAC_DEC_SET_PARAM_FAIL;
      }
      switch (pcmLimiter_SetAttack(hPcmTdl, value)) {
        case TDLIMIT_OK:
          break;
        case TDLIMIT_INVALID_HANDLE:
          return AAC_DEC_INVALID_HANDLE;
        case TDLIMIT_INVALID_PARAMETER:
        default:
          return AAC_DEC_SET_PARAM_FAIL;
      }
      break;

    case AAC_PCM_LIMITER_RELEAS_TIME:
      if (value <= 0) { /* module function converts value to unsigned */
        return AAC_DEC_SET_PARAM_FAIL;
      }
      switch (pcmLimiter_SetRelease(hPcmTdl, value)) {
        case TDLIMIT_OK:
          break;
        case TDLIMIT_INVALID_HANDLE:
          return AAC_DEC_INVALID_HANDLE;
        case TDLIMIT_INVALID_PARAMETER:
        default:
          return AAC_DEC_SET_PARAM_FAIL;
      }
      break;

    case AAC_PCM_OUTPUT_CHANNEL_MAPPING:
      if (value < 0 || value > 2) {
        return AAC_DEC_SET_PARAM_FAIL;
      }

      break;

    /* Target layout switch */
    case AAC_TARGET_LAYOUT_CICP:
      if ((int)value != -1 && (int)value != 0 &&
          cicp2geometry_get_numChannels_from_cicp(value) <= 0) {
        return AAC_DEC_SET_PARAM_FAIL;
      } else if (self->ascChannels[0] > 0) {
        /* Once the decoder has configured it is not possible to change the reference layout. */
        return AAC_DEC_SET_PARAM_FAIL;
      } else {
        int numChannels = 0;

        if (value > 0) {
          numChannels = cicp2geometry_get_numChannels_from_cicp(value);
        }

        self->targetLayout = value;
        if (transportDec_SetParam(hTpDec, TPDEC_PARAM_TARGETLAYOUT, self->targetLayout) !=
            TRANSPORTDEC_OK) {
          return AAC_DEC_SET_PARAM_FAIL;
        }
        if (self->targetLayout == -1) {
          uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_LOUDNESS_NORMALIZATION_ON,
                                          (FIXP_DBL)0);
          self->limiterEnableUser = 0;
        }

        if (transportDec_GetFormat(self->hInput) != TT_MHAS &&
            transportDec_GetFormat(self->hInput) != TT_MHAS_PACKETIZED &&
            transportDec_GetFormat(self->hInput) != TT_MHA_RAW) {
          if (numChannels > 0) {
            uniDrcErr =
                FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_TARGET_CHANNEL_COUNT_REQUESTED,
                                    (FIXP_DBL)numChannels);
          }
        }
      }
      break;

    case AAC_DRC_ATTENUATION_FACTOR:
      /* DRC compression factor (where 0 is no and 127 is max compression) */
      if ((value < 0) || (value > 127)) {
        return AAC_DEC_SET_PARAM_FAIL;
      }
      uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_COMPRESS,
                                      value * (FL2FXCONST_DBL(0.5f / 127.0f)));
      break;

    case AAC_DRC_BOOST_FACTOR:
      /* DRC boost factor (where 0 is no and 127 is max boost) */
      if ((value < 0) || (value > 127)) {
        return AAC_DEC_SET_PARAM_FAIL;
      }
      uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_BOOST,
                                      value * (FL2FXCONST_DBL(0.5f / 127.0f)));
      break;

    case AAC_DRC_REFERENCE_LEVEL:
      if (TT_IS_MPEGH(transportDec_GetFormat(self->hInput)) && (value < 0)) {
        /* Disabling loudness normalisation is not supported for MPEG-H */
        return AAC_DEC_SET_PARAM_FAIL;
      }
      if ((value >= 0) && ((value < 40) || (value > 127))) /* allowed range: -10 to -31.75 LKFS */
        return AAC_DEC_SET_PARAM_FAIL;
      /* DRC target reference level quantized in 0.25 LU steps using values [40..127].
         Negative values switch off loudness normalisation.
         Negative values also switch off MPEG-4 DRC, while MPEG-D DRC can be separately switched
         on/off with AAC_UNIDRC_SET_EFFECT */

      /* set target loudness also for MPEG-D DRC */
      uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_LOUDNESS_NORMALIZATION_ON,
                                      (FIXP_DBL)(value >= 0));
      self->defaultTargetLoudness = (SCHAR)value;
      break;

    case AAC_UNIDRC_SET_EFFECT:
      if ((value < -1) || (value > 6)) return AAC_DEC_SET_PARAM_FAIL;
      uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_EFFECT_TYPE, (FIXP_DBL)value);
      self->defaultDrcSetEffect = (SCHAR)value;
      break;
    case AAC_UNIDRC_ALBUM_MODE:
      uniDrcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_ALBUM_MODE, (FIXP_DBL)value);
      break;

    case AAC_TPDEC_PARAM_MINIMIZE_DELAY:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_MINIMIZE_DELAY, (int)value);
      break;
    case AAC_TPDEC_PARAM_IGNORE_BUFFERFULLNESS:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_IGNORE_BUFFERFULLNESS, (int)value);
      break;
    case AAC_TPDEC_PARAM_EARLY_CONFIG:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_EARLY_CONFIG, (int)value);
      break;
    case AAC_TPDEC_CLEAR_BUFFER:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_RESET, 1);
      self->streamInfo.numLostAccessUnits = 0;
      self->streamInfo.numBadBytes = 0;
      self->streamInfo.numTotalBytes = 0;
      /* aacDecoder_SignalInterruption(self); */
      break;
    case AAC_TPDEC_PARAM_SET_BITRATE:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_SET_BITRATE, (int)value);
      break;
    case AAC_TPDEC_PARAM_SET_BURSTPERIOD:
      errTp = transportDec_SetParam(hTpDec, TPDEC_PARAM_BURST_PERIOD, (int)value);
      break;
    case AAC_CONCEAL_METHOD:
      /* Changing the concealment method can introduce additional bitstream delay. And
         that in turn affects sub libraries and modules which makes the whole thing quite
         complex.  So the complete changing routine is packed into a helper function which
         keeps all modules and libs in a consistent state even in the case an error occures. */
      errorStatus = setConcealMethod(self, value);
      if (errorStatus == AAC_DEC_OK) {
        self->concealMethodUser = (CConcealmentMethod)value;
      }
      break;

    case AAC_CONCEAL_FADEOUT_SLOPE:
      errorStatus =
          CConcealment_SetParams(pConcealData,
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMethod
                                 (int)value,                          // concealFadeOutSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeInSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMuteRelease
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED   // concealComfNoiseLevel
          );
      break;

    case AAC_CONCEAL_FADEIN_SLOPE:
      errorStatus =
          CConcealment_SetParams(pConcealData,
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMethod
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeOutSlope
                                 (int)value,                          // concealFadeInSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMuteRelease
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED   // concealComfNoiseLevel
          );
      break;

    case AAC_CONCEAL_MUTE_RELEASE:
      errorStatus =
          CConcealment_SetParams(pConcealData,
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMethod
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeOutSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeInSlope
                                 (int)value,                          // concealMuteRelease
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED   // concealComfNoiseLevel
          );
      break;

    case AAC_CONCEAL_COMFORT_NOISE_LEVEL:
      errorStatus =
          CConcealment_SetParams(pConcealData,
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMethod
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeOutSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealFadeInSlope
                                 AACDEC_CONCEAL_PARAM_NOT_SPECIFIED,  // concealMuteRelease
                                 (FIXP_DBL)value                      // concealComfNoiseLevel
          );
      break;

    default:
      return AAC_DEC_SET_PARAM_FAIL;
  } /* switch(param) */

bail:

  if (errTp != TRANSPORTDEC_OK && errorStatus == AAC_DEC_OK) {
    errorStatus = AAC_DEC_SET_PARAM_FAIL;
  }

  if (errorStatus == AAC_DEC_OK) {
    /* Check error code returned by MPEG-D DRC decoder library: */
    switch (uniDrcErr) {
      case 0:
        break;
      case -9998:
        errorStatus = AAC_DEC_INVALID_HANDLE;
        break;
      default:
        errorStatus = AAC_DEC_SET_PARAM_FAIL;
        break;
    }
  }

  return (errorStatus);
}

LINKSPEC_CPP AAC_DECODER_ERROR
aacDecoder_SetParamVector(const HANDLE_AACDECODER self, /*!< Handle of the decoder instance. */
                          const AACDEC_PARAM param,     /*!< Parameter to set.     */
                          const void* pValue) /*!< Pointer to vector holding parameter values. */
{
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;
  CConcealParams* pConcealData = NULL;

  /* check decoder handle */
  if (self == NULL) {
    return AAC_DEC_INVALID_HANDLE;
  } else {
    pConcealData = &self->concealCommonData;
  }

  /* now configure the subsystems */
  switch (param) {
    case AAC_CONCEAL_FADEOUT_ATTENUATION_VECTOR:
      errorStatus = CConcealment_SetAttenuation(
          pConcealData, (const SHORT*)pValue, /* concealFadeOutAttenuationVector */
          NULL                                /* concealFadeInAttenuationVector  */
      );
      break;
    case AAC_CONCEAL_FADEIN_ATTENUATION_VECTOR:
      errorStatus =
          CConcealment_SetAttenuation(pConcealData, NULL,  /* concealFadeOutAttenuationVector */
                                      (const SHORT*)pValue /* concealFadeInAttenuationVector  */
          );
      break;
    default:
      return AAC_DEC_SET_PARAM_FAIL;
  } /* switch(param) */

  return (errorStatus);
}
LINKSPEC_CPP HANDLE_AACDECODER aacDecoder_Open(TRANSPORT_TYPE transportFmt, UINT nrOfLayers) {
  AAC_DECODER_INSTANCE* aacDec = NULL;
  HANDLE_TRANSPORTDEC pIn;
  int err = 0;

  UINT nrOfLayers_min = fMin(nrOfLayers, (UINT)TPDEC_MAX_LAYERS);

  /* Allocate transport layer struct. */
  pIn = transportDec_Open(transportFmt, 0, nrOfLayers_min);
  if (pIn == NULL) {
    return NULL;
  }

  /* Allocate AAC decoder core struct. */
  aacDec = CAacDecoder_Open(transportFmt);

  if (aacDec == NULL) {
    transportDec_Close(&pIn);
    goto bail;
  }
  aacDec->hInput = pIn;

  aacDec->nrOfLayers = nrOfLayers_min;

  aacDec->targetLayout = 6; /* default channel layout is 5.1 */
  transportDec_SetParam(pIn, TPDEC_PARAM_TARGETLAYOUT, aacDec->targetLayout);

  /* Register Config Update callback. */
  transportDec_RegisterAscCallback(pIn, aacDecoder_ConfigCallback, (void*)aacDec);

  /* Register Decode Frame callback. */
  transportDec_RegisterDecodeFrameCallback(pIn, aacDecoder_DecodeFrameCallback, (void*)aacDec);

  /* Register Free Memory callback. */
  transportDec_RegisterFreeMemCallback(pIn, aacDecoder_FreeMemCallback, (void*)aacDec);

  /* Register config switch control callback. */
  transportDec_RegisterCtrlCFGChangeCallback(pIn, aacDecoder_CtrlCFGChangeCallback, (void*)aacDec);
  transportDec_RegisterTruncationMsgCallback(pIn, aacDecoder_TruncationMsgCallback, (void*)aacDec);
  FDK_mpeghUiInitialize(aacDec);

  {
    if (FDK_drcDec_Open(&(aacDec->hUniDrcDecoder), DRC_DEC_ALL) != 0) {
      err = -1;
      goto bail;
    }
  }

  transportDec_RegisterUniDrcConfigCallback(pIn, aacDecoder_UniDrcCallback, (void*)aacDec,
                                            aacDec->loudnessInfoSetPosition);
  aacDec->defaultTargetLoudness = (SCHAR)96;

  transportDec_RegisterParseDmxMatrixCallback(pIn, aacDecoder_ParseDmxMatrixCallback,
                                              (void*)aacDec);

  transportDec_RegisterEarconSetBSCallBack(pIn, aacDecoder_EarconSetBSCallback, (void*)aacDec);
  transportDec_RegisterEarconConfigCallBack(pIn, aacDecoder_EarconSetConfigCallback, (void*)aacDec);
  transportDec_RegisterEarconInfoCallBack(pIn, aacDecoder_EarconSetInfoCallback, (void*)aacDec);

  if (UI_Manager_Create(&aacDec->hUiManager) != UI_MANAGER_OK) {
    err = -1;
    goto bail;
  }
  transportDec_SetAsiParsing(pIn, UI_Manager_GetAsiPointer(aacDec->hUiManager));

  EarconDecoder_Init(&aacDec->earconDecoder);

  aacDec->hLimiter = pcmLimiter_Create(TDL_MPEGH3DA_DEFAULT_ATTACK, TDL_RELEASE_DEFAULT_MS,
                                       (FIXP_DBL)MAXVAL_DBL, (24), 48000);
  if (NULL == aacDec->hLimiter) {
    err = -1;
    goto bail;
  }
  aacDec->limiterEnableUser = (UCHAR)-1;
  aacDec->limiterEnableCurr = 0;
  aacDec->discardSamplesAtStartCnt = -1; /* initialized for startup detection */

  aacDec->truncateStartOffset = -128;
  aacDec->truncateStopOffset = -128;
  aacDec->truncateFromEndFlag = 0;

  /* Assure that all modules have same delay */
  if (setConcealMethod(aacDec, CConcealment_GetMethod(&aacDec->concealCommonData))) {
    err = -1;
    goto bail;
  }

bail:
  if (err == -1) {
    aacDecoder_Close(aacDec);
    aacDec = NULL;
  }
  return aacDec;
}

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_Fill(HANDLE_AACDECODER self, const UCHAR* const pBuffer[],
                                               const UINT bufferSize[], UINT* pBytesValid) {
  AAC_DECODER_ERROR errorStatus = AAC_DEC_OK;

  if (self == NULL) {
    errorStatus = AAC_DEC_INVALID_HANDLE;
  } else if ((pBuffer == NULL) || (bufferSize == NULL) || (pBytesValid == NULL)) {
    errorStatus = AAC_DEC_INVALID_PARAM;
  } else {
    TRANSPORTDEC_ERROR tpErr;
    /* loop counter for layers; if not TT_MP4_RAWPACKETS used as index for only
       available layer                                                           */
    INT layer = 0;
    INT nrOfLayers = self->nrOfLayers;

    for (layer = 0; layer < nrOfLayers; layer++) {
      {
        tpErr = transportDec_FillData(self->hInput, pBuffer[layer], bufferSize[layer],
                                      &pBytesValid[layer], layer);
        if (tpErr != TRANSPORTDEC_OK) {
          return AAC_DEC_UNKNOWN; /* Must be an internal error */
        }
      }
    }
  }

  return errorStatus;
}

static void aacDecoder_SignalInterruption(HANDLE_AACDECODER self) {
  CAacDecoder_SignalInterruption(self);

  self->uiStatusValid = 0;
  self->drcStatusValid = 0;
}

static void aacDecoder_UpdateBitStreamCounters(CStreamInfo* pSi, HANDLE_FDK_BITSTREAM hBs,
                                               INT nBits, AAC_DECODER_ERROR ErrorStatus) {
  /* calculate bit difference (amount of bits moved forward) */
  nBits = nBits - FDKgetValidBits(hBs);

  /* Note: The amount of bits consumed might become negative when parsing a
     bit stream with several sub frames, and we find out at the last sub frame
     that the total frame length does not match the sum of sub frame length.
     If this happens, the transport decoder might want to rewind to the supposed
     ending of the transport frame, and this position might be before the last
     access unit beginning. */

  /* Calc bitrate. */
  if (pSi->frameSize > 0) {
    /* bitRate = nBits * sampleRate / frameSize */
    int ratio_e = 0;
    FIXP_DBL ratio_m = fDivNorm(pSi->sampleRate, pSi->frameSize, &ratio_e);
    pSi->bitRate = (INT)fMultNorm(nBits, DFRACT_BITS - 1, ratio_m, ratio_e, DFRACT_BITS - 1);
  }

  /* bit/byte counters */
  {
    int nBytes;

    nBytes = nBits >> 3;
    pSi->numTotalBytes += nBytes;
    if (IS_OUTPUT_VALID(ErrorStatus)) {
      pSi->numTotalAccessUnits++;
    }
    if (IS_DECODE_ERROR(ErrorStatus)) {
      pSi->numBadBytes += nBytes;
      pSi->numBadAccessUnits++;
    }
  }
}

static INT aacDecoder_EstimateNumberOfLostFrames(HANDLE_AACDECODER self) {
  INT n;

  transportDec_GetMissingAccessUnitCount(&n, self->hInput);

  return n;
}

LINKSPEC_CPP AAC_DECODER_ERROR aacDecoder_DecodeFrame(HANDLE_AACDECODER self, INT_PCM* pTimeData,
                                                      const INT timeDataSize, const UINT flags) {
  AAC_DECODER_ERROR ErrorStatus;
  INT layer;
  INT nBits;
  INT timeData2Size;
  INT timeDataHeadroom;
  HANDLE_FDK_BITSTREAM hBs;
  int fTpInterruption = 0; /* Transport originated interruption detection. */
  int fTpConceal = 0;      /* Transport originated concealment. */
  int streamIndex = 0;
  UINT accessUnit = 0;
  UINT numAccessUnits = 1;
  UINT numPrerollAU = 0;
  int fEndAuNotAdjusted = 0; /* The end of the access unit was not adjusted */
  PCM_DEC* pTimeData2;
  PCM_AAC* pTimeData3;
  INT pcmLimiterScale = 0;

  if (self == NULL) {
    return AAC_DEC_INVALID_HANDLE;
  }
  if (pTimeData == NULL) {
    return AAC_DEC_INVALID_PARAM;
  }

  if (flags & AACDEC_INTR) {
    self->streamInfo.numLostAccessUnits = 0;
  }
  self->streamInfo.mpeghAUSize = 0;

  hBs = transportDec_GetBitstream(self->hInput, 0);

  /* Get current bits position for bitrate calculation. */
  nBits = FDKgetValidBits(hBs);

  if (!((flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) ||
        (self->flushStatus == AACDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON) ||
        (self->flushStatus == AACDEC_USAC_DASH_IPF_FLUSH_ON) ||
        (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_IDLE_IN_BAND))) {
    TRANSPORTDEC_ERROR err;

    for (layer = 0; layer < self->nrOfLayers; layer++) {
      err = transportDec_ReadAccessUnit(self->hInput, layer);
      if (err != TRANSPORTDEC_OK) {
        switch (err) {
          case TRANSPORTDEC_NOT_ENOUGH_BITS:
            ErrorStatus = AAC_DEC_NOT_ENOUGH_BITS;
            goto bail;
          case TRANSPORTDEC_SYNC_ERROR:
            self->streamInfo.numLostAccessUnits = aacDecoder_EstimateNumberOfLostFrames(self);
            fTpInterruption = 1;
            break;
          case TRANSPORTDEC_NEED_TO_RESTART:
            ErrorStatus = AAC_DEC_NEED_TO_RESTART;
            goto bail;
          case TRANSPORTDEC_CRC_ERROR:
            fTpConceal = 1;
            break;
          case TRANSPORTDEC_UNSUPPORTED_FORMAT:
            ErrorStatus = AAC_DEC_UNSUPPORTED_FORMAT;
            goto bail;
          default:
            ErrorStatus = AAC_DEC_UNKNOWN;
            goto bail;
        }
      }
    }
  } else {
    if (self->streamInfo.numLostAccessUnits > 0) {
      self->streamInfo.numLostAccessUnits--;
    }
  }

  self->frameOK = 1;

  if (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_IDLE) {
    /* return without decoding audio to allow UI persistency module to restore settings first */
    self->streamInfo.frameSize = 0;
    ErrorStatus = AAC_DEC_INTERMEDIATE_OK;
    goto bail;
  }

  UINT prerollAUOffset[AACDEC_MAX_NUM_PREROLL_AU * TPDEC_MAX_LAYERS];
  for (int i = 0; i < AACDEC_MAX_NUM_PREROLL_AU + 1; i++) self->prerollAULength[i] = 0;

  INT auStartAnchor[TPDEC_MAX_LAYERS];
  HANDLE_FDK_BITSTREAM hBsAu;

  /* Process preroll frames and current frame */
  do {
    if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) &&
        (self->flushStatus != AACDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON) && (accessUnit == 0) &&
        (self->hasAudioPreRoll || (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_IDLE_IN_BAND)) &&
        !fTpInterruption &&
        !fTpConceal /* Bit stream pointer needs to be at the beginning of a (valid) AU. */
    ) {
      ErrorStatus = CAacDecoder_PreRollExtensionPayloadParse(self, &numPrerollAU, prerollAUOffset);

      if (ErrorStatus != AAC_DEC_OK) {
        switch (ErrorStatus) {
          case AAC_DEC_NOT_ENOUGH_BITS:
            goto bail;
          case AAC_DEC_PARSE_ERROR:
            self->frameOK = 0;
            break;
          default:
            break;
        }
      }

      numAccessUnits += numPrerollAU;
    }

    if (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_IDLE_IN_BAND) {
      /* return without decoding audio to allow UI persistency module to restore settings first */
      self->streamInfo.frameSize = 0;
      ErrorStatus = AAC_DEC_INTERMEDIATE_OK;
      goto bail;
    }

    for (int i = TPDEC_MAX_LAYERS - 1; i >= 0; i--) {
      hBsAu = transportDec_GetBitstream(self->hInput, i);
      auStartAnchor[i] = (INT)FDKgetValidBits(hBsAu);

      if (auStartAnchor[i] > 0 && accessUnit < numPrerollAU) {
        FDKpushFor(hBsAu, prerollAUOffset[accessUnit + i]);
      }
    }
    self->accessUnit = accessUnit;

    /* Signal bit stream interruption to other modules if required. */
    if (fTpInterruption || ((flags & AACDEC_INTR) && (accessUnit == 0))) {
      aacDecoder_SignalInterruption(self);
      if (!((flags & AACDEC_INTR) && (accessUnit == 0))) {
        ErrorStatus = AAC_DEC_TRANSPORT_SYNC_ERROR;
        goto bail;
      }
    }

    /* Clearing core data will be done in CAacDecoder_DecodeFrame() below.
       Tell other modules to clear states if required. */
    if (flags & AACDEC_CLRHIST) {
    }

    /* Empty bit buffer in case of flush request. */
    if (flags & AACDEC_FLUSH && !(flags & AACDEC_CONCEAL)) {
      if (!self->flushStatus) {
        transportDec_SetParam(self->hInput, TPDEC_PARAM_RESET, 1);
        self->streamInfo.numLostAccessUnits = 0;
        self->streamInfo.numBadBytes = 0;
        self->streamInfo.numTotalBytes = 0;
      }
    }
    /* Reset the output delay field. The modules will add their figures one after another. */
    self->streamInfo.outputDelay = 0;

    if (self->limiterEnableUser == (UCHAR)-1) {
      /* Enable limiter for all non-lowdelay and non-HD AOT's. */
      self->limiterEnableCurr = (self->flags[0] & (AC_LD | AC_ELD | AC_HDAAC)) ? 0 : 1;
    } else {
      /* Use limiter configuration as requested. */
      self->limiterEnableCurr = self->limiterEnableUser;
    }

    pTimeData2 = self->pTimeData2;
    timeData2Size = self->timeData2Size / sizeof(PCM_DEC);
    pTimeData3 = (PCM_AAC*)self->pTimeData2;

    ErrorStatus = CAacDecoder_DecodeFrame(
        self,
        flags | (fTpConceal ? AACDEC_CONCEAL : 0) |
            ((self->flushStatus && !(flags & AACDEC_CONCEAL)) ? AACDEC_FLUSH : 0),
        pTimeData2 + 256, timeData2Size - 256, self->streamInfo.aacSamplesPerFrame + 256);

    timeDataHeadroom = self->aacOutDataHeadroom;

    if (!((flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) || fTpConceal || self->flushStatus) &&
        (!(IS_OUTPUT_VALID(ErrorStatus)) || !(accessUnit < numPrerollAU))) {
      TRANSPORTDEC_ERROR tpErr;
      tpErr = transportDec_EndAccessUnit(self->hInput);
      if (tpErr != TRANSPORTDEC_OK) {
        self->frameOK = 0;
      }
    } else { /* while preroll processing later possibly an error in the renderer part occurrs */
      if (IS_OUTPUT_VALID(ErrorStatus)) {
        fEndAuNotAdjusted = 1;
      }
    }

    /* If the current pTimeData2 does not contain a valid signal, there nothing else we can do, so
     * bail. */
    if (!IS_OUTPUT_VALID(ErrorStatus)) {
      goto bail;
    }

    {
      self->streamInfo.sampleRate = self->streamInfo.aacSampleRate;
      self->streamInfo.frameSize = self->streamInfo.aacSamplesPerFrame;
    }

    self->streamInfo.numChannels = self->streamInfo.aacNumChannels;

    /* MPEG-H Rendering */

    self->truncateFrameSize = -1; /* not set */

    if ((self->flags[0] & AC_MPEGH3DA) && (self->targetLayout_config >= 0) &&
        (self->streamInfo.numChannels > 0)) {
      int grp = 0;
      /* Temporary downmix buffers */
      PCM_DEC* pTimeData_in = pTimeData2;

      {
        AAC_DECODER_ERROR ErrorUI;
        /* apply user interactivity */
        ErrorUI = applyUserInteractivity(self, pTimeData_in + 256);

        if (ErrorUI != AAC_DEC_OK) {
          ErrorStatus = ErrorUI;
          goto bail;
        }
      }

      /* Apply DRC 1 (before downmix/rendering) */
      int drcNumChannels = 0, drcStartChannel = 0, drcTotalChannels = 0;
      int signalsPrevStreams = 0;
      FDK_drcDec_Preprocess(self->hUniDrcDecoder);

      for (streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
        if (self->pUsacConfig[streamIndex] == NULL) break;
        for (grp = 0; grp < self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
          int transportStartChannel =
              self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
              signalsPrevStreams;
          int transportNumChannels = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;
          int signalGroupType = self->pUsacConfig[streamIndex]->m_signalGroupType[grp].type;
          int processTimeDomainDrc = !self->multibandDrcPresent;
          int processStftDomainDrcObjectsOrHoa = 0;

          /* get start channel index referenced in DRC configuration */
          switch (signalGroupType) {
            case 0: /* channels */
              drcNumChannels = transportNumChannels;
              break;
            case 1: /* objects */
              processStftDomainDrcObjectsOrHoa = self->multibandDrcPresent;
              drcNumChannels = transportNumChannels;
              break;
            default:
              drcNumChannels = transportNumChannels;
              break;
          }

          /* substract inactive channels */
          int transportNumChannels2 = transportNumChannels;
          for (int sig = 0; sig < transportNumChannels; sig++) {
            if (!getOnOffFlag(self, sig + transportStartChannel)) {
              transportNumChannels2--;
            }
          }
          transportNumChannels = transportNumChannels2;
          int transportStartChannel2 = transportStartChannel;
          for (int sig = 0; sig < transportStartChannel; sig++) {
            if (!getOnOffFlag(self, sig)) {
              transportStartChannel2--;
            }
          }
          transportStartChannel = transportStartChannel2;
          drcTotalChannels += transportNumChannels;

          if (drcTotalChannels > self->aacChannels) {
            /* configuration mismatch between signal groups and channel elements */
            ErrorStatus = AAC_DEC_UNKNOWN;
            goto bail;
          }

          if (processTimeDomainDrc && transportNumChannels > 0)
            FDK_drcDec_ProcessTime(self->hUniDrcDecoder, 0, DRC_DEC_DRC1, transportStartChannel,
                                   drcStartChannel - transportStartChannel, transportNumChannels,
                                   pTimeData2 + 256, self->streamInfo.frameSize + 256);

          if (processStftDomainDrcObjectsOrHoa && transportNumChannels > 0) {
            C_AALLOC_SCRATCH_START(stftBuffer, FIXP_DBL, 512) /* 2*(stft frame size) */
            for (int i = 0; i < self->numTimeSlots; i++) {
              /* get headroom */
              int ch;
              int STFT_headroom = 31;
              int STFT_headroom_prescaling = 0, STFT_headroom_prescaling_min = 0;
              for (ch = 0; ch < transportNumChannels; ch++) {
                STFT_headroom =
                    fMin(STFT_headroom,
                         getScalefactor(&(pTimeData2[(transportStartChannel + ch) *
                                                         (self->streamInfo.frameSize + 256) +
                                                     i * self->stftFrameSize + 256]),
                                        self->stftFrameSize));
              }
              /* Ensure 8 bit headroom */
              STFT_headroom_prescaling = fMax(STFT_headroom - 8, 0);
              /* Don't prescale more than necessary */
              STFT_headroom_prescaling = fMin(STFT_headroom_prescaling, 8);
              /* Ensure at least one bit headroom for FFT */
              if (STFT_headroom == 0) STFT_headroom_prescaling = -1;

              STFT_headroom_prescaling_min =
                  fMin((INT)self->STFT_headroom_prescaling[grp], STFT_headroom_prescaling);

              /* Transform to frequency domain*/
              for (ch = 0; ch < transportNumChannels; ch++) {
                StftFilterbank_Process(
                    &(pTimeData2[(transportStartChannel + ch) * (self->streamInfo.frameSize + 256) +
                                 i * self->stftFrameSize + 256]),
                    stftBuffer, self->stftFilterbankAnalysis[transportStartChannel + ch],
                    STFT_headroom_prescaling_min);

                /* Delay of DRC gains: Delay of STFT Analysis (256) - 128 because MPEG-D DRC
                 * downsamples at the middle of each timeslot */
                FDK_drcDec_ProcessFreq(
                    self->hUniDrcDecoder, 128, DRC_DEC_DRC1, transportStartChannel + ch,
                    drcStartChannel - transportStartChannel, 1, i, (FIXP_DBL**)&stftBuffer, NULL);

                /* clear output buffer first, as StftFilterbank_Process accumulates its output
                 * signal to the buffer */
                FDKmemclear(
                    &(pTimeData2[(transportStartChannel + ch) * (self->streamInfo.frameSize + 256) +
                                 i * self->stftFrameSize + 256]),
                    self->stftFrameSize * sizeof(FIXP_DBL));
                /* Transform to time domain */
                StftFilterbank_Process(
                    stftBuffer,
                    &(pTimeData2[(transportStartChannel + ch) * (self->streamInfo.frameSize + 256) +
                                 i * self->stftFrameSize + 256]),
                    self->stftFilterbankSynthesis[transportStartChannel + ch],
                    STFT_headroom_prescaling_min);
              }
              /* Keep the current signal headroom correction for use in the next frame */
              self->STFT_headroom_prescaling[grp] = STFT_headroom_prescaling;
            }
            C_AALLOC_SCRATCH_END(stftBuffer, FIXP_DBL, 512)
          }
          drcStartChannel += drcNumChannels;

          /* Compensate STFT delay in object and HOA signal path in case of time domain DRC. */
          if (processTimeDomainDrc) {
            for (int ch = 0; ch < transportNumChannels; ch++) {
              FDKmemcpy(
                  pTimeData2 + (transportStartChannel + ch) * (self->streamInfo.frameSize + 256),
                  self->delayBuffer[transportStartChannel + ch], sizeof(PCM_DEC) * 256);
              FDKmemcpy(self->delayBuffer[transportStartChannel + ch],
                        pTimeData2 +
                            (transportStartChannel + ch) * (self->streamInfo.frameSize + 256) +
                            self->streamInfo.frameSize,
                        sizeof(PCM_DEC) * 256);
            }
          }
        }
        signalsPrevStreams += self->ascChannels[streamIndex];
      }
      streamIndex = 0;

      /* Amount of rendered channels. Does not change when we export the MPEGH channels/objects/HOA,
       * since the format converter is skipped*/
      {
        self->streamInfo.numChannels =
            cicp2geometry_get_numChannels_from_cicp(self->targetLayout_config);
      }

      /* Clear output buffer, were each renderer will mix its output into. */
      if ((self->flags[0] & AC_MPEGH3DA) && (self->targetLayout_config > -1)) {
        FDKmemclear(self->workBufferCore2, (self->streamInfo.numChannels) *
                                               (self->streamInfo.frameSize) * sizeof(FIXP_DBL));
      }

      {
        signalsPrevStreams = 0;
        for (streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
          if (self->pUsacConfig[streamIndex] == NULL) break;
          /* Apply format converter if there is at least one channel signal group (always first). */
          if (self->pUsacConfig[streamIndex]->m_signalGroupType[0].type == 0) {
            int err;

            err = IIS_FormatConverter_Process(
                self->pFormatConverter[streamIndex],
                self->multibandDrcPresent ? self->hUniDrcDecoder : NULL,
                pTimeData_in + 256 + signalsPrevStreams * (self->streamInfo.frameSize + 256),
                (PCM_DEC*)self->workBufferCore2, self->streamInfo.aacSamplesPerFrame + 256);

            if (err != 0) {
              ErrorStatus = AAC_DEC_UNKNOWN;
              goto bail;
            }
          }

          signalsPrevStreams += self->ascChannels[streamIndex];
        }
        streamIndex = 0;
      }

      int numObjGroup = 0;
      /* DMX processing IN: pTimeData_tmp (working buffer) OUT: pTimeData_tmp2 ( working buffer for
       * deinterleaving / ouput buffer for interleaving )*/
      signalsPrevStreams = 0;
      for (streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
        if (self->pUsacConfig[streamIndex] == NULL) break;
        for (grp = 0; grp < self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
          int signalOffset = self->multibandDrcPresent ? 256 : 0;

          if (!getOnOffFlag(self,
                            self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                                signalsPrevStreams)) {
            continue;
          }

          /* Apply object rendering on the object signal group */
          if (self->pUsacConfig[streamIndex]->m_signalGroupType[grp].type == 1 &&
              self->hgVBAPRenderer[numObjGroup] != NULL) {
            gVBAPRenderer_RenderFrame_Time(self->hgVBAPRenderer[numObjGroup],
                                           pTimeData_in + signalOffset, self->workBufferCore2, 256,
                                           self->streamInfo.frameSize + 256);
            numObjGroup++;
          }

          pTimeData_in += self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count *
                          (self->streamInfo.frameSize + 256);
        }
        signalsPrevStreams += self->ascChannels[streamIndex];
      }
      streamIndex = 0;

      /* Apply DRC 2/3 (after downmix/rendering) */
      FDK_drcDec_ProcessTime(self->hUniDrcDecoder, 256, DRC_DEC_DRC2_DRC3, 0, 0,
                             self->streamInfo.numChannels, self->workBufferCore2,
                             self->streamInfo.frameSize);

      INT ovSamples = 0;
      INT splitFrameSize = 0;
      INT newFrameSize = self->streamInfo.frameSize;
      INT newSampleRate = self->streamInfo.sampleRate;
      TD_FAC_UPSAMPLE sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_1_1;

      switch (self->streamInfo.aacSampleRate) {
        case 14700:
        case 16000:
          newFrameSize = 3 * self->streamInfo.frameSize;
          newSampleRate = 3 * self->streamInfo.sampleRate;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_3_1;
          splitFrameSize = (newFrameSize - self->mpegH_rendered_delay.delay) / 3;
          ovSamples = newFrameSize - self->mpegH_rendered_delay.delay - (3 * splitFrameSize);
          if (ovSamples) {
            ovSamples = 3 - ovSamples;
            splitFrameSize++;
          }
          break;
        case 22050:
        case 24000:
          newFrameSize = 2 * self->streamInfo.frameSize;
          newSampleRate = 2 * self->streamInfo.sampleRate;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_2_1;
          splitFrameSize = (newFrameSize - self->mpegH_rendered_delay.delay) >> 1;
          ovSamples = newFrameSize - self->mpegH_rendered_delay.delay - (2 * splitFrameSize);
          if (ovSamples) {
            splitFrameSize++;
          }
          break;
        case 29400:
        case 32000:
          newFrameSize = (3 * self->streamInfo.frameSize) >> 1;
          newSampleRate = (3 * self->streamInfo.sampleRate) >> 1;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_3_2;
          splitFrameSize = ((newFrameSize - self->mpegH_rendered_delay.delay) / 3) << 1;
          ovSamples = newFrameSize - self->mpegH_rendered_delay.delay - splitFrameSize -
                      (splitFrameSize >> 1);
          if (ovSamples) {
            ovSamples = 3 - ovSamples;
            splitFrameSize = splitFrameSize + 2;
          }
          break;
        case 44100:
        case 48000:
          newFrameSize = self->streamInfo.frameSize;
          newSampleRate = self->streamInfo.sampleRate;
          sampleRateConverter_facUpsampling = TD_FAC_UPSAMPLE_1_1;
          splitFrameSize = newFrameSize - self->mpegH_rendered_delay.delay;
          ovSamples = 0;
          break;
      }

      /* upsampling */
      for (int ch = 0; ch < self->streamInfo.numChannels; ch++) {
        FDK_ASSERT(ch < self->mpegH_rendered_delay.num_channels);
        FDK_ASSERT(newFrameSize >= self->mpegH_rendered_delay.delay);

        /* Copy delayed samples from delay buffer to time buffer */
        if (self->mpegH_rendered_delay.delay) {
          FDKmemcpy(pTimeData2 + newFrameSize * ch,
                    &self->mpegH_rendered_delay.delay_line[self->mpegH_rendered_delay.delay * ch],
                    self->mpegH_rendered_delay.delay * sizeof(FIXP_DBL));
        }

        /* Resample first part of the current time signal and store the result in the time buffer */
        TD_upsampler(sampleRateConverter_facUpsampling,
                     &self->workBufferCore2[self->streamInfo.frameSize * ch], splitFrameSize,
                     &pTimeData2[newFrameSize * ch + self->mpegH_rendered_delay.delay],
                     &self->mpegH_sampleRateConverter_filterStates[ch][0]);

        /* Copy hang over samples from the time buffer to the delay buffer */
        if (ovSamples) {
          FDKmemcpy(&self->mpegH_rendered_delay.delay_line[self->mpegH_rendered_delay.delay * ch],
                    pTimeData2 + newFrameSize * ch + newFrameSize, ovSamples * sizeof(FIXP_DBL));
        }

        /* Resample second part of the current time signal and store the result in the delay buffer
         */
        TD_upsampler(sampleRateConverter_facUpsampling,
                     &self->workBufferCore2[self->streamInfo.frameSize * ch + splitFrameSize],
                     self->streamInfo.frameSize - splitFrameSize,
                     &self->mpegH_rendered_delay
                          .delay_line[self->mpegH_rendered_delay.delay * ch + ovSamples],
                     &self->mpegH_sampleRateConverter_filterStates[ch][0]);
      }

      /* change streamInfo parameters */
      self->streamInfo.frameSize = newFrameSize;
      self->streamInfo.sampleRate = newSampleRate;

      if ((self->flushStatus || flags & AACDEC_FLUSH) && !(flags & AACDEC_CONCEAL)) {
        self->streamInfo.mpeghAUSize = 0;
      } else if (accessUnit == numAccessUnits - 1) {
        self->streamInfo.mpeghAUSize = self->streamInfo.frameSize - self->truncateSampleCount;
        self->truncateSampleCount = 0;
      }

      /* Truncation */
      if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
        int truncStart, truncStop, truncLength;

        /* if a truncation right occurs at startup frameSize is not yet initialized in
         * TruncationMsgCallback thus it is added here */
        if (self->truncateFromEndFlag) {
          self->truncateFromEndFlag = 0;
          self->truncateStartOffset += self->streamInfo.frameSize;
          self->truncateStopOffset += self->streamInfo.frameSize;
        }

        /* extend truncation for flushed and preroll frames */
        if ((self->flushStatus) || (accessUnit < numPrerollAU)) {
          if (self->truncateStopOffset > 0) self->truncateStopOffset += self->streamInfo.frameSize;
        }
        /* current frame start and stop sample index of truncated part */
        truncStart = fMax(0, fMin(self->streamInfo.frameSize, (INT)self->truncateStartOffset));
        truncStop = fMax(0, fMin(self->streamInfo.frameSize, (INT)self->truncateStopOffset));

        /* length of truncated and remaining part */
        truncLength = truncStop - truncStart;
        self->truncateFrameSize = self->streamInfo.frameSize - truncLength;

        /* save samples for crossfade */
        if ((self->truncateStartOffset + 128 > 0) &&
            (self->truncateStartOffset < self->streamInfo.frameSize)) {
          int ch, l;

          l = fMin(self->streamInfo.frameSize, (INT)self->truncateStartOffset + 128) - truncStart;

          for (ch = 0; ch < self->streamInfo.numChannels; ch++) {
            FDKmemcpy(self->crossfadeMem + 128 * ch + truncStart - self->truncateStartOffset,
                      pTimeData2 + self->streamInfo.frameSize * ch + truncStart,
                      l * sizeof(PCM_DEC));
          }
        }

        /* apply crossfade */
        if (self->applyCrossfade != AACDEC_CROSSFADE_BITMASK_OFF) {
          if ((self->truncateStopOffset + 128 > 0) &&
              (self->truncateStopOffset < self->streamInfo.frameSize)) {
            int i, ch;

            {
              FIXP_SGL alpha, step = FL2FXCONST_SGL(1.0 / 127);
              int stop;

              stop = fMin(self->streamInfo.frameSize, (INT)self->truncateStopOffset + 128);
              alpha = (FIXP_SGL)((truncStop - self->truncateStopOffset) * (int)step) - step;
              for (i = truncStop; i < stop; i++) {
                alpha += step; /* increment alpha before the loop to avoid FIXP_SGL overflow */
                for (ch = 0; ch < self->streamInfo.numChannels; ch++) {
                  PCM_DEC tmpIn, tmpOut;

                  tmpIn = pTimeData2[self->streamInfo.frameSize * ch + i];
                  tmpOut = self->crossfadeMem[128 * ch + i - self->truncateStopOffset];
                  pTimeData2[self->streamInfo.frameSize * ch + i] =
                      fMult(alpha, tmpIn) + fMult((FIXP_SGL)(FL2FXCONST_SGL(1.0) - alpha), tmpOut);
                }
              }
            }

            if (self->truncateStopOffset + 128 <= self->streamInfo.frameSize) {
              self->applyCrossfade =
                  AACDEC_CROSSFADE_BITMASK_OFF; /* disable cross-fade between frames at nect config
                                                   change */
            }
          }
        }

        /* apply truncation */
        /* This version keeps the distance (self->streamInfo.frameSize) of the channel data
           There are 2 exclusive cases:
           In case of truncStart !=0, we truncate all data [truncStart
           ...self->streamInfo.frameSize-1]
              => no copy required
           In case of truncStop !=0, we truncate all data [0 ... truncStop-1]
              => copy [truncStop ... self->streamInfo.frameSize-1] to [0 ...truncFrameSize-1]
         */
        if ((truncLength > 0) && (self->truncateFrameSize > 0)) {
          int ch;

          for (ch = 0; ch < self->streamInfo.numChannels; ch++) {
            if (truncStop < self->streamInfo.frameSize) {
              /* Truncate first samples [0..truncStop-1]: move remaining to start at 0 for each
               * channel */
              FDKmemmove(pTimeData2 + self->streamInfo.frameSize * ch,
                         pTimeData2 + self->streamInfo.frameSize * ch + truncStop,
                         self->truncateFrameSize * sizeof(PCM_DEC));
              if (truncStart) truncStart += 0;
            }
          }
        }

        /* update offsets */
        self->truncateStartOffset =
            fMax(-128, self->truncateStartOffset - self->streamInfo.frameSize);
        self->truncateStopOffset =
            fMax(-128, self->truncateStopOffset - self->streamInfo.frameSize);

        /* set new frame size */
        /* self->streamInfo.frameSize = truncFrameSize; */
      }

      if (self->flags[0] & AC_MPEGH3DA) {
        if ((accessUnit == numAccessUnits - 1) && (self->truncateFrameSize > 0)) {
          PcmDataPayload(&self->earconDecoder, self->pTimeData2, self->streamInfo.frameSize,
                         self->drcStatus.targetLoudness, self->defaultTargetLoudness,
                         self->targetLayout, self->truncateFrameSize);
        }
      }

      /* Target Layout dependency */
      FDK_ASSERT(cicp2geometry_get_numChannels_from_cicp(self->targetLayout_config) != 0);

      timeDataHeadroom = PCM_OUT_HEADROOM;

    } else {
      {
        for (int ii = 0; ii < self->streamInfo.aacNumChannels; ii++) {
          FDKmemmove(pTimeData3 + ii * (self->streamInfo.frameSize),
                     pTimeData2 + ii * (self->streamInfo.aacSamplesPerFrame + 256) + 256,
                     self->streamInfo.frameSize * sizeof(PCM_AAC));
        }
      }
    }

    /* sbr decoder */

    /* SBR decoder for Unified Stereo Config (stereoConfigIndex == 3) */

    if (!((self->flags[0] & AC_MPEGH3DA) && (self->targetLayout_config >= 0) &&
          (self->streamInfo.numChannels > 0))) {
      if ((INT)PCM_OUT_HEADROOM != timeDataHeadroom) {
        scaleValues(pTimeData2, (PCM_DEC*)pTimeData3,
                    self->streamInfo.frameSize * self->streamInfo.numChannels,
                    -(PCM_OUT_HEADROOM - timeDataHeadroom));
      }
    }

    if (FDK_drcDec_GetParam(self->hUniDrcDecoder, DRC_DEC_IS_ACTIVE)) {
      /* return output loudness information for MPEG-D DRC */
      LONG outputLoudness = FDK_drcDec_GetParam(self->hUniDrcDecoder, DRC_DEC_OUTPUT_LOUDNESS);
      if (outputLoudness == DRC_DEC_LOUDNESS_NOT_PRESENT) {
        /* no valid MPEG-D DRC loudness value contained */
        self->streamInfo.outputLoudness = -1;
      } else {
        if (outputLoudness > 0) {
          /* positive output loudness values (very unusual) are limited to 0 LKFS */
          self->streamInfo.outputLoudness = 0;
        } else {
          self->streamInfo.outputLoudness =
              -(INT)outputLoudness >> 22; /* negate and scale from e = 7 to e = (31-2) */
        }
      }
    } else {
      /* return output loudness information for MPEG-4 DRC */
      if (self->streamInfo.drcProgRefLev < 0) { /* no MPEG-4 DRC loudness metadata contained */
        self->streamInfo.outputLoudness = -1;
      } else {
        if (self->defaultTargetLoudness < 0) { /* loudness normalization is off */
          self->streamInfo.outputLoudness = self->streamInfo.drcProgRefLev;
        } else {
          self->streamInfo.outputLoudness = self->defaultTargetLoudness;
        }
      }
    }

    if (self->streamInfo.extAot != AOT_AAC_SLS) {
    }

    /* Signal interruption to take effect in next frame. */
    if ((flags & AACDEC_FLUSH || self->flushStatus) && !(flags & AACDEC_CONCEAL)) {
      aacDecoder_SignalInterruption(self);
    }

    /* Update externally visible copy of flags */
    self->streamInfo.flags = self->flags[0];

    if (accessUnit < numPrerollAU) {
      for (int i = TPDEC_MAX_LAYERS - 1; i >= 0; i--) {
        hBsAu = transportDec_GetBitstream(self->hInput, i);
        if (auStartAnchor[i] > 0) {
          FDKpushBack(hBsAu, auStartAnchor[i] - FDKgetValidBits(hBsAu));
        }
      }
    } else {
      if ((self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON) ||
          (self->buildUpStatus == AACDEC_MPEGH_BUILD_UP_ON_IN_BAND) ||
          (self->buildUpStatus == AACDEC_USAC_BUILD_UP_ON)) {
        self->buildUpCnt--;

        if (self->buildUpCnt < 0) {
          self->buildUpStatus = 0;
        }
      }
    }

    if (self->flushStatus != AACDEC_USAC_DASH_IPF_FLUSH_ON) {
      accessUnit++;
    }
  } while ((accessUnit < numAccessUnits) ||
           ((self->flushStatus == AACDEC_USAC_DASH_IPF_FLUSH_ON) && !(flags & AACDEC_CONCEAL)));

  if (self->streamInfo.extAot != AOT_AAC_SLS) {
    pcmLimiterScale += PCM_OUT_HEADROOM;

    if (flags & AACDEC_CLRHIST) {
      /* Delete the delayed signal. */
      pcmLimiter_Reset(self->hLimiter);
    }

    /* Check whether time data buffer is large enough. */
    if (timeDataSize < (self->streamInfo.numChannels * self->streamInfo.frameSize)) {
      ErrorStatus = AAC_DEC_OUTPUT_BUFFER_TOO_SMALL;
      goto bail;
    }

    /* use workBufferCore2 buffer for interleaving */
    PCM_AAC* pInterleaveBuffer = (PCM_AAC*)self->workBufferCore2;
    int blockLength = self->streamInfo.frameSize;

    /* applyLimiter requests for interleaved data and doesn't support in-place processing */
    /* Interleave output buffer */
    FDK_interleave(pTimeData2, pInterleaveBuffer, self->streamInfo.numChannels, blockLength,
                   self->streamInfo.frameSize);

    if (self->truncateFrameSize != -1) {
      self->streamInfo.frameSize = self->truncateFrameSize;
    }

    if (self->limiterEnableCurr) {
      /* limiter work buffer */
      PCM_AAC* tmpBuffer = (PCM_AAC*)pTimeData2; /* points to workBufferCore5 */

      /* Set actual signal parameters */
      pcmLimiter_SetNChannels(self->hLimiter, self->streamInfo.numChannels);
      if (pcmLimiter_SetSampleRate(self->hLimiter, self->streamInfo.sampleRate) != TDLIMIT_OK) {
        return AAC_DEC_SET_PARAM_FAIL;
      }

      pcmLimiter_Apply(self->hLimiter, pInterleaveBuffer, pTimeData, tmpBuffer, NULL,
                       pcmLimiterScale, self->streamInfo.frameSize);

      if (self->flags[streamIndex] & AC_MPEGH3DA) {
        if (!(accessUnit <
              numPrerollAU)) /* after preroll frame decoding no valid samples to discard exist */
        {
          /* discard samples of constant decoder delay and pcmLimiter delay at start */
          if (self->discardSamplesAtStartCnt ==
              -1) /* no left truncation has happend before thus the constant decoder delay is not
                     truncated yet */
          {
            self->discardSamplesAtStartCnt =
                pcmLimiter_GetDelay(self->hLimiter) + 3 * 256 + TD_UPSAMPLER_MAX_DELAY;
          } else {
            if (self->discardSamplesAtStartCnt == -2) /* one left truncation happend before and
                                                         truncated the constant decoder delay */
            {
              self->discardSamplesAtStartCnt = pcmLimiter_GetDelay(self->hLimiter);
            }
          }

          if (self->discardSamplesAtStartCnt > 0 && self->streamInfo.numChannels > 0) {
            if (self->streamInfo.frameSize <= self->discardSamplesAtStartCnt) {
              self->discardSamplesAtStartCnt -= self->streamInfo.frameSize;
              self->streamInfo.frameSize = 0;
            } else {
              FDKmemmove(pTimeData,
                         &pTimeData[self->discardSamplesAtStartCnt * self->streamInfo.numChannels],
                         (self->streamInfo.frameSize - self->discardSamplesAtStartCnt) *
                             self->streamInfo.numChannels * sizeof(INT_PCM));
              self->streamInfo.frameSize -= self->discardSamplesAtStartCnt;
              self->discardSamplesAtStartCnt = 0;
            }
          }

          if (self->streamInfo.frameSize > 0)
            self->streamInfo.outputDelay += self->discardSamplesAtStartCnt;
        }
      } else {
        /* Announce the additional limiter output delay */
        self->streamInfo.outputDelay += pcmLimiter_GetDelay(self->hLimiter);
      }
    } else {
      scaleValuesSaturate(pTimeData, (PCM_DEC*)pInterleaveBuffer,
                          self->streamInfo.frameSize * self->streamInfo.numChannels,
                          pcmLimiterScale);
    }
  } /* if (self->streamInfo.extAot != AOT_AAC_SLS)*/

bail:

  /* error in renderer part occurred, ErrorStatus was set to invalid output */
  if (fEndAuNotAdjusted && !IS_OUTPUT_VALID(ErrorStatus) && (accessUnit < numPrerollAU)) {
    transportDec_EndAccessUnit(self->hInput);
  }

  /* Update Statistics */
  aacDecoder_UpdateBitStreamCounters(&self->streamInfo, hBs, nBits, ErrorStatus);

  /* Ensure consistency of IS_OUTPUT_VALID() macro. */
  FDK_ASSERT((((self->streamInfo.numChannels <= 0) || (self->streamInfo.sampleRate <= 0)) &&
              IS_OUTPUT_VALID(ErrorStatus)) == 0);

  if (!(IS_OUTPUT_VALID(ErrorStatus) || (ErrorStatus == AAC_DEC_INTERMEDIATE_OK))) {
    self->streamInfo.mpeghAUSize = -1;
    self->streamInfo.frameSize = 0;
  }

  /*Logical AND of the independent frame flag and "frame decoded correctly" flag*/
  self->streamInfo.fBsRestartOk &= (self->frameOK ? 1 : 0);

  return ErrorStatus;
}

LINKSPEC_CPP void aacDecoder_Close(HANDLE_AACDECODER self) {
  if (self == NULL) return;

  if (self->hLimiter != NULL) {
    pcmLimiter_Destroy(self->hLimiter);
  }

  FDK_drcDec_Close(&self->hUniDrcDecoder);

  if (self->hUiManager) {
    UI_Manager_Delete(&self->hUiManager);
  }

  if (self->hInput != NULL) {
    transportDec_Close(&self->hInput);
  }

  CAacDecoder_Close(self);
}

LINKSPEC_CPP CStreamInfo* aacDecoder_GetStreamInfo(HANDLE_AACDECODER self) {
  return CAacDecoder_GetStreamInfo(self);
}
