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

/******************* MPEG transport format decoder library *********************

   Author(s):   Daniel Homm

   Description:

*******************************************************************************/

#include "tpdec_lib.h"
#include "tp_data.h"

#include "FDK_cicp2geometry.h"

#include "common_fix.h"

#define MAX_CICP_VALUE (21)

void CProgramConfig_Reset(CProgramConfig* pPce) {
  pPce->elCounter = 0;
}

void CProgramConfig_Init(CProgramConfig* pPce) {
  FDKmemclear(pPce, sizeof(CProgramConfig));
}

int CProgramConfig_IsValid(const CProgramConfig* pPce) {
  return ((pPce->isValid) ? 1 : 0);
}

int CProgramConfig_LookupElement(CProgramConfig* pPce, UINT channelConfig, const UINT tag,
                                 const UINT channelIdx, UCHAR chMapping[],
                                 AUDIO_CHANNEL_TYPE chType[], UCHAR chIndex[],
                                 const UINT chDescrLen, UCHAR* elMapping, MP4_ELEMENT_ID elList[],
                                 MP4_ELEMENT_ID elType) {
  {
    {
      /* Implicit channel mapping. */
      if (IS_MP4_CHANNEL_ELEMENT(elType)) {
        /* Store all channel element IDs */
        elList[pPce->elCounter] = elType;
        *elMapping = pPce->elCounter++;
      }
    }
  }

  return 1;
}

static INT getSampleRate(HANDLE_FDK_BITSTREAM bs, UCHAR* index, int nBits) {
  INT sampleRate;
  int idx;

  idx = FDKreadBits(bs, nBits);
  if (idx == (1 << nBits) - 1) {
    if (FDKgetValidBits(bs) < 24) {
      return 0;
    }
    sampleRate = FDKreadBits(bs, 24);
  } else {
    sampleRate = SamplingRateTable[idx];
  }

  *index = idx;

  return sampleRate;
}

/*
  subroutine for parsing extension element configuration:
  UsacExtElementConfig() q.v. ISO/IEC FDIS 23003-3:2011(E) Table 14
  mpegh3daExtElementConfig() q.v. ISO/IEC DIS 23008-3 Table 13
*/
static TRANSPORTDEC_ERROR extElementConfig(CSUsacExtElementConfig* extElement,
                                           HANDLE_FDK_BITSTREAM hBs, const CSTpCallBacks* cb,
                                           const UCHAR numSignalsInGroup,
                                           const UCHAR numObjectGroups,
                                           const UCHAR numChannelGroups, const UINT coreFrameLength,
                                           const int subStreamIndex, const AUDIO_OBJECT_TYPE aot) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;

  UINT usacExtElementType = escapedValue(hBs, 4, 8, 16);

  int usacExtElementConfigLength = escapedValue(hBs, 4, 8, 16);

  INT bsAnchor;

  if (FDKreadBit(hBs)) /* usacExtElementDefaultLengthPresent */
    extElement->usacExtElementDefaultLength = escapedValue(hBs, 8, 16, 0) + 1;
  else
    extElement->usacExtElementDefaultLength = 0;

  extElement->usacExtElementPayloadFrag = FDKreadBit(hBs);

  bsAnchor = (INT)FDKgetValidBits(hBs);

  /* Return an error in case the bitbuffer fill level is too low. */
  if (bsAnchor < usacExtElementConfigLength * 8) {
    return TRANSPORTDEC_NOT_ENOUGH_BITS;
  }

  switch (usacExtElementType) {
    case ID_EXT_ELE_UNKNOWN:
    case ID_EXT_ELE_FILL:
      break;
    case ID_EXT_ELE_AUDIOPREROLL:
      /* No configuration element */
      extElement->usacExtElementHasAudioPreRoll = 1;
      break;
    case ID_EXT_ELE_FMT_CNVRTR:
      /* No configuration element */
      break;
    case ID_EXT_ELE_OBJ_METADATA:
      /* parsing of ObjectMetadataConfig() */

      /* save number of objects */
      extElement->extConfig.oam.numObjectSignals = numSignalsInGroup;
      extElement->extConfig.oam.lowDelayMetadataCoding = FDKreadBit(hBs);
      if (extElement->extConfig.oam.lowDelayMetadataCoding != 1) {
        return TRANSPORTDEC_UNSUPPORTED_FORMAT; /* only low delay supported */
      }
      extElement->extConfig.oam.hasCoreLength = FDKreadBit(hBs);
      if (!extElement->extConfig.oam
               .hasCoreLength) /* does not have the frame length of the corecoder */
      {
        extElement->extConfig.oam.OAMframeLength =
            FDKreadBits(hBs, 6) + 1; /*framelength = blocksize/64 */
        extElement->extConfig.oam.OAMframeLength = extElement->extConfig.oam.OAMframeLength
                                                   << 6; /*blocksize*/
        if ((extElement->extConfig.oam.OAMframeLength != coreFrameLength) &&
            (extElement->extConfig.oam.OAMframeLength != (coreFrameLength >> 1)) &&
            (extElement->extConfig.oam.OAMframeLength != (coreFrameLength >> 2))) {
          return TRANSPORTDEC_PARSE_ERROR;
        }
      } else {
        extElement->extConfig.oam.OAMframeLength = coreFrameLength;
      }
      if (extElement->extConfig.oam.OAMframeLength > coreFrameLength) {
        return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }
      extElement->extConfig.oam.hasScreenRelativeObjects = FDKreadBit(hBs);
      if (extElement->extConfig.oam.hasScreenRelativeObjects) {
        extElement->extConfig.oam.isScreenRelativeObject = 0;
        for (int i = 0; i < numSignalsInGroup; i++) {
          extElement->extConfig.oam.isScreenRelativeObject |= FDKreadBit(hBs) << (31 - i);
        }
      }
      extElement->extConfig.oam.hasDynamicObjectPriority = FDKreadBit(hBs);
      extElement->extConfig.oam.hasUniformSpread = FDKreadBit(hBs);
      break;
    case ID_EXT_ELE_MCT:
      /* MCTConfig() */
      for (int chan = 0; chan < numSignalsInGroup; chan++) {
        extElement->extConfig.mct.mctChanMask[chan] = FDKreadBit(hBs);
      }
      break;
    case ID_EXT_ELE_HOA: {
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    } break;
    case ID_EXT_ELE_UNI_DRC: {
      if (cb->cbUniDrc != NULL) {
        ErrorStatus = (TRANSPORTDEC_ERROR)cb->cbUniDrc(
            cb->cbUniDrcData, hBs, usacExtElementConfigLength, 0, /* uniDrcConfig */
            subStreamIndex, 0, aot);
        if (ErrorStatus != TRANSPORTDEC_OK) {
          return ErrorStatus;
        }
      }
    } break;
    case ID_EXT_ELE_PROD_METADATA:
      if (usacExtElementConfigLength > TP_MPEGH_MAX_PROD_METADATA_CONFIG_LEN) {
        ErrorStatus = TRANSPORTDEC_PARSE_ERROR;
      } else {
        INT gp;

        extElement->extConfig.prodMetadata.hasReferenceDistance = FDKreadBit(hBs);

        if (extElement->extConfig.prodMetadata.hasReferenceDistance) {
          extElement->extConfig.prodMetadata.bsReferenceDistance = FDKreadBits(hBs, 7);
        } else {
          extElement->extConfig.prodMetadata.bsReferenceDistance = 57;
        }

        for (gp = 0; gp < numObjectGroups; gp++) {
          extElement->extConfig.prodMetadata.hasObjectDistance[gp] = FDKreadBit(hBs);
        }

        for (gp = 0; gp < numChannelGroups; gp++) {
          extElement->extConfig.prodMetadata.directHeadphone[gp] = FDKreadBit(hBs);
        }
      }
      break;
    default:
      usacExtElementType = ID_EXT_ELE_UNKNOWN;
      break;
  }
  extElement->usacExtElementType = (USAC_EXT_ELEMENT_TYPE)usacExtElementType;

  /* Adjust bit stream position. This is required because of byte alignment and unhandled
   * extensions. */
  {
    INT left_bits = (usacExtElementConfigLength << 3) - (bsAnchor - (INT)FDKgetValidBits(hBs));
    if (left_bits >= 0) {
      FDKpushFor(hBs, left_bits);
    } else {
      /* parsed too many bits */
      ErrorStatus = TRANSPORTDEC_PARSE_ERROR;
    }
  }

  return ErrorStatus;
}

