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

/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/

#include "FDK_formatConverter_data.h"

#include "FDK_formatConverter_init.h"
#include "FDK_formatConverter_activeDmx_stft.h"

/**********************************************************************************************************************************/

int setFormatConverterParams(const FIXP_DBL* centerFrequenciesNormalized,
                             IIS_FORMATCONVERTER_INTERNAL* fcInt) {
  UINT i, j;
  INT status = 0;

  FDK_ASSERT(fcInt != NULL);

  HANDLE_FORMAT_CONVERTER_PARAMS params = fcInt->fcParams;
  FDK_ASSERT(params != NULL);

  /* freq domain params */
  if (fcInt->mode != IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN) {
    /* apply equalization filters */
    params->applyEqFilters = 1;
    FDK_ASSERT(centerFrequenciesNormalized != NULL);

    if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
        fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
      for (i = 0; i < fcInt->stftNumErbBands; i++) {
        params->centerFrequenciesNormalized[i] = centerFrequenciesNormalized[i];
      }
    }

    /* aeq limits */
    params->eqLimitMax = (FIXP_DBL)0x50615FA7; /* FL2FXCONST_DBL(2.511886432f/4.0f);
                                                  pow(10.f, 8.f/20.f) in format Q3.29*/
    params->eqLimitMin = (FIXP_DBL)0x50F44D89; /* 0x0A1E89B1; FL2FXCONST_DBL(0.316227766f/4.0f);
                                                  pow(10.f, -10.f/20.f) in format Q0.32 */

    /* time domain params */
  } else {
    /* Do not apply equalization filters */
    params->applyEqFilters = 0;

    /* aeq limits */
    params->eqLimitMax = (FIXP_DBL)0;
    params->eqLimitMin = (FIXP_DBL)0;
  }

  /* switch off equalizer filters in case of generic setups with external DmxMtx */
  if (params->genericIOFmt) {
    params->applyEqFilters = 0;
  }

  params->randomFlag = 0;

  /* init DMX matrix */
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    for (j = 0; j < fcInt->numOutputChannels; j++) {
      params->dmxMtx[i][j] = (FIXP_DBL)0;
    }
  }

  params->dmxMtxIsSet = 0;

  return status;
}

/**********************************************************************************************************************************/

