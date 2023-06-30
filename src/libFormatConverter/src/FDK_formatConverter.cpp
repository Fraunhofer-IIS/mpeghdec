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

#include "FDK_formatConverter.h"

#include "FDK_formatConverter_process.h"
#include "FDK_formatConverter_data.h"
#include "FDK_formatConverter_activeDmx_stft.h"

#include "common_fix.h"

/**********************************************************************************************************************************/

void formatConverterSetInOutFormat(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, const INT inout,
                                   const INT numChannels,
                                   const AFC_FORMAT_CONVERTER_CHANNEL_ID* channel_vector) {
  FDK_ASSERT((inout == 0) || (inout == 1));
  converter_set_inout_format(fcInt, inout, numChannels, channel_vector);
}

/**********************************************************************************************************************************/

INT formatConverterMatchChConfig2Channels(const CICP2GEOMETRY_CHANNEL_GEOMETRY* chConfig,
                                          const INT inout, const INT numChannels,
                                          AFC_FORMAT_CONVERTER_CHANNEL_ID* channel_vec,
                                          UINT* numUnknownCh, INT* unknownCh_vec) {
  UINT k, l;
  INT az, el, azstart, azend, elstart, elend;
  INT lfeIdx[FDK_FORMAT_CONVERTER_MAX_LFE] = {0};
  UINT numLfes;

  static const INT azel[CH_LFE2 + 1][6] = {
      /*   azi    ele  azstart azend elstart elend */
      {0, 0, -7, 7, -9, 20},          {22, 0, 8, 22, -9, 20},
      {-22, 0, -22, -8, -9, 20},      {30, 0, 23, 37, -9, 20},
      {-30, 0, -37, -23, -9, 20},     {45, 0, 38, 52, -9, 20},
      {-45, 0, -52, -38, -9, 20},     {60, 0, 53, 75, -9, 20},
      {-60, 0, -75, -53, -9, 20},     {90, 0, 76, 100, -45, 20},
      {-90, 0, -100, -76, -45, 20},   {110, 0, 101, 124, -45, 20},
      {-110, 0, -124, -101, -45, 20}, {135, 0, 125, 142, -45, 20},
      {-135, 0, -142, -125, -45, 20}, {150, 0, 143, 157, -45, 20},
      {-150, 0, -157, -143, -45, 20}, {180, 0, 158, -158, -45, 20},
      {0, 35, -10, 10, 21, 60},       {45, 35, 38, 66, 21, 60},
      {-45, 35, -66, -38, 21, 60},    {30, 35, 11, 37, 21, 60},
      {-30, 35, -37, -11, 21, 60},    {90, 35, 67, 100, 21, 60},
      {-90, 35, -100, -67, 21, 60},   {110, 35, 101, 124, 21, 60},
      {-110, 35, -124, -101, 21, 60}, {135, 35, 125, 157, 21, 60},
      {-135, 35, -157, -125, 21, 60}, {180, 35, 158, -158, 21, 60},
      {0, 90, -180, 180, 61, 90},     {0, -15, -10, 10, -45, -10},
      {45, -15, 11, 75, -45, -10},    {-45, -15, -75, -11, -45, -10},
      {45, 0, 0, 0, -20, 20},  /* LFE1 */
      {-45, 0, 0, 0, -20, 20}, /* LFE2 */
  };

  /* no known channel used in the beginning */
  INT channelUsed[CH_LFE1] = {0};

  /* count number of LFEs */
  numLfes = 0;
  for (k = 0; k < (UINT)numChannels; k++)
    if ((chConfig[k]).LFE == 1) numLfes++;

  /* all channels are unknown at the beginning */
  *numUnknownCh = numChannels;

  /* ==== match non-LFE channels to known channels ==== */
  for (k = 0; k < (UINT)numChannels; k++) {
    /* initialize channel k as unknown channel, 0 offset */
    channel_vec[k] = CH_EMPTY;
    if ((chConfig[k]).LFE == 0) { /* <== non-LFEs only */
      /* search for matching sector. sectors are non-overlapping, thus only
         1 sector may match (but multiple unknowns may match the 1 known). */
      for (l = 0; l < CH_LFE1; l++) {
        az = (chConfig[k]).Az; /* read az inside l-loop since we may overwrite it below */
        el = (chConfig[k]).El;
        azstart = azel[l][2];
        azend = azel[l][3];
        elstart = azel[l][4];
        elend = azel[l][5];
        if ((azstart > 0) && (azend < 0)) { /* +-180 deg. wrap around */
          azend = azend + 360;
          if (az < 0) az = az + 360;
        }
        if ((az >= azstart) && (az <= azend) && (el >= elstart) && (el <= elend)) {
          /* multiple use of known channels only allowed in input setup */
          if ((channelUsed[l] == 0) || (inout == 0)) {
            channel_vec[k] = (AFC_FORMAT_CONVERTER_CHANNEL_ID)l;
            channelUsed[l] = 1;
            (*numUnknownCh)--;
          } else {
            return 1;
          }
        }
      }
    }

  } /* end of non-LFE matching */

  /* ==== match LFE channel(s) ==== */
  /* find the indices of the numLfes LFE(s) in the input config */
  if (numLfes > 0) {
    l = 0;
    for (k = 0; k < (UINT)numChannels; k++)
      if ((chConfig[k]).LFE == 1) {
        lfeIdx[l] = k;
        l++;
      }
  }

  /*
  LFE channel in config: => assign rule +azimuth -> LFE1
                                        -azimuth -> LFE2
  */
  for (l = 0; l < numLfes; l++) { /* loop over LFEs in config */
    az = ((chConfig[lfeIdx[l]]).Az % 360);
    if (((az >= 0) && (az < 180)) || ((az < -180) && (az >= -360))) {
      channel_vec[lfeIdx[l]] = CH_LFE1;
      (*numUnknownCh)--;
    } else {
      channel_vec[lfeIdx[l]] = CH_LFE2;
      (*numUnknownCh)--;
    }
  }

  /* signal list of the (*numUnknownCh) unknown channels to the caller, ignore unmatched LFEs */
  if (*numUnknownCh) {
    l = 0;
    for (k = 0; k < (UINT)numChannels; k++) {
      if (channel_vec[k] == CH_EMPTY) {
        {
          unknownCh_vec[l] = k;
          l++;
        }
      }
    }
  }

  return 0;
}

