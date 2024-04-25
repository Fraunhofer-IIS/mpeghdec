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
#include "drcGainDec_init.h"
#include "drcGainDec_process.h"
#include "drcDec_tools.h"

#if defined(__arm__)
#include "arm/drcDec_gainDecoder_arm.cpp"
#endif

/*******************************************/
/* static functions                        */
/*******************************************/

static void _setChannelGains(HANDLE_DRC_GAIN_DECODER hGainDec, const int numChannelGains,
                             const FIXP_DBL* channelGainDb) {
  int i, channelGain_e;
  FIXP_DBL channelGain;
  FDK_ASSERT(numChannelGains <= 28);
  for (i = 0; i < numChannelGains; i++) {
    if (channelGainDb[i] == (FIXP_DBL)MINVAL_DBL) {
      hGainDec->channelGain[i] = (FIXP_DBL)0;
    } else {
      /* add loudness normalisation gain (dB) to channel gain (dB) */
      FIXP_DBL tmp_channelGainDb =
          (channelGainDb[i] >> 1) + (hGainDec->loudnessNormalisationGainDb >> 2);
      tmp_channelGainDb = SATURATE_LEFT_SHIFT(tmp_channelGainDb, 1, DFRACT_BITS);
      channelGain = dB2lin(tmp_channelGainDb, 8, &channelGain_e);
      hGainDec->channelGain[i] = scaleValueSaturate(channelGain, channelGain_e - 8);
    }
  }
}

/*******************************************/
/* public functions                        */
/*******************************************/

