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

/******************* Library for basic calculation routines ********************

   Author(s):   Manuel Jander

   Description: LPC related functions

*******************************************************************************/

#include "FDK_lpc.h"

/* Internal scaling of LPC synthesis to avoid overflow of filte states.
   This depends on the LPC order, because the LPC order defines the amount
   of MAC operations. */
static const SCHAR order_ld[LPC_MAX_ORDER] = {
    /* Assume that Synthesis filter output does not clip and filter
       accu does change no more than 1.0 for each iteration. ceil(0.5*log((1:24))/log(2)) */
    0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};

#if defined(__arm__)
#include "arm/FDK_lpc_arm.cpp"
#endif

/* IIRLattice */
#ifndef FUNCTION_CLpc_SynthesisLattice_SGL
void CLpc_SynthesisLattice(FIXP_DBL* signal, const int signal_size, const int signal_e,
                           const int signal_e_out, const int inc, const FIXP_SGL* coeff,
                           const int order, FIXP_DBL* state) {
  int i, j;
  FIXP_DBL* pSignal;
  int shift;

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(order > 0);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /*
    tmp = x(k) - K(M)*g(M);
    for m=M-1:-1:1
            tmp = tmp - K(m) * g(m);
            g(m+1) = g(m) + K(m) * tmp;
    endfor
    g(1) = tmp;

    y(k) = tmp;
  */

  shift = -order_ld[order - 1];

  for (i = signal_size; i != 0; i--) {
    FIXP_DBL* pState = state + order - 1;
    const FIXP_SGL* pCoeff = coeff + order - 1;
    FIXP_DBL tmp;

    tmp = scaleValue(*pSignal, shift + signal_e) - fMultDiv2(*pCoeff--, *pState--);
    for (j = order - 1; j != 0; j--) {
      tmp = fMultSubDiv2(tmp, pCoeff[0], pState[0]);
      pState[1] = pState[0] + (fMultDiv2(*pCoeff--, tmp) << 2);
      pState--;
    }

    *pSignal = scaleValueSaturate(tmp, -shift - signal_e_out);

    /* exponent of state[] is -1 */
    pState[1] = tmp << 1;
    pSignal += inc;
  }
}
#endif

#ifndef FUNCTION_CLpc_SynthesisLattice_DBL
void CLpc_SynthesisLattice(FIXP_DBL* signal, const int signal_size, const int signal_e,
                           const int signal_e_out, const int inc, const FIXP_DBL* coeff,
                           const int order, FIXP_DBL* state) {
  int i, j;
  FIXP_DBL* pSignal;

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(order > 0);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  FDK_ASSERT(signal_size > 0);
  for (i = signal_size; i != 0; i--) {
    FIXP_DBL* pState = state + order - 1;
    const FIXP_DBL* pCoeff = coeff + order - 1;
    FIXP_DBL tmp;

    FIXP_DBL accu;
    accu = fMultSubDiv2(scaleValue(*pSignal, signal_e - 1), *pCoeff--, *pState--);
    tmp = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);
    for (j = order - 1; j != 0; j--) {
      accu = fMultSubDiv2(tmp >> 1, pCoeff[0], pState[0]);
      tmp = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);

      accu = fMultAddDiv2(pState[0] >> 1, *pCoeff--, tmp);
      pState[1] = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);
      pState--;
    }

    *pSignal = scaleValue(tmp, -signal_e_out);

    /* exponent of state[] is 0 */
    pState[1] = tmp;
    pSignal += inc;
  }
}

#endif

/* LPC_SYNTHESIS_IIR version */
#ifndef FUNCTION_CLpc_Synthesis_DBL