/**********************************************************************************************************************************/

INT formatConverterOpen(const IIS_FORMATCONVERTER_MODE formatConverterMode,
                        IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  INT status = 0;

  /**************************************************************************************/
  /* allocation */

  fcInt->fcParams = (HANDLE_FORMAT_CONVERTER_PARAMS)FDKcalloc(1, sizeof *fcInt->fcParams);
  if (fcInt->fcParams == NULL) {
    status = 1;
    goto bail;
  }
  fcInt->fcState = (HANDLE_FORMAT_CONVERTER_STATE)FDKcalloc(1, sizeof *fcInt->fcState);
  if (fcInt->fcState == NULL) {
    status = 1;
    goto bail;
  }
  /**************************************************************************************/
  /* check input parameters */

  /* delay */
  fcInt->fcParams->formatConverterDelay = formatConverterGetDelaySamples(fcInt->mode);
  /**delaySamples = fcInt->fcParams->formatConverterDelay;*/

  /* generic formats (for use with external DMXmtx) */
  if (fcInt->fcInputFormat == FDK_FORMAT_CONVERTER_INPUT_FORMAT_GENERIC) {
    if (fcInt->fcOutputFormat != FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) {
      status = 1;
    }
  }

  if (fcInt->fcOutputFormat == FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) {
    if (fcInt->fcInputFormat != FDK_FORMAT_CONVERTER_INPUT_FORMAT_GENERIC) {
      status = 1;
    }
  }

  /* input format */
  switch (fcInt->fcInputFormat) {
    case FDK_FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS:
    case FDK_FORMAT_CONVERTER_INPUT_FORMAT_GENERIC:
      break;
    default:
      fcInt->fcInputFormat = FDK_FORMAT_CONVERTER_INPUT_FORMAT_INVALID;
      status = 1;
  }

  /* output format */
  switch (fcInt->fcOutputFormat) {
    case FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS:
    case FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC:
      break;
    default:
      fcInt->fcOutputFormat = FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID;
      status = 1;
  }

  /* sampling rate */
  if ((fcInt->samplingRate < 8000) || (fcInt->samplingRate > 384000)) {
    fcInt->samplingRate = 0;
    status = 1;
  }

  /**************************************************************************************/

  /* on error clean up so far allocated structs and return */
  if (status == 1) {
  bail:
    FDKfree(fcInt->fcParams);
    fcInt->fcParams = NULL;
    FDKfree(fcInt->fcState);
    fcInt->fcState = NULL;
    return 1;
  }

  /**************************************************************************************/

  /* buffer allocation */
  if (allocateFormatConverterParams(fcInt) != 0) {
    status = 1;
  }
  /**************************************************************************************/

  if (status != 0) {
    /* clean up */
    formatConverterClose(fcInt);
  }

  return status;
}

