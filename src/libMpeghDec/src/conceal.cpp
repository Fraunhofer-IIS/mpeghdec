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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Josef Hoepfl

   Description: independent channel concealment

*******************************************************************************/

/*!
  \page concealment AAC core concealment

  This AAC core implementation includes a concealment function, which can be enabled
  using the several defines during compilation.

  There are various tests inside the core, starting with simple CRC tests and ending in
  a variety of plausibility checks. If such a check indicates an invalid bitstream, then
  concealment is applied.

  Concealment is also applied when the calling main program indicates a distorted or missing
  data frame using the frameOK flag. This is used for error detection on the transport layer.
  (See below)

  There are three concealment-modes:

  1) Muting: The spectral data is simply set to zero in case of an detected error.

  2) Noise substitution: In case of an detected error, concealment copies the last frame and adds
     attenuates the spectral data. For this mode you have to set the #CONCEAL_NOISE define.
     Noise substitution adds no additional delay.

  3) Interpolation: The interpolation routine swaps the spectral data from the previous and the
     current frame just before the final frequency to time conversion. In case a single frame is
     corrupted, concealmant interpolates between the last good and the first good frame to create
     the spectral data for the missing frame. If multiple frames are corrupted, concealment
     implements first a fade out based on slightly modified spectral values from the last good
     frame. As soon as good frames are available, concealmant fades in the new spectral data.
     For this mode you have to set the #CONCEAL_INTER define. Note that in this case, you also
     need to set #SBR_BS_DELAY_ENABLE, which basically adds approriate delay in the SBR decoder.
     Note that the Interpolating-Concealment increases the delay of your decoder by one frame
     and that it does require additional resources such as memory and computational complexity.

  <h2>How concealment can be used with errors on the transport layer</h2>

  Many errors can or have to be detected on the transport layer. For example in IP based systems
  packet loss can occur. The transport protocol used should indicate such packet loss by inserting
  an empty frame with frameOK=0.
*/

#include "conceal.h"

#include "aac_rom.h"
#include "genericStds.h"

#include "block.h"

#define CONCEAL_DFLT_COMF_NOISE_LEVEL (0x100000)

#define CONCEAL_NOT_DEFINED ((UCHAR)-1)

/* default settings */
#define CONCEAL_DFLT_FADEOUT_FRAMES (6)
#define CONCEAL_DFLT_FADEIN_FRAMES (5)
#define CONCEAL_DFLT_MUTE_RELEASE_FRAMES (0)

#define CONCEAL_DFLT_FADE_FACTOR (0.707106781186548f) /* 1/sqrt(2) */

/* some often used constants: */
#define FIXP_ZERO FL2FXCONST_DBL(0.0f)
#define FIXP_ONE FL2FXCONST_DBL(1.0f)
#define FIXP_FL_CORRECTION FL2FXCONST_DBL(0.53333333333333333f)

/* For parameter conversion */
#define CONCEAL_PARAMETER_BITS (8)
#define CONCEAL_MAX_QUANT_FACTOR ((1 << CONCEAL_PARAMETER_BITS) - 1)
/*#define CONCEAL_MIN_ATTENUATION_FACTOR_025  ( FL2FXCONST_DBL(0.971627951577106174) )*/ /* -0.25 dB
                                                                                          */
#define CONCEAL_MIN_ATTENUATION_FACTOR_025_LD FL2FXCONST_DBL(-0.041524101186092029596853445212299)
/*#define CONCEAL_MIN_ATTENUATION_FACTOR_050  ( FL2FXCONST_DBL(0.944060876285923380) )*/ /* -0.50 dB
                                                                                          */
#define CONCEAL_MIN_ATTENUATION_FACTOR_050_LD FL2FXCONST_DBL(-0.083048202372184059253597008145293)

static int CConcealment_ApplyNoise(CConcealmentInfo* pConcealmentInfo,
                                   CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                   CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                   const SamplingRateInfo* pSamplingRateInfo,
                                   const int samplesPerFrame, const UINT flags);

static void CConcealment_UpdateState(CConcealmentInfo* pConcealmentInfo, int frameOk,
                                     CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                     const int samplesPerFrame,
                                     CAacDecoderChannelInfo* pAacDecoderChannelInfo);

static void CConcealment_ApplyRandomSign(int iRandomPhase, FIXP_DBL* spec, int samplesPerFrame);

/* TimeDomainFading */
static void CConcealment_TDFadePcmAtt(int start, int len, FIXP_DBL fadeStart, FIXP_DBL fadeStop,
                                      PCM_DEC* pcmdata);
static void CConcealment_TDFadeFillFadingStations(FIXP_DBL* fadingStations, int* fadingSteps,
                                                  FIXP_DBL fadeStop, FIXP_DBL fadeStart,
                                                  TDfadingType fadingType);
static void CConcealment_TDFading_doLinearFadingSteps(int* fadingSteps);

/* Streamline the state machine */
static int CConcealment_ApplyFadeOut(int mode, CConcealmentInfo* pConcealmentInfo,
                                     CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                     const int samplesPerFrame,
                                     CAacDecoderChannelInfo* pAacDecoderChannelInfo);

static int CConcealment_TDNoise_Random(ULONG* seed);
static void CConcealment_TDNoise_Apply(CConcealmentInfo* const pConcealmentInfo, const int len,
                                       const INT aacOutDataHeadroom, PCM_DEC* const pcmdata);

static BLOCK_TYPE CConcealment_GetWinSeq(int prevWinSeq) {
  BLOCK_TYPE newWinSeq = BLOCK_LONG;

  /* Try to have only long blocks */
  if (prevWinSeq == BLOCK_START || prevWinSeq == BLOCK_SHORT) {
    newWinSeq = BLOCK_STOP;
  }

  return (newWinSeq);
}

/*!
  \brief Init common concealment information data

  \param pConcealCommonData Pointer to the concealment common data structure.
*/
void CConcealment_InitCommonData(CConcealParams* pConcealCommonData) {
  if (pConcealCommonData != NULL) {
    int i;

    /* Set default error concealment technique */
    pConcealCommonData->method = ConcealMethodNoise;

    pConcealCommonData->numFadeOutFrames = CONCEAL_DFLT_FADEOUT_FRAMES;
    pConcealCommonData->numFadeInFrames = CONCEAL_DFLT_FADEIN_FRAMES;
    pConcealCommonData->numMuteReleaseFrames = CONCEAL_DFLT_MUTE_RELEASE_FRAMES;

    pConcealCommonData->comfortNoiseLevel = (FIXP_DBL)CONCEAL_DFLT_COMF_NOISE_LEVEL;

    /* Init fade factors (symetric) */
    pConcealCommonData->fadeOutFactor[0] = FL2FXCONST_SGL(CONCEAL_DFLT_FADE_FACTOR);
    pConcealCommonData->fadeInFactor[0] = pConcealCommonData->fadeOutFactor[0];

    for (i = 1; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
      pConcealCommonData->fadeOutFactor[i] = FX_DBL2FX_SGL(fMult(
          pConcealCommonData->fadeOutFactor[i - 1], FL2FXCONST_SGL(CONCEAL_DFLT_FADE_FACTOR)));
      pConcealCommonData->fadeInFactor[i] = pConcealCommonData->fadeOutFactor[i];
    }
  }
}

