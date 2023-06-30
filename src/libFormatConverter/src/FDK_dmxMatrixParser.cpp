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

/******************** MPEG-H 3DA channel rendering library *********************

   Author(s):

   Description:

*******************************************************************************/

#define ARRAYLENGTH(a) (sizeof(a) / sizeof(a[0]))

#include "FDK_dmxMatrixParser.h"

typedef struct {
  UCHAR downmixId[DMX_MATRIX_SET_COUNT];
  UCHAR CICPspeakerLayoutIdx[DMX_MATRIX_SET_COUNT];
} FDK_DOWNMIX_BASIC_DATA;

/* exp = 5 */
static const FIXP_DBL eqMinRanges[2][4] = {
    {FL2FXCONST_DBL(-8.0 / 32.0), FL2FXCONST_DBL(-8.0 / 32.0), FL2FXCONST_DBL(-8.0 / 32.0),
     FL2FXCONST_DBL(-6.4 / 32.0)},
    {FL2FXCONST_DBL(-16.0 / 32.0), FL2FXCONST_DBL(-16.0 / 32.0), FL2FXCONST_DBL(-16.0 / 32.0),
     FL2FXCONST_DBL(-12.8 / 32.0)}};

/* 22_2 to 5_1 */
static char compactTemplate_CICP13_to_CICP6[15 * 4] = {
    1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0};

/* 5_2_1 to 5_1 */
static char compactTemplate_CICP14_to_CICP6[5 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                                                      1, 0, 0, 0, 0, 1, 1, 0, 0, 0};

/* 7_1 to 5_1 */
static char compactTemplate_CICP12_to_CICP6[5 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                                                      1, 0, 0, 0, 0, 1, 0, 0, 0, 1};

/* 7_1_ALT to 5_1 */
static char compactTemplate_CICP7_to_CICP6[5 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                                                     1, 0, 0, 0, 0, 1, 1, 0, 0, 1};

/* 22_2 to 5_2_1 */
static char compactTemplate_CICP13_to_CICP14[15 * 5] = {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};

/* 22_2 to 7_1 */
static char compactTemplate_CICP13_to_CICP12[15 * 5] = {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1,
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};

/* 22_2 to 7_1_ALT */
static char compactTemplate_CICP13_to_CICP7[15 * 5] = {
    0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1,
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};

/* 22_2 to 2_0 */
static char compactTemplate_CICP13_to_CICP2[15 * 1] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

static const SHORT compactTemplates_InputIndex[] = {13, 13, 13, 13, 14, 12, 7, 13};

static const SHORT compactTemplates_OutputIndex[] = {6, 14, 12, 7, 6, 6, 6, 2};

static char* compactTemplates_Data[] = {
    (char*)&compactTemplate_CICP13_to_CICP6,  (char*)&compactTemplate_CICP13_to_CICP14,
    (char*)&compactTemplate_CICP13_to_CICP12, (char*)&compactTemplate_CICP13_to_CICP7,
    (char*)&compactTemplate_CICP14_to_CICP6,  (char*)&compactTemplate_CICP12_to_CICP6,
    (char*)&compactTemplate_CICP7_to_CICP6,   (char*)&compactTemplate_CICP13_to_CICP2};

/*
  ReadRange()
*/

UINT ReadRange(HANDLE_FDK_BITSTREAM hBs, UINT alphabetSize) {
  UINT nBits = 32 - CntLeadingZeros(alphabetSize) - 1; /* floor(log2(alphabetSize)) */
  UINT nUnused = (1 << (nBits + 1)) - alphabetSize;
  UINT range = FDKreadBits(hBs, nBits);

  if (range >= nUnused) {
    UINT rangeExtra = FDKreadBit(hBs);
    range = (range << 1) - nUnused + rangeExtra;
  }

  return range;
}

/*
  EqualizerConfig() subroutine for equalizers
*/