int allocateFormatConverterParams(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  UINT i;
  INT status = 0;
  /* dmx mtx */
  fcInt->fcParams->dmxMtx =
      (FIXP_DMX_H**)FDKcalloc(fcInt->numTotalInputChannels, sizeof *fcInt->fcParams->dmxMtx);
  if (fcInt->fcParams->dmxMtx == NULL) {
    return status = -1;
  }
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    fcInt->fcParams->dmxMtx[i] =
        (FIXP_DMX_H*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->dmxMtx[i]);
    if (fcInt->fcParams->dmxMtx[i] == NULL) {
      status = -1;
    }
  }

  /* dmx mtx sorted */
  fcInt->fcParams->dmxMtx_sorted =
      (FIXP_DMX_H*)FDKcalloc(fcInt->numTotalInputChannels * fcInt->numOutputChannels,
                             sizeof *fcInt->fcParams->dmxMtx_sorted);
  if (fcInt->fcParams->dmxMtx_sorted == NULL) {
    status = -1;
  }

  /* dmx mtx L */
  fcInt->fcParams->dmxMtxL =
      (FIXP_DMX_H**)FDKcalloc(fcInt->numTotalInputChannels, sizeof *fcInt->fcParams->dmxMtxL);
  if (fcInt->fcParams->dmxMtxL == NULL) {
    return -1;
  }
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    fcInt->fcParams->dmxMtxL[i] =
        (FIXP_DMX_H*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->dmxMtxL[i]);
    if (fcInt->fcParams->dmxMtxL[i] == NULL) {
      return -1;
    }
  }

  /* dmx mtx L sorted */
  fcInt->fcParams->dmxMtxL_sorted =
      (FIXP_DMX_H*)FDKcalloc(fcInt->numTotalInputChannels * fcInt->numOutputChannels,
                             sizeof *fcInt->fcParams->dmxMtxL_sorted);
  if (fcInt->fcParams->dmxMtxL_sorted == NULL) {
    return -1;
  }

  /* dmx mtx 2 */
  fcInt->fcParams->dmxMtx2 =
      (FIXP_DMX_H**)FDKcalloc(fcInt->numTotalInputChannels, sizeof *fcInt->fcParams->dmxMtx2);
  if (fcInt->fcParams->dmxMtx2 == NULL) {
    return -1;
  }
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    fcInt->fcParams->dmxMtx2[i] =
        (FIXP_DMX_H*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->dmxMtx2[i]);
    if (fcInt->fcParams->dmxMtx2[i] == NULL) {
      return -1;
    }
  }

  /* dmx mtx 2 sorted */
  fcInt->fcParams->dmxMtx2_sorted =
      (FIXP_DMX_H*)FDKcalloc(fcInt->numTotalInputChannels * fcInt->numOutputChannels,
                             sizeof *fcInt->fcParams->dmxMtx2_sorted);
  if (fcInt->fcParams->dmxMtx2_sorted == NULL) {
    return -1;
  }

  /* Eq index vector */
  fcInt->fcParams->eqIndexVec =
      (INT**)FDKcalloc(fcInt->numTotalInputChannels, sizeof *fcInt->fcParams->eqIndexVec);
  if (fcInt->fcParams->eqIndexVec == NULL) {
    return status = -1;
  }
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    fcInt->fcParams->eqIndexVec[i] =
        (INT*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->eqIndexVec[i]);
    if (fcInt->fcParams->eqIndexVec[i] == NULL) {
      status = -1;
    }
  }
  /* Eq index vector 2 */
  fcInt->fcParams->eqIndexVec2 =
      (INT**)FDKcalloc(fcInt->numTotalInputChannels, sizeof *fcInt->fcParams->eqIndexVec2);
  if (fcInt->fcParams->eqIndexVec2 == NULL) {
    return -1;
  }
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    fcInt->fcParams->eqIndexVec2[i] =
        (INT*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->eqIndexVec2[i]);
    if (fcInt->fcParams->eqIndexVec2[i] == NULL) {
      return -1;
    }
  }

  /* Eq index vector sorted */
  fcInt->fcParams->eqIndexVec_sorted =
      (INT*)FDKcalloc(fcInt->numTotalInputChannels * fcInt->numOutputChannels,
                      sizeof *fcInt->fcParams->eqIndexVec_sorted);
  if (fcInt->fcParams->eqIndexVec_sorted == NULL) {
    status = -1;
  }

  /* Eq index vector sorted 2 */
  fcInt->fcParams->eqIndexVec2_sorted =
      (INT*)FDKcalloc(fcInt->numTotalInputChannels * fcInt->numOutputChannels,
                      sizeof *fcInt->fcParams->eqIndexVec2_sorted);
  if (fcInt->fcParams->eqIndexVec2_sorted == NULL) {
    return -1;
  }

  /* internal structure */
  fcInt->fcParams->formatConverterParams_internal =
      (converter_pr_t*)FDKcalloc(1, sizeof *fcInt->fcParams->formatConverterParams_internal);
  if (fcInt->fcParams->formatConverterParams_internal == NULL) {
    status = -1;
  }

  /* centerFrequenciesNormalized */
  if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    fcInt->fcParams->centerFrequenciesNormalized =
        (FIXP_DBL*)FDKcalloc(fcInt->fcNumFreqBands, sizeof(FIXP_DBL));
    if (fcInt->fcParams->centerFrequenciesNormalized == NULL) {
      status = -1;
    }
  }

  /* azimuthElevationDeviation */
  fcInt->fcParams->azimuthElevationDeviation = (INT*)FDKcalloc(
      2 * fcInt->numOutputChannels, sizeof *fcInt->fcParams->azimuthElevationDeviation);
  if (fcInt->fcParams->azimuthElevationDeviation == NULL) {
    status = -1;
  }

  /* distance */
  fcInt->fcParams->distance =
      (INT*)FDKcalloc(fcInt->numOutputChannels, sizeof *fcInt->fcParams->distance);
  if (fcInt->fcParams->distance == NULL) {
    status = -1;
  }

  return status;
}

