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

#include "fixpoint_math.h"
#include "drcDec_reader.h"
#include "drcDec_tools.h"
#include "drcDec_rom.h"
#include "drcDecoder.h"

/* MPEG-D DRC AMD 1 */

#define UNIDRCGAINEXT_TERM 0x0
#define UNIDRCLOUDEXT_TERM 0x0
#define UNIDRCCONFEXT_TERM 0x0

static int _getZ(const int nNodesMax) {
  /* Z is the minimum codeword length that is needed to encode all possible timeDelta values */
  /* Z = ceil(log2(2*nNodesMax)) */
  int Z = 1;
  while ((1 << Z) < (2 * nNodesMax)) {
    Z++;
  }
  return Z;
}

static int _getTimeDeltaMin(const GAIN_SET* pGset, const int deltaTminDefault) {
  if (pGset->timeDeltaMinPresent) {
    return pGset->timeDeltaMin;
  } else {
    return deltaTminDefault;
  }
}

/* compare and assign */
static inline int _compAssign(UCHAR* dest, const UCHAR src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int _compAssign(USHORT* dest, const USHORT src) {
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

static inline int _compAssign(FIXP_SGL* dest, const FIXP_SGL src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

typedef const SCHAR (*Huffman)[2];

static int _decodeHuffmanCW(Huffman h,                /*!< pointer to huffman codebook table */
                            HANDLE_FDK_BITSTREAM hBs) /*!< Handle to bitbuffer */
{
  SCHAR index = 0;
  int value, bit;

  while (index >= 0) {
    bit = FDKreadBits(hBs, 1);
    index = h[index][bit];
  }

  value = index + 64; /* Add offset */

  return value;
}

/**********/
/* uniDrc */
/**********/

/**************/
/* uniDrcGain */
/**************/

static FIXP_SGL _decodeGainInitial(HANDLE_FDK_BITSTREAM hBs,
                                   const GAIN_CODING_PROFILE gainCodingProfile) {
  int sign, magn;
  FIXP_SGL gainInitial = (FIXP_SGL)0;
  switch (gainCodingProfile) {
    case GCP_REGULAR:
      sign = FDKreadBits(hBs, 1);
      magn = FDKreadBits(hBs, 8);

      gainInitial = (FIXP_SGL)(magn << (FRACT_BITS - 1 - 3 - 7)); /* magn * 0.125; */
      if (sign) gainInitial = -gainInitial;
      break;
    case GCP_FADING:
      sign = FDKreadBits(hBs, 1);
      if (sign == 0)
        gainInitial = (FIXP_SGL)0;
      else {
        magn = FDKreadBits(hBs, 10);
        gainInitial =
            (FIXP_SGL)(-(magn + 1) << (FRACT_BITS - 1 - 3 - 7)); /* - (magn + 1) * 0.125; */
      }
      break;
    case GCP_CLIPPING_DUCKING:
      sign = FDKreadBits(hBs, 1);
      if (sign == 0)
        gainInitial = (FIXP_SGL)0;
      else {
        magn = FDKreadBits(hBs, 8);
        gainInitial =
            (FIXP_SGL)(-(magn + 1) << (FRACT_BITS - 1 - 3 - 7)); /* - (magn + 1) * 0.125; */
      }
      break;
    case GCP_CONSTANT:
      break;
  }
  return gainInitial;
}

static int _decodeNNodes(HANDLE_FDK_BITSTREAM hBs) {
  int nNodes = 0, endMarker = 0;

  /* decode number of nodes */
  while (endMarker != 1) {
    nNodes++;
    if (nNodes >= 128) break;
    endMarker = FDKreadBits(hBs, 1);
  }
  return nNodes;
}

static void _decodeGains(HANDLE_FDK_BITSTREAM hBs, const GAIN_CODING_PROFILE gainCodingProfile,
                         const int nNodes, GAIN_NODE* pNodes) {
  int k, deltaGain;
  Huffman deltaGainCodebook;

  pNodes[0].gainDb = _decodeGainInitial(hBs, gainCodingProfile);

  if (gainCodingProfile == GCP_CLIPPING_DUCKING) {
    deltaGainCodebook = (Huffman)&deltaGain_codingProfile_2_huffman;
  } else {
    deltaGainCodebook = (Huffman)&deltaGain_codingProfile_0_1_huffman;
  }

  for (k = 1; k < nNodes; k++) {
    deltaGain = _decodeHuffmanCW(deltaGainCodebook, hBs);
    if (k >= 32) continue;
    /* gain_dB_e = 7 */
    pNodes[k].gainDb = (pNodes[k - 1].gainDb >> 1) +
                       (FIXP_SGL)(deltaGain << (FRACT_BITS - 1 - 7 - 3 -
                                                1)); /* pNodes[k-1].gainDb + 0.125*deltaGain */
    pNodes[k].gainDb = SATURATE_LEFT_SHIFT(pNodes[k].gainDb, 1, FRACT_BITS);
  }
}

static void _decodeSlopes(HANDLE_FDK_BITSTREAM hBs,
                          const GAIN_INTERPOLATION_TYPE gainInterpolationType, const int nNodes,
                          GAIN_NODE* pNodes) {
  int k = 0;

  if (gainInterpolationType == GIT_SPLINE) {
    /* decode slope steepness */
    for (k = 0; k < nNodes; k++) {
      _decodeHuffmanCW((Huffman)&slopeSteepness_huffman, hBs);
    }
  }
}

static int _decodeTimeDelta(HANDLE_FDK_BITSTREAM hBs, const int Z) {
  int prefix, mu;

  prefix = FDKreadBits(hBs, 2);
  switch (prefix) {
    case 0x0:
      return 1;
    case 0x1:
      mu = FDKreadBits(hBs, 2);
      return mu + 2;
    case 0x2:
      mu = FDKreadBits(hBs, 3);
      return mu + 6;
    case 0x3:
      mu = FDKreadBits(hBs, Z);
      return mu + 14;
    default:
      return 0;
  }
}

static void _decodeTimes(HANDLE_FDK_BITSTREAM hBs, const int deltaTmin, const int frameSize,
                         const int fullFrame, const int timeOffset, const int Z, const int nNodes,
                         GAIN_NODE* pNodes) {
  int timeDelta, k;
  int timeOffs = timeOffset;
  int frameEndFlag, nodeTimeTmp, nodeResFlag;

  if (fullFrame == 0) {
    frameEndFlag = FDKreadBits(hBs, 1);
  } else {
    frameEndFlag = 1;
  }

  if (frameEndFlag ==
      1) { /* frameEndFlag == 1 signals that the last node is at the end of the DRC frame */
    nodeResFlag = 0;
    for (k = 0; k < nNodes - 1; k++) {
      /* decode a delta time value */
      timeDelta = _decodeTimeDelta(hBs, Z);
      if (k >= (32 - 1)) continue;
      /* frameEndFlag == 1 needs special handling for last node with node reservoir */
      nodeTimeTmp = timeOffs + timeDelta * deltaTmin;
      if (nodeTimeTmp > frameSize + timeOffset) {
        if (nodeResFlag == 0) {
          pNodes[k].time = frameSize + timeOffset;
          nodeResFlag = 1;
        }
        pNodes[k + 1].time = nodeTimeTmp;
      } else {
        pNodes[k].time = nodeTimeTmp;
      }
      timeOffs = nodeTimeTmp;
    }
    if (nodeResFlag == 0) {
      k = fMin(k, 32 - 1);
      pNodes[k].time = frameSize + timeOffset;
    }
  } else {
    for (k = 0; k < nNodes; k++) {
      /* decode a delta time value */
      timeDelta = _decodeTimeDelta(hBs, Z);
      if (k >= 32) continue;
      pNodes[k].time = timeOffs + timeDelta * deltaTmin;
      timeOffs = pNodes[k].time;
    }
  }
}

static void _readNodes(HANDLE_FDK_BITSTREAM hBs, GAIN_SET* gainSet, const int frameSize,
                       const int timeDeltaMin, UCHAR* pNNodes, GAIN_NODE* pNodes) {
  int timeOffset, drcGainCodingMode, nNodes;
  int Z = _getZ(frameSize / timeDeltaMin);
  if (gainSet->timeAlignment == 0) {
    timeOffset = -1;
  } else {
    timeOffset = -timeDeltaMin +
                 (timeDeltaMin - 1) / 2; /* timeOffset = - deltaTmin + floor((deltaTmin-1)/2); */
  }

  drcGainCodingMode = FDKreadBits(hBs, 1);
  if (drcGainCodingMode == 0) {
    /* "simple" mode: only one node at the end of the frame with slope = 0 */
    nNodes = 1;
    pNodes[0].gainDb = _decodeGainInitial(hBs, (GAIN_CODING_PROFILE)gainSet->gainCodingProfile);
    pNodes[0].time = frameSize + timeOffset;
  } else {
    nNodes = _decodeNNodes(hBs);

    _decodeSlopes(hBs, (GAIN_INTERPOLATION_TYPE)gainSet->gainInterpolationType, nNodes, pNodes);

    _decodeTimes(hBs, timeDeltaMin, frameSize, gainSet->fullFrame, timeOffset, Z, nNodes, pNodes);

    _decodeGains(hBs, (GAIN_CODING_PROFILE)gainSet->gainCodingProfile, nNodes, pNodes);
  }
  *pNNodes = (UCHAR)nNodes;
}

static void _readDrcGainSequence(HANDLE_FDK_BITSTREAM hBs, GAIN_SET* gainSet, const int frameSize,
                                 const int timeDeltaMin, UCHAR* pNNodes, GAIN_NODE pNodes[32]) {
  SHORT timeBufPrevFrame[32], timeBufCurFrame[32];
  int nNodesNodeRes, nNodesCur, k, m;

  if (gainSet->gainCodingProfile == GCP_CONSTANT) {
    *pNNodes = 1;
    pNodes[0].time = frameSize - 1;
    pNodes[0].gainDb = (FIXP_SGL)0;
  } else {
    _readNodes(hBs, gainSet, frameSize, timeDeltaMin, pNNodes, pNodes);

    /* count number of nodes in node reservoir */
    nNodesNodeRes = 0;
    nNodesCur = 0;
    /* count and buffer nodes from node reservoir */
    for (k = 0; k < *pNNodes; k++) {
      if (k >= 32) break;
      if (pNodes[k].time >= 2 * frameSize) break; /* time is out of range */
      if (pNodes[k].time >= frameSize) {
        /* write node reservoir times into buffer */
        timeBufPrevFrame[nNodesNodeRes] = pNodes[k].time;
        nNodesNodeRes++;
      } else { /* times from current frame */
        timeBufCurFrame[nNodesCur] = pNodes[k].time;
        nNodesCur++;
      }
    }
    /* restrict nNodes to valid ones if break occured */
    *pNNodes = k;

    /* insert dummy node if no node is available */
    if (*pNNodes == 0) {
      *pNNodes = 1;
      pNodes[0].time = frameSize - 1;
      pNodes[0].gainDb = (FIXP_SGL)0;
    }

    /* compose right time order (bit reservoir first) */
    for (k = 0; k < nNodesNodeRes; k++) {
      /* subtract two time frameSize: one to remove node reservoir offset and one to get the
       * negative index relative to the current frame
       */
      pNodes[k].time = timeBufPrevFrame[k] - 2 * frameSize;
    }
    /* ...and times from current frame */
    for (m = 0; m < nNodesCur; m++, k++) {
      pNodes[k].time = timeBufCurFrame[m];
    }
  }
}

static DRC_ERROR _readUniDrcGainExtension(HANDLE_FDK_BITSTREAM hBs, UNI_DRC_GAIN_EXTENSION* pExt) {
  DRC_ERROR err = DE_OK;
  int k, bitSizeLen, extSizeBits, bitSize;

  k = 0;
  pExt->uniDrcGainExtType[k] = FDKreadBits(hBs, 4);
  while (pExt->uniDrcGainExtType[k] != UNIDRCGAINEXT_TERM) {
    if (k >= (8 - 1)) return DE_MEMORY_ERROR;
    bitSizeLen = FDKreadBits(hBs, 3);
    extSizeBits = bitSizeLen + 4;

    bitSize = FDKreadBits(hBs, extSizeBits);
    pExt->extBitSize[k] = bitSize + 1;

    switch (pExt->uniDrcGainExtType[k]) {
      /* add future extensions here */
      default:
        FDKpushFor(hBs, pExt->extBitSize[k]);
        break;
    }
    k++;
    pExt->uniDrcGainExtType[k] = FDKreadBits(hBs, 4);
  }

  return err;
}

static int _getUniDrcGainStatus(DRC_COEFFICIENTS_UNI_DRC* pCoef, HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  /* The status of uniDrcGain is set to 1 if all expected gain sequences have been read.
  If the status stays 0, concealment should be applied. */
  int s;
  for (s = 0; s < 4; s++) {
    if (hUniDrcGain->nDecodedSequences[s] != pCoef->streamGainSequenceCount[s]) return 0;
  }

  return 1;
}

DRC_ERROR
drcDec_readUniDrcGain(HANDLE_FDK_BITSTREAM hBs, DRC_COEFFICIENTS_UNI_DRC* pCoef,
                      const int frameSize, const int deltaTminDefault, const int subStreamIndex,
                      HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  DRC_ERROR err = DE_OK;
  int seq, gainSequenceCount = 0;
  int startIndexGainSequence = 0;
  int sequenceSkipped = 0;
  if (hUniDrcGain == NULL) return DE_NOT_OK;
  if (subStreamIndex >= 4) return DE_OK;

  if (pCoef) {
    int s;
    /* accumulate gainSequenceCount from lower substreams */
    for (s = 0; s < subStreamIndex; s++) {
      startIndexGainSequence += pCoef->streamGainSequenceCount[s];
    }
    gainSequenceCount = pCoef->streamGainSequenceCount[subStreamIndex];
  } else {
    hUniDrcGain->status = 0;
    return DE_OK;
  }

  for (seq = 0; seq < gainSequenceCount; seq++) {
    GAIN_SET* gainSet;
    int timeDeltaMin;
    UCHAR index, tmpNNodes = 0;
    GAIN_NODE tmpNodes[32];

    if ((startIndexGainSequence + seq) >= 48) {
      sequenceSkipped = 1;
      continue;
    }

    /* insert the gainSequences of this substream, starting from startIndexGainSequence */
    index = pCoef->gainSetIndexForGainSequence[startIndexGainSequence + seq];
    if ((index >= pCoef->gainSetCount) || (index >= 48)) return DE_NOT_OK;
    gainSet = &(pCoef->gainSet[index]);

    timeDeltaMin = _getTimeDeltaMin(gainSet, deltaTminDefault);

    _readDrcGainSequence(hBs, gainSet, frameSize, timeDeltaMin, &tmpNNodes, tmpNodes);

    hUniDrcGain->nNodes[startIndexGainSequence + seq] = tmpNNodes;
    FDKmemcpy(hUniDrcGain->gainNode[startIndexGainSequence + seq], tmpNodes,
              fMin(tmpNNodes, (UCHAR)32) * sizeof(GAIN_NODE));
  }

  if (!sequenceSkipped) { /* all sequences have been read */
    hUniDrcGain->uniDrcGainExtPresent = FDKreadBits(hBs, 1);
    if (hUniDrcGain->uniDrcGainExtPresent == 1) {
      if (subStreamIndex != 0)
        return DE_NOT_OK; /* uniDrcGainExtension is not allowed for side-stream */

      err = _readUniDrcGainExtension(hBs, &(hUniDrcGain->uniDrcGainExtension));
      if (err) return err;
    }
  }

  if (err == DE_OK) {
    /* keep number of read (or skipped) gain sequences for this substream. */
    hUniDrcGain->nDecodedSequences[subStreamIndex] = gainSequenceCount;
  }

  hUniDrcGain->status = _getUniDrcGainStatus(pCoef, hUniDrcGain);

  return err;
}

/****************/
/* uniDrcConfig */
/****************/

static void _decodeDuckingModification(HANDLE_FDK_BITSTREAM hBs, DUCKING_MODIFICATION* pDMod) {
  int duckingScalingPresent, bsDuckingScaling, sigma, mu;

  duckingScalingPresent = FDKreadBits(hBs, 1);

  if (duckingScalingPresent) {
    bsDuckingScaling = FDKreadBits(hBs, 4);
    sigma = bsDuckingScaling >> 3;
    mu = bsDuckingScaling & 0x7;

    if (sigma) {
      pDMod->duckingScaling =
          (FIXP_SGL)((7 - mu) << (FRACT_BITS - 1 - 3 - 2)); /* 1.0 - 0.125 * (1 + mu); */
    } else {
      pDMod->duckingScaling =
          (FIXP_SGL)((9 + mu) << (FRACT_BITS - 1 - 3 - 2)); /* 1.0 + 0.125 * (1 + mu); */
    }
  } else {
    pDMod->duckingScaling = (FIXP_SGL)(1 << (FRACT_BITS - 1 - 2)); /* 1.0 */
  }
}

static void _decodeGainModification(HANDLE_FDK_BITSTREAM hBs, const int version, int bandCount,
                                    GAIN_MODIFICATION* pGMod) {
  int sign, bsGainOffset, bsAttenuationScaling, bsAmplificationScaling;
  int gainScalingPresent, gainOffsetPresent;

  {
    int b;
    FIXP_SGL attenuationScaling = FL2FXCONST_SGL(1.0f / (float)(1 << 2)),
             amplificationScaling = FL2FXCONST_SGL(1.0f / (float)(1 << 2)),
             gainOffset = (FIXP_SGL)0;
    gainScalingPresent = FDKreadBits(hBs, 1);
    if (gainScalingPresent) {
      bsAttenuationScaling = FDKreadBits(hBs, 4);
      attenuationScaling =
          (FIXP_SGL)(bsAttenuationScaling
                     << (FRACT_BITS - 1 - 3 - 2)); /* bsAttenuationScaling * 0.125; */
      bsAmplificationScaling = FDKreadBits(hBs, 4);
      amplificationScaling =
          (FIXP_SGL)(bsAmplificationScaling
                     << (FRACT_BITS - 1 - 3 - 2)); /* bsAmplificationScaling * 0.125; */
    }
    gainOffsetPresent = FDKreadBits(hBs, 1);
    if (gainOffsetPresent) {
      sign = FDKreadBits(hBs, 1);
      bsGainOffset = FDKreadBits(hBs, 5);
      gainOffset =
          (FIXP_SGL)((1 + bsGainOffset) << (FRACT_BITS - 1 - 2 - 4)); /* (1+bsGainOffset) * 0.25; */
      if (sign) {
        gainOffset = -gainOffset;
      }
    }
    for (b = 0; b < 4; b++) {
      pGMod[b].attenuationScaling = attenuationScaling;
      pGMod[b].amplificationScaling = amplificationScaling;
      pGMod[b].gainOffset = gainOffset;
    }
  }
}

static void _readDrcCharacteristic(HANDLE_FDK_BITSTREAM hBs, const int version,
                                   DRC_CHARACTERISTIC* pDChar) {
  if (version == 0) {
    pDChar->cicpIndex = FDKreadBits(hBs, 7);
    if (pDChar->cicpIndex > 0) {
      pDChar->present = 1;
      pDChar->isCICP = 1;
    } else {
      pDChar->present = 0;
    }
  }
}

static void _readBandBorder(HANDLE_FDK_BITSTREAM hBs, BAND_BORDER* pBBord, int drcBandType) {
  if (drcBandType) {
    pBBord->crossoverFreqIndex = FDKreadBits(hBs, 4);
  } else {
    pBBord->startSubBandIndex = FDKreadBits(hBs, 10);
  }
}

static DRC_ERROR _readGainSet(HANDLE_FDK_BITSTREAM hBs, const int version, int* gainSequenceIndex,
                              GAIN_SET* pGSet) {
  pGSet->gainCodingProfile = FDKreadBits(hBs, 2);
  pGSet->gainInterpolationType = FDKreadBits(hBs, 1);
  pGSet->fullFrame = FDKreadBits(hBs, 1);
  pGSet->timeAlignment = FDKreadBits(hBs, 1);
  pGSet->timeDeltaMinPresent = FDKreadBits(hBs, 1);

  if (pGSet->timeDeltaMinPresent) {
    int bsTimeDeltaMin;
    bsTimeDeltaMin = FDKreadBits(hBs, 11);
    pGSet->timeDeltaMin = bsTimeDeltaMin + 1;
  }

  if (pGSet->gainCodingProfile != GCP_CONSTANT) {
    int i;
    pGSet->bandCount = FDKreadBits(hBs, 4);
    if (pGSet->bandCount > 4) return DE_MEMORY_ERROR;

    if (pGSet->bandCount > 1) {
      pGSet->drcBandType = FDKreadBits(hBs, 1);
    }

    for (i = 0; i < pGSet->bandCount; i++) {
      if (version == 0) {
        *gainSequenceIndex = (*gainSequenceIndex) + 1;
      }
      pGSet->gainSequenceIndex[i] = *gainSequenceIndex;
      _readDrcCharacteristic(hBs, version, &(pGSet->drcCharacteristic[i]));
    }
    for (i = 1; i < pGSet->bandCount; i++) {
      _readBandBorder(hBs, &(pGSet->bandBorder[i]), pGSet->drcBandType);
    }
  } else {
    pGSet->bandCount = 1;
    *gainSequenceIndex = (*gainSequenceIndex) + 1;
    pGSet->gainSequenceIndex[0] = *gainSequenceIndex;
  }

  return DE_OK;
}

static void _deriveGainSetIndexForGainSequence(DRC_COEFFICIENTS_UNI_DRC* pCoef) {
  int gs, b;
  for (gs = 0; gs < 48; gs++) {
    pCoef->gainSetIndexForGainSequence[gs] = 255;
  }
  for (gs = 0; gs < pCoef->gainSetCount; gs++) {
    for (b = 0; b < pCoef->gainSet[gs].bandCount; b++) {
      if (pCoef->gainSet[gs].gainSequenceIndex[b] >= 48) continue;
      pCoef->gainSetIndexForGainSequence[pCoef->gainSet[gs].gainSequenceIndex[b]] = gs;
    }
  }
}

static DRC_ERROR _readDrcCoefficientsUniDrc(HANDLE_FDK_BITSTREAM hBs, const int version,
                                            DRC_COEFFICIENTS_UNI_DRC* pCoef) {
  DRC_ERROR err = DE_OK;
  int i, bsDrcFrameSize;
  int gainSequenceIndex = -1;

  pCoef->drcLocation = FDKreadBits(hBs, 4);
  pCoef->drcFrameSizePresent = FDKreadBits(hBs, 1);

  if (pCoef->drcFrameSizePresent == 1) {
    bsDrcFrameSize = FDKreadBits(hBs, 15);
    pCoef->drcFrameSize = bsDrcFrameSize + 1;
  }
  if (version == 0) {
    int gainSequenceCount = 0, gainSetCount;
    gainSetCount = FDKreadBits(hBs, 6);
    pCoef->gainSetCount = fMin(gainSetCount, 48);
    for (i = 0; i < gainSetCount; i++) {
      GAIN_SET tmpGset;
      FDKmemclear(&tmpGset, sizeof(GAIN_SET));
      err = _readGainSet(hBs, version, &gainSequenceIndex, &tmpGset);
      if (err) return err;
      gainSequenceCount += tmpGset.bandCount;

      if (i >= 48) continue;
      pCoef->gainSet[i] = tmpGset;
    }
    pCoef->gainSequenceCount = gainSequenceCount;
  }

  _deriveGainSetIndexForGainSequence(pCoef);

  return err;
}

static DRC_ERROR _mergeSubstreamDrcCoefficients(DRC_COEFFICIENTS_UNI_DRC* pCoefSubstream,
                                                DRC_COEFFICIENTS_UNI_DRC* pCoef,
                                                const int subStreamIndex, int* pDiff) {
  int b, gs, s, tmp_diff = 0;
  int startIndexGainSet, startIndexGainSequence;
  tmp_diff |= _compAssign(&pCoef->drcLocation, pCoefSubstream->drcLocation);
  tmp_diff |= _compAssign(&pCoef->drcFrameSizePresent, pCoefSubstream->drcFrameSizePresent);
  tmp_diff |= _compAssign(&pCoef->drcFrameSize, pCoefSubstream->drcFrameSize);

  if (subStreamIndex == 0) {
    *pDiff |= tmp_diff;
  } else {
    /* check if side stream fits to main stream */
    if (tmp_diff) return DE_NOT_OK;
  }

  /* merge together substream element to full element */
  *pDiff |= _compAssign(&pCoef->streamGainSetCount[subStreamIndex], pCoefSubstream->gainSetCount);
  *pDiff |= _compAssign(&pCoef->streamGainSequenceCount[subStreamIndex],
                        pCoefSubstream->gainSequenceCount);

  /* accumulate gainSetCount and gainSequenceCount */
  pCoef->gainSetCount = 0;
  pCoef->gainSequenceCount = 0;
  for (s = 0; s < 4; s++) {
    pCoef->gainSetCount += pCoef->streamGainSetCount[s];
    pCoef->gainSequenceCount += pCoef->streamGainSequenceCount[s];
  }
  pCoef->gainSetCount = fMin((INT)pCoef->gainSetCount, 48);

  /* accumulate gainSetCount and gainSequenceCount from lower substreams */
  startIndexGainSet = 0;
  startIndexGainSequence = 0;
  for (s = 0; s < subStreamIndex; s++) {
    startIndexGainSet += pCoef->streamGainSetCount[s];
    startIndexGainSequence += pCoef->streamGainSequenceCount[s];
  }

  /* insert the gainSet elements of this substream, starting from startIndexGainSet */
  for (gs = 0; gs < pCoef->streamGainSetCount[subStreamIndex]; gs++) {
    if ((startIndexGainSet + gs) >= 48) continue;
    /* adapt gainSequenceIndex */
    for (b = 0; b < pCoefSubstream->gainSet[gs].bandCount; b++) {
      pCoefSubstream->gainSet[gs].gainSequenceIndex[b] += startIndexGainSequence;
    }
    if (!*pDiff)
      *pDiff |= (FDKmemcmp(&pCoefSubstream->gainSet[gs], &pCoef->gainSet[startIndexGainSet + gs],
                           sizeof(GAIN_SET)) != 0);
    pCoef->gainSet[startIndexGainSet + gs] = pCoefSubstream->gainSet[gs];
  }

  _deriveGainSetIndexForGainSequence(pCoef);

  return DE_OK;
}

static DRC_ERROR _readDrcInstructionsUniDrc(HANDLE_FDK_BITSTREAM hBs, const int version,
                                            HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                            const UCHAR baseChannelCount,
                                            DRC_INSTRUCTIONS_UNI_DRC* pInst) {
  DRC_ERROR err = DE_OK;
  int i, g, c;
  int downmixIdPresent, additionalDownmixIdPresent, additionalDownmixIdCount;
  int bsLimiterPeakTarget, channelCount;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = NULL;
  int repeatParameters, bsRepeatParametersCount;
  int repeatSequenceIndex, bsRepeatSequenceCount;
  SCHAR* gainSetIndex = pInst->gainSetIndex;
  SCHAR channelGroupForChannel[2 * 28];
  DUCKING_MODIFICATION duckingModificationForChannelGroup[28];

  pInst->drcSetId = FDKreadBits(hBs, 6);
  if (version == 0) {
    /* Assume all v0 DRC sets to be manageable in terms of complexity */
    pInst->drcSetComplexityLevel = 2;
  } else {
    pInst->drcSetComplexityLevel = FDKreadBits(hBs, 4);
  }
  pInst->drcLocation = FDKreadBits(hBs, 4);
  if (version == 0) {
    downmixIdPresent = 1;
  } else {
    downmixIdPresent = FDKreadBits(hBs, 1);
  }
  if (downmixIdPresent) {
    pInst->downmixId[0] = FDKreadBits(hBs, 7);
    if (version == 0) {
      if (pInst->downmixId[0] == 0)
        pInst->drcApplyToDownmix = 0;
      else
        pInst->drcApplyToDownmix = 1;
    } else {
      pInst->drcApplyToDownmix = FDKreadBits(hBs, 1);
    }

    additionalDownmixIdPresent = FDKreadBits(hBs, 1);
    if (additionalDownmixIdPresent) {
      additionalDownmixIdCount = FDKreadBits(hBs, 3);
      if ((1 + additionalDownmixIdCount) > 8) return DE_MEMORY_ERROR;
      for (i = 0; i < additionalDownmixIdCount; i++) {
        pInst->downmixId[i + 1] = FDKreadBits(hBs, 7);
      }
      pInst->downmixIdCount = 1 + additionalDownmixIdCount;
    } else {
      pInst->downmixIdCount = 1;
    }
  } else {
    pInst->downmixId[0] = 0;
    pInst->downmixIdCount = 1;
  }

  pInst->drcSetEffect = FDKreadBits(hBs, 16);

  if ((pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) == 0) {
    pInst->limiterPeakTargetPresent = FDKreadBits(hBs, 1);
    if (pInst->limiterPeakTargetPresent) {
      bsLimiterPeakTarget = FDKreadBits(hBs, 8);
      pInst->limiterPeakTarget =
          -(FIXP_SGL)(bsLimiterPeakTarget
                      << (FRACT_BITS - 1 - 3 - 5)); /* - bsLimiterPeakTarget * 0.125; */
    }
  }

  pInst->drcSetTargetLoudnessPresent = FDKreadBits(hBs, 1);

  /* set default values */
  pInst->drcSetTargetLoudnessValueUpper = 0;
  pInst->drcSetTargetLoudnessValueLower = -63;

  if (pInst->drcSetTargetLoudnessPresent == 1) {
    int bsDrcSetTargetLoudnessValueUpper, bsDrcSetTargetLoudnessValueLower;
    int drcSetTargetLoudnessValueLowerPresent;
    bsDrcSetTargetLoudnessValueUpper = FDKreadBits(hBs, 6);
    pInst->drcSetTargetLoudnessValueUpper = bsDrcSetTargetLoudnessValueUpper - 63;
    drcSetTargetLoudnessValueLowerPresent = FDKreadBits(hBs, 1);
    if (drcSetTargetLoudnessValueLowerPresent == 1) {
      bsDrcSetTargetLoudnessValueLower = FDKreadBits(hBs, 6);
      pInst->drcSetTargetLoudnessValueLower = bsDrcSetTargetLoudnessValueLower - 63;
    }
  }

  pInst->dependsOnDrcSetPresent = FDKreadBits(hBs, 1);

  pInst->noIndependentUse = 0;
  if (pInst->dependsOnDrcSetPresent) {
    pInst->dependsOnDrcSet = FDKreadBits(hBs, 6);
  } else {
    pInst->noIndependentUse = FDKreadBits(hBs, 1);
  }

  if (version == 0) {
    pInst->requiresEq = 0;
  } else {
    pInst->requiresEq = FDKreadBits(hBs, 1);
  }

  pCoef = selectDrcCoefficients(hUniDrcConfig, pInst->drcLocation);

  channelCount = baseChannelCount;
  pInst->drcChannelCount = baseChannelCount;

  if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    DUCKING_MODIFICATION* pDModForChannel = pInst->duckingModificationForChannel;
    c = 0;
    while (c < channelCount) {
      int bsGainSetIndex;
      bsGainSetIndex = FDKreadBits(hBs, 6);
      if (c >= 2 * 28) return DE_MEMORY_ERROR;
      gainSetIndex[c] = bsGainSetIndex - 1;
      _decodeDuckingModification(hBs, &(pDModForChannel[c]));

      c++;
      repeatParameters = FDKreadBits(hBs, 1);
      if (repeatParameters == 1) {
        bsRepeatParametersCount = FDKreadBits(hBs, 5);
        bsRepeatParametersCount += 1;
        for (i = 0; i < bsRepeatParametersCount; i++) {
          if (c >= 2 * 28) return DE_MEMORY_ERROR;
          gainSetIndex[c] = gainSetIndex[c - 1];
          pDModForChannel[c] = pDModForChannel[c - 1];
          c++;
        }
      }
    }
    if (c > channelCount) {
      return DE_NOT_OK;
    }

    err = deriveDrcChannelGroups(pInst->drcSetEffect, pInst->drcChannelCount, gainSetIndex,
                                 pDModForChannel, &pInst->nDrcChannelGroups,
                                 pInst->gainSetIndexForChannelGroup, channelGroupForChannel,
                                 duckingModificationForChannelGroup);
    if (err) return (err);
  } else {
    int deriveChannelCount = 0;
    if (((version == 0) || (pInst->drcApplyToDownmix != 0)) &&
        (pInst->downmixId[0] != DOWNMIX_ID_BASE_LAYOUT) &&
        (pInst->downmixId[0] != DOWNMIX_ID_ANY_DOWNMIX) && (pInst->downmixIdCount == 1)) {
      if (hUniDrcConfig->downmixInstructionsCount != 0) {
        DOWNMIX_INSTRUCTIONS* pDown = selectDownmixInstructions(hUniDrcConfig, pInst->downmixId[0]);
        if (pDown == NULL) return DE_NOT_OK;
        pInst->drcChannelCount = channelCount =
            pDown->targetChannelCount; /* targetChannelCountFromDownmixId*/
      } else {
        deriveChannelCount = 1;
        channelCount = 1;
      }
    } else if (((version == 0) || (pInst->drcApplyToDownmix != 0)) &&
               ((pInst->downmixId[0] == DOWNMIX_ID_ANY_DOWNMIX) || (pInst->downmixIdCount > 1))) {
      /* Set maximum channel count as upper border. The effective channel count is set at the
       * process function. */
      pInst->drcChannelCount = 2 * 28;
      channelCount = 1;
    }

    c = 0;
    while (c < channelCount) {
      int bsGainSetIndex;
      bsGainSetIndex = FDKreadBits(hBs, 6);
      if (c >= 2 * 28) return DE_MEMORY_ERROR;
      gainSetIndex[c] = bsGainSetIndex - 1;
      c++;
      repeatSequenceIndex = FDKreadBits(hBs, 1);

      if (repeatSequenceIndex == 1) {
        bsRepeatSequenceCount = FDKreadBits(hBs, 5);
        bsRepeatSequenceCount += 1;
        if (deriveChannelCount) {
          channelCount = 1 + bsRepeatSequenceCount;
        }
        for (i = 0; i < bsRepeatSequenceCount; i++) {
          if (c >= 2 * 28) return DE_MEMORY_ERROR;
          gainSetIndex[c] = bsGainSetIndex - 1;
          c++;
        }
      }
    }
    if (c > channelCount) {
      return DE_NOT_OK;
    }
    if (deriveChannelCount) {
      pInst->drcChannelCount = channelCount;
    }

    /* DOWNMIX_ID_ANY_DOWNMIX: channelCount is 1. Distribute gainSetIndex to all channels. */
    if ((pInst->downmixId[0] == DOWNMIX_ID_ANY_DOWNMIX) || (pInst->downmixIdCount > 1)) {
      for (c = 1; c < pInst->drcChannelCount; c++) {
        gainSetIndex[c] = gainSetIndex[0];
      }
    }

    err = deriveDrcChannelGroups(pInst->drcSetEffect, pInst->drcChannelCount, gainSetIndex, NULL,
                                 &pInst->nDrcChannelGroups, pInst->gainSetIndexForChannelGroup,
                                 channelGroupForChannel, NULL);
    if (err) return (err);

    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      int set, bandCount;
      set = pInst->gainSetIndexForChannelGroup[g];

      /* get bandCount */
      if (pCoef != NULL && set < pCoef->gainSetCount) {
        bandCount = pCoef->gainSet[set].bandCount;
      } else {
        bandCount = 1;
      }

      _decodeGainModification(hBs, version, bandCount, pInst->gainModificationForChannelGroup[g]);
    }
  }

  return err;
}

static DRC_ERROR _mergeSubstreamDrcInstructions(DRC_INSTRUCTIONS_UNI_DRC* pInstSubstream,
                                                DRC_INSTRUCTIONS_UNI_DRC* pInst,
                                                HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                const int subStreamIndex, int* pDiff) {
  int d, s, c, cg, tmp_diff = 0;
  int startIndexChannel, startIndexChannelGroup, startIndexGainSet;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = selectDrcCoefficients(hUniDrcConfig, pInst->drcLocation);

  tmp_diff |= _compAssign(&pInst->drcInstructionsType, pInstSubstream->drcInstructionsType);
  if (pInst->drcInstructionsType != 0) {
    if (pInst->drcInstructionsType == 2) {
      tmp_diff |= _compAssign(&pInst->mae_groupID, pInstSubstream->mae_groupID);
    } else if (pInst->drcInstructionsType == 3) {
      tmp_diff |= _compAssign(&pInst->mae_groupPresetID, pInstSubstream->mae_groupPresetID);
    }
  }
  tmp_diff |= _compAssign(&pInst->drcSetId, pInstSubstream->drcSetId);
  tmp_diff |= _compAssign(&pInst->drcSetComplexityLevel, pInstSubstream->drcSetComplexityLevel);

  tmp_diff |= _compAssign(&pInst->drcLocation, pInstSubstream->drcLocation);
  tmp_diff |= _compAssign(&pInst->drcApplyToDownmix, pInstSubstream->drcApplyToDownmix);
  tmp_diff |= _compAssign(&pInst->downmixIdCount, pInstSubstream->downmixIdCount);
  for (d = 0; d < pInst->downmixIdCount; d++) {
    tmp_diff |= _compAssign(&pInst->downmixId[d], pInstSubstream->downmixId[d]);
  }
  tmp_diff |= _compAssign(&pInst->drcSetEffect, pInstSubstream->drcSetEffect);
  tmp_diff |=
      _compAssign(&pInst->limiterPeakTargetPresent, pInstSubstream->limiterPeakTargetPresent);
  if (pInst->limiterPeakTargetPresent) {
    tmp_diff |= _compAssign(&pInst->limiterPeakTarget, pInstSubstream->limiterPeakTarget);
  }
  tmp_diff |=
      _compAssign(&pInst->drcSetTargetLoudnessPresent, pInstSubstream->drcSetTargetLoudnessPresent);
  if (pInst->drcSetTargetLoudnessPresent) {
    tmp_diff |= _compAssign(&pInst->drcSetTargetLoudnessValueUpper,
                            pInstSubstream->drcSetTargetLoudnessValueUpper);
    tmp_diff |= _compAssign(&pInst->drcSetTargetLoudnessValueLower,
                            pInstSubstream->drcSetTargetLoudnessValueLower);
  }
  tmp_diff |= _compAssign(&pInst->dependsOnDrcSetPresent, pInstSubstream->dependsOnDrcSetPresent);
  if (pInst->dependsOnDrcSetPresent) {
    tmp_diff |= _compAssign(&pInst->dependsOnDrcSet, pInstSubstream->dependsOnDrcSet);
  } else {
    tmp_diff |= _compAssign(&pInst->noIndependentUse, pInstSubstream->noIndependentUse);
  }

  if (subStreamIndex == 0) {
    *pDiff |= tmp_diff;
  } else {
    /* check if side stream fits to main stream */
    if (tmp_diff) return DE_NOT_OK;
    /* side streams are valid only for DRC-1. */
    if ((pInst->downmixIdCount != 1) && (pInst->downmixId[0] != DOWNMIX_ID_BASE_LAYOUT))
      return DE_NOT_OK;
    if (pInstSubstream->drcChannelCount != hUniDrcConfig->streamChannelCount[subStreamIndex])
      return DE_NOT_OK;
  }

  if ((pInst->downmixIdCount == 1) && (pInst->downmixId[0] == DOWNMIX_ID_BASE_LAYOUT)) {
    /* DRC-1: accumulated baseChannelCount of all streams */
    pInst->drcChannelCount = hUniDrcConfig->channelLayout.baseChannelCount;
  } else { /* DRC-2/3: use existing drcChannelCount*/
    pInst->drcChannelCount = pInstSubstream->drcChannelCount;
  }

  *pDiff |= _compAssign(&pInst->streamNDrcChannelGroups[subStreamIndex],
                        pInstSubstream->nDrcChannelGroups);

  /* accumulate nDrcChannelGroups */
  pInst->nDrcChannelGroups = 0;
  for (s = 0; s < 4; s++) {
    pInst->nDrcChannelGroups += pInst->streamNDrcChannelGroups[s];
  }
  if (pInst->nDrcChannelGroups > 28) return DE_NOT_OK;

  /* accumulate streamChannelCount, nDrcChannelGroups and gainSetCount from lower substreams */
  startIndexChannel = 0;
  startIndexChannelGroup = 0;
  startIndexGainSet = 0;
  for (s = 0; s < subStreamIndex; s++) {
    startIndexChannel += hUniDrcConfig->streamChannelCount[s];
    startIndexChannelGroup += pInst->streamNDrcChannelGroups[s];
    if (pCoef) {
      startIndexGainSet += pCoef->streamGainSetCount[s];
    }
  }

  if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    for (c = 0; c < pInstSubstream->drcChannelCount; c++) {
      if ((startIndexChannel + c) >= 2 * 28) continue;

      /* insert the duckingModificationForChannel elements of this substream, starting from
       * startIndexChannel */
      if (!*pDiff)
        *pDiff |= (FDKmemcmp(&pInstSubstream->duckingModificationForChannel[c],
                             &pInst->duckingModificationForChannel[startIndexChannel + c],
                             sizeof(DUCKING_MODIFICATION)) != 0);
      pInst->duckingModificationForChannel[startIndexChannel + c] =
          pInstSubstream->duckingModificationForChannel[c];
    }
  } else {
    for (cg = 0; cg < pInst->streamNDrcChannelGroups[subStreamIndex]; cg++) {
      int b;
      if ((startIndexChannelGroup + cg) >= 28) continue;

      /* insert the gainModificationForChannelGroup elements of this substream, starting from
       * startIndexChannelGroup */
      for (b = 0; b < 4; b++) {
        if (!*pDiff)
          *pDiff |= (FDKmemcmp(pInstSubstream->gainModificationForChannelGroup[cg],
                               pInst->gainModificationForChannelGroup[startIndexChannelGroup + cg],
                               4 * sizeof(GAIN_MODIFICATION)) != 0);
        pInst->gainModificationForChannelGroup[startIndexChannelGroup + cg][b] =
            pInstSubstream->gainModificationForChannelGroup[cg][b];
      }
    }
  }

  /* insert the gainSetIndex elements of this substream, starting from startIndexChannel */
  for (c = 0; c < pInstSubstream->drcChannelCount; c++) {
    if ((startIndexChannel + c) >= 2 * 28) continue;
    *pDiff |= _compAssign(&pInst->gainSetIndex[startIndexChannel + c],
                          pInstSubstream->gainSetIndex[c] + startIndexGainSet);
  }

  /* insert the gainSetIndexForChannelGroup elements of this substream, starting from
   * startIndexChannelGroup */
  for (cg = 0; cg < pInst->streamNDrcChannelGroups[subStreamIndex]; cg++) {
    if ((startIndexChannelGroup + cg) >= 28) continue;
    *pDiff |= _compAssign(&pInst->gainSetIndexForChannelGroup[startIndexChannelGroup + cg],
                          pInstSubstream->gainSetIndexForChannelGroup[cg] + startIndexGainSet);
  }

  return DE_OK;
}