INT EqualizerConfig(HANDLE_FDK_BITSTREAM hBs, SpeakerInformation* inputConfig, UINT inputCount,
                    eqConfigStruct* eqConfig) {
  SCHAR headroom;
  UINT i;
  UINT qFactorExtra, qFactorIndex, numSections, lastCenterFreqP10, lastCenterFreqLd2,
      maxCenterFreqLd2, centerFreq, centerFreqP10, centerFreqLd2, centerGainIndex, scalingGainIndex;
  FIXP_DBL centerGain, scalingGain, qualityFactor;
  INT centerGain_e = 0, scalingGain_e = 0, qualityFactor_e = 0;
  UINT numEqualizers;
  UINT eqPrecisionLevel;
  UINT sgPrecisionLevel;
  UINT eqExtendedRange;
  UINT equalizerPresent;

  equalizerPresent = FDKreadBit(hBs);
  if (equalizerPresent == 0) {
    eqConfig->numEQs = 0;
    FDKmemclear(eqConfig->eqMap, sizeof(eqConfig->eqMap));
    return 0;
  }

  numEqualizers = escapedValue(hBs, 3, 5, 0) + 1;
  eqConfig->numEQs = numEqualizers;
  eqPrecisionLevel = FDKreadBits(hBs, 2);
  sgPrecisionLevel = fMin(eqPrecisionLevel + 1, (UINT)3);
  eqExtendedRange = FDKreadBit(hBs);

  for (i = 0; i < numEqualizers; i++) {
    numSections = escapedValue(hBs, 2, 4, 0) + 1;
    eqConfig->eqParams[i].nPkFilter = numSections;
    lastCenterFreqP10 = 0;
    lastCenterFreqLd2 = 10;
    maxCenterFreqLd2 = 99;

    for (UINT j = 0; j < numSections; j++) {
      /* decode centerFreq */
      centerFreqP10 = lastCenterFreqP10 + ReadRange(hBs, (4 - lastCenterFreqP10));
      if (centerFreqP10 > lastCenterFreqP10) lastCenterFreqLd2 = 10;
      if (centerFreqP10 == 3) maxCenterFreqLd2 = 24;
      centerFreqLd2 =
          lastCenterFreqLd2 + ReadRange(hBs, (1 + maxCenterFreqLd2 - lastCenterFreqLd2));
      centerFreq = centerFreqLd2;
      for (UINT k = 0; k < centerFreqP10; ++k) {
        centerFreq *= 10;
      }
      lastCenterFreqP10 = centerFreqP10;
      lastCenterFreqLd2 = centerFreqLd2;
      headroom = (SCHAR)fNormz((FIXP_DBL)centerFreq) - 1;
      eqConfig->eqParams[i].pkFilterParams[j].f = (((FIXP_DBL)centerFreq) << (INT)headroom);
      eqConfig->eqParams[i].pkFilterParams[j].f_e = (32 - headroom - 1);
      /* decode qualityFactor */
      FIXP_DBL tmp;
      qFactorIndex = FDKreadBits(hBs, 5);
      if (qFactorIndex > 19) {
        qFactorExtra = FDKreadBits(hBs, 3);
        tmp = (FIXP_DBL)((qFactorIndex - 20) * 8 + qFactorExtra + 1);
        headroom = (SCHAR)fNormz(tmp) - 1;
        tmp <<= headroom;
        qualityFactor = fAddNorm(((FIXP_DBL)0x40000000), 1, fMult(FL2FXCONST_DBL(0.8), tmp),
                                 ((32 - headroom) - 3 - 1), &qualityFactor_e);
      } else {
        tmp = (FIXP_DBL)(qFactorIndex + 1);
        headroom = (SCHAR)fNormz(tmp) - 1;
        tmp <<= headroom;
        qualityFactor = fMult(FL2FXCONST_DBL(0.8), tmp); /*  */
        qualityFactor_e = ((32 - headroom - 1) - 4);
      }
      eqConfig->eqParams[i].pkFilterParams[j].q = qualityFactor;
      eqConfig->eqParams[i].pkFilterParams[j].q_e = qualityFactor_e;
      /* decode centerGain */
      UINT cgBits = 4 + eqExtendedRange + eqPrecisionLevel;
      centerGainIndex = FDKreadBits(hBs, cgBits);
      headroom = (SCHAR)fNormz((FIXP_DBL)centerGainIndex) - 1;
      centerGain = ((FIXP_DBL)centerGainIndex) << (INT)headroom;
      /* We have to apply a factor eqPrecisions[eqPrecisionLevel], that is defined as [1.0, 0.5,
       * 0.25, 0.1].    */
      /* We replace that by adding an offset (eqPrecisionLevel) to the exponent and replace 0.1 by
       * 0.8, if used */
      if ((eqPrecisionLevel) == 3) centerGain = fMult(FL2FXCONST_DBL(0.8), centerGain);
      centerGain = fAddNorm(eqMinRanges[eqExtendedRange][eqPrecisionLevel], 5, centerGain,
                            32 - headroom - 1 - (INT)eqPrecisionLevel, &centerGain_e);
      eqConfig->eqParams[i].pkFilterParams[j].g = centerGain;
      eqConfig->eqParams[i].pkFilterParams[j].g_e = centerGain_e;
    }
    /* decode scalingGain */
    UINT sgBits = 4 + eqExtendedRange + sgPrecisionLevel;
    scalingGainIndex = FDKreadBits(hBs, sgBits);
    headroom = (SCHAR)fNormz((FIXP_DBL)scalingGainIndex) - 1;
    scalingGain = ((FIXP_DBL)scalingGainIndex) << (INT)headroom;
    /* We have to apply a factor eqPrecisions[sgPrecisionLevel], that is defined as [1.0, 0.5, 0.25,
     * 0.1].    */
    /* We replace that by adding an offset (sgPrecisionLevel) to the exponent and replace 0.1 by
     * 0.8, if used */
    if ((sgPrecisionLevel) == 3) scalingGain = fMult(FL2FXCONST_DBL(0.8), scalingGain);
    scalingGain = fAddNorm(eqMinRanges[eqExtendedRange][sgPrecisionLevel], 5, scalingGain,
                           (32 - headroom - 1 - (INT)sgPrecisionLevel), &scalingGain_e);
    eqConfig->eqParams[i].G = scalingGain;
    eqConfig->eqParams[i].G_e = scalingGain_e;
  }

  for (i = 0; i < inputCount; i++) {
    UINT hasEqualizer = FDKreadBit(hBs);
    if (hasEqualizer) {
      eqConfig->eqMap[i] = ReadRange(hBs, numEqualizers) + 1;
    } else {
      eqConfig->eqMap[i] = 0;
    }
  }

  return 0;
}