/*!
  \brief Get current concealment method.

  \param pConcealCommonData Pointer to common concealment data (for all channels)
*/
CConcealmentMethod CConcealment_GetMethod(CConcealParams* pConcealCommonData) {
  CConcealmentMethod method = ConcealMethodNone;

  if (pConcealCommonData != NULL) {
    method = pConcealCommonData->method;
  }

  return (method);
}

/*!
  \brief Init concealment information for each channel

  \param pConcealChannelInfo Pointer to the channel related concealment info structure to be
  initialized. \param pConcealCommonData  Pointer to common concealment data (for all channels)
  \param initRenderMode      Initial render mode to be set for the current channel.
  \param samplesPerFrame     The number of samples per frame.
*/
void CConcealment_InitChannelData(CConcealmentInfo* pConcealChannelInfo,
                                  CConcealParams* pConcealCommonData,
                                  AACDEC_RENDER_MODE initRenderMode, int samplesPerFrame) {
  int i;
  pConcealChannelInfo->TDNoiseSeed = 0;
  FDKmemclear(pConcealChannelInfo->TDNoiseStates, sizeof(pConcealChannelInfo->TDNoiseStates));
  pConcealChannelInfo->TDNoiseCoef[0] = FL2FXCONST_SGL(0.05f);
  pConcealChannelInfo->TDNoiseCoef[1] = FL2FXCONST_SGL(0.5f);
  pConcealChannelInfo->TDNoiseCoef[2] = FL2FXCONST_SGL(0.45f);

  pConcealChannelInfo->pConcealParams = pConcealCommonData;

  pConcealChannelInfo->lastRenderMode = initRenderMode;

  pConcealChannelInfo->windowShape = CONCEAL_NOT_DEFINED;
  pConcealChannelInfo->windowSequence = BLOCK_LONG; /* default type */
  pConcealChannelInfo->lastWinGrpLen = 1;

  pConcealChannelInfo->concealState = ConcealState_Ok;

  FDKmemclear(pConcealChannelInfo->spectralCoefficient, 1024 * sizeof(FIXP_CNCL));

  for (i = 0; i < 8; i++) {
    pConcealChannelInfo->specScale[i] = 0;
  }

  pConcealChannelInfo->iRandomPhase = 0;

  pConcealChannelInfo->prevFrameOk[0] = 1;
  pConcealChannelInfo->prevFrameOk[1] = 1;

  pConcealChannelInfo->cntFadeFrames = 0;
  pConcealChannelInfo->cntValidFrames = 0;
  pConcealChannelInfo->fade_old = (FIXP_DBL)MAXVAL_DBL;
  pConcealChannelInfo->winGrpOffset[0] = 0;
  pConcealChannelInfo->winGrpOffset[1] = 0;
  pConcealChannelInfo->attGrpOffset[0] = 0;
  pConcealChannelInfo->attGrpOffset[1] = 0;
}

/*!
  \brief Set error concealment parameters

  \param concealParams
  \param method
  \param fadeOutSlope
  \param fadeInSlope
  \param muteRelease
  \param comfNoiseLevel
*/
AAC_DECODER_ERROR
CConcealment_SetParams(CConcealParams* concealParams, int method, int fadeOutSlope, int fadeInSlope,
                       int muteRelease, FIXP_DBL comfNoiseLevel) {
  /* set concealment technique */
  if (method != AACDEC_CONCEAL_PARAM_NOT_SPECIFIED) {
    switch ((CConcealmentMethod)method) {
      case ConcealMethodMute:
      case ConcealMethodNoise:
        if (concealParams == NULL) {
          return AAC_DEC_INVALID_HANDLE;
        } else {
          /* set param */
          concealParams->method = (CConcealmentMethod)method;
        }
        break;

      default:
        return AAC_DEC_SET_PARAM_FAIL;
    }
  }

  /* set number of frames for fade-out slope */
  if (fadeOutSlope != AACDEC_CONCEAL_PARAM_NOT_SPECIFIED) {
    if ((fadeOutSlope < CONCEAL_MAX_NUM_FADE_FACTORS) && (fadeOutSlope >= 0)) {
      if (concealParams == NULL) {
        return AAC_DEC_INVALID_HANDLE;
      } else {
        /* set param */
        concealParams->numFadeOutFrames = fadeOutSlope;
      }
    } else {
      return AAC_DEC_SET_PARAM_FAIL;
    }
  }

  /* set number of frames for fade-in slope */
  if (fadeInSlope != AACDEC_CONCEAL_PARAM_NOT_SPECIFIED) {
    if ((fadeInSlope < CONCEAL_MAX_NUM_FADE_FACTORS) && (fadeInSlope >= 0)) {
      if (concealParams == NULL) {
        return AAC_DEC_INVALID_HANDLE;
      } else {
        /* set param */
        concealParams->numFadeInFrames = fadeInSlope;
      }
    } else {
      return AAC_DEC_SET_PARAM_FAIL;
    }
  }

  /* set number of error-free frames after which the muting will be released */
  if (muteRelease != AACDEC_CONCEAL_PARAM_NOT_SPECIFIED) {
    if ((muteRelease < (CONCEAL_MAX_NUM_FADE_FACTORS << 1)) && (muteRelease >= 0)) {
      if (concealParams == NULL) {
        return AAC_DEC_INVALID_HANDLE;
      } else {
        /* set param */
        concealParams->numMuteReleaseFrames = muteRelease;
      }
    } else {
      return AAC_DEC_SET_PARAM_FAIL;
    }
  }

  /* set confort noise level which will be inserted while in state 'muting' */
  if (comfNoiseLevel != (FIXP_DBL)AACDEC_CONCEAL_PARAM_NOT_SPECIFIED) {
    if ((comfNoiseLevel < (FIXP_DBL)0)) {
      return AAC_DEC_SET_PARAM_FAIL;
    }
    if (concealParams == NULL) {
      return AAC_DEC_INVALID_HANDLE;
    } else {
      concealParams->comfortNoiseLevel = (FIXP_DBL)comfNoiseLevel;
    }
  }

  return (AAC_DEC_OK);
}