static void _readMpegh3daUniDrcChannelLayout(HANDLE_FDK_BITSTREAM hBs, CHANNEL_LAYOUT* pChan) {
  pChan->baseChannelCount = FDKreadBits(hBs, 7);
  pChan->layoutSignalingPresent = 0;
}

static DRC_ERROR _readUniDrcConfigExtension(HANDLE_FDK_BITSTREAM hBs,
                                            HANDLE_UNI_DRC_CONFIG hUniDrcConfig) {
  DRC_ERROR err = DE_OK;
  int k, bitSizeLen, extSizeBits, bitSize;
  UNI_DRC_CONFIG_EXTENSION* pExt = &(hUniDrcConfig->uniDrcConfigExt);

  k = 0;
  pExt->uniDrcConfigExtType[k] = FDKreadBits(hBs, 4);
  while (pExt->uniDrcConfigExtType[k] != UNIDRCCONFEXT_TERM) {
    if (k >= (8 - 1)) return DE_MEMORY_ERROR;
    bitSizeLen = FDKreadBits(hBs, 4);
    extSizeBits = bitSizeLen + 4;

    bitSize = FDKreadBits(hBs, extSizeBits);
    pExt->extBitSize[k] = bitSize + 1;

    switch (pExt->uniDrcConfigExtType[k]) {
      /* add future extensions here */
      default:
        FDKpushFor(hBs, pExt->extBitSize[k]);
        break;
    }
    k++;
    pExt->uniDrcConfigExtType[k] = FDKreadBits(hBs, 4);
  }

  return err;
}