/*
  DownmixMatrix() subroutine for downmix matrix.
*/

void ConvertToCompactConfig(SpeakerInformation* inputConfig, INT inputCount, INT* compactInputCount,
                            SpeakerInformation* compactInputConfig[]) {
  int i, j;

  for (i = 0; i < inputCount; ++i) {
    inputConfig[i].originalPosition = (SHORT)i;
    inputConfig[i].isAlreadyUsed = 0;
    inputConfig[i].symmetricPair = NULL;
  }

  for (i = 0; i < inputCount; ++i) {
    if (inputConfig[i].isAlreadyUsed) continue;

    if ((inputConfig[i].azimuth == 0) || (fAbs(inputConfig[i].azimuth) == 180)) {
      /* the speaker is in the center, it has no pair */
      compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
      inputConfig[i].symmetricPair = NULL;
      inputConfig[i].pairType = SP_PAIR_CENTER;
      inputConfig[i].isAlreadyUsed = 1;
    } else {
      for (j = i + 1; j < inputCount; ++j) {
        if (inputConfig[j].isAlreadyUsed) continue;

        if ((inputConfig[i].isLFE == inputConfig[j].isLFE) &&
            (inputConfig[i].elevation == inputConfig[j].elevation) &&
            (inputConfig[i].azimuth == -inputConfig[j].azimuth)) {
          /* we found a symmetric L/R pair for speaker on position i */
          if (inputConfig[i].azimuth > 0) {
            compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
            inputConfig[i].symmetricPair = &(inputConfig[j]);
            inputConfig[i].pairType = SP_PAIR_SYMMETRIC;
            inputConfig[j].symmetricPair = NULL;
            inputConfig[j].pairType = SP_PAIR_NONE;
          } else {
            compactInputConfig[(*compactInputCount)++] = &(inputConfig[j]);
            inputConfig[j].symmetricPair = &(inputConfig[i]);
            inputConfig[j].pairType = SP_PAIR_SYMMETRIC;
            inputConfig[i].symmetricPair = NULL;
            inputConfig[i].pairType = SP_PAIR_NONE;
          }

          inputConfig[i].isAlreadyUsed = 1;
          inputConfig[j].isAlreadyUsed = 1;
          break;
        }
      }

      if (!inputConfig[i].isAlreadyUsed) {
        /* we did not found a symmetric L/R pair for speaker on position i */
        compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
        inputConfig[i].symmetricPair = NULL;
        inputConfig[i].pairType = SP_PAIR_SINGLE;
        inputConfig[i].isAlreadyUsed = 1;
      }
    }
  }
}

void DecodeFlatCompactMatrix(HANDLE_FDK_BITSTREAM hBs, signed char* flatCompactMatrix,
                             int totalCount) {
  int maxHead;
  int nBits = 3;
  int runLGRParam = -1;
  int idx, run;

  if (totalCount >= 256) nBits = 4;
  runLGRParam = FDKreadBits(hBs, nBits);
  maxHead = totalCount >> runLGRParam;
  idx = 0;
  do {
    int head = 0;
    int t;

    while (head < maxHead) {
      if (0 != FDKreadBit(hBs)) break;
      ++head;
    }
    run = (head << runLGRParam) + FDKreadBits(hBs, runLGRParam);
    for (t = 0; t < run; ++t) {
      flatCompactMatrix[idx++] = 0;
    }
    if (idx < totalCount) flatCompactMatrix[idx++] = 1;
  } while (idx < totalCount);
}