/*
  downmixConfig() subroutine for parisng downmix configuration and downmix matrix parsing.
*/

static TRANSPORTDEC_ERROR downmixConfig(CSUsacConfig* usc, HANDLE_FDK_BITSTREAM hBs,
                                        const CSTpCallBacks* cb) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;
  usc->downmixConfigType = 0;
  usc->passiveDownmixFlag = 0;
  usc->immersiveDownmixFlag = 0;

  usc->downmixConfigType = FDKreadBits(hBs, 2);
  if (usc->downmixConfigType == 0 || usc->downmixConfigType == 2) {
    usc->passiveDownmixFlag = FDKreadBits(hBs, 1);
    if (usc->passiveDownmixFlag == 0) {
      usc->phaseAlignStrength = FDKreadBits(hBs, 3);
    }
    usc->immersiveDownmixFlag = FDKreadBits(hBs, 1);
    if (usc->immersiveDownmixFlag) {
      usc->downmixConfigType = 0;
    }
  }
  if (usc->downmixConfigType == 1 || usc->downmixConfigType == 2) {
    if (cb->cbParseDmxMatrix(cb->cbParseDmxMatrixData, hBs, usc) != 0) {
      ErrorStatus = TRANSPORTDEC_UNKOWN_ERROR;
    }
  }

  return ErrorStatus;
}