/*!
  \brief Set fade-out/in attenuation factor vectors

  \param concealParams
  \param fadeOutAttenuationVector
  \param fadeInAttenuationVector

  \return 0 if OK all other values indicate errors
*/
AAC_DECODER_ERROR
CConcealment_SetAttenuation(CConcealParams* concealParams, const SHORT* fadeOutAttenuationVector,
                            const SHORT* fadeInAttenuationVector) {
  if ((fadeOutAttenuationVector == NULL) && (fadeInAttenuationVector == NULL)) {
    return AAC_DEC_SET_PARAM_FAIL;
  }

  /* Fade-out factors */
  if (fadeOutAttenuationVector != NULL) {
    int i;

    /* check quantized factors first */
    for (i = 0; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
      if ((fadeOutAttenuationVector[i] < 0) ||
          (fadeOutAttenuationVector[i] > CONCEAL_MAX_QUANT_FACTOR)) {
        return AAC_DEC_SET_PARAM_FAIL;
      }
    }
    if (concealParams == NULL) {
      return AAC_DEC_INVALID_HANDLE;
    }

    /* now dequantize factors */
    for (i = 0; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
      concealParams->fadeOutFactor[i] = FX_DBL2FX_SGL(
          fLdPow(CONCEAL_MIN_ATTENUATION_FACTOR_025_LD, 0,
                 (FIXP_DBL)((INT)(FL2FXCONST_DBL(1.0 / 2.0) >> (CONCEAL_PARAMETER_BITS - 1)) *
                            (INT)fadeOutAttenuationVector[i]),
                 CONCEAL_PARAMETER_BITS));
    }
  }

  /* Fade-in factors */
  if (fadeInAttenuationVector != NULL) {
    int i;

    /* check quantized factors first */
    for (i = 0; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
      if ((fadeInAttenuationVector[i] < 0) ||
          (fadeInAttenuationVector[i] > CONCEAL_MAX_QUANT_FACTOR)) {
        return AAC_DEC_SET_PARAM_FAIL;
      }
    }
    if (concealParams == NULL) {
      return AAC_DEC_INVALID_HANDLE;
    }

    /* now dequantize factors */
    for (i = 0; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
      concealParams->fadeInFactor[i] = FX_DBL2FX_SGL(fLdPow(
          CONCEAL_MIN_ATTENUATION_FACTOR_025_LD, 0,
          (FIXP_DBL)((INT)(FIXP_ONE >> CONCEAL_PARAMETER_BITS) * (INT)fadeInAttenuationVector[i]),
          CONCEAL_PARAMETER_BITS));
    }
  }

  return (AAC_DEC_OK);
}

/*!
  \brief Get state of concealment module.

  \param pConcealChannelInfo

  \return Concealment state.
*/
CConcealmentState CConcealment_GetState(CConcealmentInfo* pConcealChannelInfo) {
  CConcealmentState state = ConcealState_Ok;

  if (pConcealChannelInfo != NULL) {
    state = pConcealChannelInfo->concealState;
  }

  return (state);
}

/*!
  \brief Store data for concealment techniques applied later

  Interface function to store data for different concealment strategies
 */
void CConcealment_Store(CConcealmentInfo* hConcealmentInfo,
                        CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                        CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo) {
  if (!(pAacDecoderChannelInfo->renderMode == AACDEC_RENDER_LPD))

  {
    FIXP_DBL* pSpectralCoefficient = SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);
    SHORT* pSpecScale = pAacDecoderChannelInfo->specScale;

    /* store new scale factors */
    FDKmemcpy(hConcealmentInfo->specScale, pSpecScale, 8 * sizeof(SHORT));

    C_ALLOC_ALIGNED_CHECK(pSpectralCoefficient);
    C_ALLOC_ALIGNED_CHECK(hConcealmentInfo->spectralCoefficient);

    if (hConcealmentInfo->pConcealParams->method < ConcealMethodInter) {
      /* store new spectral bins */
#if (CNCL_FRACT_BITS == DFRACT_BITS)
      FDKmemcpy(hConcealmentInfo->spectralCoefficient, pSpectralCoefficient,
                1024 * sizeof(FIXP_CNCL));
#else
      FIXP_CNCL* RESTRICT pCncl = &hConcealmentInfo->spectralCoefficient[1024 - 1];
      FIXP_DBL* RESTRICT pSpec = &pSpectralCoefficient[1024 - 1];
      int i;
      for (i = 1024; i != 0; i--) {
        *pCncl-- = FX_DBL2FX_CNCL(*pSpec--);
      }
#endif
    }
  }

  if (pAacDecoderChannelInfo->renderMode != AACDEC_RENDER_LPD) {
    CIcsInfo* pIcsInfo = &pAacDecoderChannelInfo->icsInfo;

    /* store new window infos */
    hConcealmentInfo->windowSequence = GetWindowSequence(pIcsInfo);
    hConcealmentInfo->windowShape = GetWindowShape(pIcsInfo);
    hConcealmentInfo->lastWindowGroups = GetWindowGroups(pIcsInfo);
    FDKmemcpy(hConcealmentInfo->lastWindowGroupLength, GetWindowGroupLengthTable(pIcsInfo),
              sizeof(UCHAR) * 8);
    hConcealmentInfo->lastWinGrpLen =
        *(GetWindowGroupLengthTable(pIcsInfo) + GetWindowGroups(pIcsInfo) - 1);
  }
}

/*!
  \brief Apply concealment

  Interface function to different concealment strategies
 */