INT DecodeGainValue(HANDLE_FDK_BITSTREAM hBs, CoderState* cs, FIXP_DBL* gainValue) {
  *gainValue = FDK_DMX_MATRIX_GAIN_ZERO;

  if (cs->rawCodingNonzeros) {
    INT nAlphabet = ((cs->maxGain - cs->minGain) << cs->precisionLevel) + 2;
    INT gainValueIndex = ReadRange(hBs, nAlphabet);
    *gainValue = FIXP_DBL(cs->maxGain - (gainValueIndex >> cs->precisionLevel));
  } else {
    INT gainValueIndex = -1;
    INT maxHead = (cs->gainTableSize - 1) >> cs->gainLGRParam;
    INT head = 0;

    while (head < maxHead) {
      if (0 != FDKreadBit(hBs)) break;
      ++head;
    }
    gainValueIndex = (head << cs->gainLGRParam) + FDKreadBits(hBs, cs->gainLGRParam);
    if (gainValueIndex >= cs->gainTableSize) {
      return -1;
    }
    *gainValue = cs->gainTable[gainValueIndex];
  }

  if (*gainValue < (FIXP_DBL)(cs->minGain << 23)) *gainValue = FDK_DMX_MATRIX_GAIN_ZERO;

  return 0;
}

void CoderStateGenerateGainTable(CoderState* cs) {
  INT i, p;
  INT index = 0;
  /* Word: Q24 (range -96 to 96 dB) */
  FIXP_DBL f;
  FIXP_DBL step;

  FDK_ASSERT(cs->precisionLevel <= 2); /* 2 fractional bits give 0.25 dB resolution */

  /* add multiples of 3 to the table, first values <= 0 and then values > 0 */
  for (i = 0; i >= cs->minGain; i -= 3) {
    cs->gainTable[index++] = (FIXP_DBL)(i << 23);
  }
  for (i = 3; i <= cs->maxGain; i += 3) {
    cs->gainTable[index++] = (FIXP_DBL)(i << 23);
  }

  /* add the rest of the integers which are not multiples of 3 to the table */
  for (i = 0; i >= cs->minGain; --i) {
    if ((i % 3) != 0) cs->gainTable[index++] = (FIXP_DBL)(i << 23);
  }

  for (i = 1; i <= cs->maxGain; ++i) {
    if ((i % 3) != 0) cs->gainTable[index++] = (FIXP_DBL)(i << 23);
  }

  /* add values which are multiples of 0.5, then multiples of 0.25,
     up to and finally multiples of 2 ^ (-precisionLevel) to the table,
     but do not add values which are already in the table */
  for (p = 1; p <= cs->precisionLevel; ++p) {
    step = (FIXP_DBL)(0x00400000 >> (p - 1));

    for (f = (FIXP_DBL)0; f >= ((FIXP_DBL)(cs->minGain << 23)); f -= step) {
      if ((f & step) != (FIXP_DBL)0x0) cs->gainTable[index++] = f;
    }
    for (f = step; f <= ((FIXP_DBL)(cs->maxGain << 23)); f += step) {
      if ((f & step) != (FIXP_DBL)0x0) cs->gainTable[index++] = f;
    }
  }

  cs->gainTable[index++] = FDK_DMX_MATRIX_GAIN_ZERO;
  cs->gainTableSize = index;
  FDK_ASSERT(cs->gainTableSize == ((cs->maxGain - cs->minGain) << cs->precisionLevel) + 2);
}

signed char* FindCompactTemplate(int inputIndex, int outputIndex) {
  INT i;

  /* a compact template should be used only for CICP speaker layouts */
  FDK_ASSERT((inputIndex != -1) && (outputIndex != -1));

  for (i = 0; i < (INT)ARRAYLENGTH(compactTemplates_Data); ++i) {
    if (compactTemplates_InputIndex[i] != inputIndex) continue;
    if (compactTemplates_OutputIndex[i] != outputIndex) continue;

    /* a matching template was found */
    return (signed char*)compactTemplates_Data[i];
  }

  /* a matching template was not found */
  return NULL;
}

void CoderStateInit(CoderState* cs) {
  cs->minGain = -1;
  cs->maxGain = 0;
  cs->precisionLevel = 0;
  cs->rawCodingNonzeros = 1;
  cs->gainLGRParam = -1;
  cs->historyCount = 0;
  cs->gainTableSize = 0;
}