/**********************************************************************************************************************************/

INT formatConverterInit(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt,
                        const FIXP_DBL* centerFrequenciesNormalized, INT* p_buffer) {
  INT errorFlag;
  FDK_ASSERT(fcInt->fcParams != NULL);
  FDK_ASSERT(fcInt->fcState != NULL);

  /* set format converter parameters */
  errorFlag = setFormatConverterParams(centerFrequenciesNormalized, fcInt);
  if (errorFlag != 0) {
    return 1;
  }

  if (!fcInt->fcParams->genericIOFmt) {
    /* format converter init internal */
    errorFlag = formatConverterInit_internal(fcInt, p_buffer);
    if (errorFlag != 0) {
      return errorFlag;
    } else {
      fcInt->fcParams->dmxMtxIsSet = 1;
    }
  } else {
    fcInt->fcParams->dmxMtxIsSet = 0;
  }

  /* set format converter states */
  errorFlag = setFormatConverterState(fcInt);
  if (errorFlag != 0) {
    return -1;
  }

  return 0;
}

/**********************************************************************************************************************************/

void formatConverterClose(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  FDK_ASSERT(fcInt != NULL);

  if (fcInt->fcState != NULL) {
    /* free buffers */
    freeFormatConverterState(fcInt);
    FDKfree(fcInt->fcState);
    fcInt->fcState = NULL;
  }
  if (fcInt->fcParams != NULL) {
    freeFormatConverterParams(fcInt);
    FDKfree(fcInt->fcParams);
    fcInt->fcParams = NULL;
  }
}

/**********************************************************************************************************************************/
INT formatConverterPostprocessDmxMtx(FIXP_DMX_H** dmxMtx, const UINT numInputChans,
                                     const UINT numOutputChans) {
  UINT i, j;
  FIXP_DBL origEne, modEne, normFactor;
  INT origEne_e, modEne_e;
  FIXP_DMX_H maxGain;
  FIXP_DMX_H thr = FL2FXCONST_DMX_H(
      0.3f); /* threshold for setting DMX gains to zero: 0.3=-10.5dB (approx 3/4 panpot) */

  if (dmxMtx == NULL) return 1;

  /* post-process gains for each input channel individually */
  for (i = 0; i < numInputChans; i++) {
    INT e = 0;
    /* post-process only if there is at least one gain
       larger than the threshold for the current input channel */
    maxGain = (FIXP_DMX_H)0;
    for (j = 0; j < numOutputChans; j++)
      if (dmxMtx[i][j] > maxGain) maxGain = dmxMtx[i][j];
    if (maxGain <= thr) continue;

    /* calculate energy before post-processing */
    origEne = (FIXP_DBL)0;
    origEne_e = 0;
    for (j = 0; j < numOutputChans; j++) /* max output channels 24 */
    {
      origEne = fAddNorm(origEne, origEne_e, fPow2(dmxMtx[i][j]), 0, &origEne_e);
    }

    /* apply threshold */
    for (j = 0; j < numOutputChans; j++) { /* max output channels 24 */
      if (dmxMtx[i][j] <= thr) dmxMtx[i][j] = (FIXP_DMX_H)0;
    }

    /* calculate power after modification */
    modEne = (FIXP_DBL)0;
    modEne_e = 0;
    for (j = 0; j < numOutputChans; j++) {
      modEne = fAddNorm(modEne, modEne_e, fPow2(dmxMtx[i][j]), 0, &modEne_e);
    }

    /* normalize gains */
    e = 0;
    FIXP_DBL tmp = fDivNormHighPrec(origEne, modEne, &e);
    e = origEne_e - modEne_e + e;
    if (e & 1) {
      normFactor = sqrtFixp(tmp >> 1);
      e += 1;
      e >>= 1;
    } else {
      normFactor = sqrtFixp(tmp);
      e >>= 1;
    }
    for (j = 0; j < numOutputChans; j++) {
      dmxMtx[i][j] = FX_DBL2FX_DMX_H(scaleValue(fMult(normFactor, dmxMtx[i][j]), e));
    }
  }
  return 0;
}

