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
#include "drcDec_tools.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_multiband.h"
#include "drcGainDec_init.h"

static DRC_ERROR _generateDrcInstructionsDerivedData(HANDLE_DRC_GAIN_DECODER hGainDec,
                                                     HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                     DRC_INSTRUCTIONS_UNI_DRC* pInst,
                                                     DRC_COEFFICIENTS_UNI_DRC* pCoef,
                                                     ACTIVE_DRC* pActiveDrc) {
  DRC_ERROR err = DE_OK;
  int g;
  int gainElementCount = 0;
  UCHAR nDrcChannelGroups = 0;
  SCHAR gainSetIndexForChannelGroup[28];

  err = deriveDrcChannelGroups(
      pInst->drcSetEffect, pInst->drcChannelCount, pInst->gainSetIndex,
      pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF) ? pInst->duckingModificationForChannel
                                                           : NULL,
      &nDrcChannelGroups, gainSetIndexForChannelGroup, pActiveDrc->channelGroupForChannel,
      pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)
          ? pActiveDrc->duckingModificationForChannelGroup
          : NULL);
  if (err) return (err);

  /* sanity check */
  if (nDrcChannelGroups != pInst->nDrcChannelGroups) return DE_NOT_OK;
  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    if (gainSetIndexForChannelGroup[g] != pInst->gainSetIndexForChannelGroup[g]) return DE_NOT_OK;
  }

  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    int seq = pInst->gainSetIndexForChannelGroup[g];
    if (seq != -1 &&
        (hUniDrcConfig->drcCoefficientsUniDrcCount == 0 || seq >= pCoef->gainSetCount)) {
      pActiveDrc->channelGroupIsParametricDrc[g] = 1;
    } else {
      pActiveDrc->channelGroupIsParametricDrc[g] = 0;
      if (seq >= pCoef->gainSetCount) {
        return DE_NOT_OK;
      }
    }
  }

  /* gainElementCount */
  if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      pActiveDrc->bandCountForChannelGroup[g] = 1;
    }
    pActiveDrc->gainElementCount =
        pInst->nDrcChannelGroups; /* one gain element per channel group */
  } else {
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      if (pActiveDrc->channelGroupIsParametricDrc[g]) {
        gainElementCount++;
        pActiveDrc->bandCountForChannelGroup[g] = 1;
      } else {
        int seq, bandCount;
        seq = pInst->gainSetIndexForChannelGroup[g];
        bandCount = pCoef->gainSet[seq].bandCount;
        pActiveDrc->bandCountForChannelGroup[g] = bandCount;
        gainElementCount += bandCount;
      }
    }
    pActiveDrc->gainElementCount = gainElementCount;
  }

  /* prepare gainElementForGroup (cumulated sum of bandCountForChannelGroup) */
  pActiveDrc->gainElementForGroup[0] = 0;
  for (g = 1; g < pInst->nDrcChannelGroups; g++) {
    pActiveDrc->gainElementForGroup[g] =
        pActiveDrc->gainElementForGroup[g - 1] +
        pActiveDrc
            ->bandCountForChannelGroup[g - 1]; /* index of first gain sequence in channel group */
  }

  return DE_OK;
}

DRC_ERROR
initGainDec(HANDLE_DRC_GAIN_DECODER hGainDec) {
  int h, i, j, k;

  /* sanity check */
  if (hGainDec->deltaTminDefault > hGainDec->frameSize) return DE_NOT_OK;

  for (h = 0; h < ACTIVE_DRC_LOCATIONS; h++) {
    for (i = 0; i < MAX_ACTIVE_DRCS; i++) {
      for (j = 0; j < 28; j++) {
        /* use startup node at the beginning */
        hGainDec->activeDrc[h][i].lnbIndexForChannel[j][0] = 0;
        for (k = 1; k < NUM_LNB_FRAMES; k++) {
          hGainDec->activeDrc[h][i].lnbIndexForChannel[j][k] = -1;
        }
      }
    }
  }

  for (j = 0; j < 28; j++) {
    hGainDec->channelGain[j] = FL2FXCONST_DBL(1.0f / (float)(1 << 8));
  }

  for (i = 0; i < 4 * (1024 / 256); i++) {
    hGainDec->dummySubbandGains[i] = FL2FXCONST_DBL(1.0f / (float)(1 << 7));
  }

  hGainDec->status = 0; /* startup */
  hGainDec->startupMs = 2500;

  return DE_OK;
}

void initDrcGainBuffers(const int frameSize, DRC_GAIN_BUFFERS* drcGainBuffers) {
  int i, j;

  /* prepare 48 instances of node buffers */
  for (i = 0; i < 48; i++) {
    for (j = 0; j < NUM_LNB_FRAMES; j++) {
      drcGainBuffers->linearNodeBuffer[i].nNodes[j] = 1;
      drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].gainLin =
          FL2FXCONST_DBL(1.0f / (float)(1 << 7));
      if (j == 0) {
        drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].time =
            0; /* initialize last node with startup node */
      } else {
        drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].time = frameSize - 1;
      }
    }
  }

  /* prepare dummyLnb, a linearNodeBuffer containing a constant gain of 0 dB, for the "no DRC
   * processing" case */
  drcGainBuffers->dummyLnb.gainInterpolationType = GIT_LINEAR;
  for (i = 0; i < NUM_LNB_FRAMES; i++) {
    drcGainBuffers->dummyLnb.nNodes[i] = 1;
    drcGainBuffers->dummyLnb.linearNode[i][0].gainLin = FL2FXCONST_DBL(1.0f / (float)(1 << 7));
    drcGainBuffers->dummyLnb.linearNode[i][0].time = frameSize - 1;
  }

  drcGainBuffers->lnbPointer = 0;
}