INT DecodeDownmixMatrix(INT inputIndex, INT inputCount, SpeakerInformation* inputConfig,
                        INT outputIndex, INT outputCount, SpeakerInformation* outputConfig,
                        HANDLE_FDK_BITSTREAM hBs, FIXP_SGL* downmixMatrix, eqConfigStruct* eqConfig,
                        FIXP_DBL* p_buffer) {
  INT i = 0, j = 0;
  FIXP_DBL* downmixMatrixTmp = (FIXP_DBL*)p_buffer;
  p_buffer += (FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS * FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS);
  SpeakerInformation* compactInputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  INT compactInputCount = 0;
  SpeakerInformation* compactOutputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  INT compactOutputCount = 0;
  INT precisionLevel = 0;
  INT isSeparable[DMX_MATRIX_MAX_SPEAKER_COUNT];
  INT isSymmetric[DMX_MATRIX_MAX_SPEAKER_COUNT];
  INT isAllSeparable = 1;
  INT isAllSymmetric = 1;
  INT(*compactDownmixMatrix)
  [DMX_MATRIX_MAX_SPEAKER_COUNT] = (INT(*)[DMX_MATRIX_MAX_SPEAKER_COUNT])p_buffer;
  p_buffer =
      (FIXP_DBL*)((INT*)p_buffer + (DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT));
  signed char* compactTemplate = NULL; /* compactTemplate[compactInputCount][compactOutputCount]
                                          stored in the C array layout */

  INT useCompactTemplate = 1;
  INT fullForAsymmetricInputs = 1;
  INT mixLFEOnlyToLFE = 1;
  INT rawCodingCompactMatrix = 0;
  INT rawCodingNonzeros = 0;

  CoderState cs;
  INT minGain = 96;
  INT maxGain = -96;

  EqualizerConfig(hBs, inputConfig, inputCount, eqConfig);

  precisionLevel = FDKreadBits(hBs, 2);
  if (precisionLevel > 2) {
    return -1;
  }

  maxGain = escapedValue(hBs, 3, 4, 0);
  minGain = -(INT)(escapedValue(hBs, 4, 5, 0) + 1);

  ConvertToCompactConfig(inputConfig, inputCount, &compactInputCount, compactInputConfig);
  ConvertToCompactConfig(outputConfig, outputCount, &compactOutputCount, compactOutputConfig);

  isAllSeparable = FDKreadBit(hBs);

  if (!isAllSeparable) {
    for (i = 0; i < compactOutputCount; i++) {
      if (compactOutputConfig[i]->pairType == SP_PAIR_SYMMETRIC) {
        isSeparable[i] = FDKreadBit(hBs);
      }
    }
  } else {
    for (i = 0; i < compactOutputCount; i++) {
      if (compactOutputConfig[i]->pairType == SP_PAIR_SYMMETRIC) {
        isSeparable[i] = 1;
      }
    }
  }

  isAllSymmetric = FDKreadBit(hBs);
  if (!isAllSymmetric) {
    for (i = 0; i < compactOutputCount; i++) {
      isSymmetric[i] = FDKreadBit(hBs);
    }
  } else {
    for (i = 0; i < compactOutputCount; i++) {
      isSymmetric[i] = 1;
    }
  }

  /* decode the compact template */
  mixLFEOnlyToLFE = FDKreadBit(hBs);
  rawCodingCompactMatrix = FDKreadBit(hBs);

  if (rawCodingCompactMatrix) {
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (!mixLFEOnlyToLFE || (compactInputConfig[i]->isLFE == compactOutputConfig[j]->isLFE)) {
          compactDownmixMatrix[i][j] = FDKreadBit(hBs);
        } else {
          compactDownmixMatrix[i][j] = 0;
        }
      }
    }
  } else {
    signed char* flatCompactMatrix = (signed char*)p_buffer;
    INT count = 0;
    INT totalCount = 0;

    useCompactTemplate = FDKreadBit(hBs);

    /* compute totalCount as described in the ComputeTotalCount() function */
    if (mixLFEOnlyToLFE) {
      int compactInputLFECount = 0;
      int compactOutputLFECount = 0;

      for (i = 0; i < compactInputCount; ++i) {
        if (compactInputConfig[i]->isLFE) compactInputLFECount++;
      }
      for (i = 0; i < compactOutputCount; ++i) {
        if (compactOutputConfig[i]->isLFE) compactOutputLFECount++;
      }
      totalCount =
          (compactInputCount - compactInputLFECount) * (compactOutputCount - compactOutputLFECount);
    } else {
      totalCount = compactInputCount * compactOutputCount;
    }

    if (useCompactTemplate) {
      /* a compact template should be used only for CICP speaker layouts */
      if ((inputIndex == -1) || (outputIndex == -1)) {
        /* when useCompactTemplate is enabled and one of the CICP indexes are -1, it means
          that the decoder version is too old and it does not have the required CICP tables */
        return -1;
      }

      compactTemplate = FindCompactTemplate(inputIndex, outputIndex);
      if (compactTemplate == NULL) { /* a template was not found */
        /* when useCompactTemplate is enabled and a template was not found, it means
          that the decoder version is too old and it does not have the required tables */
        return -1;
      }
    }

    DecodeFlatCompactMatrix(hBs, flatCompactMatrix, totalCount);
    count = 0;
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (mixLFEOnlyToLFE && compactInputConfig[i]->isLFE && compactOutputConfig[j]->isLFE) {
          compactDownmixMatrix[i][j] = FDKreadBit(hBs);
        } else if (mixLFEOnlyToLFE &&
                   (compactInputConfig[i]->isLFE ^ compactOutputConfig[j]->isLFE)) {
          compactDownmixMatrix[i][j] = 0;
        } else { /* !mixLFEOnlyToLFE || (!compactInputConfig[i]->isLFE &&
                    !compactOutputConfig[j]->isLFE) */
          compactDownmixMatrix[i][j] = flatCompactMatrix[count++];
          if (useCompactTemplate) {
            compactDownmixMatrix[i][j] ^= compactTemplate[i * compactOutputCount + j];
          }
        }
      }
    }
    FDK_ASSERT(count == totalCount);
  }

  /* decode values for the nonzero entries in compactDownmixMatrix */
  fullForAsymmetricInputs = FDKreadBit(hBs);
  rawCodingNonzeros = FDKreadBit(hBs);

  CoderStateInit(&cs);
  cs.historyCount = 0;
  cs.rawCodingNonzeros = rawCodingNonzeros;
  cs.gainLGRParam = -1;
  cs.minGain = minGain;
  cs.maxGain = maxGain;
  cs.precisionLevel = precisionLevel;
  cs.gainTableSize = 0;

  if (!rawCodingNonzeros) {
    cs.gainLGRParam = FDKreadBits(hBs, 3);
    CoderStateGenerateGainTable(&cs);
  }

  for (i = 0; i < compactInputCount; ++i) {
    int iType = compactInputConfig[i]->pairType;
    FDK_ASSERT(iType != SP_PAIR_NONE);

    for (j = 0; j < compactOutputCount; ++j) {
      int oType = compactOutputConfig[j]->pairType;
      int i1 = compactInputConfig[i]->originalPosition;
      int o1 = compactOutputConfig[j]->originalPosition;
      FDK_ASSERT(oType != SP_PAIR_NONE);

      if ((iType != SP_PAIR_SYMMETRIC) && (oType != SP_PAIR_SYMMETRIC)) {
        downmixMatrixTmp[i1 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o1]) != 0) {
          return -1;
        }
      } else if (iType != SP_PAIR_SYMMETRIC) {
        int o2 = compactOutputConfig[j]->symmetricPair->originalPosition;
        int useFull = (iType == SP_PAIR_SINGLE) && fullForAsymmetricInputs;
        downmixMatrixTmp[i1 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        downmixMatrixTmp[i1 * outputCount + o2] = FDK_DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o1]) != 0) {
          return -1;
        }
        if (isSymmetric[j] && !useFull) {
          downmixMatrixTmp[i1 * outputCount + o2] = downmixMatrixTmp[i1 * outputCount + o1];
        } else {
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o2]) != 0) {
            return -1;
          }
        }
      } else if (oType != SP_PAIR_SYMMETRIC) {
        int i2 = compactInputConfig[i]->symmetricPair->originalPosition;
        downmixMatrixTmp[i1 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        downmixMatrixTmp[i2 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o1]) != 0) {
          return -1;
        }
        if (isSymmetric[j]) {
          downmixMatrixTmp[i2 * outputCount + o1] = downmixMatrixTmp[i1 * outputCount + o1];
        } else {
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i2 * outputCount + o1]) != 0) {
            return -1;
          }
        }
      } else {
        int i2 = compactInputConfig[i]->symmetricPair->originalPosition;
        int o2 = compactOutputConfig[j]->symmetricPair->originalPosition;
        downmixMatrixTmp[i1 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        downmixMatrixTmp[i1 * outputCount + o2] = FDK_DMX_MATRIX_GAIN_ZERO;
        downmixMatrixTmp[i2 * outputCount + o1] = FDK_DMX_MATRIX_GAIN_ZERO;
        downmixMatrixTmp[i2 * outputCount + o2] = FDK_DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o1]) != 0) {
          return -1;
        }
        if (isSeparable[j] && isSymmetric[j]) {
          downmixMatrixTmp[i2 * outputCount + o2] = downmixMatrixTmp[i1 * outputCount + o1];
        } else if (!isSeparable[j] && isSymmetric[j]) {
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o2]) != 0) {
            return -1;
          }
          downmixMatrixTmp[i2 * outputCount + o1] = downmixMatrixTmp[i1 * outputCount + o2];
          downmixMatrixTmp[i2 * outputCount + o2] = downmixMatrixTmp[i1 * outputCount + o1];
        } else if (isSeparable[j] && !isSymmetric[j]) {
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i2 * outputCount + o2]) != 0) {
            return -1;
          };
        } else { /* !isSeparable[j] && !isSymmetric[j] */
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i1 * outputCount + o2]) != 0) {
            return -1;
          }
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i2 * outputCount + o1]) != 0) {
            return -1;
          }
          if (DecodeGainValue(hBs, &cs, &downmixMatrixTmp[i2 * outputCount + o2]) != 0) {
            return -1;
          }
        }
      }
    }
  }
  /* convert the downmix matrix to linear domain */
  for (i = 0; i < inputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      FIXP_DBL value = downmixMatrixTmp[i * outputCount + j];
      if ((value >> 23) != (FIXP_DBL)DMX_MATRIX_GAIN_ZERO) {
        INT exp_e = 0;
        FIXP_DBL exp_m = fDivNormSigned(value, FL2FX_DBL(0.625), &exp_e);
        exp_e += 3; /* Value (Q23) / FL2FX_DBL(0.625) (Q26) -> exp_e = (8-5) */
        INT pow_e = 0;
        FIXP_DBL pow_m = 0;
        if (exp_m != (FIXP_DBL)0) {
          pow_m = fPow(FL2FX_DBL(0.625), 4, exp_m, exp_e, &pow_e);
        } else {
          pow_m = (FIXP_DBL)MAXVAL_DBL;
          pow_e = 0;
        }
        downmixMatrix[i * outputCount + j] = FX_DBL2FX_SGL(scaleValue(pow_m, pow_e));
      } else {
        downmixMatrix[i * outputCount + j] = (FIXP_SGL)0;
      }
    }
  }
  return 0;
}