/*
  subroutine for parsing the USAC / MPEG-H configuration extension:
  UsacConfigExtension() q.v. ISO/IEC FDIS 23003-3:2011(E) Table 15
  mpegh3daConfigExtension() q.v. ISO/IEC DIS 23008-3 Table 14
*/
static TRANSPORTDEC_ERROR configExtension(CSUsacConfig* usc, AUDIO_SCENE_INFO* pASI,
                                          const AUDIO_OBJECT_TYPE aot, HANDLE_FDK_BITSTREAM hBs,
                                          const CSTpCallBacks* cb) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;

  int numConfigExtensions;
  UINT usacConfigExtType;
  int usacConfigExtLength;
  int extIsPresent = 0; /* Bit field to detect unallowed multiple extension configs */
  int loudnessInfoSetIndex =
      -1; /* index of loudnessInfoSet config extension. -1 if not contained. */
  int tmp_subStreamIndex = 0;
  AUDIO_OBJECT_TYPE tmp_aot = AOT_USAC;
  tmp_subStreamIndex = usc->subStreamIndex;
  tmp_aot = aot;

  numConfigExtensions = (int)escapedValue(hBs, 2, 4, 8) + 1;
  for (int confExtIdx = 0; confExtIdx < numConfigExtensions; confExtIdx++) {
    INT nbits;
    int loudnessInfoSetConfigExtensionPosition = FDKgetValidBits(hBs);
    usacConfigExtType = escapedValue(hBs, 4, 8, 16);
    usacConfigExtLength = (int)escapedValue(hBs, 4, 8, 16);

    /* Start bit position of config extension */
    nbits = FDKgetValidBits(hBs);

    /* Return an error in case the bitbuffer fill level is too low. */
    if (nbits < usacConfigExtLength * 8) {
      return TRANSPORTDEC_NOT_ENOUGH_BITS;
    }

    switch (usacConfigExtType) {
      case ID_CONFIG_EXT_FILL:
        for (int i = 0; i < usacConfigExtLength; i++) {
          if (FDKreadBits(hBs, 8) != 0xa5) {
            return TRANSPORTDEC_PARSE_ERROR;
          }
        }
        break;
      case ID_CONFIG_EXT_DOWNMIX:
        if (extIsPresent & (1 << ID_CONFIG_EXT_DOWNMIX)) {
          return TRANSPORTDEC_PARSE_ERROR;
        }
        extIsPresent |= (1 << ID_CONFIG_EXT_DOWNMIX);
        ErrorStatus = downmixConfig(usc, hBs, cb);
        if (ErrorStatus != TRANSPORTDEC_OK) {
          return ErrorStatus;
        }
        break;
      case ID_CONFIG_EXT_LOUDNESS_INFO:
        /* mpegh3daLoudnessInfoSet(); */
        if (extIsPresent & (1 << ID_CONFIG_EXT_LOUDNESS_INFO)) {
          return TRANSPORTDEC_PARSE_ERROR;
        }
        extIsPresent |= (1 << ID_CONFIG_EXT_LOUDNESS_INFO);
        {
          if (cb->cbUniDrc != NULL) {
            ErrorStatus = (TRANSPORTDEC_ERROR)cb->cbUniDrc(
                cb->cbUniDrcData, hBs, usacConfigExtLength, 1, /* loudnessInfoSet */
                tmp_subStreamIndex, loudnessInfoSetConfigExtensionPosition, tmp_aot);
            if (ErrorStatus != TRANSPORTDEC_OK) {
              return ErrorStatus;
            }
            loudnessInfoSetIndex = confExtIdx;
          }
        }
        break;
      case ID_CONFIG_EXT_AUDIOSCENE_INFO:
        if (extIsPresent & (1 << ID_CONFIG_EXT_AUDIOSCENE_INFO)) {
          return TRANSPORTDEC_PARSE_ERROR;
        }
        extIsPresent |= (1 << ID_CONFIG_EXT_AUDIOSCENE_INFO);
        if (usacConfigExtLength > 0 && pASI != NULL) {
          int i;
          int numMaxElementIDs = (usc->numAudioChannels + usc->numAudioObjects);
          for (i = 0; i < usc->bsNumSignalGroups; i++) {
            if (usc->m_signalGroupType[i].type == 3 /* HOA */) {
              /* for HOA one ID per signal group */
              numMaxElementIDs++;
            }
          }

          asiReset(pASI);

          if (mae_AudioSceneInfo(pASI, hBs, numMaxElementIDs, usc->subStreamIndex) != 0) {
            return TRANSPORTDEC_PARSE_ERROR;
          }
        }
        break;
      case ID_CONFIG_EXT_HOA_MATRIX:
        /* HoaRenderingMatrixSet(); */
        break;
      case ID_CONFIG_EXT_SIG_GROUP_INFO:
        for (int grp = 0; grp < usc->bsNumSignalGroups; grp++) {
          FDKreadBits(hBs, 3); /* groupPriority[grp] */
          FDKreadBits(hBs, 1); /* fixedPosition[grp] */
        }
        break;
      default:
        break;
    }

    /* Skip remaining bits. If too many bits were parsed, assume error. */
    usacConfigExtLength = 8 * usacConfigExtLength - (nbits - FDKgetValidBits(hBs));
    if (usacConfigExtLength < 0) {
      return TRANSPORTDEC_PARSE_ERROR;
    }
    FDKpushFor(hBs, usacConfigExtLength);
  }

  if (loudnessInfoSetIndex == -1 && cb->cbUniDrc != NULL) {
    /* no loudnessInfoSet contained. Clear the loudnessInfoSet struct by feeding an empty config
     * extension */
    ErrorStatus = (TRANSPORTDEC_ERROR)cb->cbUniDrc(
        cb->cbUniDrcData, NULL, 0, 1 /* loudnessInfoSet */, tmp_subStreamIndex, 0, tmp_aot);
    if (ErrorStatus != TRANSPORTDEC_OK) {
      return ErrorStatus;
    }
  }

  return ErrorStatus;
}

/* This function unifies decoder config parsing of USAC and MPEGH:
   mpegh3daDecoderConfig() ISO/IEC DIS 23008-3   Table 8
   UsacDecoderConfig()     ISO/IEC FDIS 23003-3  Table 6
  */