void CLpc_Synthesis(FIXP_DBL* signal, const int signal_size, const int inc,
                    const FIXP_LPC_TNS* lpcCoeff_m, const int lpcCoeff_e, const int order,
                    FIXP_DBL* state, int* pStateIndex) {
  int i, j;
  FIXP_DBL* pSignal;
  int stateIndex = *pStateIndex;
  int lpcCoeffShift = lpcCoeff_e + 1;

  FIXP_LPC_TNS coeff[2 * LPC_MAX_ORDER];
  FDKmemcpy(&coeff[0], lpcCoeff_m, order * sizeof(FIXP_LPC_TNS));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order * sizeof(FIXP_LPC_TNS));

  FDK_ASSERT(lpcCoeffShift >= 0);
  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(stateIndex < order);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */

  for (i = 0; i < signal_size; i++) {
    FIXP_DBL x;
    const FIXP_LPC_TNS* pCoeff = coeff + order - stateIndex;

    x = (*pSignal) >> lpcCoeffShift;
    for (j = 0; j < order; j++) {
      x = fMultSubDiv2(x, state[j], pCoeff[j]);
    }
    x = SATURATE_LEFT_SHIFT(x, lpcCoeffShift, DFRACT_BITS);

    /* Update states */
    stateIndex = ((stateIndex - 1) < 0) ? (order - 1) : (stateIndex - 1);
    state[stateIndex] = x;

    *pSignal = x;
    pSignal += inc;
  }
  *pStateIndex = stateIndex;
}
#endif /* #ifndef FUNCTION_CLpc_Synthesis_DBL */
/* default version */
void CLpc_Synthesis(FIXP_DBL* signal, const int signal_size, const int inc,
                    const FIXP_LPC* lpcCoeff_m, const int lpcCoeff_e, const int order,
                    FIXP_DBL* state, int* pStateIndex) {
  int i, j;
  FIXP_DBL* pSignal;
  int stateIndex = *pStateIndex;
  int lpcCoeffShift = lpcCoeff_e + 1;

  FIXP_LPC coeff[2 * LPC_MAX_ORDER];
  FDKmemcpy(&coeff[0], lpcCoeff_m, order * sizeof(FIXP_LPC));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order * sizeof(FIXP_LPC));

  FDK_ASSERT(lpcCoeffShift >= 0);
  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(stateIndex < order);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */

  for (i = 0; i < signal_size; i++) {
    FIXP_DBL x;
    const FIXP_LPC* pCoeff = coeff + order - stateIndex;

    x = (*pSignal) >> lpcCoeffShift;
    for (j = 0; j < order; j++) {
      x = fMultSubDiv2(x, state[j], pCoeff[j]);
    }
    x = SATURATE_LEFT_SHIFT(x, lpcCoeffShift, DFRACT_BITS);

    /* Update states */
    stateIndex = ((stateIndex - 1) < 0) ? (order - 1) : (stateIndex - 1);
    state[stateIndex] = x;

    *pSignal = x;
    pSignal += inc;
  }

  *pStateIndex = stateIndex;
}

#define AUTOCORR_DYNAMICSCALE_ENABLE
//#define AUTOCORRELATION_NORMALIZE_ENABLE

FIXP_DBL CLpc_AutoCorr(const FIXP_DBL* signal_m, const int signal_e, FIXP_DBL* acorr, INT* pAcorr_e,
                       const int signal_size, const int order) {
  int l, i, scale;
  FIXP_DBL norm_m = (FIXP_DBL)0, nrg;

  FDK_ASSERT(signal_m != acorr);

  scale = 0;

  for (i = 0; i < signal_size; i++) {
    if (norm_m > FL2FXCONST_DBL(0.5) ||
        (signal_m[i] >> scale) > FL2FXCONST_DBL(0.70710678118654752440084436210485)) {
      scale++;
      norm_m >>= 1;
    }
    norm_m += fPow2(signal_m[i]) >> scale;
  }
  nrg = norm_m;
  acorr[0] = nrg;

  for (l = 1; l < order; l++) {
    FIXP_DBL result = FL2FXCONST_DBL(0.f);

    for (i = 0; i < signal_size - l; i++) {
      result += fMult(signal_m[i], signal_m[i + l]) >> scale;
    }
    acorr[l] = result;
  }

  *pAcorr_e = signal_e * 2 + scale;

  return nrg;
}

