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

   Author(s): Arthur Tritthart, Thomas Blender, Alfonso Pino Garcia

   Description:

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Alexander Adami (AudioLabs) AND Fraunhofer IIS

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

 Copyright (c) ISO/IEC 2015.

 ***********************************************************************************/

#include "FDK_formatConverter_activeDmx_stft.h"

INT activeDmxStftInit(void** handle, UINT numInChans, UINT numOutChans, FIXP_DBL** inputBufferStft,
                      FIXP_DBL** prevInputBufferStft, FIXP_DBL** outputBufferStft, INT aes) {
  UINT ch;
  INT status = 0;
  activeDownmixer* h;
  UINT erb;

  FDK_ASSERT(numInChans <= MAX_CHANNELS);
  FDK_ASSERT(numOutChans <= MAX_CHANNELS);

  *handle = (activeDownmixer*)FDKaalloc(sizeof(activeDownmixer), ALIGNMENT_DEFAULT);
  if (*handle == NULL) {
    return status = -1;
  }
  h = (activeDownmixer*)(*handle);

  h->numInChans = numInChans;
  h->numOutChans = numOutChans;
  h->inputBufferStft = inputBufferStft;
  h->prevInputBufferStft = prevInputBufferStft;
  h->outputBufferStft = outputBufferStft;

  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    h->inBufStftHeadroomPrev[erb] = 31;   /* start value */
    h->realizedSigHeadroomPrev[erb] = 31; /* start value */
  }

  h->targetEnePrev = (FIXP_DBL**)FDKcalloc(numOutChans, sizeof(FIXP_DBL*));
  if (h->targetEnePrev == NULL) {
    return status = -1;
  }
  h->realizedEnePrev = (FIXP_DBL**)FDKcalloc(numOutChans, sizeof(FIXP_DBL*));
  if (h->realizedEnePrev == NULL) {
    return status = -1;
  }
  for (ch = 0; ch < numOutChans; ch++) {
    h->targetEnePrev[ch] =
        (FIXP_DBL*)FDKaalloc(STFT_ERB_BANDS * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
    if (h->targetEnePrev[ch] == NULL) {
      status = -1;
    }
    h->realizedEnePrev[ch] =
        (FIXP_DBL*)FDKaalloc(STFT_ERB_BANDS * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
    if (h->realizedEnePrev[ch] == NULL) {
      status = -1;
    }
  }

  h->targetEnePrevExp = (INT*)FDKaalloc(STFT_ERB_BANDS * sizeof(INT), ALIGNMENT_DEFAULT);
  if (h->targetEnePrevExp == NULL) {
    status = -1;
  }

  activeDmxSetAES(aes, h); /* 7 active downmix, 0 passive downmix */

  return status;
}

/*****************************************************************************************************************/

/*****************************************************************************************************************/

/*****************************************************************************************************************/

/*****************************************************************************************************************/
void activeDmxClose_STFT(void* handle) {
  UINT ch;
  activeDownmixer* h = (activeDownmixer*)(handle);

  if (h) {
    for (ch = 0; ch < h->numOutChans; ch++) {
      if (h->targetEnePrev) {
        if (h->targetEnePrev[ch]) {
          FDKafree(h->targetEnePrev[ch]);
        }
      }
      if (h->realizedEnePrev) {
        if (h->realizedEnePrev[ch]) {
          FDKafree(h->realizedEnePrev[ch]);
        }
      }
    }

    FDKfree(h->targetEnePrev);
    FDKfree(h->realizedEnePrev);
    if (h->targetEnePrevExp) {
      FDKafree(h->targetEnePrevExp);
    }

    FDKafree(h);
  }
}

/*
   Computation of return value EQ:                                   EQ_val           EQ_exp
   - division targetEne / realizedEne                                0.250 ... 1.0[  -30 ... +32
   - inverse square root of division result                          0.500 ... 1.0[  -15 ... +16
   - multiply invSqrt result with division result                    0.500 ... 1.0[  -15 ... +16
   - limitation of result into range apporximately [0.32 ... 2.51]   0.316 ... 2.51    2
   - multiply with AES in range 1/7...7/7                            0.045 ... 2.51    2
   - add (1.0-AES) in range 0/7... 6/7                               0.316 ... 2.51    2
   Due to the range limitation, we choose format Q29 for the following variables:
   - h->eqLimitMax
   - h->eqLimitMin
   - h->fixpAES
   - return value
 */

FDK_INLINE FIXP_DBL fDivNormNE(FIXP_DBL L_num, FIXP_DBL L_denum, INT* result_e) {
  /* Helper function: Copy of fDivNorm but both L_num > 0 and L_denum > 0 */
  FIXP_DBL div;
  INT norm_num, norm_den;

  norm_num = fMax(0, CntLeadingZeros(L_num) - 1);
  norm_den = fMax(0, CntLeadingZeros(L_denum) - 1);
  L_num = L_num << norm_num;
  L_num = L_num >> 1;
  L_denum = L_denum << norm_den;
  *result_e = norm_den - norm_num + 1;

  div = schur_div(L_num, L_denum, FRACT_BITS);
  return div;
}

FDK_INLINE FIXP_DBL computeEQAndClip(FIXP_DBL targetEne, FIXP_DBL realizedEne, INT diffExp,
                                     INT eq_exp, INT* EQ_exp) {
  const FIXP_DBL eqLimitMax =
      FL2FXCONST_DBL(2.511886432f / 4.0f); /* pow(10.f, 8.f/20.f) with exp = 2 */
  const FIXP_DBL eqLimitMin =
      FL2FXCONST_DBL(0.316227766f / 4.0f); /* pow(10.f, -10.f/20.f) with exp = 2  */
  FIXP_DBL EQ_val = eqLimitMin;
  INT EQ_exp_sqrt, scale;

  /* EQ = (float)sqrt(*targetEne/(h->epsilon + *realizedEne)); */
  realizedEne = fMax(realizedEne, (FIXP_DBL)1);
  targetEne = fMax(targetEne, (FIXP_DBL)1); /* zeros are small numbers in reality (float). */
  /* if we let it at 0 then the result is always eqLimitMin */
  /* set it to 1 results in a EQ of 1 when realizedEne is also 0 */
  /* that is a better deviation from float code than the eqLimitMin */
  EQ_val = fDivNormNE(targetEne, realizedEne, EQ_exp);
  *EQ_exp += diffExp;

  const int odd = *EQ_exp & 1;
  EQ_val >>= odd; /* make exponent even */
  *EQ_exp += odd;

  EQ_val = fMult(EQ_val, invSqrtNorm2(EQ_val, &EQ_exp_sqrt));
  *EQ_exp = EQ_exp_sqrt + (*EQ_exp / 2);

  /* Normalize mantissa to exponent 2 */
  scale = *EQ_exp - 2;
  EQ_val = scaleValueSaturate(EQ_val, scale);
  /*
    if (*EQ > eqLimitMax) *EQ = eqLimitMax;
    if (*EQ < eqLimitMin) *EQ = eqLimitMin;
  */
  EQ_val = fMin(EQ_val, eqLimitMax);
  EQ_val = fMax(EQ_val, eqLimitMin);
  scale = CountLeadingBits(EQ_val);
  EQ_val <<= scale;

  /* Add equalizer exponent */
  *EQ_exp = 2 - scale + eq_exp;
  return EQ_val;
}

#if defined(__arm__)
#include "arm/FDK_formatConverter_activeDmx_stft_arm.cpp"
#endif

FDK_INLINE INT get1xScalefactor(FIXP_DBL* buf) {
  FIXP_DBL buf0 = buf[0];
  buf0 = buf0 ^ (buf0 >> 31);
  return fNormz(buf0) - 1; /* in range 0..31 */
}

FDK_INLINE INT get2xScalefactor(FIXP_DBL* buf) {
  FIXP_DBL buf0 = buf[0];
  buf0 = buf0 ^ (buf0 >> 31);
  FIXP_DBL buf1 = buf[1];
  buf1 = buf1 ^ (buf1 >> 31);
  return fMin(fNormz(buf0), fNormz(buf1)) - 1; /* in range 0..31 */
}

#ifndef FUNCTION_activeDmxProcess_STFT
/*****************************************************************************************************************/
void activeDmxProcess_STFT(void* handle) {
  IIS_FORMATCONVERTER_INTERNAL* _p;
  _p = (IIS_FORMATCONVERTER_INTERNAL*)handle;
  activeDownmixer* h = (activeDownmixer*)_p->fcState->handleActiveDmxStft;

  INT* eqIndex_FDK;

  FIXP_DMX_H* dmxMatrixL_FDK;
  FIXP_DMX_H* dmxMatrixH_FDK;

  UINT i, chIn, numInChans, chOut, numOutChans, erb;
  FIXP_DBL** realizedSig = h->outputBufferStft;
  const FIXP_DBL Alpha = ALPHA_AEQ;
  const FIXP_DBL One_subAlpha = FIXP_DBL(MAXVAL_DBL) - Alpha;

  INT erb_start_ix, erb_stop_ix;

  RAM_ALIGN INT inBufStftHeadroom[STFT_ERB_BANDS];
  RAM_ALIGN INT realizedSigHeadroom[STFT_ERB_BANDS];
  RAM_ALIGN INT minHeadroomTargetEne[STFT_ERB_BANDS];
  RAM_ALIGN INT minHeadroomRealizedEne[STFT_ERB_BANDS];
  RAM_ALIGN INT exp_diff[STFT_ERB_BANDS];
  RAM_ALIGN INT exp_diff_realized[STFT_ERB_BANDS];

  FIXP_DBL _targetEneArr[IIS_FORMATCONVERTER_MAX_OUT_CH * STFT_ERB_BANDS +
                         (ALIGNMENT_DEFAULT - 1) / sizeof(FIXP_DBL)];
  FIXP_DBL* targetEneArr = (FIXP_DBL*)ALIGN_PTR(_targetEneArr);

  numOutChans = h->numOutChans;
  numInChans = h->numInChans;

  const FIXP_EQ_H* eq[N_EQ];
  INT eq_e;
  FIXP_EQ_H const_1dot0_times58[STFT_ERB_BANDS];
  /*********************************************/
  /* Immersive mode initializer                */
  /*********************************************/

  INT immersiveMode = _p->fcParams->immersiveMode;
  INT Mode3Drendering = _p->fcParams->Mode3Drendering;
#ifdef FUNCTION_immersiveModeInit
  immersiveModeInit(&immersiveMode, &Mode3Drendering, _p->fcParams->rendering3DTypeFlag_internal,
                    _p->rendering3DTypeFlag, _p);
#else
  /* rendering3DTypeFlag_internal is only updated internally. rendering3DTypeFlag is updated every
   * frame. */
  INT rendering3DTypeFlag_internal = _p->fcParams->rendering3DTypeFlag_internal;

  /* Update control for immersive mode. The rendering3D flag could change under immersive mode.
   * Transitions from 3D to 2D (or the other way round) rendering are possible. */
  if ((immersiveMode) && ((UINT)rendering3DTypeFlag_internal != _p->rendering3DTypeFlag)) {
    /* Update control */
    formatConverterDmxMatrixControl(_p);
    immersiveMode = _p->fcParams->immersiveMode;
    Mode3Drendering = _p->fcParams->Mode3Drendering;
  }
#endif /*FUNCTION_immersiveModeInit*/

  dmxMatrixL_FDK = _p->fcParams->dmxMatrixL_FDK;
  dmxMatrixH_FDK = _p->fcParams->dmxMatrixH_FDK;
  eqIndex_FDK = _p->fcParams->eqIndex_FDK;

  INT *chOut_exp = _p->fcParams->chOut_exp, *chOut_count = _p->fcParams->chOut_count;
  INT dmx_iterations = _p->fcParams->dmx_iterations;

  /* Equalizers init */
  if (_p->amountOfAddedDmxMatricesAndEqualizers) {
    eq_e = EQ_BITSTREAM_H_EXP;
    for (i = 0; i < _p->amountOfAddedDmxMatricesAndEqualizers; i++) {
      eq[i] = _p->eqGains[i];
    }
  } else {
    eq[0] = const_1dot0_times58;
    eq_e = EQ_H_EXP;
    for (i = RULE_EQ1; i <= RULE_EQ5; i++) {
      eq[i] = _p->fcParams->formatConverterParams_internal->eq[i - 1];
    }
    if (immersiveMode != 0) {
      eq[6] = const_1dot0_times58;
      for (i = RULE_EQVF; i <= RULE_18_11; i++) {
        eq[i] = _p->fcParams->formatConverterParams_internal->eq[i - 1];
      }
    }
  }

  /* - init inputStftHeadroom[0..57] with (SCHAR) 32
     - init const_1dot0_times58 with 1.0f with scale=1
     - for all input buffer: ch=0..numInChans-1 &&
           all prev  buffer: ch=TFC..TFRA,  if Mode3Drendering
       - get headroom from buffer[0] for erb = 0  (DC)
       - get headroom from buffer[1] for erb = 57 (Nyquist)
       - for all erb=1..57
         - get buffer`s headroom over all fftBands of this erb
         - get minimum headroom in inBufStftHeadroom[erb]
  */
  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    const_1dot0_times58[erb] = FIXP_EQ_H_FORMAT_1_dot_0;
    inBufStftHeadroom[erb] = 32;
  }

  for (chIn = 0; chIn < numInChans; chIn++) {
    FIXP_DBL* inBuf = h->inputBufferStft[chIn];
    inBufStftHeadroom[0] = fMin(inBufStftHeadroom[0], get1xScalefactor(&inBuf[0]));
    inBufStftHeadroom[STFT_ERB_BANDS - 1] =
        fMin(inBufStftHeadroom[STFT_ERB_BANDS - 1], get1xScalefactor(&inBuf[1]));
    for (erb = 1; erb < 33; erb++) { /* These erb use only 1 complex frequency band */
      inBufStftHeadroom[erb] = fMin(inBufStftHeadroom[erb], get2xScalefactor(&inBuf[2 * erb]));
    }
    erb_start_ix = erb;
    for (; erb < STFT_ERB_BANDS; erb++) { /* These erb use 3..19 frequency bands */
      erb_stop_ix = erb_freq_idx_256_58[erb];
      inBufStftHeadroom[erb] =
          fMin(inBufStftHeadroom[erb],
               getScalefactor(&inBuf[2 * erb_start_ix], 2 * (erb_stop_ix - erb_start_ix)));
      erb_start_ix = erb_stop_ix; /* for next loop iteration */
    }
  }
  if (Mode3Drendering) {
    for (chIn = TFC; chIn <= TFRA; chIn++) {
      FIXP_DBL* inBuf = h->prevInputBufferStft[chIn];
      inBufStftHeadroom[0] = fMin(inBufStftHeadroom[0], get1xScalefactor(&inBuf[0]));
      inBufStftHeadroom[STFT_ERB_BANDS - 1] =
          fMin(inBufStftHeadroom[STFT_ERB_BANDS - 1], get1xScalefactor(&inBuf[1]));
      for (erb = 1; erb < 33; erb++) { /* These erb use only 1 complex frequency band */
        inBufStftHeadroom[erb] = fMin(inBufStftHeadroom[erb], get2xScalefactor(&inBuf[2 * erb]));
      }
      erb_start_ix = erb;
      for (; erb < STFT_ERB_BANDS; erb++) { /* These erb use 3..19 frequency bands */
        erb_stop_ix = erb_freq_idx_256_58[erb];
        inBufStftHeadroom[erb] =
            fMin(inBufStftHeadroom[erb],
                 getScalefactor(&inBuf[2 * erb_start_ix], 2 * (erb_stop_ix - erb_start_ix)));
        erb_start_ix = erb_stop_ix; /* for next loop iteration */
      }
    }
  }

  /*********************************************/
  /*               Clear Buffer                */
  /*********************************************/
  for (chOut = 0; chOut < numOutChans; chOut++) {
    FDKmemclear(realizedSig[chOut], STFT_LENGTH * sizeof(FIXP_DBL));
  }
  FDKmemclear(targetEneArr, STFT_ERB_BANDS * numOutChans * sizeof(FIXP_DBL));

  UINT erb_is4GVH_L = 58, erb_is4GVH_H = 58;

  if (Mode3Drendering) {
    erb_is4GVH_L = _p->erb_is4GVH_L;
    erb_is4GVH_H = _p->erb_is4GVH_H;
  }
  /*********************************************/
  /* Compute target energy and realized signal */
  /*********************************************/

  for (chIn = 0; chIn < numInChans; chIn++) {
    FIXP_DBL* inputBuffer;

    INT inputBuffer_index = -1; /* means: inputBuffer = h->inputBufferStft[chIn]; */
    if (Mode3Drendering) {
      if (chIn == (UINT)_p->topIn[TFC])
        inputBuffer_index = TFC;
      else if (chIn == (UINT)_p->topIn[TFL])
        inputBuffer_index = TFL;
      else if (chIn == (UINT)_p->topIn[TFR])
        inputBuffer_index = TFR;
      else if (chIn == (UINT)_p->topIn[TFLA])
        inputBuffer_index = TFLA;
      else if (chIn == (UINT)_p->topIn[TFRA])
        inputBuffer_index = TFRA;
    }
    for (chOut = 0; chOut < numOutChans; chOut++) {
      if ((dmxMatrixL_FDK[chOut] != (FIXP_DMX_H)0) || (dmxMatrixH_FDK[chOut] != (FIXP_DMX_H)0)) {
        const FIXP_DBL* eq_ptr = eq[eqIndex_FDK[chOut]]; /* pointer to equalizer */
        if ((inputBuffer_index != -1) && (_p->midOut[chOut] > SW)) {
          inputBuffer = h->prevInputBufferStft[inputBuffer_index];
        } else {
          inputBuffer = h->inputBufferStft[chIn]; /* undelayed input signal */
        }

#ifdef FUNCTION_activeDmxProcess_STFT_func1
        activeDmxProcess_STFT_func1(
            inputBuffer, realizedSig[chOut], &targetEneArr[STFT_ERB_BANDS * chOut],
            erb_freq_idx_256_58, eq_ptr, dmxMatrixL_FDK[chOut], dmxMatrixH_FDK[chOut], erb_is4GVH_L,
            erb_is4GVH_H, chOut_exp[chOut], dmx_iterations, inBufStftHeadroom
#if defined(__arm__)
            ,
            erb_freq_idx_256_58_exp
#endif
        );
#else
        FIXP_DBL* realizedSig_chOut = realizedSig[chOut];
        FIXP_DBL* targetEnergy = &targetEneArr[STFT_ERB_BANDS * chOut];
        FIXP_DBL savIm0 = inputBuffer[1];
        inputBuffer[1] = FIXP_DBL(0);

        FIXP_DBL targetEne;
        INT target_exp = 0;
        FIXP_DBL tmpRe, tmpIm;
        FIXP_DBL bufRe, bufIm;
        FIXP_DBL dmx_coeff =
            (FIXP_DBL)0; /* product of equalizer coefficient with dmx matrix coefficient */
        FIXP_DMX_H dmx_coeff_mtx;

        UINT fftBand = 0;
        UINT max_erb = erb_is4GVH_L;
        erb = 0;
        dmx_coeff_mtx = dmxMatrixL_FDK[chOut];

        for (INT it = 0; it < dmx_iterations; it++) {
          for (; erb < max_erb; erb++) {
            target_exp = erb_freq_idx_256_58_exp[erb] + chOut_exp[chOut];
            /* need only one exponent for each erb therefore set first component fixed to 0 until
             * switch to 1D-Array*/
            UINT maxfftBand = erb_freq_idx_256_58[erb];

            dmx_coeff = fMult(dmx_coeff_mtx, eq_ptr[erb]);
            targetEne = targetEnergy[erb];
            for (; fftBand < maxfftBand; fftBand++) /* minimal 1 iteration, maximal 19 iterations */
            {
              bufRe = inputBuffer[fftBand * 2 + 0];
              bufIm = inputBuffer[fftBand * 2 + 1];
              tmpRe = fMult(dmx_coeff, bufRe << inBufStftHeadroom[erb]);
              tmpIm = fMult(dmx_coeff, bufIm << inBufStftHeadroom[erb]);
              realizedSig_chOut[fftBand * 2 + 0] =
                  fAddSaturate(realizedSig_chOut[fftBand * 2 + 0], tmpRe >> inBufStftHeadroom[erb]);
              realizedSig_chOut[fftBand * 2 + 1] =
                  fAddSaturate(realizedSig_chOut[fftBand * 2 + 1], tmpIm >> inBufStftHeadroom[erb]);

              tmpIm = fPow2Div2(tmpIm); /* [0...0x40000000) */
              tmpRe = fPow2Div2(tmpRe); /* [0...0x40000000) */

              targetEne += (tmpIm + tmpRe) >> target_exp;
            } /* j = fftBand */
            targetEnergy[erb] = targetEne;
          } /* for(; erb < max_erb; erb++) */
          if (it == 0) {
            max_erb = erb_is4GVH_H;
            dmx_coeff_mtx = dmxMatrixH_FDK[chOut];
          } else if (it == 1) {
            max_erb = 58;
            dmx_coeff_mtx = dmxMatrixL_FDK[chOut];
          }
        }      /* for (INT it = 0; it < dmx_iterations; it++) */
        erb--; /* switch back to last erb */
        inputBuffer[1] = savIm0;
        tmpIm = fMult(dmx_coeff, savIm0 << inBufStftHeadroom[erb]);
        realizedSig_chOut[1] =
            fAddSaturate(realizedSig_chOut[1], (tmpIm >> inBufStftHeadroom[erb]));
        targetEnergy[erb] += (fPow2Div2(tmpIm) >> target_exp);
#endif /* #ifdef FUNCTION_activeDmxProcess_STFT_func1 */

      } /* if ( ( dmxMatrixL_FDK[chOut] != (FIXP_DMX_H)0 ) || ( dmxMatrixH_FDK[chOut] !=
           (FIXP_DMX_H)0 ) ) */
    }   /* for(chOut = 0; chOut < numOutChans; chOut++) */
    dmxMatrixH_FDK += numOutChans;
    dmxMatrixL_FDK += numOutChans;
    eqIndex_FDK += numOutChans;
  } /* for(chIn = 0; chIn < numInChans; chIn++) */

  /* headroom of realized signal */
  /* initialize headroom values with maximum  */
  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    realizedSigHeadroom[erb] = inBufStftHeadroom[erb];
  }

  for (chOut = 0; chOut < numOutChans; chOut++) {
    /* first fft band (DC) and last fft band (Nyquist)*/
    realizedSigHeadroom[0] = fMin(realizedSigHeadroom[0], get1xScalefactor(&realizedSig[chOut][0]));
    realizedSigHeadroom[STFT_ERB_BANDS - 1] =
        fMin(realizedSigHeadroom[STFT_ERB_BANDS - 1], get1xScalefactor(&realizedSig[chOut][1]));
    /* all other fft bands */
    for (erb = 1; erb < 33; erb++) {
      realizedSigHeadroom[erb] =
          fMin(realizedSigHeadroom[erb], get2xScalefactor(&realizedSig[chOut][2 * erb]));
    }
    for (erb_start_ix = erb; erb < STFT_ERB_BANDS; erb++) {
      erb_stop_ix = erb_freq_idx_256_58[erb];
      realizedSigHeadroom[erb] = fMin(
          realizedSigHeadroom[erb],
          getScalefactor(&realizedSig[chOut][2 * erb_start_ix], 2 * (erb_stop_ix - erb_start_ix)));
      erb_start_ix = erb_stop_ix; /* for next loop iteration */
    }
  }

  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    /* diff = target exponent - previous target exponent (does not depend on erb) */
    /* exponent = e_chOut + 1 - (2*inBufStftHeadroom) + erb_freq_idx_256_58_exp[i] + 20 + 2;  */
    exp_diff[erb] = 2 * (-(inBufStftHeadroom[erb] + _p->STFT_headroom_prescaling)) +
                    h->inBufStftHeadroomPrev[erb];
    exp_diff[erb] = fMax(fMin(exp_diff[erb], 31), -31);
    exp_diff_realized[erb] = 2 * (-(realizedSigHeadroom[erb] + _p->STFT_headroom_prescaling)) +
                             h->realizedSigHeadroomPrev[erb];
    exp_diff_realized[erb] = fMax(fMin(exp_diff_realized[erb], 31), -31);
  }

  /*********************************************/
  /*            Update delay buffer            */
  /*********************************************/
  if (immersiveMode) {
    for (chIn = 0; chIn < h->numInChans; chIn++) {
      if (chIn == (UINT)_p->topIn[TFL])
        FDKmemcpy(h->prevInputBufferStft[TFL], h->inputBufferStft[chIn],
                  STFT_LENGTH * sizeof(FIXP_DBL));
      if (chIn == (UINT)_p->topIn[TFC])
        FDKmemcpy(h->prevInputBufferStft[TFC], h->inputBufferStft[chIn],
                  STFT_LENGTH * sizeof(FIXP_DBL));
      if (chIn == (UINT)_p->topIn[TFR])
        FDKmemcpy(h->prevInputBufferStft[TFR], h->inputBufferStft[chIn],
                  STFT_LENGTH * sizeof(FIXP_DBL));
      if (chIn == (UINT)_p->topIn[TFLA])
        FDKmemcpy(h->prevInputBufferStft[TFLA], h->inputBufferStft[chIn],
                  STFT_LENGTH * sizeof(FIXP_DBL));
      if (chIn == (UINT)_p->topIn[TFRA])
        FDKmemcpy(h->prevInputBufferStft[TFRA], h->inputBufferStft[chIn],
                  STFT_LENGTH * sizeof(FIXP_DBL));
    }
  }

  FIXP_DBL* realizedEnergy =
      (FIXP_DBL*)(&minHeadroomTargetEne[0]); /* Only possible, if both are of type INT[] */

  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    minHeadroomRealizedEne[erb] = 31; /* start value */
  }

  for (chOut = 0; chOut < numOutChans; chOut++) {
    if (chOut_count[chOut] != 0) {
#ifdef FUNCTION_activeDmxProcess_STFT_func5
      activeDmxProcess_STFT_func5(realizedSig[chOut], realizedSigHeadroom, realizedEnergy,
                                  erb_freq_idx_256_58, erb_freq_idx_256_58_exp, chOut_exp[chOut]);
#else
      FIXP_DBL realizedEne;
      UINT fftBand = 0;
      erb = 0;

      /* DC frequency, only real part */
      INT realized_exp = erb_freq_idx_256_58_exp[erb] + chOut_exp[chOut];
      FIXP_DBL tmpRe = realizedSig[chOut][2 * fftBand + 0];
      INT Hdr = realizedSigHeadroom[erb];
      tmpRe = fPow2Div2(tmpRe << Hdr); /* 0...0x40000000 */
      realizedEnergy[erb] = ((FIXP_DBL)(ULONG)tmpRe >> realized_exp);

      /* Nyquist frequency, only real part, counts to erb=57 */
      FIXP_DBL NyqRe = realizedSig[chOut][2 * fftBand + 1], tmpIm;

      for (erb = 1, fftBand = 1; erb < 33; erb++, fftBand++) /* can be vectorized */
      {
        realized_exp = erb_freq_idx_256_58_exp[erb] + chOut_exp[chOut];
        tmpRe = realizedSig[chOut][2 * fftBand + 0];
        tmpIm = realizedSig[chOut][2 * fftBand + 1];
        Hdr = realizedSigHeadroom[erb];
        tmpRe = fPow2Div2(tmpRe << Hdr); /* 0...0x40000000 */
        tmpIm = fPow2Div2(tmpIm << Hdr); /* 0...0x40000000 */
        FIXP_DBL tmpNrg = (tmpRe + tmpIm) ^ ((tmpRe + tmpIm) >> 31);
        realizedEnergy[erb] = (tmpNrg) >> realized_exp;
      }
      for (; erb < STFT_ERB_BANDS; erb++) {
        realized_exp = erb_freq_idx_256_58_exp[erb] + chOut_exp[chOut];
        UINT max_fftBand = erb_freq_idx_256_58[erb];
        realizedEne = (FIXP_DBL)0;
        for (; fftBand < max_fftBand; fftBand++) /* can be vectorized, 3..19 iterations */
        {
          tmpRe = realizedSig[chOut][2 * fftBand + 0];
          tmpIm = realizedSig[chOut][2 * fftBand + 1];
          Hdr = realizedSigHeadroom[erb];
          tmpRe = fPow2Div2(tmpRe << Hdr); /* 0...0x40000000 */
          tmpIm = fPow2Div2(tmpIm << Hdr); /* 0...0x40000000 */
          FIXP_DBL tmpNrg = (tmpRe + tmpIm) ^ ((tmpRe + tmpIm) >> 31);
          realizedEne += (tmpNrg) >> realized_exp;
        }
        realizedEnergy[erb] = realizedEne ^ (realizedEne >> 31);
      }
      /* Add Nyquist energy to last erb, valid variables: NyqRe, Hdr, realized_exp */
      NyqRe = fPow2Div2(NyqRe << Hdr); /* 0...0x40000000 */
      realizedEnergy[erb - 1] += ((FIXP_DBL)(ULONG)NyqRe >> realized_exp);
#endif /* FUNCTION_activeDmxProcess_STFT_func5 */

#ifdef FUNCTION_activeDmxProcess_STFT_func4
      activeDmxProcess_STFT_func4(h->realizedEnePrev[chOut], realizedEnergy, exp_diff_realized,
                                  minHeadroomRealizedEne, Alpha, One_subAlpha);
#else
      for (erb = 0; erb < STFT_ERB_BANDS; erb++) /* can be vectorized */
      {
        /*********************************************/
        /*    Energy smoothing and buffer update     */
        /*********************************************/
        FIXP_DBL summand1 = fMult(Alpha, realizedEnergy[erb]);
        FIXP_DBL summand2 = fMult(One_subAlpha, h->realizedEnePrev[chOut][erb]);
        if (exp_diff_realized[erb] >= 0)
          realizedEnergy[erb] = summand1 + (summand2 >> exp_diff_realized[erb]);
        else
          realizedEnergy[erb] = (summand1 >> (-exp_diff_realized[erb])) + summand2;
        h->realizedEnePrev[chOut][erb] = realizedEnergy[erb];
        minHeadroomRealizedEne[erb] =
            fMin(minHeadroomRealizedEne[erb], CntLeadingZeros(realizedEnergy[erb]) - 1);
      }                                          /* erb */
#endif /* FUNCTION_activeDmxProcess_STFT_func4 */
    }  /* chOut_count[chOut] != 0 */
  }    /* chOut */
  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    minHeadroomTargetEne[erb] = 31;
  }

  for (chOut = 0; chOut < numOutChans; chOut++) {
    if (chOut_count[chOut] != 0) {
      FIXP_DBL* targetEnergy = &(targetEneArr[chOut * STFT_ERB_BANDS]);

#ifdef FUNCTION_activeDmxProcess_STFT_func4
      activeDmxProcess_STFT_func4(h->targetEnePrev[chOut], targetEnergy, exp_diff,
                                  minHeadroomTargetEne, Alpha, One_subAlpha);
#else
      for (erb = 0; erb < STFT_ERB_BANDS; erb++) /* can be vectorized */
      {
        /*********************************************/
        /*    Energy smoothing and buffer update     */
        /*********************************************/
        FIXP_DBL targetEne;
        FIXP_DBL summand1 = fMult(Alpha, targetEnergy[erb]);
        FIXP_DBL summand2 = fMult(One_subAlpha, h->targetEnePrev[chOut][erb]);
        if (exp_diff[erb] >= 0)
          targetEne = summand1 + (summand2 >> exp_diff[erb]);
        else
          targetEne = (summand1 >> (-exp_diff[erb])) + summand2;
        h->targetEnePrev[chOut][erb] = targetEne;
        minHeadroomTargetEne[erb] = fMin(minHeadroomTargetEne[erb], CntLeadingZeros(targetEne) - 1);
      }
#endif /* FUNCTION_activeDmxProcess_STFT_func4 */
    }  /* chOut_count[chOut] != 0 */
  }    /* chOut */

  for (erb = 0; erb < STFT_ERB_BANDS; erb++) {
    FDK_ASSERT(minHeadroomTargetEne[erb] >= 0);
    FDK_ASSERT(minHeadroomRealizedEne[erb] >= 0);
    for (chOut = 0; chOut < h->numOutChans; chOut++) {
      if (chOut_count[chOut] != 0) {
        h->targetEnePrev[chOut][erb] =
            scaleValue(h->targetEnePrev[chOut][erb], minHeadroomTargetEne[erb]);
        h->realizedEnePrev[chOut][erb] =
            scaleValue(h->realizedEnePrev[chOut][erb], minHeadroomRealizedEne[erb]);
      }
    }

    if (exp_diff[erb] >= 0)
      h->inBufStftHeadroomPrev[erb] =
          2 * (inBufStftHeadroom[erb] + _p->STFT_headroom_prescaling) + minHeadroomTargetEne[erb];
    else
      h->inBufStftHeadroomPrev[erb] = h->inBufStftHeadroomPrev[erb] + minHeadroomTargetEne[erb];

    if (exp_diff_realized[erb] >= 0)
      h->realizedSigHeadroomPrev[erb] =
          2 * (realizedSigHeadroom[erb] + _p->STFT_headroom_prescaling) +
          minHeadroomRealizedEne[erb];
    else
      h->realizedSigHeadroomPrev[erb] =
          h->realizedSigHeadroomPrev[erb] + minHeadroomRealizedEne[erb];
  }

  if (h->fixpAES == (FIXP_DBL)0) {
    for (chOut = 0; chOut < numOutChans; chOut++) {
      if (chOut_count[chOut] != 0) {
        /* Passive downmix, AES == 0. EQ = 1.0 */
        scaleValues(realizedSig[chOut], 2 * erb_freq_idx_256_58[STFT_ERB_BANDS - 1], eq_e);
      }
    }
  } else
    for (chOut = 0; chOut < numOutChans; chOut++) {
      if (chOut_count[chOut] != 0) {
        /*********************************************/
        /*          Compute and apply EQ             */
        /*********************************************/
#ifdef FUNCTION_activeDmxProcess_STFT_func3
        activeDmxProcess_STFT_func3(h->targetEnePrev[chOut], realizedSig[chOut],
                                    h->realizedEnePrev[chOut], targetEneArr, erb_freq_idx_256_58,
                                    STFT_ERB_BANDS, h->inBufStftHeadroomPrev,
                                    h->realizedSigHeadroomPrev, eq_e);
#else
        FIXP_DBL* targetEne_ChOut = h->targetEnePrev[chOut];
        FIXP_DBL* realizedEne_ChOut = h->realizedEnePrev[chOut];
        FIXP_DBL* realizedSig_ChOut = realizedSig[chOut];

        INT EQ_exp;
        FIXP_DBL EQ;
        erb = 0;
        INT diffExp = h->realizedSigHeadroomPrev[erb] - h->inBufStftHeadroomPrev[erb];

        EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e, &EQ_exp);
        realizedSig_ChOut[0] = fMult(EQ, realizedSig_ChOut[0]) << EQ_exp;

        UINT fftBand = 1;
        for (erb = 1; erb < 33; erb++, fftBand++) {
          diffExp = h->realizedSigHeadroomPrev[erb] - h->inBufStftHeadroomPrev[erb];
          EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e,
                                &EQ_exp);
          realizedSig_ChOut[fftBand * 2 + 0] = fMult(EQ, realizedSig_ChOut[fftBand * 2 + 0])
                                               << EQ_exp;
          realizedSig_ChOut[fftBand * 2 + 1] = fMult(EQ, realizedSig_ChOut[fftBand * 2 + 1])
                                               << EQ_exp;
        }
        for (; erb < STFT_ERB_BANDS; erb++) {
          diffExp = h->realizedSigHeadroomPrev[erb] - h->inBufStftHeadroomPrev[erb];
          EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e,
                                &EQ_exp);
          UINT runs = erb_freq_idx_256_58[erb] - fftBand;
          do {
            realizedSig_ChOut[fftBand * 2 + 0] = fMult(EQ, realizedSig_ChOut[fftBand * 2 + 0])
                                                 << EQ_exp;
            realizedSig_ChOut[fftBand * 2 + 1] = fMult(EQ, realizedSig_ChOut[fftBand * 2 + 1])
                                                 << EQ_exp;
            fftBand++;
          } while (--runs != 0);
        }
        realizedSig_ChOut[1] = fMult(EQ, realizedSig_ChOut[1]) << EQ_exp;
#endif  /* FUNCTION_activeDmxProcess_STFT_func3 */
      } /* if ( chOut_count[chOut] != 0 ) */
    }   /* for(chOut = 0; chOut < numOutChans; chOut++) */
}
#endif /* #ifndef FUNCTION_activeDmxProcess_STFT */

/*****************************************************************************************************************/
void activeDmxSetAES(INT AES, void* handle) {
  activeDownmixer* h = (activeDownmixer*)handle;
  FDK_ASSERT(AES <= 7);
  h->fixpAES = (FIXP_DBL)(AES * 0x09249249 + 1) >>
               1; /* fixpAES = AES/7.0f   change format from INT to Q2.29 */
}
/*****************************************************************************************************************/