static TRANSPORTDEC_ERROR UsacMpegHDecoderConfig_Parse(CSAudioSpecificConfig* asc,
                                                       HANDLE_FDK_BITSTREAM hBs,
                                                       const CSTpCallBacks* cb) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;
  CSUsacConfig* usc = &asc->m_sc.m_usacConfig;
  int i, numberOfElements;
  int channelElementIdx = 0; /* index for elements which contain audio channels (sce, cpe, lfe) */
  int grp = 0;               /* index of current signal group */
  int cntSignals = 0;        /* count number of signals according to one group */
  int nbits;
  int mpeghMCTElement = -1;
  USAC_EXT_ELEMENT_TYPE lastExtElement =
      ID_EXT_ELE_UNKNOWN; /* Last extension element bevore signal element. */
  int uniDrcElement = -1; /* index of uniDrc extension element. -1 if not contained. */

  numberOfElements = (int)escapedValue(hBs, 4, 8, 16) + 1;
  if (asc->m_aot == AOT_MPEGH3DA) {
    asc->m_sc.m_usacConfig.elementLengthPresent = FDKreadBit(hBs);
  }
  usc->m_usacNumElements = numberOfElements;
  if (numberOfElements > TP_USAC_MAX_ELEMENTS) {
    return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  }
  usc->m_nUsacChannels = 0;
  usc->m_channelConfigurationIndex = asc->m_channelConfiguration;

  for (i = 0; i < numberOfElements; i++) {
    MP4_ELEMENT_ID usacElementType =
        (MP4_ELEMENT_ID)(FDKreadBits(hBs, 2) |
                         USAC_ID_BIT); /* set USAC_ID_BIT to map usacElementType to MP4_ELEMENT_ID
                                          enum */
    usc->element[i].usacElementType = usacElementType;

    if (asc->m_aot == AOT_MPEGH3DA) {
      /* calculate the current group */
      if (IS_CHANNEL_ELEMENT(usacElementType)) {
        cntSignals++;
        if (usacElementType == ID_USAC_CPE) cntSignals++;
        /* check whether we are in the next signal group PART I */
        /* This is for the case we have no extension element in one group  */
        if ((cntSignals > asc->m_sc.m_usacConfig.m_signalGroupType[grp].count) &&
            (asc->m_sc.m_usacConfig.bsNumSignalGroups > 1)) {
          cntSignals -= asc->m_sc.m_usacConfig.m_signalGroupType[grp].count;
          grp++;
          if ((cntSignals > asc->m_sc.m_usacConfig.m_signalGroupType[grp].count) ||
              (grp >= asc->m_sc.m_usacConfig.bsNumSignalGroups)) {
            return TRANSPORTDEC_PARSE_ERROR;
          }
          mpeghMCTElement = -1;
        }
        lastExtElement = ID_EXT_ELE_UNKNOWN;
      }
    }

    switch (usacElementType) {
      case ID_USAC_SCE:
        /* UsacCoreConfig() ISO/IEC FDIS 23003-3  Table 10 */
        if (FDKreadBit(hBs)) { /* tw_mdct */
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
        }
        if (asc->m_aot == AOT_MPEGH3DA) {
          usc->element[i].fullbandLpd = FDKreadBits(hBs, 1);
        }
        usc->element[i].m_noiseFilling = FDKreadBits(hBs, 1);
        /* end of UsacCoreConfig() */
        if (asc->m_aot == AOT_MPEGH3DA) {
          /* mpegh3daCoreConfig() ISO/IEC FDIS 23008-3  Table 11 */
          usc->element[i].enhancedNoiseFilling = FDKreadBits(hBs, 1);
          if (usc->element[i].enhancedNoiseFilling) {
            usc->element[i].igfUseEnf = FDKreadBit(hBs);
            usc->element[i].igfUseHighRes = FDKreadBit(hBs);
            usc->element[i].igfUseWhitening = FDKreadBit(hBs);
            usc->element[i].igfAfterTnsSynth = FDKreadBit(hBs);
            usc->element[i].igfStartIndex = FDKreadBits(hBs, 5);
            usc->element[i].igfStopIndex = FDKreadBits(hBs, 4);
          }
          /* end of mpegh3daCoreConfig() */
        }
        usc->m_nUsacChannels += 1;
        channelElementIdx++;
        break;

      case ID_USAC_CPE:
        /* UsacCoreConfig() ISO/IEC FDIS 23003-3  Table 10 */
        if (FDKreadBit(hBs)) { /* tw_mdct */
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
        }
        if (asc->m_aot == AOT_MPEGH3DA) {
          usc->element[i].fullbandLpd = FDKreadBits(hBs, 1);
        }
        usc->element[i].m_noiseFilling = FDKreadBits(hBs, 1);
        /* end of UsacCoreConfig() */
        if (asc->m_aot == AOT_MPEGH3DA) {
          /* mpegh3daCoreConfig() ISO/IEC FDIS 23008-3  Table 11*/
          usc->element[i].enhancedNoiseFilling = FDKreadBits(hBs, 1);
          if (usc->element[i].enhancedNoiseFilling) {
            usc->element[i].igfUseEnf = FDKreadBit(hBs);
            usc->element[i].igfUseHighRes = FDKreadBit(hBs);
            usc->element[i].igfUseWhitening = FDKreadBit(hBs);
            usc->element[i].igfAfterTnsSynth = FDKreadBit(hBs);
            usc->element[i].igfStartIndex = FDKreadBits(hBs, 5);
            usc->element[i].igfStopIndex = FDKreadBits(hBs, 4);
          }
          /* end of mpegh3daCoreConfig() */

          /* IGF IndependentTiling*/
          if (usc->element[i].enhancedNoiseFilling) {
            usc->element[i].igfIndependentTiling = FDKreadBits(hBs, 1);
          }
        }
        { usc->element[i].m_stereoConfigIndex = 0; }
        usc->m_nUsacChannels += 2;

        if (asc->m_aot == AOT_MPEGH3DA) {
          UINT sum_tmp;
          UINT qceIndex;

          /* parse quad channel element config */

          /* formula for calculating nBits q.v.: ISO/IEC DIS 23008-3  Table 10
          nbits = floor(log2(numAudioChannels + numAudioObjects + numHOATransportChannels +
          numSAOCTransportChannels - 1)) + 1
          */
          sum_tmp = usc->numAudioChannels + usc->numAudioObjects + usc->numHOATransportChannels - 1;
          if (sum_tmp <= 0) return TRANSPORTDEC_PARSE_ERROR;
          nbits = 15 - fixnormz_S((SHORT)sum_tmp) + 1;

          qceIndex = FDKreadBits(hBs, 2);
          if (qceIndex != 0) {
            /* QCE not supported by MPEG-H LC Profile */
            return TRANSPORTDEC_PARSE_ERROR;
          }
          usc->element[i].shiftIndex1 = FDKreadBits(hBs, 1);
          if (usc->element[i].shiftIndex1 > 0) {
            usc->element[i].shiftChannel1 = FDKreadBits(hBs, nbits);
          }
          if (usc->m_sbrRatioIndex == 0 /* && (usc->element[i].qceIndex == 0) */) {
            usc->element[i].lpdStereoIndex = FDKreadBit(hBs);
          }
        }
        channelElementIdx++;
        break;

      case ID_USAC_LFE:
        /* Check if MCT is active and if this LFE is part of the MCT signals, because that is not
         * valid. */
        if (mpeghMCTElement != -1) {
          if ((cntSignals > TP_MAX_CHANNELS_PER_SIGNAL_GROUP) ||
              (usc->element[mpeghMCTElement]
                   .extElement.extConfig.mct.mctChanMask[cntSignals - 1])) {
            return ErrorStatus = TRANSPORTDEC_PARSE_ERROR;
          }
        }
        usc->element[i].fullbandLpd = 0;
        usc->element[i].enhancedNoiseFilling = 0;
        usc->element[i].m_noiseFilling = 0;
        usc->m_nUsacChannels += 1;
        channelElementIdx++;
        break;

      case ID_USAC_EXT:
        /* check whether we are in the next signal group PART II */
        /* works only if extension elements comes first */
        if ((cntSignals >= asc->m_sc.m_usacConfig.m_signalGroupType[grp].count) &&
            (asc->m_sc.m_usacConfig.bsNumSignalGroups > 1)) {
          cntSignals -= asc->m_sc.m_usacConfig.m_signalGroupType[grp].count;
          FDK_ASSERT(cntSignals == 0);

          grp++;
          if (grp > asc->m_sc.m_usacConfig.bsNumSignalGroups - 1)
            grp = asc->m_sc.m_usacConfig.bsNumSignalGroups - 1;
        }

        // asc->m_sc.m_usacConfig.m_signalGroupType[grp].type;  /* Type 0: Channel signals / 1:
        // Object signals / 2: SAOC signals / 3: HOA signals / Other: USAC */
        ErrorStatus = extElementConfig(&usc->element[i].extElement, hBs, cb,
                                       asc->m_sc.m_usacConfig.m_signalGroupType[grp].count,
                                       asc->m_sc.m_usacConfig.objNumSignalGroups,
                                       asc->m_sc.m_usacConfig.chNumSignalGroups,
                                       asc->m_samplesPerFrame, usc->subStreamIndex, asc->m_aot);
        if (usc->element[i].extElement.usacExtElementType == ID_EXT_ELE_MCT) {
          mpeghMCTElement = i;
        }
        if (usc->element[i].extElement.usacExtElementType != ID_EXT_ELE_FILL) {
          lastExtElement = usc->element[i].extElement.usacExtElementType;
        }
        if (usc->element[i].extElement.usacExtElementType == ID_EXT_ELE_PROD_METADATA && i > 0) {
          for (int j = i - 1; j >= 0; j--) {
            if (IS_CHANNEL_ELEMENT(usc->element[j].usacElementType)) {
              return TRANSPORTDEC_PARSE_ERROR; /* ID_EXT_ELE_PROD_METADATA must precede channel
                                                  elements */
            } else if (usc->element[j].extElement.usacExtElementType == ID_EXT_ELE_PROD_METADATA) {
              return TRANSPORTDEC_PARSE_ERROR; /* ID_EXT_ELE_PROD_METADATA can only occur once */
            }
          }
        }
        if (usc->element[i].extElement.usacExtElementType == ID_EXT_ELE_UNI_DRC) {
          uniDrcElement = i;
        }

        if (ErrorStatus) {
          return ErrorStatus;
        }
        break;

      default:
        /* non USAC-element encountered */
        return TRANSPORTDEC_PARSE_ERROR;
    }
  }

  /* sanity check */
  if ((asc->m_aot == AOT_MPEGH3DA) &&
      (usc->m_nUsacChannels !=
           (asc->m_sc.m_usacConfig.numAudioChannels + asc->m_sc.m_usacConfig.numAudioObjects +
            asc->m_sc.m_usacConfig.numHOATransportChannels) ||
       (lastExtElement != ID_EXT_ELE_UNKNOWN && lastExtElement != ID_EXT_ELE_FILL))) {
    return TRANSPORTDEC_PARSE_ERROR;
  }

  if (uniDrcElement == -1 && cb->cbUniDrc != NULL) {
    /* no uniDrcConfig contained. Clear the uniDrcConfig struct by feeding an empty extension
     * element */
    int subStreamIndex = 0;
    subStreamIndex = usc->subStreamIndex;
    ErrorStatus = (TRANSPORTDEC_ERROR)cb->cbUniDrc(cb->cbUniDrcData, NULL, 0, 0 /* uniDrcConfig */,
                                                   subStreamIndex, 0, asc->m_aot);
    if (ErrorStatus != TRANSPORTDEC_OK) {
      return ErrorStatus;
    }
  }

  return ErrorStatus;
}