static INT SetUniDrcDownmixInfo(HANDLE_DRC_DECODER hUniDrcDec, const int downmixIdCount,
                                FDK_DOWNMIX_BASIC_DATA* DMX_BD, const int downmixId) {
  int err = 0;
  int i;
  int downmixIdList[DMX_MATRIX_SET_COUNT];
  int targetLayout[DMX_MATRIX_SET_COUNT];
  int targetChannelCount[DMX_MATRIX_SET_COUNT];

  if (downmixIdCount > DMX_MATRIX_SET_COUNT) {
    return -1;
  }

  for (i = 0; i < downmixIdCount; i++) {
    downmixIdList[i] = DMX_BD->downmixId[i];
    targetLayout[i] = DMX_BD->CICPspeakerLayoutIdx[i];
    /* get targetChannelCount */
    err = cicp2geometry_get_numChannels_from_cicp(DMX_BD->CICPspeakerLayoutIdx[i],
                                                  &(targetChannelCount[i]));
    if (err) {
      return -1;
    }
  }
  err = FDK_drcDec_SetDownmixInstructions(hUniDrcDec, downmixIdCount, downmixIdList, targetLayout,
                                          targetChannelCount);
  if (err) {
    return -1;
  }

  err = FDK_drcDec_SetParam(hUniDrcDec, DRC_DEC_DOWNMIX_ID, (FIXP_DBL)downmixId);
  if (err) {
    return -1;
  }

  return err;
}