/* For the LPC_SYNTHESIS_IIR version */
INT CLpc_ParcorToLpc(const FIXP_LPC_TNS reflCoeff[], FIXP_LPC_TNS LpcCoeff[], const int numOfCoeff,
                     FIXP_QDL workBuffer[]) {
  INT i, j;
  INT shiftval = (INT)0;
  FIXP_QDL maxVal = (FIXP_QDL)0;

  workBuffer[0] = (FIXP_QDL)FX_LPC_TNS2FX_DBL(reflCoeff[0]);
  for (i = 1; i < numOfCoeff; i++) {
    for (j = 0; j < i / 2; j++) {
      FIXP_QDL tmp1, tmp2;

      tmp1 = workBuffer[j];
      tmp2 = workBuffer[i - 1 - j];
      workBuffer[j] += fMult(reflCoeff[i], tmp2);
      workBuffer[i - 1 - j] += fMult(reflCoeff[i], tmp1);
    }
    if (i & 1) {
      workBuffer[j] += fMult(reflCoeff[i], workBuffer[j]);
    }

    workBuffer[i] = (FIXP_QDL)FX_LPC_TNS2FX_DBL(reflCoeff[i]);
  }

  /* calculate exponent */
  for (i = 0; i < numOfCoeff; i++) {
    maxVal = fMax(maxVal, fAbs(workBuffer[i]));
  }

  if (maxVal > (FIXP_QDL)MAXVAL_DBL) {
    shiftval = (DFRACT_BITS - 1) - fNorm((FIXP_DBL)(maxVal >> (DFRACT_BITS - 1)));
  }

  for (i = 0; i < numOfCoeff; i++) {
    LpcCoeff[i] = FX_DBL2FX_LPC_TNS((FIXP_DBL)(workBuffer[i] >> shiftval));
  }

  return (shiftval);
}

/* For the LPC_SYNTHESIS_IIR version */
INT CLpc_ParcorToLpc(const FIXP_LPC reflCoeff[], FIXP_LPC LpcCoeff[], const int numOfCoeff,
                     FIXP_QDL workBuffer[]) {
  INT i, j;
  INT shiftval = (INT)0;
  FIXP_QDL maxVal = (FIXP_QDL)0;

  workBuffer[0] = (FIXP_QDL)FX_LPC2FX_DBL(reflCoeff[0]);
  for (i = 1; i < numOfCoeff; i++) {
    for (j = 0; j < i / 2; j++) {
      FIXP_QDL tmp1, tmp2;

      tmp1 = workBuffer[j];
      tmp2 = workBuffer[i - 1 - j];
      workBuffer[j] += fMult(FX_LPC2FX_DBL(reflCoeff[i]), tmp2);
      workBuffer[i - 1 - j] += fMult(FX_LPC2FX_DBL(reflCoeff[i]), tmp1);
    }
    if (i & 1) {
      workBuffer[j] += fMult(FX_LPC2FX_DBL(reflCoeff[i]), workBuffer[j]);
    }

    workBuffer[i] = (FIXP_QDL)FX_LPC2FX_DBL(reflCoeff[i]);
  }

  /* calculate exponent */
  for (i = 0; i < numOfCoeff; i++) {
    maxVal = fMax(maxVal, fAbs(workBuffer[i]));
  }

  if (maxVal > (FIXP_QDL)MAXVAL_DBL) {
    shiftval = (DFRACT_BITS - 1) - fNorm((FIXP_DBL)(maxVal >> (DFRACT_BITS - 1)));
  }

  for (i = 0; i < numOfCoeff; i++) {
    LpcCoeff[i] = FX_DBL2FX_LPC((FIXP_DBL)(workBuffer[i] >> shiftval));
  }

  return (shiftval);
}