/**********************************************************************************************************************************/

int allocateFormatConverterEQs(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, INT NumSignalGroups) {
  /* Allocate new equalizers for every new signal group: */

  INT numInputChannels = 0;

  for (INT grp = 0; grp < NumSignalGroups; grp++) {
    numInputChannels += fcInt->numInputChannels[grp];
  }

  /* eqGains */
  if (fcInt->eqGains[0] == NULL) {
    for (INT i = 0; i < (INT)numInputChannels; i++) {
      fcInt->eqGains[i] = (FIXP_EQ_H*)FDKcalloc(fcInt->stftNumErbBands, sizeof *fcInt->eqGains[i]);
      if (fcInt->eqGains[i] == NULL) {
        return -1;
      }
    }
  }

  return 0;
}

/**********************************************************************************************************************************/

void freeFormatConverterParams(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  unsigned int i;

  /* dmx mtx */

  if (fcInt->fcParams->dmxMtx != NULL) {
    for (i = 0; i < fcInt->numTotalInputChannels; i++) {
      /*
      if( fcInt->fcParams->dmxMtx[i] != NULL && fcInt->fcParams->dmxMtxIsSet )
      {*/
      FDKfree(fcInt->fcParams->dmxMtx[i]);
      /*}*/
    }
  }
  FDKfree(fcInt->fcParams->dmxMtx);
  fcInt->fcParams->dmxMtx = NULL;

  /* dmx mtx sorted */

  FDKfree(fcInt->fcParams->dmxMtx_sorted);
  fcInt->fcParams->dmxMtx_sorted = NULL;

  /* dmx mtx L */

  if (fcInt->fcParams->dmxMtxL != NULL) {
    for (i = 0; i < fcInt->numTotalInputChannels; i++) {
      /*
      if( fcInt->fcParams->dmxMtxL[i] != NULL && fcInt->fcParams->dmxMtxLIsSet )
      {*/
      FDKfree(fcInt->fcParams->dmxMtxL[i]);
      /*}*/
    }
  }
  FDKfree(fcInt->fcParams->dmxMtxL);
  fcInt->fcParams->dmxMtxL = NULL;

  /* dmx mtx L sorted */

  FDKfree(fcInt->fcParams->dmxMtxL_sorted);
  fcInt->fcParams->dmxMtxL_sorted = NULL;

  /* dmx mtx 2 */
  if (fcInt->fcParams->dmxMtx2 != NULL) {
    for (i = 0; i < fcInt->numTotalInputChannels; i++) {
      /*
      if( fcInt->fcParams->dmxMtx2[i] != NULL && fcInt->fcParams->dmxMtx2IsSet )
      {*/
      FDKfree(fcInt->fcParams->dmxMtx2[i]);
      /*}*/
    }
  }
  FDKfree(fcInt->fcParams->dmxMtx2);
  fcInt->fcParams->dmxMtx2 = NULL;

  /* dmx mtx 2 sorted */

  FDKfree(fcInt->fcParams->dmxMtx2_sorted);
  fcInt->fcParams->dmxMtx2_sorted = NULL;

  /* eqIndexVec */
  if (fcInt->fcParams->eqIndexVec != NULL) {
    for (i = 0; i < fcInt->numTotalInputChannels; i++) {
      FDKfree(fcInt->fcParams->eqIndexVec[i]);
    }
  }
  FDKfree(fcInt->fcParams->eqIndexVec);
  fcInt->fcParams->eqIndexVec = NULL;
  /* eqIndexVec sorted */

  FDKfree(fcInt->fcParams->eqIndexVec_sorted);

  /* eqIndexVec 2 */
  if (fcInt->fcParams->eqIndexVec2 != NULL) {
    for (i = 0; i < fcInt->numTotalInputChannels; i++) {
      FDKfree(fcInt->fcParams->eqIndexVec2[i]);
    }
  }
  FDKfree(fcInt->fcParams->eqIndexVec2);
  fcInt->fcParams->eqIndexVec2 = NULL; /* eqIndexVec sorted */

  FDKfree(fcInt->fcParams->eqIndexVec2_sorted);

  fcInt->fcParams->eqIndexVec2_sorted = NULL;

  /* eqGains */
  for (i = 0; i < FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS; i++) {
    if (fcInt->eqGains[i] != NULL) {
      FDKfree(fcInt->eqGains[i]);
    }
  }

  /* internal structure */
  if (fcInt->fcParams->formatConverterParams_internal != NULL) {
    FDKfree(fcInt->fcParams->formatConverterParams_internal);
    fcInt->fcParams->formatConverterParams_internal = NULL;
  }

  /* centerFrequenciesNormalized */
  if (fcInt->fcParams->centerFrequenciesNormalized != NULL) {
    FDKfree(fcInt->fcParams->centerFrequenciesNormalized);
    fcInt->fcParams->centerFrequenciesNormalized = NULL;
  }

  /* azimuthElevationDeviation */
  if (fcInt->fcParams->azimuthElevationDeviation != NULL) {
    FDKfree(fcInt->fcParams->azimuthElevationDeviation);
    fcInt->fcParams->azimuthElevationDeviation = NULL;
  }

  /* distance */
  if (fcInt->fcParams->distance != NULL) {
    FDKfree(fcInt->fcParams->distance);
    fcInt->fcParams->distance = NULL;
  }
}

