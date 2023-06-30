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

#include "drcDec_types.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_multiband.h"
#include "drcDec_rom.h"

#define LOG10_2_INV 3.32192809f  /* 1.0 / log10(2.0) */
#define LR_FILTER_SLOPE (-24.0f) /* slope of Linkwitz/Riley filters in dB per octave */
#define NORM_CONST FL2FXCONST_DBL(LR_FILTER_SLOPE* LOG10_2_INV / 20.0f / (float)(1 << 2))

static DRC_ERROR _initFcenterNormSb(const int nSubbands,
                                    const SUBBAND_DOMAIN_MODE subbandDomainMode,
                                    FIXP_DBL* fCenterNormSb) {
  int s;
  LONG invNSubbands = (LONG)invFixp((FIXP_DBL)nSubbands);
  switch (subbandDomainMode) {
    case SDM_STFT256:
      for (s = 0; s < nSubbands; s++) {
        /* fCenterNormSb[s] = s / (2.0f * nSubbands); */
        fCenterNormSb[s] = (FIXP_DBL)(s * (invNSubbands >> 1));
      }
      break;
    case SDM_QMF71:
    default:
      return DE_NOT_OK; /* not supported */
  }
  return DE_OK;
}

static void _generateSlope(const int audioDecoderSubbandCount, const FIXP_DBL* fCenterNormSb,
                           const FIXP_DBL fCrossNormLo, const FIXP_DBL fCrossNormHi,
                           FIXP_SGL* response) {
  int s;

  for (s = 0; s < audioDecoderSubbandCount; s++) {
    FIXP_DBL ratio, power;
    int r_e;
    if (fCenterNormSb[s] < fCrossNormLo) {
      /* response[s] = pow (10.0, NORM_CONST * log10(fCrossNormLo / fCenterNormSb[s]))
                     = pow (fCrossNormLo / fCenterNormSb[s], NORM_CONST) */
      if (fCenterNormSb[s] > (FIXP_DBL)0) {
        ratio = fDivNorm(fCrossNormLo, fCenterNormSb[s], &r_e);
        power = fPow(ratio, r_e, NORM_CONST, 2, &r_e);
        response[s] = FX_DBL2FX_SGL(scaleValue(power, r_e));
      } else {
        response[s] = (FIXP_SGL)0;
      }
    } else if (fCenterNormSb[s] < fCrossNormHi) {
      response[s] = (FIXP_SGL)MAXVAL_SGL; /* nearly 1.0f */
    } else {
      /* response[s] = pow (10.0, norm * log10(fCenterNormSb[s] / fCrossNormHi))
                     = pow (fCenterNormSb[s] / fCrossNormHi, NORM_CONST) */
      ratio = fDivNorm(fCenterNormSb[s], fCrossNormHi, &r_e);
      power = fPow(ratio, r_e, NORM_CONST, 2, &r_e);
      response[s] = FX_DBL2FX_SGL(scaleValue(power, r_e));
    }
  }
}