int CConcealment_Apply(CConcealmentInfo* hConcealmentInfo,
                       CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                       CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                       const SamplingRateInfo* pSamplingRateInfo, const int samplesPerFrame,
                       const UCHAR lastLpdMode, const int frameOk, const UINT flags) {
  int appliedProcessing = 0;
  const int mute_release_active = frameOk &&
                                  (hConcealmentInfo->concealState >= ConcealState_Mute) &&
                                  (hConcealmentInfo->cntValidFrames + 1 <=
                                   hConcealmentInfo->pConcealParams->numMuteReleaseFrames);

  if (hConcealmentInfo->windowShape == CONCEAL_NOT_DEFINED) {
    /* Initialize window_shape with same value as in the current (parsed) frame.
       Because section 4.6.11.3.2 (Windowing and block switching) of ISO/IEC 14496-3:2009 says:
       For the first raw_data_block() to be decoded the window_shape of the left and right
       half of the window are identical. */
    hConcealmentInfo->windowShape = pAacDecoderChannelInfo->icsInfo.WindowShape;
  }

  if (frameOk && !mute_release_active) {
    /* Update render mode if frameOk except for ongoing mute release state. */
    hConcealmentInfo->lastRenderMode = (SCHAR)pAacDecoderChannelInfo->renderMode;

    /* Rescue current data for concealment in future frames */
    CConcealment_Store(hConcealmentInfo, pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo);
    /* Reset index to random sign vector to make sign calculation frame agnostic
       (only depends on number of subsequently concealed spectral blocks) */
    hConcealmentInfo->iRandomPhase = 0;
  } else {
    if (hConcealmentInfo->lastRenderMode == AACDEC_RENDER_INVALID) {
      hConcealmentInfo->lastRenderMode = AACDEC_RENDER_IMDCT;
    }
    pAacDecoderChannelInfo->renderMode = (AACDEC_RENDER_MODE)hConcealmentInfo->lastRenderMode;
  }

  /* hand current frame status to the state machine */
  CConcealment_UpdateState(hConcealmentInfo, frameOk, pAacDecoderStaticChannelInfo, samplesPerFrame,
                           pAacDecoderChannelInfo);

  {
    /* Create data for signal rendering according to the selected concealment method and decoder
     * operating mode. */

    if (!(pAacDecoderChannelInfo->renderMode == AACDEC_RENDER_LPD)) {
      switch (hConcealmentInfo->pConcealParams->method) {
        default:
        case ConcealMethodMute:
          if (!frameOk) {
            /* Mute spectral data in case of errors */
            FDKmemclear(pAacDecoderChannelInfo->pSpectralCoefficient,
                        samplesPerFrame * sizeof(FIXP_DBL));
            /* Set last window shape */
            pAacDecoderChannelInfo->icsInfo.WindowShape = hConcealmentInfo->windowShape;
            appliedProcessing = 1;
          }
          break;

        case ConcealMethodNoise:
          /* Noise substitution error concealment technique */
          appliedProcessing = CConcealment_ApplyNoise(hConcealmentInfo, pAacDecoderChannelInfo,
                                                      pAacDecoderStaticChannelInfo,
                                                      pSamplingRateInfo, samplesPerFrame, flags);
          break;
      }
    } else if (!frameOk || mute_release_active) {
      /* simply restore the buffer */
      FIXP_DBL* pSpectralCoefficient = SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);
      SHORT* pSpecScale = pAacDecoderChannelInfo->specScale;
      CIcsInfo* pIcsInfo = &pAacDecoderChannelInfo->icsInfo;
#if (CNCL_FRACT_BITS != DFRACT_BITS)
      FIXP_CNCL* RESTRICT pCncl = &hConcealmentInfo->spectralCoefficient[1024 - 1];
      FIXP_DBL* RESTRICT pSpec = &pSpectralCoefficient[1024 - 1];
      int i;
#endif

      pIcsInfo->WindowSequence = hConcealmentInfo->windowSequence;
      pIcsInfo->WindowShape = hConcealmentInfo->windowShape;

      if (hConcealmentInfo->concealState != ConcealState_Mute) {
        /* restore scale factors */
        FDKmemcpy(pSpecScale, hConcealmentInfo->specScale, 8 * sizeof(SHORT));

        /* restore spectral bins */
#if (CNCL_FRACT_BITS == DFRACT_BITS)
        FDKmemcpy(pSpectralCoefficient, hConcealmentInfo->spectralCoefficient,
                  1024 * sizeof(FIXP_DBL));
#else
        for (i = 1024; i != 0; i--) {
          *pSpec-- = FX_CNCL2FX_DBL(*pCncl--);
        }
#endif
      } else {
        /* clear scale factors */
        FDKmemclear(pSpecScale, 8 * sizeof(SHORT));

        /* clear buffer */
        FDKmemclear(pSpectralCoefficient, 1024 * sizeof(FIXP_CNCL));
      }
    }
  }
  /* update history */
  hConcealmentInfo->prevFrameOk[0] = hConcealmentInfo->prevFrameOk[1];
  hConcealmentInfo->prevFrameOk[1] = frameOk;

  return mute_release_active ? -1 : appliedProcessing;
}

/*!
\brief Apply concealment noise substitution

  In case of frame lost this function produces a noisy frame with respect to the
  energies values of past frame.
 */
static int CConcealment_ApplyNoise(CConcealmentInfo* pConcealmentInfo,
                                   CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                   CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                   const SamplingRateInfo* pSamplingRateInfo,
                                   const int samplesPerFrame, const UINT flags) {
  FIXP_DBL* pSpectralCoefficient = SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);
  CIcsInfo* pIcsInfo = &pAacDecoderChannelInfo->icsInfo;

  int appliedProcessing = 0;

  FDK_ASSERT(pConcealmentInfo != NULL);
  FDK_ASSERT((samplesPerFrame >= 120) && (samplesPerFrame <= 1024));

  switch (pConcealmentInfo->concealState) {
    case ConcealState_Ok:
      /* Nothing to do here! */
      break;

    case ConcealState_Single:
    case ConcealState_FadeOut:
      appliedProcessing =
          CConcealment_ApplyFadeOut(/*mode =*/1, pConcealmentInfo, pAacDecoderStaticChannelInfo,
                                    samplesPerFrame, pAacDecoderChannelInfo);
      break;

    case ConcealState_Mute: {
      /* set dummy window parameters */
      pIcsInfo->Valid = 0; /* Trigger the generation of a consitent IcsInfo */
      pIcsInfo->WindowShape =
          pConcealmentInfo
              ->windowShape; /* Prevent an invalid WindowShape (required for F/T transform) */
      pIcsInfo->WindowSequence = CConcealment_GetWinSeq(pConcealmentInfo->windowSequence);
      pConcealmentInfo->windowSequence =
          pIcsInfo->WindowSequence; /* Store for next frame (spectrum in concealment buffer can't be
                                       used at all) */
      pConcealmentInfo->winGrpOffset[0] = 0;
      pConcealmentInfo->winGrpOffset[1] = 0;

      /* mute spectral data */
      FDKmemclear(pSpectralCoefficient, samplesPerFrame * sizeof(FIXP_DBL));
      FDKmemclear(pConcealmentInfo->spectralCoefficient, samplesPerFrame * sizeof(FIXP_DBL));

      appliedProcessing = 1;
    } break;

    case ConcealState_FadeIn: {
      /* TimeDomainFading:                                        */
      /* Attenuation of signal is done in CConcealment_TDFading() */

      appliedProcessing = 1;
    } break;

    default:
      /* we shouldn't come here anyway */
      FDK_ASSERT(0);
      break;
  }

  return appliedProcessing;
}

/*!
  \brief Find next fading frame in case of changing fading direction

  \param pConcealCommonData Pointer to the concealment common data structure.
  \param actFadeIndex Last index used for fading
  \param direction Direction of change: 0 : change from FADE-OUT to FADE-IN,  1 : change from
  FADE-IN to FADE-OUT

  This function determines the next fading index to be used for the fading direction to be changed
  to.
*/