/**********************************************************************************************************************************/

int setFormatConverterState(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  int errorFlag1 = 0;
  int errorFlag2 = 0;

  /* init phase align dmx */
  /* init phase align dmx */

  if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT)
  /* IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN_STFT: This only depends on the aes parameter, 7
     for active and 0 for passive. */
  {
    errorFlag2 = activeDmxStftInit(
        &fcInt->fcState->handleActiveDmxStft, fcInt->numTotalInputChannels,
        fcInt->numOutputChannels, fcInt->inputBufferStft, fcInt->prevInputBufferStft,
        fcInt->outputBufferStft, fcInt->stftFrameSize, fcInt->stftLength,
        fcInt->fcParams->eqLimitMax, fcInt->fcParams->eqLimitMin, fcInt->fcNumFreqBands,
        fcInt->stftNumErbBands, fcInt->stftErbFreqIdx, fcInt->aes);
  }

  if (errorFlag1 != 0 || errorFlag2 != 0)
    return -1;
  else
    return 0;
}

/**********************************************************************************************************************************/

void freeFormatConverterState(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  if (fcInt->fcState != NULL) {
    /* free stft dmx */
    if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
        fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
      if (fcInt->fcState->handleActiveDmxStft != NULL)
        activeDmxClose_STFT(fcInt->fcState->handleActiveDmxStft);
    }
  }
}

/**********************************************************************************************************************************/
/**********************************************************************************************************************************/