static DRC_ERROR _generateOverlapWeights(const int nDrcBands, const int drcBandType,
                                         const BAND_BORDER* pBandBorder,
                                         const SUBBAND_DOMAIN_MODE subbandDomainMode,
                                         OVERLAP_PARAMETERS overlapForGroup[4]) {
  FIXP_DBL fCenterNormSb[MAX_NUM_SUBBANDS];
  FIXP_DBL wNorm, tmp_overlapWeight;
  FIXP_DBL fCrossNormLo, fCrossNormHi;
  int b, s, startSubBandIndex, stopSubBandIndex, e_tmp;
  int audioDecoderSubbandCount = 0;
  switch (subbandDomainMode) {
    case SDM_QMF64:
      audioDecoderSubbandCount = SUBBAND_NUM_BANDS_QMF64;
      break;
    case SDM_QMF71:
      audioDecoderSubbandCount = SUBBAND_NUM_BANDS_QMF71;
      break;
    case SDM_STFT256:
      audioDecoderSubbandCount = SUBBAND_NUM_BANDS_STFT256;
      break;
    case SDM_OFF:
    default:
      return DE_NOT_OK;
  }

  if (drcBandType == 1) {
    if (_initFcenterNormSb(audioDecoderSubbandCount, subbandDomainMode, fCenterNormSb)) {
      return DE_NOT_OK;
    }

    fCrossNormLo = (FIXP_DBL)0;
    for (b = 0; b < nDrcBands; b++) {
      if (b < nDrcBands - 1) {
        fCrossNormHi = filterBankParameters[pBandBorder[b + 1].crossoverFreqIndex].fCrossNorm;
      } else {
        fCrossNormHi = FL2FXCONST_DBL(0.5f);
      }
      _generateSlope(audioDecoderSubbandCount, fCenterNormSb, fCrossNormLo, fCrossNormHi,
                     overlapForGroup[b].overlapWeight);

      fCrossNormLo = fCrossNormHi;
    }
    for (s = 0; s < audioDecoderSubbandCount; s++) {
      wNorm = (FIXP_DBL)0;
      for (b = 0; b < nDrcBands; b++) {
        /* e_wNorm = ld(4) = 3 */
        wNorm += FX_SGL2FX_DBL(overlapForGroup[b].overlapWeight[s]) >> 3;
      }
      for (b = 0; b < nDrcBands; b++) {
        /* overlapForGroup[b].overlapWeight[s] /= wNorm; */
        tmp_overlapWeight =
            fDivNorm(FX_SGL2FX_DBL(overlapForGroup[b].overlapWeight[s]), wNorm, &e_tmp);
        overlapForGroup[b].overlapWeight[s] =
            FX_DBL2FX_SGL(scaleValueSaturate(tmp_overlapWeight, e_tmp - 3));
      }
    }
    for (b = 0; b < nDrcBands; b++) {
      /* get start and stop indices of nonzero entries of overlapWeight */
      for (s = 0; s < audioDecoderSubbandCount; s++) {
        if (overlapForGroup[b].overlapWeight[s] > (FIXP_DBL)0) {
          overlapForGroup[b].start_index = s;
          break;
        }
      }
      for (; s < audioDecoderSubbandCount; s++) {
        if (overlapForGroup[b].overlapWeight[s] > (FIXP_DBL)0) {
          overlapForGroup[b].stop_index = s;
        } else {
          break;
        }
      }
      /* stop_index is excluding, i.e. it is the first index outside of the nonzero entry section */
      overlapForGroup[b].stop_index++;
    }
  } else {
    startSubBandIndex = 0;
    for (b = 0; b < nDrcBands; b++) {
      if (b < nDrcBands -
                  1) /* stopSubBandIndex is including, i.e. it is the last index within the band */
      {
        stopSubBandIndex = pBandBorder[b + 1].startSubBandIndex - 1;
        if (stopSubBandIndex < startSubBandIndex) return DE_NOT_OK;
        if (stopSubBandIndex > (audioDecoderSubbandCount - 1)) return DE_NOT_OK;
      } else {
        stopSubBandIndex = audioDecoderSubbandCount - 1;
      }
      overlapForGroup[b].start_index = startSubBandIndex;
      overlapForGroup[b].stop_index = stopSubBandIndex + 1;
      for (s = 0; s < audioDecoderSubbandCount; s++) {
        if (s >= startSubBandIndex && s <= stopSubBandIndex) {
          overlapForGroup[b].overlapWeight[s] = (FIXP_SGL)MAXVAL_SGL; /* nearly 1.0f */
        } else {
          overlapForGroup[b].overlapWeight[s] = (FIXP_SGL)0;
        }
      }
      startSubBandIndex = stopSubBandIndex + 1;
    }
  }

  return DE_OK;
}

DRC_ERROR
initOverlapWeight(const DRC_COEFFICIENTS_UNI_DRC* pCoef, const DRC_INSTRUCTIONS_UNI_DRC* pInst,
                  UCHAR* bandCountForChannelGroup, const SUBBAND_DOMAIN_MODE subbandDomainMode,
                  OVERLAP_PARAMETERS pOverlap[28][4]) {
  DRC_ERROR err = DE_OK;
  int g;

  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    if (bandCountForChannelGroup[g] > 1) {
      err =
          _generateOverlapWeights(bandCountForChannelGroup[g],
                                  pCoef->gainSet[pInst->gainSetIndexForChannelGroup[g]].drcBandType,
                                  pCoef->gainSet[pInst->gainSetIndexForChannelGroup[g]].bandBorder,
                                  subbandDomainMode, pOverlap[g]);
      if (err) return err;
    }
  }

  return DE_OK;
}
