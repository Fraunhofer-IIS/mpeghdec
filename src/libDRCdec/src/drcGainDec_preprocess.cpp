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
#include "drcGainDec_preprocess.h"
#include "drcDec_tools.h"
#include "drcDec_rom.h"

#define SLOPE_FACTOR_DB_TO_LINEAR FL2FXCONST_DBL(0.1151f * (float)(1 << 3)) /* ln(10) / 20 */

typedef struct {
  int drcSetEffect;
  DUCKING_MODIFICATION* pDMod;
  GAIN_MODIFICATION* pGMod;
  int limiterPeakTargetPresent;
  FIXP_SGL limiterPeakTarget;
  FIXP_DBL loudnessNormalizationGainDb;
  FIXP_SGL compress;
  FIXP_SGL boost;
} NODE_MODIFICATION;

static DRC_ERROR _toLinear(const NODE_MODIFICATION* nodeMod, const int drcBand,
                           const FIXP_SGL gainDb,  /* in: gain value in dB, e = 7 */
                           const FIXP_SGL slopeDb, /* in: slope value in dB/deltaTmin, e = 2 */
                           FIXP_DBL* gainLin,      /* out: linear gain value, e = 7 */
                           FIXP_DBL* slopeLin)     /* out: linear slope value, e = 7 */
{
  FIXP_DBL gainRatio_m = FL2FXCONST_DBL(1.0f / (float)(1 << 1));
  GAIN_MODIFICATION* pGMod = NULL;
  DUCKING_MODIFICATION* pDMod = nodeMod->pDMod;
  FIXP_DBL tmp_dbl, gainDb_modified, gainDb_offset, gainDb_out, gainLin_m, slopeLin_m;
  int gainLin_e, gainRatio_e = 1, gainDb_out_e;
  if (nodeMod->pGMod != NULL) {
    pGMod = &(nodeMod->pGMod[drcBand]);
  }
  if (((nodeMod->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) == 0) &&
      (nodeMod->drcSetEffect != EB_FADE) && (nodeMod->drcSetEffect != EB_CLIPPING)) {
    if (gainDb < (FIXP_SGL)0) {
      gainRatio_m = fMultDiv2(gainRatio_m, nodeMod->compress);
    } else {
      gainRatio_m = fMultDiv2(gainRatio_m, nodeMod->boost);
    }
    gainRatio_e += 2;
  }
  if ((pGMod != NULL) && (pGMod->gainScalingPresent == 1)) {
    if (gainDb < (FIXP_SGL)0) {
      gainRatio_m = fMultDiv2(gainRatio_m, pGMod->attenuationScaling);
    } else {
      gainRatio_m = fMultDiv2(gainRatio_m, pGMod->amplificationScaling);
    }
    gainRatio_e += 3;
  }
  if ((pDMod != NULL) && (nodeMod->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) &&
      (pDMod->duckingScalingPresent == 1)) {
    gainRatio_m = fMultDiv2(gainRatio_m, pDMod->duckingScaling);
    gainRatio_e += 3;
  }

  gainDb_modified = fMultDiv2(gainDb, gainRatio_m); /* resulting e: 7 + gainRatio_e + 1*/
  gainDb_offset = (FIXP_DBL)0;

  if ((pGMod != NULL) && (pGMod->gainOffsetPresent == 1)) {
    /* *gainLin *= (float)pow(2.0, (double)(pGMod->gainOffset/6.0f)); */
    gainDb_offset += FX_SGL2FX_DBL(pGMod->gainOffset) >> 4; /* resulting e: 8 */
  }
  if ((nodeMod->limiterPeakTargetPresent == 1) &&
      (nodeMod->drcSetEffect == EB_CLIPPING)) { /* The only drcSetEffect is "clipping prevention" */
    /* loudnessNormalizationGainModificationDb is included in loudnessNormalizationGainDb */
    /* *gainLin *= (float)pow(2.0, max(0.0, -nodeModification->limiterPeakTarget -
     * nodeModification->loudnessNormalizationGainDb)/6.0); */
    gainDb_offset +=
        fMax((FIXP_DBL)0, (FX_SGL2FX_DBL(-nodeMod->limiterPeakTarget) >> 3) -
                              (nodeMod->loudnessNormalizationGainDb >> 1)); /* resulting e: 8 */
  }
  if (gainDb_offset != (FIXP_DBL)0) {
    gainDb_out = fAddNorm(gainDb_modified, 7 + gainRatio_e + 1, gainDb_offset, 8, &gainDb_out_e);
  } else {
    gainDb_out = gainDb_modified;
    gainDb_out_e = 7 + gainRatio_e + 1;
  }

  /* *gainLin = (float)pow(2.0, (double)(gainDb_modified[1] / 6.0f)); */
  gainLin_m = approxDb2lin(gainDb_out, gainDb_out_e, &gainLin_e);
  *gainLin = scaleValueSaturate(gainLin_m, gainLin_e - 7);

  /* *slopeLin = SLOPE_FACTOR_DB_TO_LINEAR * gainRatio * *gainLin * slopeDb; */
  if (slopeDb == (FIXP_SGL)0) {
    *slopeLin = (FIXP_DBL)0;
  } else {
    tmp_dbl = fMult(slopeDb, SLOPE_FACTOR_DB_TO_LINEAR); /* resulting e: 2 - 3 = -1 */
    tmp_dbl = fMult(tmp_dbl, gainRatio_m);               /* resulting e: -1 + gainRatio_e */
    if (gainDb_offset != (FIXP_DBL)0) { /* recalculate gainLin from gainDb that wasn't modified by
                                           gainOffset and limiterPeakTarget */
      gainLin_m = approxDb2lin(gainDb_modified, 7 + gainRatio_e, &gainLin_e);
    }
    slopeLin_m = fMult(tmp_dbl, gainLin_m);
    *slopeLin = scaleValueSaturate(slopeLin_m, -1 + gainRatio_e + gainLin_e - 7);
  }

  if ((nodeMod->limiterPeakTargetPresent == 1) && (nodeMod->drcSetEffect == EB_CLIPPING)) {
    if (*gainLin >= FL2FXCONST_DBL(1.0f / (float)(1 << 7))) {
      *gainLin = FL2FXCONST_DBL(1.0f / (float)(1 << 7));
      *slopeLin = (FIXP_DBL)0;
    }
  }

  return DE_OK;
}