DRC_ERROR
initActiveDrc(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
              const int drcSetIdSelected, const int downmixIdSelected) {
  int g, isMultiband = 0;
  int loc = 0; /* before or after downmix */
  DRC_ERROR err = DE_OK;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = &hGainDec->drcCoef;

  pInst = selectDrcInstructions(hUniDrcConfig, drcSetIdSelected);
  if (pInst == NULL) {
    return DE_NOT_OK;
  }

  if (pInst->drcApplyToDownmix) loc = 1;

  if (pInst->drcSetId >= 0) {
    err = _generateDrcInstructionsDerivedData(
        hGainDec, hUniDrcConfig, pInst, pCoef,
        &(hGainDec->activeDrc[loc][hGainDec->nActiveDrcs[loc]]));
    if (err) return err;
  }

  hGainDec->activeDrc[loc][hGainDec->nActiveDrcs[loc]].drcInst =
      *pInst; /* keep deep copy of drcInstructions */

  if (loc == 0) {
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      if (hGainDec->activeDrc[loc][hGainDec->nActiveDrcs[loc]].bandCountForChannelGroup[g] > 1) {
        if (hGainDec->multiBandActiveDrcIndex != -1) {
          return DE_NOT_OK;
        }
        isMultiband = 1;
      }
    }

    if (isMultiband) {
      /* Keep activeDrc index of multiband DRC set */
      hGainDec->multiBandActiveDrcIndex = hGainDec->nActiveDrcs[loc];
      if (hGainDec->subbandDomainSupported) {
        /* initialize subband domain multiband processing */
        err = initOverlapWeight(
            pCoef, pInst,
            hGainDec->activeDrc[loc][hGainDec->nActiveDrcs[loc]].bandCountForChannelGroup,
            hGainDec->subbandDomainSupported, hGainDec->overlap);
        if (err) return err;
      }
    }
  }

  hGainDec->nActiveDrcs[loc]++;
  if (hGainDec->nActiveDrcs[loc] > MAX_ACTIVE_DRCS) return DE_NOT_OK;

  return DE_OK;
}

void addVirtualToActiveDrc(HANDLE_DRC_GAIN_DECODER hGainDec) {
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;
  int a, l, virtualDrcActive;

  /* add a virtual DRC set to the activeDrcs, if there is none present yet.
     This removes clicks if nActiveDrcs changes during runtime due to a user interaction. */

  for (l = 0; l < ACTIVE_DRC_LOCATIONS; l++) {
    /* cross-check if adding a virtual DRC set is possible */
    if (hGainDec->nActiveDrcs[l] >= MAX_ACTIVE_DRCS) continue;

    /* Don't add virtual DRC for subband processing, because clicks don't occur there */
    if (l == 0 && hGainDec->multiBandActiveDrcIndex >= 0) continue;

    virtualDrcActive = 0;
    /* cross-check if there is already a virtual DRC set active */
    for (a = 0; a < hGainDec->nActiveDrcs[l]; a++) {
      pInst = &hGainDec->activeDrc[l][a].drcInst;
      if (pInst->drcSetId < 0) {
        virtualDrcActive = 1;
      }
    }

    if (!virtualDrcActive) {
      /* add a virtual DRC set */
      pInst = &hGainDec->activeDrc[l][hGainDec->nActiveDrcs[l]].drcInst;
      FDKmemclear(pInst, sizeof(DRC_INSTRUCTIONS_UNI_DRC));
      pInst->drcSetId = -1;

      if (l == 0) {
        pInst->downmixIdCount = 1;
        pInst->downmixId[0] = DOWNMIX_ID_BASE_LAYOUT;
      } else if (l == 1) {
        pInst->downmixIdCount = 1;
        pInst->downmixId[0] = DOWNMIX_ID_ANY_DOWNMIX;
        pInst->drcApplyToDownmix = 1;
      } else {
        continue;
      }

      hGainDec->nActiveDrcs[l]++;
    }
  }

  /* There should now always be a virtual DRC set on location 0 (before downmix).
     Pick the first one to apply channelGains */
  if (hGainDec->nActiveDrcs[0] > 0 && hGainDec->channelGainActiveDrcIndex == -1) {
    hGainDec->channelGainActiveDrcIndex = 0;
  }
}

DRC_ERROR
initActiveDrcOffset(HANDLE_DRC_GAIN_DECODER hGainDec) {
  int a, l, accGainElementCount;

  accGainElementCount = 0;
  for (l = 0; l < ACTIVE_DRC_LOCATIONS; l++) {
    for (a = 0; a < hGainDec->nActiveDrcs[l]; a++) {
      hGainDec->activeDrc[l][a].activeDrcOffset = accGainElementCount;
      accGainElementCount += hGainDec->activeDrc[l][a].gainElementCount;
      if (accGainElementCount > 48) {
        hGainDec->nActiveDrcs[l] = a;
        return DE_NOT_OK;
      }
    }
  }

  return DE_OK;
}