/*
  DownmixMatrixSet() subroutine for parsing the downmix matrix.
*/

INT DownmixMatrixSet(HANDLE_FDK_BITSTREAM hBs,
                     FDK_DOWNMIX_GROUPS_MATRIX_SET* groupsDownmixMatrixSet, const INT targetLayout,
                     const INT downmixConfigType, INT* downmixId, HANDLE_DRC_DECODER hUniDrcDec) {
  INT ErrorStatus = 0;
  INT cicpIndex = -1;

  *downmixId = -1;

  FDK_DOWNMIX_BASIC_DATA DMX_BD;

  INT downmixIdCount = FDKreadBits(hBs, 5);

  cicpIndex = targetLayout;

  for (UCHAR k = 0; k < downmixIdCount; k++) {
    INT dmxMatrixLenBytes = 0;
    INT dmxMatrixBitsLeft = 0;

    UCHAR downmixId_local = FDKreadBits(hBs, 7);
    UCHAR downmixType = FDKreadBits(hBs, 2);

    /* If downmixType > 1 exit */
    if (downmixType > 1) {
      ErrorStatus = -1;
      break;
    }
    DMX_BD.downmixId[k] = downmixId_local;

    UCHAR CICPspeakerLayoutIdx = FDKreadBits(hBs, 6);
    DMX_BD.CICPspeakerLayoutIdx[k] = CICPspeakerLayoutIdx;

    if (CICPspeakerLayoutIdx == cicpIndex) {
      if (cicpIndex > CICP2GEOMETRY_CICP_VALID) {
        *downmixId = downmixId_local;
      }
    }

    /*Do nothing for downmixType=0 */
    if (downmixType == 1) {
      if (CICPspeakerLayoutIdx == cicpIndex) {
        /* Parse just the needed data*/

        UCHAR bsDownmixMatrixCount = escapedValue(hBs, 1, 3, 0) + 1;
        for (UCHAR l = 0; l < bsDownmixMatrixCount; l++) {
          USHORT bsNumAssignedGroupIDs = escapedValue(hBs, 1, 4, 4) + 1;
          for (USHORT m = 0; m < bsNumAssignedGroupIDs; m++) {
            UCHAR signal_groupID = FDKreadBits(hBs, 5);
            if (signal_groupID > (FDK_MPEGHAUDIO_DEC_MAX_SIGNAL_GROUPS - 1)) {
              ErrorStatus = -1;
              break;
            }
            groupsDownmixMatrixSet->downmixMatrix[signal_groupID] = (UCHAR)l;
          }

          groupsDownmixMatrixSet->downmixMatrixSize[l] = escapedValue(hBs, 8, 8, 12);

          dmxMatrixLenBytes = (groupsDownmixMatrixSet->downmixMatrixSize[l] >> 3);
          dmxMatrixBitsLeft = (groupsDownmixMatrixSet->downmixMatrixSize[l] % 8);

          if (((groupsDownmixMatrixSet->downmixMatrixSize[l] + 7) >> 3) >
              FDK_DOWNMIX_CONFIG_MAX_DMX_MATRIX_SIZE) {
            ErrorStatus = -1;
            break;
          }

          INT byteCnt = 0;
          while (dmxMatrixLenBytes--) {
            groupsDownmixMatrixSet->downmixMatrixMemory[l][byteCnt++] = FDKreadBits(hBs, 8);
          }
          if (dmxMatrixBitsLeft) {
            groupsDownmixMatrixSet->downmixMatrixMemory[l][byteCnt] =
                FDKreadBits(hBs, dmxMatrixBitsLeft) << (8 - dmxMatrixBitsLeft);
          }
        } /*for (UCHAR l = 0; l < bsDownmixMatrixCount; l++)*/

        if (ErrorStatus != 0) {
          break;
        }

      } else {
        /* Dummy parsing*/

        UCHAR bsDownmixMatrixCount = escapedValue(hBs, 1, 3, 0) + 1;
        for (UCHAR l = 0; l < bsDownmixMatrixCount; l++) {
          USHORT bsNumAssignedGroupIDs = escapedValue(hBs, 1, 4, 4) + 1;
          for (USHORT m = 0; m < bsNumAssignedGroupIDs; m++) {
            UCHAR signal_groupID = FDKreadBits(hBs, 5);
            if (signal_groupID > (FDK_MPEGHAUDIO_DEC_MAX_SIGNAL_GROUPS - 1)) {
              ErrorStatus = -1;
              break;
            }
          }

          INT downmixMatrixSize = escapedValue(hBs, 8, 8, 12);

          dmxMatrixLenBytes = (downmixMatrixSize >> 3);
          dmxMatrixBitsLeft = (downmixMatrixSize % 8);

          if (((groupsDownmixMatrixSet->downmixMatrixSize[l] + 7) >> 3) >
              FDK_DOWNMIX_CONFIG_MAX_DMX_MATRIX_SIZE) {
            ErrorStatus = -1;
            break;
          }

          while (dmxMatrixLenBytes--) {
            FDKreadBits(hBs, 8);
          }
          if (dmxMatrixBitsLeft) {
            FDKreadBits(hBs, dmxMatrixBitsLeft);
          }
        } /*for (UCHAR l = 0; l < bsDownmixMatrixCount; l++)*/

        if (ErrorStatus != 0) {
          break;
        }

      } /* if(CICPspeakerLayoutIdx == cicpIndex)*/

    } /* if ( downmixType == 1 )*/

  } /*for ( UCHAR k = 0; k < downmixIdCount; k++)*/

  /* Transfer of downmix information to DRC decoder */
  if (ErrorStatus == 0) {
    if (hUniDrcDec) {
      ErrorStatus = SetUniDrcDownmixInfo(hUniDrcDec, downmixIdCount, &DMX_BD, *downmixId);
    }
  }

  return ErrorStatus;
}