/**********************************************************************************************************************************/

INT formatConverterAddDmxMtx(FIXP_DMX_H** dmxMtx, IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  UINT i, j, out;

  UINT numOutLfes = 0;
  INT lfeIndex = -1;

  cicp2geometry_get_number_of_lfes(fcInt->outChannelGeo, fcInt->numOutputChannels, &numOutLfes);

  if (dmxMtx == NULL) return 1;

  if (numOutLfes == 1) {
    for (out = 0; out < fcInt->numOutputChannels; out++) {
      if (fcInt->outChannelGeo[out].LFE) lfeIndex = out;
    }
  } /* End: ( numOutLfes == 1 ) */

  if (fcInt->fcParams->dmxMtxIsSet != 1) {
    return 1;
  }

  /* add external dmx matrix: element-wise addition
  NOTE: the number of input/output channels must NOT change!
  */
  for (i = 0; i < fcInt->numTotalInputChannels; i++) {
    for (j = 0; j < fcInt->numOutputChannels; j++) {
      if (lfeIndex != -1) {
        if (fcInt->inputChannelGeo[i].LFE) continue;
      }
      /* add update terms to current downmix matrix */
      fcInt->fcParams->dmxMtx[i][j] += dmxMtx[i][j];
    }
  }

  /* Resort the dmxMatrix in order to be applied in the downmix processing function.  */
  j = 0;
  UINT column, row;
  for (column = 0; column < fcInt->numTotalInputChannels; column++) {
    for (row = 0; row < fcInt->numOutputChannels; row++) {
      /* add update terms to sorted downmix matrix */
      fcInt->fcParams->dmxMtx_sorted[j++] = (fcInt->fcParams->dmxMtx[column][row]);
    }
  }

  return 0;
}

/**********************************************************************************************************************************/

INT formatConverterSetDmxMtx(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  UINT j;

  /* Resort the dmxMatrix in order to be applied in the downmix processing function.  */
  j = 0;
  UINT column, row;
  for (column = 0; column < fcInt->numTotalInputChannels; column++) {
    for (row = 0; row < fcInt->numOutputChannels; row++) {
      fcInt->fcParams->dmxMtx_sorted[j++] = (fcInt->fcParams->dmxMtx[column][row]);
    }
  }

  /* DMX matrix is valid now */
  fcInt->fcParams->dmxMtxIsSet = 1;

  /* internal EQs are switched off for external DMX matrix */
  fcInt->fcParams->applyEqFilters = 0;

  return 0;
}

void formatConverterAddParsedDmxMtx(FIXP_DMX_H* dmxMtxIn, FIXP_DMX_H* dmxMtxOut, INT numChannelsIn,
                                    INT numChannelsOut) {
  INT j = 0;
  INT column, row;
  for (column = 0; column < numChannelsIn; column++) {
    for (row = 0; row < numChannelsOut; row++) {
      dmxMtxOut[j] = dmxMtxIn[j];
      j++;
    }
  }
}

/**********************************************************************************************************************************/