static int Ch_isExplicitCenter(int azimuth_angle) {
  int ret = 0;

  if ((azimuth_angle == 0) || (azimuth_angle == 180)) {
    ret = 1;
  }

  return ret;
}

static int SpeakerConfig3d(HANDLE_FDK_BITSTREAM hBs, mpegh_speaker_t* speakers, int* pNumSpeakers) {
  int cicp = 0;
  int speakerLayoutType;

  speakerLayoutType = FDKreadBits(hBs, 2);
  if (speakerLayoutType == 0) {
    CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
    CICP2GEOMETRY_ERROR error;
    int nLfe;

    cicp = FDKreadBits(hBs, 6); /* CICPspeakerLayoutIdx */
    if (cicp >= MAX_CICP_VALUE) {
      return -1;
    }

    error = cicp2geometry_get_geometry_from_cicp(cicp, AzElLfe, pNumSpeakers, &nLfe);
    if (error != CICP2GEOMETRY_OK) {
      return -1;
    }
    *pNumSpeakers += nLfe;

    for (int i = 0; i < *pNumSpeakers; i++) {
      speakers[i].Az = AzElLfe[i].Az;
      speakers[i].El = AzElLfe[i].El;
      speakers[i].Lfe = AzElLfe[i].LFE;
    }
  } else {
    int numSpeakers = escapedValue(hBs, 5, 8, 16) + 1;
    if (numSpeakers > TP_USAC_MAX_SPEAKERS) {
      return -1;
    }
    *pNumSpeakers = numSpeakers;
    if (speakerLayoutType == 1) {
      int i;
      for (i = 0; i < numSpeakers; i++) {
        CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe;
        CICP2GEOMETRY_ERROR error;
        int cicp_idx;

        cicp_idx = FDKreadBits(hBs, 7); /* CICPspeakerIdx */
        error = cicp2geometry_get_geometry_from_cicp_loudspeaker_index(cicp_idx, &AzElLfe);
        if (error != CICP2GEOMETRY_OK) {
          return -1;
        }
        speakers[i].Az = AzElLfe.Az;
        speakers[i].El = AzElLfe.El;
        speakers[i].Lfe = AzElLfe.LFE;
      }
    } else if (speakerLayoutType == 2) {
      int i, angularPrecision = FDKreadBits(hBs, 1);
      for (i = 0; i < numSpeakers; i++) {
        int isCICPspeakerIdx = FDKreadBit(hBs);

        if (isCICPspeakerIdx) {
          CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe;
          CICP2GEOMETRY_ERROR error;
          int cicp_idx;

          cicp_idx = FDKreadBits(hBs, 7); /* CICPspeakerIdx */
          error = cicp2geometry_get_geometry_from_cicp_loudspeaker_index(cicp_idx, &AzElLfe);
          if (error != CICP2GEOMETRY_OK) {
            return -1;
          }
          speakers[i].Az = AzElLfe.Az;
          speakers[i].El = AzElLfe.El;
          speakers[i].Lfe = AzElLfe.LFE;
        } else {
          unsigned int ElevationClass = FDKreadBits(hBs, 2);

          switch (ElevationClass) {
            case 0:
              speakers[i].El = 0;
              break;
            case 1:
              speakers[i].El = 35;
              break;
            case 2:
              speakers[i].El = -15;
              break;
            case 3:
              int ElevationAngleIdx = FDKreadBits(hBs, angularPrecision ? 7 : 5);

              if (ElevationAngleIdx != 0 /* Table 44 */) {
                int ElevationDirection = FDKreadBit(hBs); /* ElevationDirection */
                if (ElevationDirection) {
                  ElevationAngleIdx = -ElevationAngleIdx;
                }
              }
              speakers[i].El = ElevationAngleIdx * (angularPrecision ? 1 : 5);
              break;
          }
          {
            int AzimuthAngleIdx;
            AzimuthAngleIdx = FDKreadBits(hBs, angularPrecision ? 8 : 6);

            speakers[i].Az = AzimuthAngleIdx * (angularPrecision ? 1 : 5);
            if (Ch_isExplicitCenter(speakers[i].Az) == 0) {
              int AzimuthDirection = FDKreadBit(hBs); /* AzimuthDirection */
              if (AzimuthDirection) {
                speakers[i].Az = -speakers[i].Az;
              }
            }
          }
          speakers[i].Lfe = FDKreadBit(hBs); /* isLFE; */
        }

        if (Ch_isExplicitCenter(speakers[i].Az) == 0) {
          int alsoAddSymmetricPair = FDKreadBit(hBs);
          if (alsoAddSymmetricPair) {
            /* Also add the speaker with the opposite AzimuthDirection */
            i++;
            speakers[i].Az = -speakers[i - 1].Az;
            speakers[i].El = speakers[i - 1].El;
            speakers[i].Lfe = 0;
          }
        }
      }
    } else {
      return -2; /* speakerLayoutType == 3 is Contribution Mode */
    }
  }
  return cicp;
}