static INT findEquiFadeFrame(CConcealParams* pConcealCommonData, INT actFadeIndex, int direction) {
  FIXP_SGL* pFactor;
  FIXP_SGL referenceVal;
  FIXP_SGL minDiff = (FIXP_SGL)MAXVAL_SGL;

  INT nextFadeIndex = 0;

  int i;

  /* init depending on direction */
  if (direction == 0) { /* FADE-OUT => FADE-IN */
    if (actFadeIndex < 0) {
      referenceVal = (FIXP_SGL)MAXVAL_SGL;
    } else {
      referenceVal = pConcealCommonData->fadeOutFactor[actFadeIndex] >> 1;
    }
    pFactor = pConcealCommonData->fadeInFactor;
  } else { /* FADE-IN => FADE-OUT */
    if (actFadeIndex < 0) {
      referenceVal = (FIXP_SGL)MAXVAL_SGL;
    } else {
      referenceVal = pConcealCommonData->fadeInFactor[actFadeIndex] >> 1;
    }
    pFactor = pConcealCommonData->fadeOutFactor;
  }

  /* search for minimum difference */
  for (i = 0; i < CONCEAL_MAX_NUM_FADE_FACTORS; i++) {
    FIXP_SGL diff = fixp_abs((pFactor[i] >> 1) - referenceVal);
    if (diff < minDiff) {
      minDiff = diff;
      nextFadeIndex = i;
    }
  }

  /* check and adjust depending on direction */
  if (direction == 0) { /* FADE-OUT => FADE-IN */
    if (((pFactor[nextFadeIndex] >> 1) <= referenceVal) && (nextFadeIndex > 0)) {
      nextFadeIndex -= 1;
    }
    if (nextFadeIndex > pConcealCommonData->numFadeInFrames - 1) {
      nextFadeIndex = fMax(pConcealCommonData->numFadeInFrames - 1, 0);
    }
  } else { /* FADE-IN => FADE-OUT */
    if (((pFactor[nextFadeIndex] >> 1) >= referenceVal) &&
        (nextFadeIndex < CONCEAL_MAX_NUM_FADE_FACTORS - 1)) {
      nextFadeIndex += 1;
    }
    if (nextFadeIndex > pConcealCommonData->numFadeOutFrames - 1) {
      nextFadeIndex = fMax(pConcealCommonData->numFadeOutFrames - 1, 0);
    }
  }

  return (nextFadeIndex);
}

/*!
  \brief Update the concealment state

  The function updates the state of the concealment state-machine. The
  states are: mute, fade-in, fade-out, interpolate and frame-ok.
*/
static void CConcealment_UpdateState(CConcealmentInfo* pConcealmentInfo, int frameOk,
                                     CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                     const int samplesPerFrame,
                                     CAacDecoderChannelInfo* pAacDecoderChannelInfo) {
  CConcealParams* pConcealCommonData = pConcealmentInfo->pConcealParams;

  switch (pConcealCommonData->method) {
    case ConcealMethodNoise: {
      if (pConcealmentInfo->concealState != ConcealState_Ok) {
        /* count the valid frames during concealment process */
        if (frameOk) {
          pConcealmentInfo->cntValidFrames += 1;
        } else {
          pConcealmentInfo->cntValidFrames = 0;
        }
      }

      /* -- STATE MACHINE for Noise Substitution -- */
      switch (pConcealmentInfo->concealState) {
        case ConcealState_Ok:
          if (!frameOk) {
            pConcealmentInfo->cntFadeFrames = 0;
            pConcealmentInfo->cntValidFrames = 0;
            pConcealmentInfo->attGrpOffset[0] = 0;
            pConcealmentInfo->attGrpOffset[1] = 0;
            pConcealmentInfo->winGrpOffset[0] = 0;
            pConcealmentInfo->winGrpOffset[1] = 0;
            if (pConcealCommonData->numFadeOutFrames > 0) {
              /* change to state SINGLE-FRAME-LOSS */
              pConcealmentInfo->concealState = ConcealState_Single;
              /* mode 0 just updates the Fading counter */
              CConcealment_ApplyFadeOut(/*mode =*/0, pConcealmentInfo, pAacDecoderStaticChannelInfo,
                                        samplesPerFrame, pAacDecoderChannelInfo);

            } else {
              /* change to state MUTE */
              pConcealmentInfo->concealState = ConcealState_Mute;
            }
          }
          break;

        case ConcealState_Single: /* Just a pre-stage before fade-out begins. Stay here only one
                                     frame! */
          if (frameOk) {
            /* change to state OK */
            pConcealmentInfo->concealState = ConcealState_Ok;
          } else {
            if (pConcealmentInfo->cntFadeFrames >= pConcealCommonData->numFadeOutFrames) {
              /* change to state MUTE */
              pConcealmentInfo->concealState = ConcealState_Mute;
            } else {
              /* change to state FADE-OUT */
              pConcealmentInfo->concealState = ConcealState_FadeOut;
              /* mode 0 just updates the Fading counter */
              CConcealment_ApplyFadeOut(/*mode =*/0, pConcealmentInfo, pAacDecoderStaticChannelInfo,
                                        samplesPerFrame, pAacDecoderChannelInfo);
            }
          }
          break;

        case ConcealState_FadeOut:
          if (pConcealmentInfo->cntValidFrames > pConcealCommonData->numMuteReleaseFrames) {
            if (pConcealCommonData->numFadeInFrames > 0) {
              /* change to state FADE-IN */
              pConcealmentInfo->concealState = ConcealState_FadeIn;
              pConcealmentInfo->cntFadeFrames = findEquiFadeFrame(
                  pConcealCommonData, pConcealmentInfo->cntFadeFrames, 0 /* FadeOut -> FadeIn */);
            } else {
              /* change to state OK */
              pConcealmentInfo->concealState = ConcealState_Ok;
            }
          } else {
            if (frameOk) {
              /* we have good frame information but stay fully in concealment - reset
               * winGrpOffset/attGrpOffset */
              pConcealmentInfo->winGrpOffset[0] = 0;
              pConcealmentInfo->winGrpOffset[1] = 0;
              pConcealmentInfo->attGrpOffset[0] = 0;
              pConcealmentInfo->attGrpOffset[1] = 0;
            }
            if (pConcealmentInfo->cntFadeFrames >= pConcealCommonData->numFadeOutFrames) {
              /* change to state MUTE */
              pConcealmentInfo->concealState = ConcealState_Mute;
            } else /* Stay in FADE-OUT */
            {
              /* mode 0 just updates the Fading counter */
              CConcealment_ApplyFadeOut(/*mode =*/0, pConcealmentInfo, pAacDecoderStaticChannelInfo,
                                        samplesPerFrame, pAacDecoderChannelInfo);
            }
          }
          break;

        case ConcealState_Mute:
          if (pConcealmentInfo->cntValidFrames > pConcealCommonData->numMuteReleaseFrames) {
            if (pConcealCommonData->numFadeInFrames > 0) {
              /* change to state FADE-IN */
              pConcealmentInfo->concealState = ConcealState_FadeIn;
              pConcealmentInfo->cntFadeFrames = pConcealCommonData->numFadeInFrames - 1;
            } else {
              /* change to state OK */
              pConcealmentInfo->concealState = ConcealState_Ok;
            }
          } else {
            if (frameOk) {
              /* we have good frame information but stay fully in concealment - reset
               * winGrpOffset/attGrpOffset */
              pConcealmentInfo->winGrpOffset[0] = 0;
              pConcealmentInfo->winGrpOffset[1] = 0;
              pConcealmentInfo->attGrpOffset[0] = 0;
              pConcealmentInfo->attGrpOffset[1] = 0;
            }
          }
          break;

        case ConcealState_FadeIn:
          pConcealmentInfo->cntFadeFrames -= 1;
          if (frameOk) {
            if (pConcealmentInfo->cntFadeFrames < 0) {
              /* change to state OK */
              pConcealmentInfo->concealState = ConcealState_Ok;
            }
          } else {
            if (pConcealCommonData->numFadeOutFrames > 0) {
              /* change to state FADE-OUT */
              pConcealmentInfo->concealState = ConcealState_FadeOut;
              pConcealmentInfo->cntFadeFrames =
                  findEquiFadeFrame(pConcealCommonData, pConcealmentInfo->cntFadeFrames + 1,
                                    1 /* FadeIn -> FadeOut */);
              pConcealmentInfo->winGrpOffset[0] = 0;
              pConcealmentInfo->winGrpOffset[1] = 0;
              pConcealmentInfo->attGrpOffset[0] = 0;
              pConcealmentInfo->attGrpOffset[1] = 0;

              pConcealmentInfo->cntFadeFrames--; /* decrease because CConcealment_ApplyFadeOut()
                                                    will increase, accordingly */
              /* mode 0 just updates the Fading counter */
              CConcealment_ApplyFadeOut(/*mode =*/0, pConcealmentInfo, pAacDecoderStaticChannelInfo,
                                        samplesPerFrame, pAacDecoderChannelInfo);
            } else {
              /* change to state MUTE */
              pConcealmentInfo->concealState = ConcealState_Mute;
            }
          }
          break;

        default:
          FDK_ASSERT(0);
          break;
      }
    } break;

    default:
      /* Don't need a state machine for other concealment methods. */
      break;
  }
}