/* get overall number of sequences contained in uniDrcGain */
int _getNumSequences(HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  int numSequences = hUniDrcGain->nDecodedSequences[0];
  int s;
  for (s = 1; s < 4; s++) {
    numSequences += hUniDrcGain->nDecodedSequences[s];
  }
  return numSequences;
}

/* prepare buffers containing linear nodes for each gain sequence */
DRC_ERROR
prepareDrcGain(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_GAIN hUniDrcGain,
               const FIXP_SGL compress, const FIXP_SGL boost,
               const FIXP_DBL loudnessNormalizationGainDb, const int activeDrcIndex,
               const int activeDrcLocation) {
  int b, g, gainElementIndex;
  DRC_GAIN_BUFFERS* drcGainBuffers = &(hGainDec->drcGainBuffers);
  NODE_MODIFICATION nodeMod;
  FDKmemclear(&nodeMod, sizeof(NODE_MODIFICATION));
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcLocation][activeDrcIndex]);
  DRC_INSTRUCTIONS_UNI_DRC* pInst = &pActiveDrc->drcInst;
  if (pInst == NULL) return DE_NOT_OK;

  nodeMod.drcSetEffect = pInst->drcSetEffect;

  nodeMod.compress = compress;
  nodeMod.boost = boost;
  nodeMod.loudnessNormalizationGainDb = loudnessNormalizationGainDb;
  nodeMod.limiterPeakTargetPresent = pInst->limiterPeakTargetPresent;
  nodeMod.limiterPeakTarget = pInst->limiterPeakTarget;

  gainElementIndex = 0;
  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    int gainSetIndex = 0;
    int nDrcBands = 0;
    DRC_COEFFICIENTS_UNI_DRC* pCoef = &hGainDec->drcCoef;
    if (pCoef == NULL) return DE_NOT_OK;

    if (!pActiveDrc->channelGroupIsParametricDrc[g]) {
      gainSetIndex = pInst->gainSetIndexForChannelGroup[g];

      if (nodeMod.drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
        nodeMod.pDMod = &(pActiveDrc->duckingModificationForChannelGroup[g]);
        nodeMod.pGMod = NULL;
      } else {
        nodeMod.pGMod = pInst->gainModificationForChannelGroup[g];
        nodeMod.pDMod = NULL;
      }

      nDrcBands = pActiveDrc->bandCountForChannelGroup[g];
      for (b = 0; b < nDrcBands; b++) {
        DRC_ERROR err = DE_OK;
        GAIN_SET* pGainSet = &(pCoef->gainSet[gainSetIndex]);
        int seq = pGainSet->gainSequenceIndex[b];

        /* linearNodeBuffer contains a copy of the gain sequences (consisting of nodes) that are
           relevant for decoding. It also contains gain sequences of previous frames. */
        LINEAR_NODE_BUFFER* pLnb =
            &(drcGainBuffers->linearNodeBuffer[pActiveDrc->activeDrcOffset + gainElementIndex]);
        int i, lnbp;
        lnbp = drcGainBuffers->lnbPointer;
        pLnb->gainInterpolationType = (GAIN_INTERPOLATION_TYPE)pGainSet->gainInterpolationType;

        /* sanity check on sequence index */
        if (seq >= _getNumSequences(hUniDrcGain)) return DE_NOT_OK;

        /* copy a node buffer and convert from dB to linear */
        pLnb->nNodes[lnbp] = fMin((int)hUniDrcGain->nNodes[seq], 32);
        for (i = 0; i < pLnb->nNodes[lnbp]; i++) {
          FIXP_DBL gainLin, slopeLin;
          err = _toLinear(&nodeMod, b, hUniDrcGain->gainNode[seq][i].gainDb, (FIXP_SGL)0, &gainLin,
                          &slopeLin);
          if (err) return err;
          pLnb->linearNode[lnbp][i].gainLin = gainLin;
          pLnb->linearNode[lnbp][i].time = hUniDrcGain->gainNode[seq][i].time;
        }
        gainElementIndex++;
      }
    } else {
      /* parametric DRC not supported */
      gainElementIndex++;
    }
  }
  return DE_OK;
}