DRC_ERROR
drcDec_GainDecoder_Open(HANDLE_DRC_GAIN_DECODER* phGainDec) {
  DRC_GAIN_DECODER* hGainDec = NULL;

  hGainDec = (DRC_GAIN_DECODER*)FDKcalloc(1, sizeof(DRC_GAIN_DECODER));
  if (hGainDec == NULL) return DE_MEMORY_ERROR;

  hGainDec->multiBandActiveDrcIndex = -1;
  hGainDec->channelGainActiveDrcIndex = -1;
  hGainDec->timeDomainSupported = 1;

  *phGainDec = hGainDec;

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Init(HANDLE_DRC_GAIN_DECODER hGainDec) {
  DRC_ERROR err = DE_OK;

  err = initGainDec(hGainDec);
  if (err) return err;

  initDrcGainBuffers(hGainDec->frameSize, &hGainDec->drcGainBuffers);

  return err;
}

DRC_ERROR
drcDec_GainDecoder_SetParam(HANDLE_DRC_GAIN_DECODER hGainDec, const GAIN_DEC_PARAM paramType,
                            const int paramValue) {
  switch (paramType) {
    case GAIN_DEC_FRAME_SIZE:
      if (paramValue < 0) return DE_PARAM_OUT_OF_RANGE;
      hGainDec->frameSize = paramValue;
      if (hGainDec->sampleRate) {
        hGainDec->msPerFrame = (hGainDec->frameSize * 1000) / hGainDec->sampleRate;
      }
      break;
    case GAIN_DEC_SAMPLE_RATE:
      if (paramValue < 0) return DE_PARAM_OUT_OF_RANGE;
      hGainDec->sampleRate = paramValue;
      if (hGainDec->sampleRate) {
        hGainDec->msPerFrame = (hGainDec->frameSize * 1000) / hGainDec->sampleRate;
      }
      hGainDec->deltaTminDefault = getDeltaTmin(hGainDec->sampleRate);
      break;
    default:
      return DE_PARAM_INVALID;
  }
  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_SetCodecDependentParameters(HANDLE_DRC_GAIN_DECODER hGainDec,
                                               const DELAY_MODE delayMode,
                                               const int timeDomainSupported,
                                               const SUBBAND_DOMAIN_MODE subbandDomainSupported) {
  if ((delayMode != DM_REGULAR_DELAY) && (delayMode != DM_LOW_DELAY)) {
    return DE_NOT_OK;
  }

  hGainDec->delayMode = delayMode;
  hGainDec->timeDomainSupported = timeDomainSupported;
  hGainDec->subbandDomainSupported = subbandDomainSupported;

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Config(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                          const UCHAR numSelectedDrcSets, const SCHAR* selectedDrcSetIds,
                          const UCHAR* selectedDownmixIds) {
  DRC_ERROR err = DE_OK;
  int a;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);

  hGainDec->nActiveDrcs[0] = 0;
  hGainDec->nActiveDrcs[1] = 0;
  hGainDec->multiBandActiveDrcIndex = -1;
  hGainDec->channelGainActiveDrcIndex = -1;

  if (pCoef) {
    hGainDec->drcCoef = *pCoef; /* keep deep copy of drcCoefficients */
  } else {
    FDKmemset(&hGainDec->drcCoef, 0, sizeof(DRC_COEFFICIENTS_UNI_DRC));
  }

  if (hGainDec->drcCoef.drcFrameSizePresent &&
      (hGainDec->drcCoef.drcFrameSize != hGainDec->frameSize)) {
    return DE_NOT_OK;
  }

  for (a = 0; a < numSelectedDrcSets; a++) {
    err = initActiveDrc(hGainDec, hUniDrcConfig, selectedDrcSetIds[a], selectedDownmixIds[a]);
    if (err) return err;
  }

  addVirtualToActiveDrc(hGainDec);

  err = initActiveDrcOffset(hGainDec);
  if (err) return err;

  return err;
}

DRC_ERROR
drcDec_GainDecoder_Close(HANDLE_DRC_GAIN_DECODER* phGainDec) {
  if (*phGainDec != NULL) {
    FDKfree(*phGainDec);
    *phGainDec = NULL;
  }

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Preprocess(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_GAIN hUniDrcGain,
                              const FIXP_DBL loudnessNormalizationGainDb, const FIXP_SGL boost,
                              const FIXP_SGL compress) {
  DRC_ERROR err = DE_OK;
  int a, c, l, s;

  /* lnbPointer is the index on the most recent node buffer */
  hGainDec->drcGainBuffers.lnbPointer++;
  if (hGainDec->drcGainBuffers.lnbPointer >= NUM_LNB_FRAMES)
    hGainDec->drcGainBuffers.lnbPointer = 0;

  for (l = 0; l < ACTIVE_DRC_LOCATIONS; l++) {
    for (a = 0; a < hGainDec->nActiveDrcs[l]; a++) {
      /* prepare gain interpolation of sequences used by copying and modifying nodes in node buffers
       */
      err =
          prepareDrcGain(hGainDec, hUniDrcGain, compress, boost, loudnessNormalizationGainDb, a, l);
      if (err) return err;
    }
  }

  for (l = 0; l < ACTIVE_DRC_LOCATIONS; l++) {
    for (a = 0; a < MAX_ACTIVE_DRCS; a++) {
      for (c = 0; c < 28; c++) {
        hGainDec->activeDrc[l][a].lnbIndexForChannel[c][hGainDec->drcGainBuffers.lnbPointer] =
            -1; /* "no DRC processing" */
      }
      hGainDec->activeDrc[l][a].subbandGainsReady = 0;
    }
  }

  for (s = 0; s < 4; s++) {
    hUniDrcGain->nDecodedSequences[s] = 0;
  }

  if (hGainDec->startupMs > 0) {
    hGainDec->startupMs -= hGainDec->msPerFrame;
  } else {
    hGainDec->startupMs = 0;
  }

  return err;
}

/* create gain sequence out of gain sequences of last frame for concealment and flushing */
DRC_ERROR
drcDec_GainDecoder_Conceal(HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                           HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  int seq, gainSequenceCount;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = &hGainDec->drcCoef;
  int s;
  if (pCoef && pCoef->gainSequenceCount) {
    gainSequenceCount = fMin(pCoef->gainSequenceCount, (UCHAR)48);
  } else {
    gainSequenceCount = 1;
  }

  for (seq = 0; seq < gainSequenceCount; seq++) {
    int lastNodeIndex = 0;
    FIXP_SGL lastGainDb = (FIXP_SGL)0;

    lastNodeIndex = hUniDrcGain->nNodes[seq] - 1;
    if ((lastNodeIndex >= 0) && (lastNodeIndex < 32)) {
      lastGainDb = hUniDrcGain->gainNode[seq][lastNodeIndex].gainDb;
    }

    hUniDrcGain->nNodes[seq] = 1;
    if (lastGainDb > (FIXP_SGL)0) {
      hUniDrcGain->gainNode[seq][0].gainDb = FX_DBL2FX_SGL(fMult(FL2FXCONST_SGL(0.9f), lastGainDb));
    } else {
      hUniDrcGain->gainNode[seq][0].gainDb =
          FX_DBL2FX_SGL(fMult(FL2FXCONST_SGL(0.98f), lastGainDb));
    }
    hUniDrcGain->gainNode[seq][0].time = hGainDec->frameSize - 1;
  }

  /* update nDecodedSequences for sanity check */
  hUniDrcGain->nDecodedSequences[0] = gainSequenceCount;
  for (s = 1; s < 4; s++) {
    hUniDrcGain->nDecodedSequences[s] = 0;
  }
  return DE_OK;
}

void drcDec_GainDecoder_SetChannelGains(HANDLE_DRC_GAIN_DECODER hGainDec, const int numChannels,
                                        const int frameSize, const FIXP_DBL* channelGainDb,
                                        const int audioBufferChannelOffset, FIXP_DBL* audioBuffer) {
  int c;

  if (hGainDec->channelGainActiveDrcIndex >= 0) {
    /* channel gains will be applied in drcDec_GainDecoder_ProcessTimeDomain or
     * drcDec_GainDecoder_ProcessSubbandDomain, respectively. */
    _setChannelGains(hGainDec, numChannels, channelGainDb);

    if (!hGainDec->status) { /* overwrite previous channel gains at startup */
      for (c = 0; c < numChannels; c++) {
        hGainDec->channelGainPrev[c] = hGainDec->channelGain[c];
      }
      hGainDec->status = 1;
    }
  } else {
    /* smooth and apply channel gains */
    FIXP_DBL prevChannelGain[28];
    for (c = 0; c < numChannels; c++) {
      prevChannelGain[c] = hGainDec->channelGain[c];
    }

    _setChannelGains(hGainDec, numChannels, channelGainDb);

    if (!hGainDec->status) { /* overwrite previous channel gains at startup */
      for (c = 0; c < numChannels; c++) prevChannelGain[c] = hGainDec->channelGain[c];
      hGainDec->status = 1;
    }

    for (c = 0; c < numChannels; c++) {
#ifndef FUNCTION_drcDec_GainDecoder_SetChannelGains_func1
      int i;
#endif
      INT n_min = fMin(
          fMin(CntLeadingZeros(prevChannelGain[c]), CntLeadingZeros(hGainDec->channelGain[c])) - 1,
          9);
      FIXP_DBL gain = prevChannelGain[c] << n_min;
      FIXP_DBL stepsize = ((hGainDec->channelGain[c] << n_min) - gain);

      if ((prevChannelGain[c] == FL2FXCONST_DBL(1.0f / (float)(1 << 8))) &&
          (hGainDec->channelGain[c] == FL2FXCONST_DBL(1.0f / (float)(1 << 8)))) {
        /* gain of 1.0 throughout the section. Skip processing to keep bit-identical signal. */
        continue;
      }

      if (stepsize != (FIXP_DBL)0) {
        if (frameSize == 1024)
          stepsize = stepsize >> 10;
        else
          stepsize = (LONG)stepsize / frameSize;
      }
      n_min = 9 - n_min;
#ifdef FUNCTION_drcDec_GainDecoder_SetChannelGains_func1
      drcDec_GainDecoder_SetChannelGains_func1(audioBuffer, gain, stepsize, n_min, frameSize);
#else
      for (i = 0; i < frameSize; i++) {
        audioBuffer[i] = fMultDiv2(audioBuffer[i], gain) << n_min;
        gain += stepsize;
      }
#endif
      audioBuffer += audioBufferChannelOffset;
    }
  }
}

DRC_ERROR
drcDec_GainDecoder_ProcessTimeDomain(HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples,
                                     const GAIN_DEC_LOCATION drcLocation, const int channelOffset,
                                     const int drcChannelOffset, const int numChannelsProcessed,
                                     const int timeDataChannelOffset, FIXP_DBL* audioIOBuffer) {
  DRC_ERROR err = DE_OK;
  int a;

  if (!hGainDec->timeDomainSupported) {
    return DE_NOT_OK;
  }

  if (!(drcLocation == GAIN_DEC_DRC1 || drcLocation == GAIN_DEC_DRC2_DRC3)) {
    return DE_NOT_OK;
  }

  for (a = 0; a < hGainDec->nActiveDrcs[drcLocation]; a++) {
    /* Apply DRC */
    err =
        processDrcTime(hGainDec, a, (int)drcLocation, delaySamples, channelOffset, drcChannelOffset,
                       numChannelsProcessed, timeDataChannelOffset, audioIOBuffer);
    if (err) return err;
  }

  return err;
}

DRC_ERROR
drcDec_GainDecoder_ProcessSubbandDomain(
    HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples, const GAIN_DEC_LOCATION drcLocation,
    const int channelOffset, const int drcChannelOffset, const int numChannelsProcessed,
    const int processSingleTimeslot, FIXP_DBL* audioIOBufferReal[], FIXP_DBL* audioIOBufferImag[]) {
  DRC_ERROR err = DE_OK;
  int a;

  if (hGainDec->subbandDomainSupported == SDM_OFF) {
    return DE_NOT_OK;
  }

  if (!(drcLocation == GAIN_DEC_DRC1 || drcLocation == GAIN_DEC_DRC2_DRC3)) {
    return DE_NOT_OK;
  }

  for (a = 0; a < hGainDec->nActiveDrcs[drcLocation]; a++) {
    /* Apply DRC */
    err = processDrcSubband(hGainDec, a, (int)drcLocation, delaySamples, channelOffset,
                            drcChannelOffset, numChannelsProcessed, processSingleTimeslot,
                            audioIOBufferReal, audioIOBufferImag);
    if (err) return err;
  }

  return err;
}

DRC_ERROR
drcDec_GainDecoder_SetLoudnessNormalizationGainDb(HANDLE_DRC_GAIN_DECODER hGainDec,
                                                  FIXP_DBL loudnessNormalizationGainDb) {
  hGainDec->loudnessNormalisationGainDb = loudnessNormalizationGainDb;

  return DE_OK;
}

int drcDec_GainDecoder_GetFrameSize(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return -1;

  return hGainDec->frameSize;
}

int drcDec_GainDecoder_GetDeltaTminDefault(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return -1;

  return hGainDec->deltaTminDefault;
}

int drcDec_GainDecoder_GetStartupPhase(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return -1;

  if (hGainDec->startupMs > hGainDec->msPerFrame)
    return 2;                            /* decoder is in 2.5 seconds startup phase */
  if (hGainDec->startupMs > 0) return 1; /* decoder is in last frame of startup phase */
  return 0;                              /* startup phase is over */
}

DRC_COEFFICIENTS_UNI_DRC* drcDec_GainDecoder_GetDrcCoefficients(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return NULL;

  return &hGainDec->drcCoef;
}