static TRANSPORTDEC_ERROR MpegHConfig_Parse(CSAudioSpecificConfig* asc, AUDIO_SCENE_INFO* pASI,
                                            HANDLE_FDK_BITSTREAM hBs, const CSTpCallBacks* cb) {
  int usacSamplingFrequency, numSpeakersRefLayout, referenceLayout;

  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  CSUsacConfig* usc = &asc->m_sc.m_usacConfig;

  usc->mpegh3daProfileLevelIndication = FDKreadBits(hBs, 8);

  usacSamplingFrequency = getSampleRate(hBs, &asc->m_samplingFrequencyIndex, 5);
  switch (usacSamplingFrequency) {
    case 48000:
    case 44100:
    case 32000:
    case 29400:
    case 24000:
    case 22050:
    case 16000:
    case 14700:
      break;

    default:
      /* unsupported sampling frequency */
      return TRANSPORTDEC_PARSE_ERROR;
  }

  asc->m_samplingFrequency = (UINT)usacSamplingFrequency;

  /* coreSbrFrameLengthIndex */
  if (FDKreadBits(hBs, 3) != 1) {
    return TRANSPORTDEC_PARSE_ERROR;
  }
  asc->m_sc.m_usacConfig.m_coreSbrFrameLengthIndex = 1;
  asc->m_samplesPerFrame = 1024;
  asc->m_sc.m_usacConfig.m_sbrRatioIndex = 0;

  FDKreadBit(hBs); /* coreQmfDelayCompensation */

  FDKreadBit(hBs); /* receiverDelayCompensation */

  asc->m_channelConfiguration = 0;

  referenceLayout = SpeakerConfig3d(hBs, usc->speakers, &numSpeakersRefLayout);
  if (referenceLayout < 0) {
    if (referenceLayout == -2) { /* contribution mode */
      referenceLayout = 0;
    } else {
      return TRANSPORTDEC_PARSE_ERROR;
    }
  }
  usc->referenceLayout = (UCHAR)referenceLayout;

  /* FrameworkConfig3d() */
  {
    /* Signals3d() */
    int grp, signalGroupType_grp_prev = 0;
    int sigIdx = 0;
    usc->chNumSignalGroups = 0;
    usc->objNumSignalGroups = 0;
    usc->bsNumSignalGroups = FDKreadBits(hBs, 5) + 1;
    if (usc->bsNumSignalGroups > TP_MPEGH_MAX_SIGNAL_GROUPS) {
      return TRANSPORTDEC_PARSE_ERROR;
    }
    for (grp = 0; grp < usc->bsNumSignalGroups; grp++) {
      int signalGroupType_grp = FDKreadBits(hBs, 3);

      int numberOfSignals_grp = escapedValue(hBs, 5, 8, 16) + 1; /* bsNumberOfSignals_grp + 1 */
      if ((sigIdx + numberOfSignals_grp) > TP_MPEGH_MAX_SIGNAL_GROUPS) {
        return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }

      if (numberOfSignals_grp > TP_MAX_CHANNELS_PER_SIGNAL_GROUP) {
        return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }

      /* Check signal group type order */
      if (signalGroupType_grp < signalGroupType_grp_prev) {
        return TRANSPORTDEC_PARSE_ERROR;
      }
      signalGroupType_grp_prev = signalGroupType_grp;

      usc->m_signalGroupType[grp].Layout = usc->referenceLayout;

      usc->m_signalGroupType[grp].count = (UCHAR)numberOfSignals_grp;
      usc->m_signalGroupType[grp].type = signalGroupType_grp;
      usc->m_signalGroupType[grp].firstSigIdx = sigIdx;

      /* Assign speaker description memory for signal group. */
      usc->m_signalGroupType[grp].speakers = &usc->m_signalGroupTypeSpeakers[sigIdx];

      for (int i = 0; i < numberOfSignals_grp; i++) {
        usc->sigIdx2GrpIdx[sigIdx] = grp;
        sigIdx++;
      }

      switch (signalGroupType_grp) {
        case 0: /* SignalGroupTypeChannels */
          usc->chNumSignalGroups++;
          usc->numAudioChannels += numberOfSignals_grp;
          int num_speakers;

          if (FDKreadBit(hBs) /* differsFromReferenceLayout */) {
            int Layout = SpeakerConfig3d(hBs, usc->m_signalGroupType[grp].speakers, &num_speakers);
            if (Layout < 0) {
              return TRANSPORTDEC_PARSE_ERROR;
            }
            usc->m_signalGroupType[grp].Layout = (UCHAR)Layout;
          } else {
            usc->m_signalGroupType[grp].Layout = usc->referenceLayout;
            usc->m_signalGroupType[grp].speakers = usc->speakers;
            num_speakers = numSpeakersRefLayout;
          }
          if (numberOfSignals_grp != num_speakers) {
            return TRANSPORTDEC_PARSE_ERROR;
          }
          break;
        case 1: /* SignalGroupTypeObject */
          usc->objSignalGroupsIndices[usc->objNumSignalGroups] =
              grp;                   /* store indice of current signal group */
          usc->objNumSignalGroups++; /* count singal groups of type object */
          usc->numAudioObjects += numberOfSignals_grp;
          break;
        case 3: /* SignalGroupTypeHOA */
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
        default:
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }
    }
  }

  err = UsacMpegHDecoderConfig_Parse(asc, hBs, cb);

  if (err != TRANSPORTDEC_OK) {
    return err;
  }

  if (FDKreadBits(hBs, 1)) { /* usacConfigExtensionPresent */
    err = configExtension(usc, pASI, asc->m_aot, hBs, cb);
  } else if (cb->cbUniDrc != NULL) {
    /* no loudnessInfoSet contained. Clear the loudnessInfoSet struct by feeding an empty config
     * extension */
    int subStreamIndex = 0;
    subStreamIndex = usc->subStreamIndex;
    err = (TRANSPORTDEC_ERROR)cb->cbUniDrc(cb->cbUniDrcData, NULL, 0, 1 /* loudnessInfoSet */,
                                           subStreamIndex, 0, asc->m_aot);
    if (err != TRANSPORTDEC_OK) {
      return err;
    }
  }

  return err;
}