int formatConverterInit_internal(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, INT* p_buffer) {
  UINT i, j;
  INT* randomization;

  converter_status_t converterState = FORMAT_CONVERTER_STATUS_OK;

  /* input format */
  switch (fcInt->fcInputFormat) {
    case FDK_FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS:
      fcInt->fcParams->formatConverterInputFormat_internal = FDK_FORMAT_IN_LISTOFCHANNELS;
      break;
    default:
      return 1;
  }

  /* output format */
  switch (fcInt->fcOutputFormat) {
    case FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS:
      fcInt->fcParams->formatConverterOutputFormat_internal = FDK_FORMAT_OUT_LISTOFCHANNELS;
      break;
    default:
      return 1;
  }

  /* randomization */
  if (fcInt->fcParams->randomFlag) {
    randomization = fcInt->fcParams->azimuthElevationDeviation;
  } else {
    randomization = NULL;
  }

  /* init internal structure */
  if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT ||
      fcInt->mode == IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN) {
    converterState =
        converter_init(fcInt, fcInt->fcParams->formatConverterParams_internal,
                       fcInt->fcParams->formatConverterInputFormat_internal,
                       fcInt->fcParams->formatConverterOutputFormat_internal, randomization,
                       fcInt->samplingRate, fcInt->frameSize, fcInt->stftNumErbBands,
                       fcInt->fcParams->centerFrequenciesNormalized, p_buffer);
  }

  if (converterState != 0) {
    return converterState;
  }

  {
    /* generate dmx matrix */
    if (fcInt->numOutputChannels != 1) {
      i = 0;
      while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src[i] >= 0) {
        fcInt->fcParams->dmxMtx[(fcInt->fcParams->formatConverterParams_internal)->in_out_src[i]]
                               [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst[i]] =
            (fcInt->fcParams->formatConverterParams_internal)->in_out_gain[i];
        i++;
      }
    } else {
      for (i = 0; i < fcInt->numTotalInputChannels; i++) {
        fcInt->fcParams->dmxMtx[i][0] = (FIXP_DBL)0;
      }
      i = 0;
      while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src[i] >= 0) {
        fcInt->fcParams
            ->dmxMtx[(fcInt->fcParams->formatConverterParams_internal)->in_out_src[i]][0] +=
            (fcInt->fcParams->formatConverterParams_internal)->in_out_gain[i];
        i++;
      }
    }
  }
  if (fcInt->immersiveDownmixFlag &&
      ((fcInt->numOutputChannels == 6) || (fcInt->numOutputChannels == 5))) {
    /* generate dmx matrix gainL*/

    i = 0;
    while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src[i] >= 0) {
      if ((fcInt->fcParams->formatConverterParams_internal)->in_out_gainL[i] > (FIXP_DMX_H)-1) {
        fcInt->fcParams->dmxMtxL[(fcInt->fcParams->formatConverterParams_internal)->in_out_src[i]]
                                [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst[i]] =
            (fcInt->fcParams->formatConverterParams_internal)->in_out_gainL[i];
      } else {
        fcInt->fcParams->dmxMtxL[(fcInt->fcParams->formatConverterParams_internal)->in_out_src[i]]
                                [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst[i]] =
            (fcInt->fcParams->formatConverterParams_internal)->in_out_gain[i];
      }
      i++;
    }

    /* generate dmx matrix 2 gain2*/
    i = 0;
    while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src2[i] >= 0) {
      fcInt->fcParams->dmxMtx2[(fcInt->fcParams->formatConverterParams_internal)->in_out_src2[i]]
                              [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst2[i]] =
          (fcInt->fcParams->formatConverterParams_internal)->in_out_gain2[i];
      i++;
    }
  }

  if (fcInt->mode == IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
      fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    /**** generate eq index matrix ****/
    i = 0;
    while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src[i] >= 0) {
      fcInt->fcParams
          ->eqIndexVec[(fcInt->fcParams->formatConverterParams_internal)->in_out_src[i]]
                      [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst[i]] =
          (fcInt->fcParams->formatConverterParams_internal)->in_out_proc[i];
      i++;
    }
    if (fcInt->immersiveDownmixFlag &&
        ((fcInt->numOutputChannels == 6) || (fcInt->numOutputChannels == 5))) {
      /**** generate eq index matrix 2 ****/
      i = 0;
      while ((fcInt->fcParams->formatConverterParams_internal)->in_out_src2[i] >= 0) {
        fcInt->fcParams
            ->eqIndexVec2[(fcInt->fcParams->formatConverterParams_internal)->in_out_src2[i]]
                         [(fcInt->fcParams->formatConverterParams_internal)->in_out_dst2[i]] =
            (fcInt->fcParams->formatConverterParams_internal)->in_out_proc2[i];
        i++;
      }
    }
  }

  /* Resort the dmxMatrix in order to be applied in the passive downmix function
   * formatConverterProcess_passive_timeDomain_frameLength.  */
  j = 0;
  UINT column, row;

  for (column = 0; column < fcInt->numTotalInputChannels; column++) {
    for (row = 0; row < fcInt->numOutputChannels; row++) {
      fcInt->fcParams->eqIndexVec_sorted[j] = fcInt->fcParams->eqIndexVec[column][row];
      fcInt->fcParams->dmxMtx_sorted[j] = fcInt->fcParams->dmxMtx[column][row];
      if (fcInt->immersiveDownmixFlag &&
          ((fcInt->numOutputChannels == 6) || (fcInt->numOutputChannels == 5))) {
        fcInt->fcParams->dmxMtxL_sorted[j] = fcInt->fcParams->dmxMtxL[column][row];
        fcInt->fcParams->eqIndexVec2_sorted[j] = fcInt->fcParams->eqIndexVec2[column][row];
        fcInt->fcParams->dmxMtx2_sorted[j] = fcInt->fcParams->dmxMtx2[column][row];
      }
      j++;
    }
  }

  return 0;
}