DRC_ERROR
drcDec_readMpegh3daUniDrcConfig(HANDLE_FDK_BITSTREAM hBs, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                const int subStreamIndex) {
  DRC_ERROR err = DE_OK;
  int i, j, diff = 0, uniDrcConfigExtPresent;
  CHANNEL_LAYOUT tmpChan;
  FDKmemclear(&tmpChan, sizeof(CHANNEL_LAYOUT));
  if (hUniDrcConfig == NULL) return DE_NOT_OK;
  if (hLoudnessInfoSet == NULL) return DE_NOT_OK;
  if (subStreamIndex >= 4) return DE_OK;

  diff |= _compAssign(&hUniDrcConfig->drcCoefficientsUniDrcCountV0[subStreamIndex],
                      FDKreadBits(hBs, 3));
  diff |= _compAssign(&hUniDrcConfig->drcInstructionsUniDrcCountV0[subStreamIndex],
                      FDKreadBits(hBs, 6));

  if (subStreamIndex > 0) {
    /* side streams may not have more elements than the main stream */
    if (hUniDrcConfig->drcCoefficientsUniDrcCountV0[subStreamIndex] >
        hUniDrcConfig->drcCoefficientsUniDrcCountV0[0])
      return DE_NOT_OK;
    if (hUniDrcConfig->drcInstructionsUniDrcCountV0[subStreamIndex] >
        hUniDrcConfig->drcInstructionsUniDrcCountV0[0])
      return DE_NOT_OK;
  }

  _readMpegh3daUniDrcChannelLayout(hBs, &tmpChan);

  diff |= _compAssign(&hUniDrcConfig->streamChannelCount[subStreamIndex], tmpChan.baseChannelCount);

  /* accumulate streamChannelCount to get baseChannelCount */
  tmpChan.baseChannelCount = 0;
  for (i = 0; i < 4; i++) {
    tmpChan.baseChannelCount += hUniDrcConfig->streamChannelCount[i];
  }

  if (!diff)
    diff |= (FDKmemcmp(&tmpChan, &hUniDrcConfig->channelLayout, sizeof(CHANNEL_LAYOUT)) != 0);
  hUniDrcConfig->channelLayout = tmpChan;

  hUniDrcConfig->drcCoefficientsUniDrcCount =
      fMin(hUniDrcConfig->drcCoefficientsUniDrcCountV0[0], (UCHAR)2);
  for (i = 0; i < hUniDrcConfig->drcCoefficientsUniDrcCountV0[subStreamIndex]; i++) {
    DRC_COEFFICIENTS_UNI_DRC* tmpCoef = (DRC_COEFFICIENTS_UNI_DRC*)hUniDrcConfig->p_scratch;
    FDKmemclear(tmpCoef, sizeof(DRC_COEFFICIENTS_UNI_DRC));

    err = _readDrcCoefficientsUniDrc(hBs, 0, tmpCoef);
    if (err) return err;

    if (subStreamIndex == 0) {
      /* main stream defines the order of drcCoefficients */
      j = i;
    } else {
      /* for side stream, find existing drcCoefficients with the same drcLocation */
      for (j = 0; j < hUniDrcConfig->drcCoefficientsUniDrcCount; j++) {
        if (hUniDrcConfig->drcInstructionsUniDrc[j].drcLocation == tmpCoef->drcLocation) break;
      }
    }

    if (j >= 2) continue;

    err = _mergeSubstreamDrcCoefficients(tmpCoef, &hUniDrcConfig->drcCoefficientsUniDrc[j],
                                         subStreamIndex, &diff);
    if (err) return err;
  }

  hUniDrcConfig->drcInstructionsUniDrcCount =
      fMin(hUniDrcConfig->drcInstructionsUniDrcCountV0[0], (UCHAR)32);
  for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCountV0[subStreamIndex]; i++) {
    DRC_INSTRUCTIONS_UNI_DRC* tmpInst = (DRC_INSTRUCTIONS_UNI_DRC*)hUniDrcConfig->p_scratch;

    FDKmemclear(tmpInst, sizeof(DRC_INSTRUCTIONS_UNI_DRC));
    tmpInst->drcInstructionsType = FDKreadBits(hBs, 1);
    if (tmpInst->drcInstructionsType != 0) {
      tmpInst->drcInstructionsType = FDKreadBits(hBs, 1) + 2;
      if (tmpInst->drcInstructionsType == 2) {
        tmpInst->mae_groupID = FDKreadBits(hBs, 7);
      } else if (tmpInst->drcInstructionsType == 3) {
        tmpInst->mae_groupPresetID = FDKreadBits(hBs, 5);
      }
    }
    err = _readDrcInstructionsUniDrc(hBs, 0, hUniDrcConfig,
                                     hUniDrcConfig->streamChannelCount[subStreamIndex], tmpInst);
    if (err) return err;

    if (subStreamIndex == 0) {
      /* main stream defines the order of drcInstructions */
      j = i;
    } else {
      /* for side stream, find existing drcInstructions with the same drcSetId */
      for (j = 0; j < hUniDrcConfig->drcInstructionsUniDrcCount; j++) {
        if (hUniDrcConfig->drcInstructionsUniDrc[j].drcSetId == tmpInst->drcSetId) break;
      }
    }

    if (j >= 32) continue;

    err = _mergeSubstreamDrcInstructions(tmpInst, &hUniDrcConfig->drcInstructionsUniDrc[j],
                                         hUniDrcConfig, subStreamIndex, &diff);
    if (err) return err;
  }

  uniDrcConfigExtPresent = FDKreadBits(hBs, 1);
  if (subStreamIndex == 0) {
    diff |= _compAssign(&hUniDrcConfig->uniDrcConfigExtPresent, uniDrcConfigExtPresent);
  } else {
    /* uniDrcConfigExtension is not allowed for side-stream */
    if (uniDrcConfigExtPresent) return DE_NOT_OK;
  }

  hUniDrcConfig->diff |= diff;

  if (uniDrcConfigExtPresent) {
    err = _readUniDrcConfigExtension(hBs, hUniDrcConfig);
    if (err) return err;
  }

  hUniDrcConfig->loudnessInfoSetPresent = FDKreadBits(hBs, 1);
  if (hUniDrcConfig->loudnessInfoSetPresent == 1) {
    err = drcDec_readMpegh3daLoudnessInfoSet(hBs, hLoudnessInfoSet, subStreamIndex);
    if (err) return err;
  }

  return err;
}