/*!
\brief Randomizes the sign of the spectral data

  The function toggles the sign of the spectral data randomly. This is
  useful to ensure the quality of the concealed frames.
 */
static void CConcealment_ApplyRandomSign(int randomPhase, FIXP_DBL* spec, int samplesPerFrame) {
  int i;
  USHORT packedSign = 0;

  /* random table 512x16bit has been reduced to 512 packed sign bits = 32x16 bit */

  /* read current packed sign word */
  packedSign = AacDec_randomSign[randomPhase >> 4];
  packedSign >>= (randomPhase & 0xf);

  for (i = 0; i < samplesPerFrame; i++) {
    if ((randomPhase & 0xf) == 0) {
      packedSign = AacDec_randomSign[randomPhase >> 4];
    }

    if (packedSign & 0x1) {
      spec[i] = -fMax(spec[i], (FIXP_DBL)(MINVAL_DBL + 1));
    }
    packedSign >>= 1;

    randomPhase = (randomPhase + 1) & (AAC_NF_NO_RANDOM_VAL - 1);
  }
}

/*!
  \brief Get fadeing factor for current concealment state.

  The function returns the state (ok or not) of the previous frame.
  If called before the function CConcealment_Apply() set the fBeforeApply
  flag to get the correct value.

  \return Frame OK flag of previous frame.
 */
int CConcealment_GetLastFrameOk(CConcealmentInfo* hConcealmentInfo, const int fBeforeApply) {
  int prevFrameOk = 1;

  if (hConcealmentInfo != NULL) {
    prevFrameOk = hConcealmentInfo->prevFrameOk[fBeforeApply & 0x1];
  }

  return prevFrameOk;
}

/*!
  \brief Get the number of delay frames introduced by concealment technique.

  \return Number of delay frames.
 */
UINT CConcealment_GetDelay(CConcealParams* pConcealCommonData) {
  UINT frameDelay = 0;

  if (pConcealCommonData != NULL) {
    switch (pConcealCommonData->method) {
      case ConcealMethodTonal:
      case ConcealMethodInter:
        frameDelay = 1;
        break;
      default:
        break;
    }
  }

  return frameDelay;
}

static int CConcealment_ApplyFadeOut(int mode, CConcealmentInfo* pConcealmentInfo,
                                     CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                                     const int samplesPerFrame,
                                     CAacDecoderChannelInfo* pAacDecoderChannelInfo) {
  /* mode 1 = apply RandomSign and mute spectral coefficients if necessary,  *
   * mode 0 = Update cntFadeFrames                                            */

  /* restore frequency coefficients from buffer with a specific muting */
  int srcWin, dstWin, numWindows = 1;
  int windowLen = samplesPerFrame;
  int srcGrpStart = 0;
  int winIdxStride = 1;
  int numWinGrpPerFac, attIdx, attIdxStride;
  int i;
  int appliedProcessing = 0;

  CIcsInfo* pIcsInfo = &pAacDecoderChannelInfo->icsInfo;
  FIXP_DBL* pSpectralCoefficient = SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);
  SHORT* pSpecScale = pAacDecoderChannelInfo->specScale;

  /* set old window parameters */
  {
    pIcsInfo->WindowShape = pConcealmentInfo->windowShape;
    pIcsInfo->WindowSequence = pConcealmentInfo->windowSequence;

    if (pConcealmentInfo->windowSequence == BLOCK_SHORT) {
      /* short block handling */
      numWindows = 8;
      windowLen = samplesPerFrame >> 3;
      srcGrpStart = numWindows - pConcealmentInfo->lastWinGrpLen;
    }
  }

  attIdxStride = fMax(1, (int)(numWindows / (pConcealmentInfo->lastWinGrpLen + 1)));

  /* load last state */
  attIdx = pConcealmentInfo->cntFadeFrames;
  numWinGrpPerFac = pConcealmentInfo->attGrpOffset[mode];
  srcWin = srcGrpStart + pConcealmentInfo->winGrpOffset[mode];

  FDK_ASSERT((srcGrpStart * windowLen + windowLen) <= samplesPerFrame);
  FDK_ASSERT((srcWin * windowLen + windowLen) <= 1024);

  for (dstWin = 0; dstWin < numWindows; dstWin += 1) {
    FIXP_CNCL* pCncl = pConcealmentInfo->spectralCoefficient + (srcWin * windowLen);
    FIXP_DBL* pOut = pSpectralCoefficient + (dstWin * windowLen);

    if (mode == 1) {
      /* mute if attIdx gets large enaugh */
      if (attIdx > pConcealmentInfo->pConcealParams->numFadeOutFrames) {
        FDKmemclear(pCncl, sizeof(FIXP_DBL) * windowLen);
      }

      /* restore frequency coefficients from buffer - attenuation is done later */
      for (i = 0; i < windowLen; i++) {
        pOut[i] = pCncl[i];
      }

      /* apply random change of sign for spectral coefficients */
      CConcealment_ApplyRandomSign(pConcealmentInfo->iRandomPhase, pOut, windowLen);

      /* Increment random phase index to avoid repetition artifacts. */
      pConcealmentInfo->iRandomPhase =
          (pConcealmentInfo->iRandomPhase + 1) & (AAC_NF_NO_RANDOM_VAL - 1);

      /* set old scale factors */
      pSpecScale[dstWin * winIdxStride] = pConcealmentInfo->specScale[srcWin * winIdxStride];
    }

    srcWin += 1;

    if (srcWin >= numWindows) {
      /* end of sequence -> rewind to first window of group */
      srcWin = srcGrpStart;
      numWinGrpPerFac += 1;
      if (numWinGrpPerFac >= attIdxStride) {
        numWinGrpPerFac = 0;
        attIdx += 1;
      }
    }
  }

  /* store current state */

  pConcealmentInfo->winGrpOffset[mode] = srcWin - srcGrpStart;
  FDK_ASSERT((pConcealmentInfo->winGrpOffset[mode] >= 0) &&
             (pConcealmentInfo->winGrpOffset[mode] < 8));
  pConcealmentInfo->attGrpOffset[mode] = numWinGrpPerFac;
  FDK_ASSERT((pConcealmentInfo->attGrpOffset[mode] >= 0) &&
             (pConcealmentInfo->attGrpOffset[mode] < attIdxStride));

  if (mode == 0) {
    pConcealmentInfo->cntFadeFrames = attIdx;
  }

  appliedProcessing = 1;

  return appliedProcessing;
}