/*
 * API Functions
 */

void AudioSpecificConfig_Init(CSAudioSpecificConfig* asc) {
  FDKmemclear(asc, sizeof(CSAudioSpecificConfig));

  /* Init all values that should not be zero. */
  asc->m_aot = AOT_NONE;
  asc->m_samplingFrequencyIndex = 0xf;
  asc->m_epConfig = -1;
  asc->m_extensionAudioObjectType = AOT_NULL_OBJECT;
}

TRANSPORTDEC_ERROR Mpegh3daConfig_Parse(CSAudioSpecificConfig* self, AUDIO_SCENE_INFO* pASI,
                                        HANDLE_FDK_BITSTREAM bs,
                                        const int fExplicitBackwardCompatible,
                                        const CSTpCallBacks* cb, const UCHAR configMode,
                                        const UCHAR configChanged, const INT targetLayout,
                                        const INT subStreamIndex, INT* pLoudnessInfoSetPosition) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;

  /* Temporal code in order to compare and signal a change in one of the signal groups */
  AudioSpecificConfig_Init(self);

  self->m_sc.m_usacConfig.subStreamIndex = subStreamIndex;
  self->m_sc.m_usacConfig.targetLayout = targetLayout;

  self->configMode = configMode;
  self->AacConfigChanged = configChanged;
  self->SbrConfigChanged = configChanged;
  self->SacConfigChanged = configChanged;

  self->m_aot = AOT_MPEGH3DA;

  self->m_samplingFrequency =
      0; /* => gets overwritten with usacSamplingFrequencyIndex in MpegHConfig_Parse() */
  self->m_channelConfiguration =
      0; /* => gets overwritten with channelConfigurationIndex in MpegHConfig_Parse() */

  /* SBR extension ( explicit non-backwards compatible mode ) */
  self->m_sbrPresentFlag = 0;
  self->m_psPresentFlag = 0;
  self->m_extensionAudioObjectType = AOT_NULL_OBJECT;

  if (subStreamIndex == 0) {
    pLoudnessInfoSetPosition[0] = FDKgetValidBits(bs);
    pLoudnessInfoSetPosition[1] = 0;
    pLoudnessInfoSetPosition[2] = 0;
  }

  /* parse MPEGH 3D audio config, q.v. ISO/IEC DIS 23008-3  mpegh3daConfig() */
  ErrorStatus = MpegHConfig_Parse(self, pASI, bs, cb);

  return ErrorStatus;
}