/*******************/
/* loudnessInfoSet */
/*******************/

static DRC_ERROR _decodeMethodValue(HANDLE_FDK_BITSTREAM hBs, const UCHAR methodDefinition,
                                    FIXP_DBL* methodValue) {
  int tmp;
  FIXP_DBL val;
  switch (methodDefinition) {
    case MD_UNKNOWN_OTHER:
    case MD_PROGRAM_LOUDNESS:
    case MD_ANCHOR_LOUDNESS:
    case MD_MAX_OF_LOUDNESS_RANGE:
    case MD_MOMENTARY_LOUDNESS_MAX:
    case MD_SHORT_TERM_LOUDNESS_MAX:
      tmp = FDKreadBits(hBs, 8);
      val = FL2FXCONST_DBL(-57.75f / (float)(1 << 7)) +
            (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 2 - 7)); /* -57.75 + tmp * 0.25; */
      break;
    case MD_LOUDNESS_RANGE:
      tmp = FDKreadBits(hBs, 8);
      if (tmp == 0)
        val = (FIXP_DBL)0;
      else if (tmp <= 128)
        val = (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 2 - 7)); /* tmp * 0.25; */
      else if (tmp <= 204) {
        val = (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 1 - 7)) -
              FL2FXCONST_DBL(32.0f / (float)(1 << 7)); /* 0.5 * tmp - 32.0f; */
      } else {
        /* downscale by 1 more bit to prevent overflow at intermediate result */
        val = (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 8)) -
              FL2FXCONST_DBL(134.0f / (float)(1 << 8)); /* tmp - 134.0; */
        val <<= 1;
      }
      break;
    case MD_MIXING_LEVEL:
      tmp = FDKreadBits(hBs, 5);
      val = (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 7)) +
            FL2FXCONST_DBL(80.0f / (float)(1 << 7)); /* tmp + 80.0; */
      break;
    case MD_ROOM_TYPE:
      tmp = FDKreadBits(hBs, 2);
      val = (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 7)); /* tmp; */
      break;
    case MD_SHORT_TERM_LOUDNESS:
      tmp = FDKreadBits(hBs, 8);
      val = FL2FXCONST_DBL(-116.0f / (float)(1 << 7)) +
            (FIXP_DBL)(tmp << (DFRACT_BITS - 1 - 1 - 7)); /* -116.0 + tmp * 0.5; */
      break;
    default:
      return DE_NOT_OK; /* invalid methodDefinition value */
  }
  *methodValue = val;
  return DE_OK;
}