/*!
  \brief Do Time domain fading (TDFading) in concealment case

  In case of concealment, this function takes care of the fading, after time domain signal has been
rendered by the respective signal rendering functions. The fading out in case of ACELP decoding is
not done by this function but by the ACELP decoder for the first concealed frame if
CONCEAL_CORE_IGNORANT_FADE is not set.

  TimeDomain fading never creates jumps in energy / discontinuities, it always does a continuous
fading. To achieve this, fading is always done from a starting point to a target point, while the
starting point is always determined to be the last target point. By varying the target point of a
fading, the fading slope can be controlled.

  This principle is applied to the fading within a frame and the fading from frame to frame.

  One frame is divided into 8 subframes to obtain 8 parts of fading slopes within a frame, each
maybe with its own gradient.

  Workflow:
  1.) Determine Fading behavior and end-of-frame target fading level, based on concealmentState
(determined by CConcealment_UpdateState()) and the core mode.
        - By _DEFAULT_,
          The target fading level is determined by fadeOutFactor[cntFadeFrames] in case of fadeOut,
or fadeInFactor[cntFadeFrames] in case of fadeIn.
          --> fading type is FADE_TIMEDOMAIN in this case. Target fading level is determined by
fading index cntFadeFrames.

        - If concealmentState is signalling a _MUTED SIGNAL_,
          TDFading decays to 0 within 1/8th of a frame if numFadeOutFrames == 0.
          --> fading type is FADE_TIMEDOMAIN_TOSPECTRALMUTE in this case.

        - If concealmentState is signalling the _END OF MUTING_,
          TDFading fades to target fading level within 1/8th of a frame if numFadeInFrames == 0.
          --> fading type is FADE_TIMEDOMAIN_FROMSPECTRALMUTE in this case. Target fading level is
determined by fading index cntFadeFrames.

#ifndef CONCEAL_CORE_IGNORANT_FADE
        - In case of an _ACELP FADEOUT_,
          TDFading leaves fading control to ACELP decoder for 1/2 frame.
          --> fading type is FADE_ACELPDOMAIN in this case.
#endif

  2.) Render fading levels within current frame and do the final fading:
      Map Fading slopes to fading levels and apply to time domain signal.


*/

INT CConcealment_TDFading(int len, CAacDecoderStaticChannelInfo** ppAacDecoderStaticChannelInfo,
                          const INT aacOutDataHeadroom, PCM_DEC* pcmdata, PCM_DEC* pcmdata_1) {
  /*
  Do the fading in Time domain based on concealment states and core mode
  */
  FIXP_DBL fadeStop, attMute = (FIXP_DBL)0;
  int idx = 0, ii;
  CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo = *ppAacDecoderStaticChannelInfo;
  CConcealmentInfo* pConcealmentInfo = &pAacDecoderStaticChannelInfo->concealmentInfo;
  CConcealParams* pConcealParams = pConcealmentInfo->pConcealParams;
  const CConcealmentState concealState = pConcealmentInfo->concealState;
  TDfadingType fadingType;
  FIXP_DBL fadingStations[9] = {0};
  int fadingSteps[8] = {0};
  const FIXP_DBL fadeStart =
      pConcealmentInfo->fade_old; /* start fading at last end-of-frame attenuation */
  FIXP_SGL* fadeFactor = pConcealParams->fadeOutFactor;
  const INT cntFadeFrames = pConcealmentInfo->cntFadeFrames;
  int TDFadeOutStopBeforeMute = 1;
  int TDFadeInStopBeforeFullLevel = 1;

  /*
  determine Fading behaviour (end-of-frame attenuation and fading type) (1.)
  */

  switch (concealState) {
    case ConcealState_Single:
    case ConcealState_Mute:
    case ConcealState_FadeOut:
      idx = (pConcealParams->method == ConcealMethodNoise) ? cntFadeFrames - 1 : cntFadeFrames;
      fadingType = FADE_TIMEDOMAIN;

      if (concealState == ConcealState_Mute ||
          (cntFadeFrames + TDFadeOutStopBeforeMute) >
              pConcealmentInfo->pConcealParams->numFadeOutFrames) {
        fadingType = FADE_TIMEDOMAIN_TOSPECTRALMUTE;
      }

      break;
    case ConcealState_FadeIn:
      idx = cntFadeFrames;
      idx -= TDFadeInStopBeforeFullLevel;
      FDK_FALLTHROUGH;
    case ConcealState_Ok:
      fadeFactor = pConcealParams->fadeInFactor;
      idx = (concealState == ConcealState_Ok) ? -1 : idx;
      fadingType = (pConcealmentInfo->concealState_old == ConcealState_Mute)
                       ? FADE_TIMEDOMAIN_FROMSPECTRALMUTE
                       : FADE_TIMEDOMAIN;
      break;
    default:
      FDK_ASSERT(0);
      fadingType = FADE_TIMEDOMAIN_TOSPECTRALMUTE;
      break;
  }

  /* determine Target end-of-frame fading level and fading slope */
  switch (fadingType) {
    case FADE_TIMEDOMAIN_FROMSPECTRALMUTE:
      fadeStop = (idx < 0) ? (FIXP_DBL)MAXVAL_DBL : FX_SGL2FX_DBL(fadeFactor[idx]);
      if (pConcealmentInfo->pConcealParams->numFadeInFrames == 0) {
        /* do step as fast as possible */
        fadingSteps[0] = 1;
        break;
      }
      CConcealment_TDFading_doLinearFadingSteps(&fadingSteps[0]);
      break;
    case FADE_TIMEDOMAIN:
      fadeStop = (idx < 0) ? (FIXP_DBL)MAXVAL_DBL : FX_SGL2FX_DBL(fadeFactor[idx]);
      CConcealment_TDFading_doLinearFadingSteps(&fadingSteps[0]);
      break;
    case FADE_TIMEDOMAIN_TOSPECTRALMUTE:
      fadeStop = attMute;
      if (pConcealmentInfo->pConcealParams->numFadeOutFrames == 0) {
        /* do step as fast as possible */
        fadingSteps[0] = 1;
        break;
      }
      CConcealment_TDFading_doLinearFadingSteps(&fadingSteps[0]);
      break;
  }

  /*
  Render fading levels within current frame and do the final fading (2.)
  */

  len >>= 3;
  CConcealment_TDFadeFillFadingStations(fadingStations, fadingSteps, fadeStop, fadeStart,
                                        fadingType);

  if ((fadingStations[8] != (FIXP_DBL)MAXVAL_DBL) || (fadingStations[7] != (FIXP_DBL)MAXVAL_DBL) ||
      (fadingStations[6] != (FIXP_DBL)MAXVAL_DBL) || (fadingStations[5] != (FIXP_DBL)MAXVAL_DBL) ||
      (fadingStations[4] != (FIXP_DBL)MAXVAL_DBL) || (fadingStations[3] != (FIXP_DBL)MAXVAL_DBL) ||
      (fadingStations[2] != (FIXP_DBL)MAXVAL_DBL) || (fadingStations[1] != (FIXP_DBL)MAXVAL_DBL) ||
      (fadingStations[0] != (FIXP_DBL)MAXVAL_DBL)) /* if there's something to fade */
  {
    int start = 0;
    for (ii = 0; ii < 8; ii++) {
      CConcealment_TDFadePcmAtt(start, len, fadingStations[ii], fadingStations[ii + 1], pcmdata);
      start += len;
    }
  }
  CConcealment_TDNoise_Apply(pConcealmentInfo, len, aacOutDataHeadroom, pcmdata);

  /* Save end-of-frame attenuation and fading type */
  pConcealmentInfo->lastFadingType = fadingType;
  pConcealmentInfo->fade_old = fadeStop;
  pConcealmentInfo->concealState_old = concealState;

  return 1;
}