void CLpc_AutoToParcor(FIXP_DBL acorr[], const int acorr_e, FIXP_LPC reflCoeff[],
                       const int numOfCoeff, FIXP_DBL* pPredictionGain_m, INT* pPredictionGain_e) {
  INT i, j, scale = 0;
  FIXP_DBL parcorWorkBuffer[LPC_MAX_ORDER];

  FIXP_DBL* workBuffer = parcorWorkBuffer;
  FIXP_DBL autoCorr_0 = acorr[0];

  FDKmemclear(reflCoeff, numOfCoeff * sizeof(FIXP_LPC));

  if (autoCorr_0 == FL2FXCONST_DBL(0.0)) {
    if (pPredictionGain_m != NULL) {
      *pPredictionGain_m = FL2FXCONST_DBL(0.5f);
      *pPredictionGain_e = 1;
    }
    return;
  }

  FDKmemcpy(workBuffer, acorr + 1, numOfCoeff * sizeof(FIXP_DBL));
  for (i = 0; i < numOfCoeff; i++) {
    LONG sign = ((LONG)workBuffer[0] >> (DFRACT_BITS - 1));
    FIXP_DBL tmp = (FIXP_DBL)((LONG)workBuffer[0] ^ sign);

    /* Check preconditions for division function: num<=denum             */
    /* For 1st iteration acorr[0] cannot be 0, it is checked before loop */
    /* Due to exor operation with "sign", num(=tmp) is greater/equal 0   */
    if (acorr[0] < tmp) break;

    /* tmp = div(num, denum, 16) */
    tmp = (FIXP_DBL)((LONG)schur_div(tmp, acorr[0], FRACT_BITS) ^ (~sign));

    reflCoeff[i] = FX_DBL2FX_LPC(tmp);

    for (j = numOfCoeff - i - 1; j >= 0; j--) {
      FIXP_DBL accu1 = fMult(tmp, acorr[j]);
      FIXP_DBL accu2 = fMult(tmp, workBuffer[j]);
      workBuffer[j] += accu1;
      acorr[j] += accu2;
    }
    /* Check preconditions for division function: denum (=acorr[0]) > 0 */
    if (acorr[0] == (FIXP_DBL)0) break;

    workBuffer++;
  }

  if (pPredictionGain_m != NULL) {
    if (acorr[0] > (FIXP_DBL)0) {
      /* prediction gain = signal power / error (residual) power */
      *pPredictionGain_m = fDivNormSigned(autoCorr_0, acorr[0], &scale);
      *pPredictionGain_e = scale;
    } else {
      *pPredictionGain_m = (FIXP_DBL)0;
      *pPredictionGain_e = 0;
    }
  }
}

void CLpc_AutoToLpcIGF(FIXP_DBL* lpcCoeff_m, INT* lpcCoeff_e, const FIXP_DBL* R, const INT R_e,
                       const int m, FIXP_DBL* rc) {
  FIXP_DBL a_m[LPC_MAX_ORDER];
  INT a_e[LPC_MAX_ORDER], a_e_max = 0;
  FIXP_DBL err_m, g_m, s_m;
  INT s_e;
  INT g_e, err_e;
  int p, t, i;

  /* reflection coefficient retrieval is not implemented */
  FDK_ASSERT(rc == NULL);
  FDK_ASSERT(m <= LPC_MAX_ORDER);

  p = m;

  a_m[0] = -fDivNormSigned(R[1], R[0]);
  a_e[0] = 0;

  /* err = R[0] + R[1] * a[0] = R[0] + R[1]*R[1]/R[0] */
  err_m = fAddNorm(R[0], R_e, fMult(R[1], a_m[0]), R_e + a_e[0], &err_e);

  /*rc[0] = a[0];*/

  for (t = 1; t < p; t++) {
    s_m = R[t + 1];
    s_e = R_e;
    for (i = 0; i < t; i++) {
      s_m = fAddNorm(s_m, s_e, fMult(a_m[i], R[t - i]), a_e[i] + R_e, &s_e);
    }
    /* tmp_e = m, v_e = m */
    g_m = fDivNormSigned(s_m, err_m, &g_e);
    g_m = -g_m; /* negate after division to circumvent possible overflow of -s_m */
    g_e += s_e - err_e;

    /* a = [ a + g * fliplr(a), g ]; */
    for (i = 0; i < t / 2; i++) {
      FIXP_DBL a0_m, a1_m;
      INT a0_e, a1_e;

      a0_m = a_m[i];
      a1_m = a_m[t - 1 - i];
      a0_e = a_e[i];
      a1_e = a_e[t - 1 - i];

      a_m[i] = fAddNorm(a0_m, a0_e, fMult(a1_m, g_m), a1_e + g_e, &a_e[i]);
      a_m[t - 1 - i] = fAddNorm(fMult(a0_m, g_m), a0_e + g_e, a1_m, a1_e, &a_e[t - 1 - i]);
    }
    if (t & 1) {
      a_m[i] = fAddNorm(a_m[i], a_e[i], fMult(a_m[i], g_m), a_e[i] + g_e, &a_e[i]);
    }

    a_m[t] = g_m;
    a_e[t] = g_e;

    /* err = err * (1 - g * g) = err - err*g*g = err + s * g; */
    err_m = fAddNorm(err_m, err_e, fMult(s_m, g_m), s_e + g_e, &err_e);

    /*rc[t] = g;*/
  }

  for (t = 0; t < p; t++) {
    a_e_max = fMax(a_e_max, a_e[t]);
  }

  for (t = 0; t < p; t++) {
    INT scale = a_e_max - a_e[t];
    if (scale < DFRACT_BITS) {
      lpcCoeff_m[t] = a_m[t] >> scale;
    } else {
      lpcCoeff_m[t] = (FIXP_DBL)0;
    }
  }
  *lpcCoeff_e = a_e_max;
}