// Syntax of earconInfo()
TRANSPORTDEC_ERROR earconInfo(HANDLE_FDK_BITSTREAM bs, EarconInfo* info) {
  info->m_numEarcons = FDKreadBits(bs, 7) + 1;
  if (info->m_numEarcons > EARCON_MAX_NUM_SIGNALS) {
    return TRANSPORTDEC_PARSE_ERROR;
  }
  for (UINT i = 0; i < info->m_numEarcons; i++) {
    FDKreadBits(bs, 1);                            // earconIsIndependent
    FDKreadBits(bs, 7);                            // earconID
    FDKreadBits(bs, 4);                            // earconType
    FDKreadBits(bs, 1);                            // earconActive
    UINT earconPositionType = FDKreadBits(bs, 2);  // earconPositionType
    if (earconPositionType == 0)
      FDKreadBits(bs, 7);
    else {
      if (earconPositionType == 1) {
        FDKreadBits(bs, 8);  // earcon_azimuth
        FDKreadBits(bs, 6);  // earcon_elevation
        FDKreadBits(bs, 9);  // earcon_distance
      }
    }

    if (FDKreadBits(bs, 1))  // earconHasGain
      FDKreadBits(bs, 7);    // earcon_gain

    if (FDKreadBits(bs, 1)) {  // earconHasTextLabel
      UINT earconNumLanguages = FDKreadBits(bs, 4);
      for (UINT n = 0; n < earconNumLanguages; n++) {
        FDKreadBits(bs, 24);                                                 // earconLanguage
        UINT earconTextDataLength = FDKreadBits(bs, 8);                      // earconTextDataLength
        for (UINT c = 0; c < earconTextDataLength; c++) FDKreadBits(bs, 8);  // earconTextData
      }
    }
  }
  return TRANSPORTDEC_OK;
}

TRANSPORTDEC_ERROR pcmDataConfig(HANDLE_FDK_BITSTREAM bs, EarconConfig* config) {
  config->m_numPcmSignals = FDKreadBits(bs, 7) + 1;  // numPcmSignals
  if (config->m_numPcmSignals > EARCON_MAX_NUM_SIGNALS) {
    return TRANSPORTDEC_PARSE_ERROR;
  }
  FDKreadBits(bs, 1);              // pcmAlignAudioFlag
  if (FDKreadBits(bs, 5) == 0x1f)  // pcmSamplingRateIndex
    FDKreadBits(bs, 24);           // pcmSamplingRate

  FDKreadBits(bs, 3);  // pcmBitsPerSampleIndex

  config->m_bsPcmFrameSize_index = FDKreadBits(bs, 3);
  switch (config->m_bsPcmFrameSize_index) {
    case 2:
      config->m_bsPcmFrameSize = 1024;
      break;

    case 3:
      config->m_bsPcmFrameSize = 1024;
      break;

    case 5:
      config->m_bsPcmFrameSize = FDKreadBits(bs, 16);  // pcmFixFrameSize
      break;
    default:
      break;
  }

  for (unsigned i = 0; i < config->m_numPcmSignals; i++) FDKreadBits(bs, 7);  // pcmSignal_ID[i]

  config->m_bsPcmLoudnessValue = FDKreadBits(bs, 8);     // bsPcmLoudnessValue
  config->m_pcmHasAttenuationGain = FDKreadBits(bs, 2);  // pcmHasAttenuationGain

  if (config->m_pcmHasAttenuationGain == 1)
    config->m_bsPcmAttenuationGain = FDKreadBits(bs, 8);  // bsPcmAttenuationGain

  return TRANSPORTDEC_OK;
}