static DRC_ERROR _readLoudnessMeasurement(HANDLE_FDK_BITSTREAM hBs, LOUDNESS_MEASUREMENT* pMeas) {
  DRC_ERROR err = DE_OK;

  pMeas->methodDefinition = FDKreadBits(hBs, 4);
  err = _decodeMethodValue(hBs, pMeas->methodDefinition, &pMeas->methodValue);
  if (err) return err;
  pMeas->measurementSystem = FDKreadBits(hBs, 4);
  pMeas->reliability = FDKreadBits(hBs, 2);

  return err;
}

static DRC_ERROR _readLoudnessInfo(HANDLE_FDK_BITSTREAM hBs, const int version,
                                   LOUDNESS_INFO* loudnessInfo) {
  DRC_ERROR err = DE_OK;
  int bsSamplePeakLevel, bsTruePeakLevel, i;
  int measurementCount;

  loudnessInfo->drcSetId = FDKreadBits(hBs, 6);
  loudnessInfo->downmixId = FDKreadBits(hBs, 7);

  loudnessInfo->samplePeakLevelPresent = FDKreadBits(hBs, 1);
  if (loudnessInfo->samplePeakLevelPresent) {
    bsSamplePeakLevel = FDKreadBits(hBs, 12);
    if (bsSamplePeakLevel == 0) {
      loudnessInfo->samplePeakLevelPresent = 0;
      loudnessInfo->samplePeakLevel = (FIXP_DBL)0;
    } else { /* 20.0 - bsSamplePeakLevel * 0.03125; */
      loudnessInfo->samplePeakLevel = FL2FXCONST_DBL(20.0f / (float)(1 << 7)) -
                                      (FIXP_DBL)(bsSamplePeakLevel << (DFRACT_BITS - 1 - 5 - 7));
    }
  }

  loudnessInfo->truePeakLevelPresent = FDKreadBits(hBs, 1);
  if (loudnessInfo->truePeakLevelPresent) {
    bsTruePeakLevel = FDKreadBits(hBs, 12);
    if (bsTruePeakLevel == 0) {
      loudnessInfo->truePeakLevelPresent = 0;
      loudnessInfo->truePeakLevel = (FIXP_DBL)0;
    } else {
      loudnessInfo->truePeakLevel = FL2FXCONST_DBL(20.0f / (float)(1 << 7)) -
                                    (FIXP_DBL)(bsTruePeakLevel << (DFRACT_BITS - 1 - 5 - 7));
    }
    FDKpushFor(hBs, 4); /* truePeakLevelMeasurementSystem */
    FDKpushFor(hBs, 2); /* truePeakLevelReliability */
  }

  measurementCount = FDKreadBits(hBs, 4);
  loudnessInfo->measurementCount = fMin(measurementCount, 16);
  for (i = 0; i < measurementCount; i++) {
    LOUDNESS_MEASUREMENT tmpMeas;
    FDKmemclear(&tmpMeas, sizeof(LOUDNESS_MEASUREMENT));
    err = _readLoudnessMeasurement(hBs, &tmpMeas);
    if (err) return err;
    if (i >= 16) continue;
    loudnessInfo->loudnessMeasurement[i] = tmpMeas;
  }

  return err;
}