void CLpc_AutoToLpc(FIXP_LPC* lpcCoeff_m, INT* lpcCoeff_e, const FIXP_DBL* R, const INT R_e,
                    const int m, FIXP_DBL* rc, const int is_ltpf) {
  FIXP_DBL a_m[LPC_MAX_ORDER];
  INT a_e[LPC_MAX_ORDER], a_e_max = 0;
  FIXP_DBL err_m, g_m, s_m;
  FIXP_DBL sigma2_m;
  INT s_e;
  INT g_e, err_e;
  INT sigma2_e;
  int p, t, i;

  FIXP_DBL rc_m[LPC_MAX_ORDER];
  int rc_e[LPC_MAX_ORDER];
  FIXP_DBL acorr_nrg;
  int acorr_headroom;

  acorr_headroom = fNorm(R[0]);
  acorr_nrg = R[0] << acorr_headroom;

  FDK_ASSERT(rc == NULL);
  FDK_ASSERT(m <= LPC_MAX_ORDER);

  p = m;

  rc_m[0] = -fDivNormSigned(R[1] << acorr_headroom, acorr_nrg); /* rc[0] = -r[1] / r[0]  */
  a_m[0] = rc_m[0];
  a_e[0] = 0;
  sigma2_m = (acorr_nrg >> 1) +
             fMultDiv2(R[1] << acorr_headroom, rc_m[0]); /* sigma2 = r[0] + r[1] * rc[0] */
  sigma2_e = R_e + 1 - acorr_headroom;                   /* + 1  due to fMultDiv2 */

  /* err = acorr_nrg + R[1] * a[0] = acorr_nrg + R[1]*R[1]/acorr_nrg */
  err_m = fAddNorm(acorr_nrg, R_e, fMult(R[1] << acorr_headroom, a_m[0]), R_e + a_e[0],
                   &err_e); /* err = CC[0] + CC[1] * RC[0]; */

  for (t = 1; t < p; t++) {
    a_e[t] = 0;
    a_m[t] = (FIXP_DBL)0;
    s_m = R[t + 1] << acorr_headroom;
    s_e = R_e;
    for (i = 0; i < t; i++) {
      s_m = fAddNorm(s_m, s_e, fMult(a_m[i], R[t - i] << acorr_headroom), a_e[i] + R_e,
                     &s_e); /* Sum */
    }
    /* tmp_e = m, v_e = m */
    g_m = fDivNormSigned(s_m, err_m, &g_e); /* RC[Loop-1] = -Sum/Sigma2 */
    g_m = -g_m; /* negate after division to circumvent possible overflow of -s_m */

    g_e += s_e - err_e; /* RC[Loop-1] = -Sum/Sigma2 */
    rc_m[t] = g_m;
    rc_e[t] = g_e;

    if (is_ltpf) {
      int rcXrc_e, sigma2xRCxRC_e;
      FIXP_DBL rcXrc_m, sigma2xRCxRC_m;

      /*  Sigma2 = Sigma2*(1.0f - RC[Loop-1]*RC[Loop-1]) ) = Sigma2 - Sigma2*RC[Loop-1]*RC[Loop-1])
       * )  */
      rcXrc_m = fPow2Div2(g_m);
      rcXrc_e = g_e * 2 + 1;
      sigma2xRCxRC_m = fMult(sigma2_m, rcXrc_m);
      sigma2xRCxRC_e = sigma2_e + rcXrc_e;
      sigma2_m = fAddNorm(sigma2_m, sigma2_e, -sigma2xRCxRC_m, sigma2xRCxRC_e, &sigma2_e);

      /* if (Sigma2<=1.0E-09f) */
      INT result_e;
      FIXP_DBL result_m = fAddNorm(sigma2_m, sigma2_e, FL2FXCONST_DBL(-1.0E-09f), 0, &result_e);
      if (result_m <= (FIXP_DBL)0) {
        /*   The autocorrelation matrix is not positive definite   */
        /*   The linear prediction filter corresponds to the   */
        /*   autocorrelation sequence truncated to the point   */
        /*   where it is positive definite.                    */
        /*   Sigma2 = 1.0E-09f;
        for (i=Loop; i<=Order; i++)
        {
        RC[i-1] = 0.0f;
        LPC[i] = 0.0f;
        } */
        for (i = 0; i < p; i++) {
          rc_m[i] = FL2FXCONST_DBL(0.0f);
          rc_e[i] = 0;
          a_m[i] = FX_DBL2FX_LPC(0.0f);
          a_e[i] = 0;
        }
        break;
      }
    }

    /*   Update Energy of the prediction residual   */

    /* a = [ a + g * fliplr(a), g ]; */
    for (i = 0; i < t / 2; i++) {
      FIXP_DBL a0_m, a1_m;
      INT a0_e, a1_e;

      a0_m = a_m[i];
      a1_m = a_m[t - 1 - i];
      a0_e = a_e[i];
      a1_e = a_e[t - 1 - i];

      a_m[i] = fAddNorm(a0_m, a0_e, fMult(a1_m, rc_m[t]), a1_e + rc_e[t], &a_e[i]);
      a_m[t - 1 - i] = fAddNorm(fMult(a0_m, rc_m[t]), a0_e + rc_e[t], a1_m, a1_e, &a_e[t - 1 - i]);
    }
    if (t & 1) {
      a_m[i] = fAddNorm(a_m[i], a_e[i], fMult(a_m[i], rc_m[t]), a_e[i] + rc_e[t], &a_e[i]);
    }

    a_m[t] = rc_m[t];
    a_e[t] = rc_e[t];

    /* err = err * (1 - g * g) = err - err*g*g = err + s * g; */
    err_m = fAddNorm(err_m, err_e, fMult(s_m, rc_m[t]), s_e + rc_e[t], &err_e);

    /*rc[t] = g;*/
  }

  for (t = 0; t < p; t++) {
    a_e_max = fMax(a_e_max, a_e[t]);
  }

  for (t = 0; t < p; t++) {
    INT scale = a_e_max - a_e[t];
    if (scale < DFRACT_BITS) {
      lpcCoeff_m[t] = FX_DBL2FX_LPC(a_m[t] >> scale);
    } else {
      lpcCoeff_m[t] = (FIXP_DBL)0;
    }
  }
  *lpcCoeff_e = a_e_max;
  if (a_e_max > 15) {
    for (t = 0; t < p; t++) {
      lpcCoeff_m[t] = (FIXP_DBL)0;
    }
    *lpcCoeff_e = 0;
  }
}
