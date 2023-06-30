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
#include "fixpoint_math.h"
#include "drcDecoder.h"

int getDeltaTmin(const int sampleRate) {
  /* half_ms = round (0.0005 * sampleRate); */
  int half_ms = (sampleRate + 1000) / 2000;
  int deltaTmin = 1;
  if (sampleRate < 1000) {
    return DE_NOT_OK;
  }
  while (deltaTmin <= half_ms) {
    deltaTmin = deltaTmin << 1;
  }
  return deltaTmin;
}

DRC_COEFFICIENTS_UNI_DRC* selectDrcCoefficients(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                const int location) {
  int n;
  int c = -1;
  for (n = 0; n < hUniDrcConfig->drcCoefficientsUniDrcCount; n++) {
    if (hUniDrcConfig->drcCoefficientsUniDrc[n].drcLocation == location) {
      c = n;
    }
  }
  if (c >= 0) {
    return &(hUniDrcConfig->drcCoefficientsUniDrc[c]);
  }
  return NULL; /* possible during bitstream parsing */
}

DRC_INSTRUCTIONS_UNI_DRC* selectDrcInstructions(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                const int drcSetId) {
  int i;
  for (i = 0; i < hUniDrcConfig->drcInstructionsCountInclVirtual; i++) {
    if (hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId == drcSetId) {
      return &(hUniDrcConfig->drcInstructionsUniDrc[i]);
    }
  }
  return NULL;
}

DOWNMIX_INSTRUCTIONS* selectDownmixInstructions(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                const int downmixId) {
  int i;
  for (i = 0; i < hUniDrcConfig->downmixInstructionsCount; i++) {
    if (hUniDrcConfig->downmixInstructions[i].downmixId == downmixId) {
      return &(hUniDrcConfig->downmixInstructions[i]);
    }
  }
  return NULL;
}

DRC_ERROR
deriveDrcChannelGroups(const int drcSetEffect,                                    /* in */
                       const int channelCount,                                    /* in */
                       const SCHAR* gainSetIndex,                                 /* in */
                       const DUCKING_MODIFICATION* duckingModificationForChannel, /* in */
                       UCHAR* nDrcChannelGroups,                                  /* out */
                       SCHAR* uniqueIndex,     /* out (gainSetIndexForChannelGroup) */
                       SCHAR* groupForChannel, /* out */
                       DUCKING_MODIFICATION* duckingModificationForChannelGroup) /* out */
{
  int duckingSequence = -1;
  int c, n, g, match, idx;
  FIXP_SGL factor;
  FIXP_SGL uniqueScaling[28];

  for (g = 0; g < 28; g++) {
    uniqueIndex[g] = -10;
    uniqueScaling[g] = FIXP_SGL(-1.0f);
  }

  g = 0;

  if (drcSetEffect & EB_DUCK_OTHER) {
    for (c = 0; c < channelCount; c++) {
      match = 0;
      if (c >= 2 * 28) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      factor = duckingModificationForChannel[c].duckingScaling;
      if (idx < 0) {
        for (n = 0; n < g; n++) {
          if (uniqueScaling[n] == factor) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 28) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          uniqueScaling[g] = factor;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        if ((duckingSequence > 0) && (duckingSequence != idx)) {
          return DE_NOT_OK;
        }
        duckingSequence = idx;
        groupForChannel[c] = -1;
      }
    }
    if (duckingSequence == -1) {
      return DE_NOT_OK;
    }
  } else if (drcSetEffect & EB_DUCK_SELF) {
    for (c = 0; c < channelCount; c++) {
      match = 0;
      if (c >= 2 * 28) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      factor = duckingModificationForChannel[c].duckingScaling;
      if (idx >= 0) {
        for (n = 0; n < g; n++) {
          if ((uniqueIndex[n] == idx) && (uniqueScaling[n] == factor)) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 28) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          uniqueScaling[g] = factor;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        groupForChannel[c] = -1;
      }
    }
  } else { /* no ducking */
    for (c = 0; c < channelCount; c++) {
      if (c >= 2 * 28) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      match = 0;
      if (idx >= 0) {
        for (n = 0; n < g; n++) {
          if (uniqueIndex[n] == idx) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 28) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        groupForChannel[c] = -1;
      }
    }
  }
  *nDrcChannelGroups = g;

  if (drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    for (g = 0; g < *nDrcChannelGroups; g++) {
      if (drcSetEffect & EB_DUCK_OTHER) {
        uniqueIndex[g] = duckingSequence;
      }
      duckingModificationForChannelGroup[g].duckingScaling = uniqueScaling[g];
      if (uniqueScaling[g] != FL2FXCONST_SGL(1.0f / (float)(1 << 2))) {
        duckingModificationForChannelGroup[g].duckingScalingPresent = 1;
      } else {
        duckingModificationForChannelGroup[g].duckingScalingPresent = 0;
      }
    }
  }

  /* sanity check */
  for (g = 0; g < *nDrcChannelGroups; g++) {
    if (uniqueIndex[g] < 0) return DE_NOT_OK;
  }

  return DE_OK;
}

FIXP_DBL
dB2lin(const FIXP_DBL dB_m, const int dB_e, int* pLin_e) {
  /* get linear value from dB.
     return lin_val = 10^(dB_val/20) = 2^(log2(10)/20*dB_val)
     with dB_val = dB_m *2^dB_e and lin_val = lin_m * 2^lin_e */
  FIXP_DBL lin_m =
      f2Pow(fMult(dB_m, FL2FXCONST_DBL(0.1660964f * (float)(1 << 2))), dB_e - 2, pLin_e);

  return lin_m;
}

FIXP_DBL
approxDb2lin(const FIXP_DBL dB_m, const int dB_e, int* pLin_e) {
  /* get linear value from approximate dB.
     return lin_val = 2^(dB_val/6)
     with dB_val = dB_m *2^dB_e and lin_val = lin_m * 2^lin_e */
  FIXP_DBL lin_m =
      f2Pow(fMult(dB_m, FL2FXCONST_DBL(0.1666667f * (float)(1 << 2))), dB_e - 2, pLin_e);

  return lin_m;
}

int bitstreamContainsMultibandDrc(HANDLE_UNI_DRC_CONFIG hUniDrcConfig, const int downmixId) {
  int i, g, d, seq;
  DRC_INSTRUCTIONS_UNI_DRC* pInst;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = NULL;
  int isMultiband = 0;

  pCoef = selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);
  if (pCoef == NULL) return 0;

  for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
    pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
    for (d = 0; d < pInst->downmixIdCount; d++) {
      if (downmixId == pInst->downmixId[d]) {
        for (g = 0; g < pInst->nDrcChannelGroups; g++) {
          seq = pInst->gainSetIndexForChannelGroup[g];
          if ((seq < pCoef->gainSetCount) && (pCoef->gainSet[seq].bandCount > 1)) {
            isMultiband = 1;
          }
        }
      }
    }
  }

  return isMultiband;
}