/* attenuate pcmdata in Time Domain Fading process */
static void CConcealment_TDFadePcmAtt(int start, int len, FIXP_DBL fadeStart, FIXP_DBL fadeStop,
                                      PCM_DEC* pcmdata) {
  int i;
  FIXP_DBL dStep;
  FIXP_DBL dGain;
  FIXP_DBL dGain_apply;

  /* set start energy */
  dGain = fadeStart;
  /* determine energy steps from sample to sample */
  dStep = (FIXP_DBL)((int)((fadeStart >> 1) - (fadeStop >> 1)) / len) << 1;

  for (i = start; i < (start + len); i++) {
    dGain -= dStep;
    /* prevent gain from getting negative due to possible fixpoint inaccuracies */
    dGain_apply = fMax((FIXP_DBL)0, dGain);
    /* finally, attenuate samples */
    pcmdata[i] = FIXP_DBL2PCM_DEC(fMult(pcmdata[i], dGain_apply));
  }
}

/*
\brief Fill FadingStations

The fadingstations are the attenuation factors, being applied to its dedicated portions of pcm data.
They are calculated using the fadingsteps. One fadingstep is the weighted contribution to the fading
slope within its dedicated portion of pcm data.

*Fadingsteps  :      0  0  0  1  0  1  2  0

                  |<-  1 Frame pcm data ->|
      fadeStart-->|__________             |
                  ^  ^  ^  ^ \____        |
 Attenuation  :   |  |  |  |  ^  ^\__     |
                  |  |  |  |  |  |  ^\    |
                  |  |  |  |  |  |  | \___|<-- fadeStop
                  |  |  |  |  |  |  |  ^  ^
                  |  |  |  |  |  |  |  |  |
Fadingstations:  [0][1][2][3][4][5][6][7][8]

(Fadingstations "[0]" is "[8] from previous frame", therefore its not meaningful to be edited)

*/
static void CConcealment_TDFadeFillFadingStations(FIXP_DBL* fadingStations, int* fadingSteps,
                                                  FIXP_DBL fadeStop, FIXP_DBL fadeStart,
                                                  TDfadingType fadingType) {
  int i;
  INT fadingSteps_sum = 0;
  INT fadeDiff;

  fadingSteps_sum = fadingSteps[0] + fadingSteps[1] + fadingSteps[2] + fadingSteps[3] +
                    fadingSteps[4] + fadingSteps[5] + fadingSteps[6] + fadingSteps[7];
  fadeDiff = ((INT)(fadeStop - fadeStart) / fMax(fadingSteps_sum, (INT)1));
  fadingStations[0] = fadeStart;
  for (i = 1; i < 8; i++) {
    fadingStations[i] = fadingStations[i - 1] + (FIXP_DBL)(fadeDiff * fadingSteps[i - 1]);
  }
  fadingStations[8] = fadeStop;
}

static void CConcealment_TDFading_doLinearFadingSteps(int* fadingSteps) {
  fadingSteps[0] = fadingSteps[1] = fadingSteps[2] = fadingSteps[3] = fadingSteps[4] =
      fadingSteps[5] = fadingSteps[6] = fadingSteps[7] = 1;
}

/* end of TimeDomainFading functions */

/* derived from int UsacRandomSign() */
static int CConcealment_TDNoise_Random(ULONG* seed) {
  *seed = (ULONG)(((UINT64)(*seed) * 69069) + 5);
  return (int)(*seed);
}

static void CConcealment_TDNoise_Apply(CConcealmentInfo* const pConcealmentInfo, const int len,
                                       const INT aacOutDataHeadroom, PCM_DEC* const pcmdata) {
  PCM_DEC* states = pConcealmentInfo->TDNoiseStates;
  PCM_DEC noiseVal;
  FIXP_DBL noiseValLong;
  FIXP_SGL* coef = pConcealmentInfo->TDNoiseCoef;
  FIXP_DBL TDNoiseAtt;
  ULONG seed = pConcealmentInfo->TDNoiseSeed =
      (ULONG)CConcealment_TDNoise_Random(&pConcealmentInfo->TDNoiseSeed) + 1;

  TDNoiseAtt = pConcealmentInfo->pConcealParams->comfortNoiseLevel;

  int ii;

  if ((pConcealmentInfo->concealState != ConcealState_Ok ||
       pConcealmentInfo->concealState_old != ConcealState_Ok) &&
      TDNoiseAtt != (FIXP_DBL)0) {
    for (ii = 0; ii < (len << 3); ii++) {
      /* create filtered noise */
      states[2] = states[1];
      states[1] = states[0];
      states[0] = FIXP_DBL2PCM_DEC((FIXP_DBL)CConcealment_TDNoise_Random(&seed));
      noiseValLong =
          fMult(states[0], coef[0]) + fMult(states[1], coef[1]) + fMult(states[2], coef[2]);
      noiseVal = FIXP_DBL2PCM_DEC(fMult(noiseValLong, TDNoiseAtt) >> aacOutDataHeadroom);

      /* add filtered noise - check for clipping, before */
      if (noiseVal > (PCM_DEC)0 && pcmdata[ii] > (PCM_DEC)MAXVAL_PCM_DEC - noiseVal) {
        noiseVal = noiseVal * (PCM_DEC)-1;
      } else if (noiseVal < (PCM_DEC)0 && pcmdata[ii] < (PCM_DEC)MINVAL_PCM_DEC - noiseVal) {
        noiseVal = noiseVal * (PCM_DEC)-1;
      }

      pcmdata[ii] += noiseVal;
    }
  }
}