int formatConverterDmxMatrixControl(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  /*********************************************/
  /* Immersive mode initializer                */
  /*********************************************/
  fcInt->fcParams->immersiveMode =
      fcInt->immersiveDownmixFlag &&
      ((fcInt->numOutputChannels == 6) || (fcInt->numOutputChannels == 5));
  fcInt->fcParams->Mode3Drendering = (fcInt->rendering3DTypeFlag) && fcInt->fcParams->immersiveMode;
  fcInt->fcParams->Mode2Drendering =
      (!fcInt->rendering3DTypeFlag) && fcInt->fcParams->immersiveMode;
  fcInt->fcParams->rendering3DTypeFlag_internal = fcInt->rendering3DTypeFlag;

  fcInt->fcParams->dmx_iterations = 1;

  /* Select right set of downmix matrices and equalizer vectors */
  if (fcInt->fcParams->Mode3Drendering) {
    fcInt->fcParams->dmxMatrixH_FDK = fcInt->fcParams->dmxMtx_sorted;
    fcInt->fcParams->dmxMatrixL_FDK = fcInt->fcParams->dmxMtxL_sorted;
    fcInt->fcParams->eqIndex_FDK = fcInt->fcParams->eqIndexVec_sorted;
    fcInt->fcParams->dmx_iterations = 3;
  } else if (fcInt->fcParams->Mode2Drendering) {
    fcInt->fcParams->dmxMatrixL_FDK = fcInt->fcParams->dmxMtx2_sorted;
    fcInt->fcParams->dmxMatrixH_FDK = fcInt->fcParams->dmxMatrixL_FDK; /* dummy */
    fcInt->fcParams->eqIndex_FDK = fcInt->fcParams->eqIndexVec2_sorted;
  } else {
    fcInt->fcParams->dmxMatrixL_FDK = fcInt->fcParams->dmxMtx_sorted;
    fcInt->fcParams->dmxMatrixH_FDK = fcInt->fcParams->dmxMatrixL_FDK; /* dummy */
    fcInt->fcParams->eqIndex_FDK = fcInt->fcParams->eqIndexVec_sorted;
  }

  return 0;
}

int formatConverterDmxMatrixExponent(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  /* This function calculate how many input channels have an impact in each output channel and its
   * exponential term in order to accumulate results. */

  UINT chIn, chOut;

  FIXP_DMX_H* dmxMtx = fcInt->fcParams->dmxMtx_sorted;
  FIXP_DMX_H* dmxMtx2 = fcInt->fcParams->dmxMtx2_sorted;
  FIXP_DMX_H* dmxMtxL = fcInt->fcParams->dmxMtxL_sorted;

  for (chOut = 0; chOut < fcInt->numOutputChannels; chOut++) {
    fcInt->fcParams->chOut_count[chOut] = 0;
  }

  for (chIn = 0; chIn < fcInt->numTotalInputChannels; chIn++) {
    for (chOut = 0; chOut < fcInt->numOutputChannels; chOut++) {
      if ((dmxMtx[chOut] != (FIXP_DMX_H)0) || (dmxMtx2[chOut] != (FIXP_DMX_H)0) ||
          (dmxMtxL[chOut] != (FIXP_DMX_H)0)) {
        fcInt->fcParams->chOut_count[chOut] += 1;
      }
    }
    dmxMtx += fcInt->numOutputChannels;
    dmxMtx2 += fcInt->numOutputChannels;
    dmxMtxL += fcInt->numOutputChannels;
  }

  for (chOut = 0; chOut < fcInt->numOutputChannels; chOut++) {
    if (fcInt->fcParams->chOut_count[chOut] > 0) {
      fcInt->fcParams->chOut_exp[chOut] =
          32 - CntLeadingZeros(fcInt->fcParams->chOut_count[chOut] - 1);
    } else {
      fcInt->fcParams->chOut_exp[chOut] = 0;
    }
  }

  return 0;
}

/**********************************************************************************************************************************/