INT formatConverterSetEQs(INT* eqIndex_sorted, UCHAR numEQs, eqParamsStruct* eqParams, UINT* eqMap,
                          UINT grp, UINT bsNumSignalGroups,
                          IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, FIXP_DBL* p_buffer) {
  INT i, k, m, n, err;
  /* Maximum number of EQs is the max input channels */
  FIXP_DBL(*eqGainsTmp)[58] = (FIXP_DBL(*)[58])p_buffer;
  p_buffer += (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS * 58);
  INT(*eqGainsTmp_e)[58] = (INT(*)[58])p_buffer;
  p_buffer = (FIXP_DBL*)((INT*)p_buffer + (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS * 58));

  if (numEQs > FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS) {
    err = -1;
    return err;
  }

  for (i = 0; i < (INT)fcInt->numInputChannels[grp]; i++) {
    if (eqMap[i] > (UINT)numEQs) {
      err = -1;
      return err;
    }
  }

  err = allocateFormatConverterEQs(fcInt, bsNumSignalGroups);
  if (err == -1) {
    return err;
  }

  /* calculate numEQs EQ responses from the numEQs eqParamsStructs */
  for (n = 0; n < (INT)numEQs; n++) {
    for (k = 0; k < (INT)fcInt->stftNumErbBands; k++) {
      FIXP_DBL tmp_sfreq_Hz = (FIXP_DBL)(fcInt->samplingRate);
      INT headroom = fNormz((FIXP_DBL)tmp_sfreq_Hz) - 1;
      tmp_sfreq_Hz <<= headroom;
      FIXP_DBL f = fMult(fAbs(fcInt->fcCenterFrequencies[k]), tmp_sfreq_Hz);
      INT f_e = 32 - headroom - 2;
      pkFilterParamsStruct pkParams;
      if (eqParams == NULL) continue;
      /* These EQ parameters were parsed and decoded in EqualizerConfig() */
      /* init with first peak filter and global gain G[dB] */
      pkParams = eqParams[n].pkFilterParams[0];
      eqGainsTmp_e[n][k] = 0;
      eqGainsTmp[n][k] =
          peak_filter(pkParams.f, pkParams.f_e, pkParams.q, pkParams.q_e, pkParams.g, pkParams.g_e,
                      eqParams[n].G, eqParams[n].G_e, f, f_e, &eqGainsTmp_e[n][k]);
      /* apply remaining filters of peak filter cascade with 0dB global gain */
      for (m = 1; m < eqParams[n].nPkFilter; m++) {
        pkParams = eqParams[n].pkFilterParams[m];
        eqGainsTmp[n][k] =
            fMult(eqGainsTmp[n][k],
                  (peak_filter(pkParams.f, pkParams.f_e, pkParams.q, pkParams.q_e, pkParams.g,
                               pkParams.g_e, 0, 0, f, f_e, &eqGainsTmp_e[n][k])));
      }
    }
  }

  /* apply EQs to freq. dep. dmx matrix. This freq. dep. matrix is calculated on the fly in the
   * active downmix processing. */
  /* The EQs from the bitstream parser are in Q29 format (exponent equals to 2), Meaning that 0.25
   * is actually 1. */

  for (m = 0; m < (INT)fcInt->numInputChannels[grp]; m++) {
    /* m is the input channel index for the current signal group equalizers */
    /* i is the input channel index for the total of equalizers taking into account every signal
     * group which parses the eq and dmx matrix info. */
    i = m + fcInt->amountOfAddedDmxMatricesAndEqualizers;
    INT eqIdx = eqMap[m];
    if (eqIdx > 0) { /* apply an EQ to this input channel */
      for (k = 0; k < (INT)fcInt->stftNumErbBands; k++) {
        fcInt->eqGains[i][k] = FX_DBL2FX_EQ_H(scaleValueSaturate(
            eqGainsTmp[eqIdx - 1][k], eqGainsTmp_e[eqIdx - 1][k] - EQ_BITSTREAM_H_EXP));
      }
      fcInt->eqGains_e[i] = EQ_BITSTREAM_H_EXP;
    } else { /* no EQ applied to this input channel - simply apply dmxMtx */
      for (k = 0; k < (INT)fcInt->stftNumErbBands; k++) {
        fcInt->eqGains[i][k] =
            FIXP_EQ_BITSTREAM_H_FORMAT_1_dot_0; /*fcInt->fcParams->dmxMtx[i][j];*/
      }
      fcInt->eqGains_e[i] = EQ_BITSTREAM_H_EXP;
    }
    for (n = 0; n < (INT)fcInt->numOutputChannels; n++) {
      /* EQ index for the activeDmxProcess_STFT function */
      eqIndex_sorted[m * fcInt->numOutputChannels + n] =
          i; /* taking into account the 20 equalizers which are stored by default as defined in
                activeDmxProcess_STFT. RULE_18_11 is the last eq for the immersive mode.  */
    }
  }

  return 0;
}

/**********************************************************************************************************************************/
INT formatConverterGetDelaySamples(IIS_FORMATCONVERTER_MODE mode) {
  INT delaySamples = -1;

  switch (mode) {
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
      delaySamples = 4096;
      break;
    case IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN:
    case IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
      delaySamples = 0;
      break;
    case IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
    case IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
      delaySamples = 256; /* same as HOPSIZE */
      break;
    default:
      break;
  }

  return delaySamples;
}

/**********************************************************************************************************************************/

INT setCustomDownmixParameters(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, INT adaptiveEQStrength,
                               INT phaseAlignStrength) {
  if (fcInt->mode == IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
    activeDmxSetAES(adaptiveEQStrength, fcInt->fcState->handleActiveDmxStft);
  }
  return 0;
}

/**********************************************************************************************************************************/