static DRC_ERROR _readLoudnessInfoSetExtension(HANDLE_FDK_BITSTREAM hBs,
                                               HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet) {
  DRC_ERROR err = DE_OK;
  int k, bitSizeLen, extSizeBits, bitSize;
  LOUDNESS_INFO_SET_EXTENSION* pExt = &(hLoudnessInfoSet->loudnessInfoSetExt);

  k = 0;
  pExt->loudnessInfoSetExtType[k] = FDKreadBits(hBs, 4);
  while (pExt->loudnessInfoSetExtType[k] != UNIDRCLOUDEXT_TERM) {
    if (k >= (8 - 1)) return DE_MEMORY_ERROR;
    bitSizeLen = FDKreadBits(hBs, 4);
    extSizeBits = bitSizeLen + 4;

    bitSize = FDKreadBits(hBs, extSizeBits);
    pExt->extBitSize[k] = bitSize + 1;

    switch (pExt->loudnessInfoSetExtType[k]) {
      /* add future extensions here */
      default:
        FDKpushFor(hBs, pExt->extBitSize[k]);
        break;
    }
    k++;
    pExt->loudnessInfoSetExtType[k] = FDKreadBits(hBs, 4);
  }

  return err;
}

/* Parser for mpegh3daLoudnessInfoSet() */
DRC_ERROR
drcDec_readMpegh3daLoudnessInfoSet(HANDLE_FDK_BITSTREAM hBs,
                                   HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                   const int subStreamIndex) {
  DRC_ERROR err = DE_OK;
  int i, loudnessInfoAlbumPresent, loudnessInfoSetExtPresent;
  int diff = 0, startIndex;
  if (hLoudnessInfoSet == NULL) return DE_NOT_OK;
  if (subStreamIndex >= 4) return DE_OK;

  diff |= _compAssign(&hLoudnessInfoSet->loudnessInfoCountV0[subStreamIndex], FDKreadBits(hBs, 6));

  /* accumulate loudnessInfoCountV0 from lower substreams */
  startIndex = 0;
  for (i = 0; i < subStreamIndex; i++) {
    startIndex += hLoudnessInfoSet->loudnessInfoCountV0[i];
  }

  hLoudnessInfoSet->loudnessInfoCount =
      (UCHAR)fMin(startIndex + hLoudnessInfoSet->loudnessInfoCountV0[subStreamIndex], 32);
  for (i = 0; i < hLoudnessInfoSet->loudnessInfoCountV0[subStreamIndex]; i++) {
    LOUDNESS_INFO tmpLoud;
    FDKmemclear(&tmpLoud, sizeof(LOUDNESS_INFO));
    tmpLoud.loudnessInfoType = FDKreadBits(hBs, 2);
    if ((tmpLoud.loudnessInfoType == 1) || (tmpLoud.loudnessInfoType == 2)) {
      tmpLoud.mae_groupID = FDKreadBits(hBs, 7);
    } else if (tmpLoud.loudnessInfoType == 3) {
      tmpLoud.mae_groupPresetID = FDKreadBits(hBs, 5);
    }

    err = _readLoudnessInfo(hBs, 0, &tmpLoud);
    if (err) return err;
    if (startIndex + i >= 32) continue;

    /* insert the loudnessInfo element of this substream, starting from startIndex */
    if (!diff)
      diff |= (FDKmemcmp(&tmpLoud, &(hLoudnessInfoSet->loudnessInfo[startIndex + i]),
                         sizeof(LOUDNESS_INFO)) != 0);
    hLoudnessInfoSet->loudnessInfo[startIndex + i] = tmpLoud;
  }

  loudnessInfoAlbumPresent = FDKreadBits(hBs, 1);
  if (loudnessInfoAlbumPresent) {
    diff |= _compAssign(&hLoudnessInfoSet->loudnessInfoAlbumCountV0[subStreamIndex],
                        FDKreadBits(hBs, 6));
  } else {
    diff |= _compAssign(&hLoudnessInfoSet->loudnessInfoAlbumCountV0[subStreamIndex], 0);
  }

  /* accumulate loudnessInfoAlbumCountV0 from lower substreams */
  startIndex = 0;
  for (i = 0; i < subStreamIndex; i++) {
    startIndex += hLoudnessInfoSet->loudnessInfoAlbumCountV0[i];
  }

  hLoudnessInfoSet->loudnessInfoAlbumCount =
      (UCHAR)fMin(startIndex + hLoudnessInfoSet->loudnessInfoAlbumCountV0[subStreamIndex], 32);
  for (i = 0; i < hLoudnessInfoSet->loudnessInfoAlbumCountV0[subStreamIndex]; i++) {
    LOUDNESS_INFO tmpLoud;
    FDKmemclear(&tmpLoud, sizeof(LOUDNESS_INFO));
    err = _readLoudnessInfo(hBs, 0, &tmpLoud);
    if (err) return err;
    if (startIndex + i >= 32) continue;

    /* insert the loudnessInfoAlbum element of this substream, starting from startIndex */
    if (!diff)
      diff |= (FDKmemcmp(&tmpLoud, &(hLoudnessInfoSet->loudnessInfoAlbum[startIndex + i]),
                         sizeof(LOUDNESS_INFO)) != 0);
    hLoudnessInfoSet->loudnessInfoAlbum[startIndex + i] = tmpLoud;
  }

  loudnessInfoSetExtPresent = FDKreadBits(hBs, 1);
  if (subStreamIndex == 0) {
    diff |= _compAssign(&hLoudnessInfoSet->loudnessInfoSetExtPresent, loudnessInfoSetExtPresent);
  } else {
    /* loudnessInfoSetExtension is not allowed for side-stream */
    if (loudnessInfoSetExtPresent) return DE_NOT_OK;
  }

  hLoudnessInfoSet->diff |= diff;

  if (loudnessInfoSetExtPresent) {
    err = _readLoudnessInfoSetExtension(hBs, hLoudnessInfoSet);
    if (err) return err;
  }

  return err;
}
