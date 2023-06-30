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

   Author(s):   Sebastian Weller, Arthur Tritthart

   Description: LPC related functions

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_AARCH64_NEON__)
#include "arm/FDK_aarch64_neon_funcs.h"
#endif

#ifdef FUNCTION_CLpc_SynthesisLattice_neonv8_order_10to13
A64_ASM_ROUTINE_START(
void, CLpc_SynthesisLattice_neonv8_order_10to13, (FIXP_DBL *signal, const INT signal_size, const INT signal_e, const INT signal_e_out, INT64 inc, const FIXP_DBL *coeff, const INT order, FIXP_DBL *state))
#ifndef __ARM_AARCH64_NEON__
//  debug registers:                        V24..V27 (e.g. intermediate vec_tmp 0.1.2)
//  int vec_oshl[4];                        V23 (-signal_e_out)
//  int vec_ishl[4];                        V22 ( signal_e)
//  int vec_invtmask[4] = {-1, -1, -1, -1}; V21: vec_intvmask[lane] =  0
//  int vec_lanemask[4] = { 0,  0,  0,  0}; V20: vec_lanemask[lane] = -1
//                      C-code sorted:      NEON sorted:
//  int vec_casc[4][4] = {{1,1,1,1},        V16: 1, 1, 1, 1
//                        {1,1,1,0},        V17: 0, 1, 1, 1
//                        {1,1,0,0},        V18: 0, 1, 1, 1
//                        {1,0,0,0}};       V19: 0, 0, 0, 1
//  FIXP_DBL vec_coeff[4][4];               V9-V12: vec_coeff[0]..vec_state[3]
//  FIXP_DBL vec_state[4][4];               V2-V5:  vec_state[0]..vec_state[3]
//  FIXP_DBL vec_mul[4];                    V1
//  FIXP_DBL vec_tmp[4];                    V0

  /* Assign call parameter to registers */
  X0 = (INT64) signal;
  W1 = signal_size;  /* used as loop counter */
  W2 = signal_e;     /* used as ishl */
  W3 = signal_e_out; /* used as oshl */
  X4 = inc;
  X5 = (INT64) coeff;
  W6 = order;  /* used for: lane = (1-order) & 3 */
  X7 = (INT64) state;
  /* preserve local variables */
//      INT64     X8;      /* used to write-back final state[0..order-1] */
//      INT       W9;      /* used as ishr/oshl */
//      INT64    *X10, *X11;     /* for debug only */
#endif

  /* Push preserved SIMD register (bottom part) accoding to calling convention */
  A64_pushD( D9, D10)
  A64_pushD(D11, D12)

  A64_mov_Xt(X8, X7)       /* X8: &state[0] */

  /* int lane = (1 - order) & 3; */
  A64_mov_Wt_imm(W9, 1)
  A64_sub_Wt(W6, W9, W6)
  A64_and_Wt_imm(W6, W6, 3)

  /* Preload vec_lanemask,  and vec_invtmask for lane=0            */
  /* Preload vec_casc: V16: 1 1 1 1, V17: 0 1 1 1,  V18: 0 0 1 1, V19: 0 0 0 1 */
  A64_movi(8, 128, V20_16B,  0)               /* V20: vec_lanemask[0..3] =  0 */
  A64_movi(8, 128, V21_16B, 0xFF)             /* V21: vec_invtmask[0..3] = -1 */
  A64_sub(32, 128, V16_4S, V20_4S, V21_4S)    /* V16:     1  1  1  1          */
  A64_ext(8, 128,V17_16B,V16_16B,V20_16B, 4)  /* V17:     0  1  1  1          */
  A64_ext(8, 128,V18_16B,V16_16B,V20_16B, 8)  /* V18:     0  0  1  1          */
  A64_ext(8, 128,V19_16B,V16_16B,V20_16B, 12) /* V19:     0  0  0  1          */
  A64_ext(8, 128,V20_16B,V21_16B,V20_16B, 12) /* V20: vec_lanemask[0] = -1    */
  A64_ext(8, 128,V21_16B,V20_16B,V21_16B, 12) /* V21: vec_invtmask[0] =  0    */

  A64_cmp_Wt_imm(W6, 1)
  A64_branch(EQ, case_ld_lane1)
  A64_cmp_Wt_imm(W6, 2)
  A64_branch(EQ, case_ld_lane2)
  A64_cmp_Wt_imm(W6, 3)
  A64_branch(EQ, case_ld_lane3)
//A64_label(case_ld_lane0)
  A64_ld1x1_IA(32, 128, V9_4S, X5, 16)
  A64_ld1x1_IA(32, 128, V2_4S, X7, 16)
  A64_branch(, case_ld_lane_end)
A64_label(case_ld_lane3)
  A64_ld1_lane_IA(32, V9_S, 3, X5, 4)         /* vec_coeff[0][3] = coeff[0] */
  A64_ld1_lane_IA(32, V2_S, 3, X7, 4)         /* vec_state[0][3] = state[0] */
  A64_ext(8, 128,V20_16B,V20_16B,V20_16B,  4) /* vec_lanemask[3] = -1;      */
  A64_ext(8, 128,V21_16B,V21_16B,V21_16B,  4) /* vec_invtmask[3] =  0;      */
  A64_branch(, case_ld_lane_end)
A64_label(case_ld_lane2)
  A64_ld1x1_IA(32, 64, V9_2S, X5, 8)          /* vec_coeff[0][0,1] = coeff[0,1] */
  A64_ld1x1_IA(32, 64, V2_2S, X7, 8)          /* vec_state[0][0,1] = state[0,1] */
  A64_ext(8, 128, V9_16B, V9_16B, V9_16B,  8) /* vec_coeff[0][2,3] = coeff[0,1] */
  A64_ext(8, 128, V2_16B, V2_16B, V2_16B,  8) /* vec_state[0][2,3] = state[0,1] */
  A64_ext(8, 128,V20_16B,V20_16B,V20_16B,  8) /* vec_lanemask[2] = -1; */
  A64_ext(8, 128,V21_16B,V21_16B,V21_16B,  8) /* vec_invtmask[2] =  0; */
  A64_branch(, case_ld_lane_end)
A64_label(case_ld_lane1)
  A64_ld1x1_IA(32, 64, V9_2S, X5, 8)
  A64_ld1x1_IA(32, 64, V2_2S, X7, 8)
  A64_ld1_lane_IA(32, V9_S, 2, X5, 4)
  A64_ld1_lane_IA(32, V2_S, 2, X7, 4)
  A64_ext(8, 128, V9_16B, V9_16B, V9_16B, 12)
  A64_ext(8, 128, V2_16B, V2_16B, V2_16B, 12)
  A64_ext(8, 128,V20_16B,V20_16B,V20_16B, 12) /* vec_lanemask[1] = -1; */
  A64_ext(8, 128,V21_16B,V21_16B,V21_16B, 12) /* vec_invtmask[1] =  0; */
A64_label(case_ld_lane_end)

  A64_ld1x2_IA(32, 128, V10_4S, V11_4S, X5, 32)  /* vec_coeff[1,2][0..3] = coeff[4 - lane + 0..7]; */
  A64_ld1x2_IA(32, 128,  V3_4S,  V4_4S, X7, 32)  /* vec_state[1,2][0..3] = state[4 - lane + 0..7]; */

  /* Load last coeff and state into lane 0 of next coeffs/states vector, clear other lanes */
  A64_movi(8, 128,  V5_16B, 0)                   /* vec_coeff[3][0..3] =  0           */
  A64_movi(8, 128, V12_16B, 0)                   /* vec_state[3][0..3] =  0           */
  A64_ld1_lane_IA(32, V12_S, 0, X5, 4)           /* vec_coeff[3][0] =  coeff[order-1] */
  A64_ld1_lane_IA(32,  V5_S, 0, X7, 4)           /* vec_state[3][0] =  state[order-1] */

  /* if (inc == -1)  pSignal += signal_size - 1;   <= pointer preset by caller */
  A64_lsl_Xt_imm(X4, X4, 2)
  A64_dup_Wt(32, 128, V22_4S, W2)  /* V22: ishl */
  A64_sub_Wt(W3, WZR, W3)
  A64_dup_Wt(32, 128, V23_4S, W3)  /* V23: oshl */

  /*outer loop for all the signals */
A64_label(CLpc_SynthesisLattice_neonv8_order_10to13_loop)

  /* Load next sample into lane 0, apply shift right/left */
  A64_ld1_lane(32, V0_S, 0, X0)
  A64_sshl(32, 128, V0_4S, V0_4S, V22_4S)       /* V0:   ????????  ????????  ????????  scaleValue(signal[i],signal_e) */
  A64_sqdmulh(32, 128, V1_4S, V5_4S, V12_4S)    /* V1:   00000000  00000000  00000000  fMult(coeff[i],state[i]) */
  A64_sub(32, 128, V0_4S, V0_4S, V1_4S)         /* V0:   ????????  ????????  ????????  scaleValue(signal[i],signal_e) - fMult(coeff[i],state[i]) */

  /* Iteration: j == 2: duplicate tmp           */
  /* Build 4 products: fMult(coeff[i],state[i]) */
  /* Subtract product[3] from tmp lanes 3,2,1,0 */
  /* Subtract product[2] from tmp lanes   2,1,0 */
  /* Subtract product[1] from tmp lanes     1,0 */
  /* Subtract product[0] from tmp lane        0 */
  /* Build 4 products: fMult(coeff[i--], tmp)   */
  /* Update: Add products to states             */
  /* shift pos. of states by 1, insert lane 3   */
  /* Iteration: j == 2                          */
  A64_dup_lane(32, 128, V0_4S, V0_4S, 0)
  A64_sqdmulh( 32, 128, V1_4S, V4_4S, V11_4S)
  A64_mls_lane(32, 128, V0_4S, V16_4S, V1_4S, 3)
  A64_mls_lane(32, 128, V0_4S, V17_4S, V1_4S, 2)
  A64_mls_lane(32, 128, V0_4S, V18_4S, V1_4S, 1)
  A64_mls_lane(32, 128, V0_4S, V19_4S, V1_4S, 0)
  A64_sqdmulh( 32, 128, V1_4S, V0_4S, V11_4S)
  A64_add(     32, 128, V4_4S, V4_4S,  V1_4S)
  A64_ext(      8, 128, V5_16B,V4_16B,V5_16B, 12)

  /* Iteration: j == 1                          */
  A64_dup_lane(32, 128, V0_4S, V0_4S,  0)
  A64_sqdmulh( 32, 128, V1_4S, V3_4S, V10_4S)
  A64_mls_lane(32, 128, V0_4S, V16_4S, V1_4S, 3)
  A64_mls_lane(32, 128, V0_4S, V17_4S, V1_4S, 2)
  A64_mls_lane(32, 128, V0_4S, V18_4S, V1_4S, 1)
  A64_mls_lane(32, 128, V0_4S, V19_4S, V1_4S, 0)
  A64_sqdmulh( 32, 128, V1_4S, V0_4S, V10_4S)
  A64_add(     32, 128, V3_4S, V3_4S,  V1_4S)
  A64_ext(      8, 128, V4_16B,V3_16B,V4_16B, 12)

  /* Iteration: j == 0                          */
  A64_dup_lane(32, 128, V0_4S, V0_4S,  0)
  A64_sqdmulh( 32, 128, V1_4S, V2_4S,  V9_4S)
  A64_mls_lane(32, 128, V0_4S, V16_4S, V1_4S, 3)
  A64_mls_lane(32, 128, V0_4S, V17_4S, V1_4S, 2)
  A64_mls_lane(32, 128, V0_4S, V18_4S, V1_4S, 1)
  A64_mls_lane(32, 128, V0_4S, V19_4S, V1_4S, 0)
  A64_sqdmulh( 32, 128, V1_4S, V0_4S,  V9_4S)
  A64_add(     32, 128, V2_4S, V2_4S,  V1_4S)
  A64_ext(      8, 128, V3_16B,V2_16B,V3_16B, 12)

  /* shift-left lowest states by 1 position, lane 0 is dummy */
  A64_ext(8, 128, V2_16B, V2_16B, V2_16B, 12)

  /* get last tmp by clearing all other lanes: AND */
  A64_and(8, 128, V0_16B, V0_16B, V20_16B)

  /* get tmp into lane 0 by adding all lanes into one: ADDV */
  A64_addv(32, 128, S1, V0_4S)

  /* Scale output sample and store it with pointer update by +inc */
  A64_sshl(32, 64, V1_2S, V1_2S, V23_2S)   /* V1:   ????????  ????????  ????????  scaleValue(tmp,-signal_e_out) */
  A64_st1_lane_PU(32, V1_S, 0, X0, X4)

  /* mask shifted state vector, lower states are cleared -> AND */
  A64_and( 8, 128, V2_16B, V2_16B, V21_16B)

  /* add actual tmp to state vector, i.e. state[0] = tmp -> ADD */
  A64_add(32, 128, V2_4S, V2_4S, V0_4S)

  A64_subs_imm(W1, W1, 1)
  A64_branch(NE, CLpc_SynthesisLattice_neonv8_order_10to13_loop)

  /* store states for next run */

  A64_cmp_Wt_imm(W6, 1)
  A64_branch(EQ, case_st_lane1)
  A64_cmp_Wt_imm(W6, 2)
  A64_branch(EQ, case_st_lane2)
  A64_cmp_Wt_imm(W6, 3)
  A64_branch(EQ, case_st_lane3)
// A64_label(case_st_lane0)
  A64_st1x1_IA(32, 128, V2_4S, X8, 16)  /* state[0..3] = vec_state[0][0..3]; */
  A64_branch(, case_st_lane_end)

A64_label(case_st_lane3)
  A64_st1_lane_IA(32, V2_S, 3, X8, 4)   /* state[0] = vec_state[0][3]; */
  A64_branch(, case_st_lane_end)

A64_label(case_st_lane2)
  A64_ext(8, 128, V2_16B, V2_16B, V2_16B, 8)
  A64_st1x1_IA(32, 64, V2_2S, X8, 8)    /* state[0,1] = vec_state[0][2,3]; */
  A64_branch(, case_st_lane_end)

A64_label(case_st_lane1)
  A64_st1_lane_IA(32, V2_S, 1, X8, 4)   /* state[0] = vec_state[0][1]; */
  A64_ext(8, 128, V2_16B, V2_16B, V2_16B, 8) /* vec_state[0][0,1] = vec_state[0][2,3] */
  A64_st1x1_IA(32, 64, V2_2S, X8, 8)    /* state[0,1] = vec_state[0][1,2]; */

A64_label(case_st_lane_end)

  A64_st1x2_IA(32, 128,  V3_4S,  V4_4S, X8, 32)  /* vec_state[1,2][0..3] = state[offs + 4*y + 0..7]; */
  /* store last state from lane 0 of upper most states vector */
  A64_st1_lane_IA(32, V5_S, 0, X8, 4)   /* state[order-1] = vec_state[5][0]; */

  /* Pop preserved SIMD register (bottom part) */
  A64_popD(D11, D12)
  A64_popD( D9, D10)

A64_ASM_ROUTINE_END()

#define FUNCTION_CLpc_SynthesisLattice_DBL

void CLpc_SynthesisLattice(
        FIXP_DBL *signal,
        const int signal_size,
        const int signal_e,
        const int signal_e_out,
        const int inc,
        const FIXP_DBL *coeff,
        const int order,
        FIXP_DBL *state
        )
{
  int i,j;
  FIXP_DBL *pSignal;

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  if(order >= 10 && order <= 13)
  {
    CLpc_SynthesisLattice_neonv8_order_10to13(pSignal, (INT) signal_size,(INT) signal_e,(INT) signal_e_out,(INT64) inc, coeff, (INT) order, state);
  }
  else
  {
    for (i = signal_size ; i != 0 ; i--)
    {
      FIXP_DBL *pState = state+order-1;
      const FIXP_DBL *pCoeff = coeff+order-1;
      FIXP_DBL tmp;
      tmp = scaleValue(*pSignal, signal_e) - fMult(*pCoeff--, *pState--);
      for (j = order-1; j != 0 ; j--)
      {
        tmp = tmp - fMult(pCoeff[0], pState[0]);
        pState[1] = pState[0] + fMult (*pCoeff--, tmp);
        pState--;
      }
      *pSignal = scaleValue(tmp, -signal_e_out);
      pState[1] = tmp;
      pSignal   += inc ;
    }
  }
}
#endif/* #ifdef FUNCTION_CLpc_SynthesisLattice_neonv8_order_10to13 */

#if defined (__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"

/* LPC_SYNTHESIS_IIR version */
#define FUNCTION_CLpc_Synthesis_DBL
#ifdef FUNCTION_CLpc_Synthesis_DBL
#define FUNCTION_CLpc_Synthesis_DBL_order_13to16
#define FUNCTION_CLpc_Synthesis_DBL_order_9to12
#define FUNCTION_CLpc_Synthesis_DBL_order_1to8
#endif /* FUNCTION_CLpc_Synthesis_DBL */

#endif /* defined(CLPC_SYNTHESIS_ENABLE) */

#ifdef FUNCTION_CLpc_Synthesis_DBL_order_13to16
FDK_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_13to16,
        (FIXP_DBL *signal,
        const int signal_size,
        const int order,
        const int inc,
        const FIXP_LPC_TNS *lpcCoeff_m,  /* coeffs in FIXP_DBL format */
        const int lpcCoeff_e,
        FIXP_DBL *state,
        int *pStateIndex) )

  /* stack contents:
       0x14: pStateIndex             (untouched for this version, assuming *pStateIndex=0)
       0x10: state
       0x0C: lpcCoeff_e
       0x08: lpcCoeff_m
       0x04: r5
       0x00: r4

     register contents:
       r0: signal                    format: <pointer to Q1.31>
       r1: signal_size               format: int, multiple of 4
       r2: order                     format: int, in range [13..16]
       r3: inc                       format: int, in range [-1,1], represents filter direction
       r4: lpcCoeff_m                format: <pointer to Q1.31>
       r5: lpcCoeff_e,               format: int, in range [-30..30]
           state                     format: <pointer to Q1.13>

     NEON registers (example for order=15):
       lane    3           2        1       0
       -------------------------------------------
       Q0:  coeff3       coeff2       coeff1   coeff0
       Q1:  coeff7       coeff6       coeff5   coeff4
       Q2:  coeff11      coeff10      coeff9   coeff8
       Q3:  00000000     coeff14      coeff13  coeff12
       Q4:  state3       state2       state1   state0
       Q5:  state7       state6       state5   state4
       Q6:  state11      state10      state9   state8
       Q7:  -------      state14      state13  state12
       Q8:  c3*s3        c2*s2        c1*s1    c0*s0  <-- x
       Q9:  c7*s7        c6*s6        c5*s5    c4*s4
       Q10: c11*s11      c10*s10      c9*s9    c8*s8
       Q11: c15*s15      c14*s14      c13*s13  c12*s12
       Q12: signal[i]    --------  -lpcCoeff_e-1 -lpcCoeff_e-1
       Q13: lpcCoeff_e+1 lpcCoeff_e+1 
       Q14:
       Q15:

  */

#ifndef __ARM_NEON__
  FIXP_DBL           * r0 = signal;
  int                  r1 = signal_size;
  int                  r2 = order;
  int                  r3 = inc;
  const FIXP_LPC_TNS * r4;
  FIXP_DBL           * r5;
#endif
  FDK_mpush(r4,r5)
  FDK_lsl_imm(r3, r3, 2)               /* Convert inc=1,-1 into inc=4,-4 */
  FDK_ldr(r4, sp, 0x08, lpcCoeff_m)
  FDK_ldr(r5, sp, 0x0C, lpcCoeff_e)
  FDK_add_imm(r5, r5, 1, 0)            /* r5:  lpcCoeff_e+1               */
  FDK_vdup_d_reg(32, D27, r5)          /* D27: lpcCoeff_e+1 lpcCoeff_e+1  */

  FDK_rsb_imm(r5, r5, 0)               /* r5: -lpcCoeff_e-1               */
  FDK_vdup_d_reg(32, D24, r5)          /* D24:-lpcCoeff_e-1 -lpcCoeff_e-1 */

  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */

  FDK_mvpush(Q4,Q7)

  /* Load coefficients into Q0,Q1,Q2,Q3, clear unused upper cells in Q3 */
  /* Load state buffer into Q4,Q5,Q6,Q7, clear unused upper cells in Q7 */
  FDK_vldm3_ia(128, Q0, Q1, Q2, r4)
  FDK_vmov_i32(128, Q3, 0)
  FDK_vldm3_ia(128, Q4, Q5, Q6, r5)
  FDK_vmov_i32(128, Q7, 0)
  FDK_tst_imm(r2, 1)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_13_15)
FDK_label(CLpc_Synthesis_DBL_order_13to16_14_16)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_14)

FDK_label(CLpc_Synthesis_DBL_order_13to16_16)
  FDK_vld1_2d(32, D6,  D7,  r4)
  FDK_vld1_2d(32, D14, D15, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

FDK_label(CLpc_Synthesis_DBL_order_13to16_14)
  FDK_vld1_1d(32, D6,  r4)
  FDK_vld1_1d(32, D14, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

FDK_label(CLpc_Synthesis_DBL_order_13to16_13_15)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_15)

FDK_label(CLpc_Synthesis_DBL_order_13to16_13)
  FDK_vld1(32, S12, r4)
  FDK_vld1(32, S28, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

FDK_label(CLpc_Synthesis_DBL_order_13to16_15)
  FDK_vldm3(32, s12, s13, s14, r4)            // <- gcc: disassembles as VLDMIA !!! to be checked
  FDK_vldm3(32, s28, s29, s30, r5)            // <- gcc: disassembles as VLDMIA !!! to be checked

FDK_label(CLpc_Synthesis_DBL_order_13to16_loop)
  FDK_vld1(32, S51, r0)                     // Q12: signal[i] ------   scale    scale
  FDK_subs_imm(r1, r1, 1)
  FDK_vshl_s32_d(D25, D25, D24)             // Q12:  x        ------   scale    scale
  FDK_vqdmulh_s32_qq(Q8,  Q4,  Q0)          // Q8:   c3*s3    c2*s2    c1*s1    c0*s0
  FDK_vqdmulh_s32_qq(Q9,  Q5,  Q1)          // Q9:   c7*s7    c6*s6    c5*s5    c4*s4
  FDK_vqdmulh_s32_qq(Q10, Q6,  Q2)          // Q10: c11*s11  c10*s10   c9*s9    c8*s8
  FDK_vqdmulh_s32_qq(Q11, Q7,  Q3)          // Q11: c15*s15  c14*s14  c13*s13  c12*s12
  FDK_vhadd_s32_q(Q8,  Q8,  Q9)             // Q8:  c3*s3+c7*s7     c2*s2+c6*s6     c1*s1+c5*s5   c0*s0+c4*s4     / 2
  FDK_vhadd_s32_q(Q10, Q10, Q11)            // Q10: c11*s11+c15*s15 c10*s10+c14*s14 c9*s9+c13*s13 c8*s8+c12*s12   / 2
  FDK_vadd_s32_q(Q8, Q8, Q10)               // Q8:  cs3,7,11,15  cs2,6,10,14 cs1.5.9.13 cs0.4.8.12
  FDK_vpadd_s32(D16, D16, D17)              // D16: cs3,7,11,15+cs2,6,10,14   cs1.5.9.13+cs0.4.8.12
  FDK_vpadd_s32(D16, D16, D16)              // D16: c[0..15] x s[0..15]   c[0..15] x s[0..15]
  FDK_vsub_s32_d(D25, D25, D16)             // Q12: x
  FDK_vqshl_s32(64, D25, D25, D27)          // S51: Q12[3]: x=SATURATE_SHIFT(x, -lpcCoeff_e-1, DFRACT_BITS);
  FDK_vext_q(32, Q7,  Q6, Q7, 3)            // Q7:  state14   state13  state12  state11
  FDK_vext_q(32, Q6,  Q5, Q6, 3)            // Q6:  state10   state9   state8   state7
  FDK_vext_q(32, Q5,  Q4, Q5, 3)            // Q5:  state6    state5   state4   state3
  FDK_vext_q(32, Q4, Q12, Q4, 3)            // Q4:  state2    state1   state0   x
  FDK_vst1_pu(32, S51, r0, r3)              // pSignal[0] = x, pSignal += inc;
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_loop)

  FDK_mvpop(Q4,Q7)

  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */


  /* Update state buffer from Q4..Q7 */
  FDK_vstm3_ia(128, Q4, Q5, Q6, r5)

  FDK_tst_imm(r2, 1)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_13_15_2)
FDK_label(CLpc_Synthesis_DBL_order_13to16_14_16_2)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_14_2)

FDK_label(CLpc_Synthesis_DBL_order_13to16_16_2)
  FDK_vst1_2d(32, D14, D15, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_13to16_end)

FDK_label(CLpc_Synthesis_DBL_order_13to16_14_2)
  FDK_vst1_1d(32, D14, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_13to16_end)

FDK_label(CLpc_Synthesis_DBL_order_13to16_13_15_2)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_13to16_15_2)

FDK_label(CLpc_Synthesis_DBL_order_13to16_13_2)
  FDK_vst1(32, S28, r5)
  FDK_branch(AL,CLpc_Synthesis_DBL_order_13to16_end)

FDK_label(CLpc_Synthesis_DBL_order_13to16_15_2)
  FDK_vstm3(32, s28, s29, s30, r5)

FDK_label(CLpc_Synthesis_DBL_order_13to16_end)
  FDK_mpop(r4,r5)
  FDK_return()
FDK_ASM_ROUTINE_END()

#endif  /* FUNCTION_CLpc_Synthesis_DBL_order_13to16 */


#ifdef FUNCTION_CLpc_Synthesis_DBL_order_9to12
FDK_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_9to12,
        (FIXP_DBL *signal,
        const int signal_size,
        const int order,
        const int inc,
        const FIXP_LPC_TNS *lpcCoeff_m,  /* coeffs in FIXP_DBL format */
        const int lpcCoeff_e,
        FIXP_DBL *state,
        int *pStateIndex) )

  /* stack contents:
       0x14: pStateIndex             (untouched for this version, assuming *pStateIndex=0)
       0x10: state
       0x0C: lpcCoeff_e
       0x08: lpcCoeff_m
       0x04: r5
       0x00: r4

     register contents:
       r0: signal                    format: <pointer to Q1.31>
       r1: signal_size               format: int, multiple of 4
       r2: order                     format: int, in range [9..12]
       r3: inc                       format: int, in range [-1,1], represents filter direction
       r4: lpcCoeff_m                format: <pointer to Q1.31>
       r5: lpcCoeff_e,               format: int, in range [-30..30]
           state                     format: <pointer to Q1.13>

     NEON registers (example for order=12):
       lane    3           2          1         0
       --------------------------------------------
       Q0:  coeff11     coeff10    coeff9     coeff8 (upper coeffs9,10,11 cleared, if not present)
       Q1:  state3      state2     state1     state0
       Q2:  state7      state6     state5     state4
       Q3:  state11     state10    state9     state8
       Q4:  (unused)
       Q5:  (unused)
       Q6:  (unused)
       Q7:  (unused)
       Q8:  c3*s3        c2*s2       c1*s1     c0*s0
       Q9:  c7*s7        c6*s6       c5*s5     c4*s4
       Q10: c11*s11      c10*s10     c9*s9     c8*s8
       Q11: 00000000   00000000   00000000   00000000
       Q12: signal[i]    --------  -lpcCoeff_e-1 -lpcCoeff_e-1
       Q13: lpcCoeff_e+1 lpcCoeff_e+1
       Q14: coeff3      coeff2     coeff1     coeff0
       Q15: coeff7      coeff6     coeff5     coeff4
  */
#ifndef __ARM_NEON__
  FIXP_DBL           * r0 = signal;
  int                  r1 = signal_size;
  int                  r2 = order;
  int                  r3 = inc;
  const FIXP_LPC_TNS * r4;
  FIXP_DBL           * r5;
#endif

  FDK_mpush(r4,r5)
  FDK_lsl_imm(r3, r3, 2)               /* Convert inc=1,-1 into inc=4,-4 */
  FDK_ldr(r4, sp, 0x08, lpcCoeff_m)
  FDK_ldr(r5, sp, 0x0C, lpcCoeff_e)
  FDK_add_imm(r5, r5, 1, 0)            /* r5:  lpcCoeff_e+1               */
  FDK_vdup_d_reg(32, D27, r5)          /* D27: lpcCoeff_e+1 lpcCoeff_e+1  */

  FDK_rsb_imm(r5, r5, 0)               /* r5: -lpcCoeff_e-1               */
  FDK_vdup_d_reg(32, D24, r5)          /* D24:-lpcCoeff_e-1 -lpcCoeff_e-1 */

  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */

  FDK_vmov_i32(128, Q11, 0)            /* dummy product = 0 */

  /* Load coefficients into Q14,Q15, Q0, clear unused upper cells in Q0 */
  /* Load state buffer into Q1,  Q2, Q3, clear unused upper cells in Q3 */
  FDK_vld1_4d_ia(32, D28, D29, D30, D31, r4)
  FDK_vmov_i32(128, Q0, 0)
  FDK_vld1_4d_ia(32, D2, D3, D4, D5, r5)
  FDK_vmov_i32(128, Q3, 0)
  FDK_tst_imm(r2, 1)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_9_11)
FDK_label(CLpc_Synthesis_DBL_order_9to12_10_12)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_10)

FDK_label(CLpc_Synthesis_DBL_order_9to12_12)
  FDK_vld1_2d(32, D0, D1, r4)
  FDK_vld1_2d(32, D6, D7, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

FDK_label(CLpc_Synthesis_DBL_order_9to12_10)
  FDK_vld1_1d(32, D0, r4)
  FDK_vld1_1d(32, D6, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

FDK_label(CLpc_Synthesis_DBL_order_9to12_9_11)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_11)

FDK_label(CLpc_Synthesis_DBL_order_9to12_9)
  FDK_vld1(32, S0, r4)
  FDK_vld1(32, S12, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

FDK_label(CLpc_Synthesis_DBL_order_9to12_11)
  FDK_vldm3(32,  s0,  s1,  s2, r4)            // <- gcc: disassembles as VLDMIA !!! to be checked
  FDK_vldm3(32, s12, s13, s14, r5)            // <- gcc: disassembles as VLDMIA !!! to be checked

FDK_label(CLpc_Synthesis_DBL_order_9to12_loop)
  FDK_vld1(32, S51, r0)                     // Q12: signal[i] ------   scale    scale
  FDK_subs_imm(r1, r1, 1)
  FDK_vshl_s32_d(D25, D25, D24)             // Q12:  x        ------   scale    scale
  FDK_vqdmulh_s32_qq(Q8,  Q1, Q14)          // Q8:   c3*s3    c2*s2    c1*s1    c0*s0
  FDK_vqdmulh_s32_qq(Q9,  Q2, Q15)          // Q9:   c7*s7    c6*s6    c5*s5    c4*s4
  FDK_vqdmulh_s32_qq(Q10, Q3,  Q0)          // Q10: c11*s11  c10*s10   c9*s9    c8*s8
  FDK_vhadd_s32_q(Q8,  Q8,  Q9)             // Q8:  c3*s3+c7*s7     c2*s2+c6*s6     c1*s1+c5*s5   c0*s0+c4*s4     / 2
  FDK_vhadd_s32_q(Q10, Q10, Q11)            // Q10: c11*s11+0       c10*s10+0       c9*s9+0       c8*s8+0         / 2
  FDK_vadd_s32_q(Q8, Q8, Q10)               // Q8:  cs3,7,11     cs2,6,10     cs1.5.9    cs0.4.8
  FDK_vpadd_s32(D16, D16, D17)              // D16: cs3,7,11+cs2,6,10      cs1.5.9+cs0.4.8
  FDK_vpadd_s32(D16, D16, D16)              // D16: c[0..11] x s[0..11]   c[0..11] x s[0..11]
  FDK_vsub_s32_d(D25, D25, D16)             // Q12: x
  FDK_vqshl_s32(64, D25, D25, D27)          // S51: Q12[3]: x=SATURATE_SHIFT(x, -lpcCoeff_e-1, DFRACT_BITS);
  FDK_vext_q(32, Q3,  Q2, Q3, 3)            // Q3:  state10   state9   state8   state7
  FDK_vext_q(32, Q2,  Q1, Q2, 3)            // Q2:  state6    state5   state4   state3
  FDK_vext_q(32, Q1, Q12, Q1, 3)            // Q1:  state2    state1   state0   x
  FDK_vst1_pu(32, S51, r0, r3)              // pSignal[0] = x, pSignal += inc;
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_loop)


  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */

  /* Update state buffer from Q1..Q3 */
  FDK_vst1_4d_ia(32, D2, D3, D4, D5, r5)

  FDK_tst_imm(r2, 1)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_9_11_2)
FDK_label(CLpc_Synthesis_DBL_order_9to12_10_12_2)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_10_2)

FDK_label(CLpc_Synthesis_DBL_order_9to12_12_2)
  FDK_vst1_2d(32, D6, D7, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_9to12_end)

FDK_label(CLpc_Synthesis_DBL_order_9to12_10_2)
  FDK_vst1_1d(32, D6, r5)
  FDK_branch(AL, CLpc_Synthesis_DBL_order_9to12_end)

FDK_label(CLpc_Synthesis_DBL_order_9to12_9_11_2)
  FDK_tst_imm(r2, 2)
  FDK_branch(NE, CLpc_Synthesis_DBL_order_9to12_11_2)

FDK_label(CLpc_Synthesis_DBL_order_9to12_9_2)
  FDK_vst1(32, S12, r5)
  FDK_branch(AL,CLpc_Synthesis_DBL_order_9to12_end)

FDK_label(CLpc_Synthesis_DBL_order_9to12_11_2)
  FDK_vstm3(32, s12, s13, s14, r5)

FDK_label(CLpc_Synthesis_DBL_order_9to12_end)
  FDK_mpop(r4,r5)
  FDK_return()
FDK_ASM_ROUTINE_END()

#endif  /* FUNCTION_CLpc_Synthesis_DBL_order_9to12 */

#ifdef FUNCTION_CLpc_Synthesis_DBL_order_1to8
FDK_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_1to8,
        (FIXP_DBL *signal,
        const int signal_size,
        const int order,  
        const int inc,
        const FIXP_LPC_TNS *lpcCoeff_m,  /* coeffs in FIXP_DBL format */
        const int lpcCoeff_e,
        FIXP_DBL *state,
        int *pStateIndex) )

  /* stack contents:
       0x14: pStateIndex             (untouched for this version, assuming *pStateIndex=0)
       0x10: state
       0x0C: lpcCoeff_e
       0x08: lpcCoeff_m
       0x04: r5
       0x00: r4

     register contents:
       r0: signal                    format: <pointer to Q1.31>
       r1: signal_size               format: int, multiple of 4
       r2: order                     format: int, in range [1..8]
       r3: inc                       format: int, in range [-1,1], represents filter direction
       r4: lpcCoeff_m                format: <pointer to Q1.31>
       r5: lpcCoeff_e,               format: int, in range [-30..30]
           state                     format: <pointer to Q1.13>

     NEON registers (example for order=8):
       lane    3           2           1          0
       -------------------------------------------------
       Q15:  coeff7      coeff6      coeff5      coeff4  <-- filter coefficients, unused ones are cleared
       Q14:  coeff3      coeff2      coeff1      coeff0
       Q13:  state7      state6      state5      state4  <-- filter states, unused ones are <don't care>
       Q12:  state3      state2      state1      state0
       Q11: lpcCoeff_e+1 lpcCoeff_e+1 
       Q10:
       Q9:    c7*s7      c6*s6       c5*s5       c4*s4
       Q8:    c3*s3      c2*s2       c1*s1       c0*s0
       Q7:  reserved    reserved    reserved    reserved
       Q6:  reserved    reserved    reserved    reserved
       Q5:  reserved    reserved    reserved    reserved
       Q4:  reserved    reserved    reserved    reserved
       Q3:
       Q2:
       Q1:
       Q0:  x,signal[i]   --     -lpcCoeff_e-1 -lpcCoeff_e-1  <- scale for signal[i]
  */
#ifndef __ARM_NEON__
  FIXP_DBL           * r0 = signal;
  int                  r1 = signal_size;
  int                  r2 = order;
  int                  r3 = inc;
  const FIXP_LPC_TNS * r4;
  FIXP_DBL           * r5;
#endif

  FDK_mpush(r4,r5)
  FDK_lsl_imm(r3, r3, 2)               /* Convert inc=1,-1 into inc=4,-4 */
  FDK_ldr(r4, sp, 0x08, lpcCoeff_m)
  FDK_ldr(r5, sp, 0x0C, lpcCoeff_e)
  FDK_add_imm(r5, r5, 1, 0)            /* r5:  lpcCoeff_e+1               */
  FDK_vdup_d_reg(32, D23, r5)          /* D23: lpcCoeff_e+1 lpcCoeff_e+1  */

  FDK_vneg_d(32, D0, D23)              /* D0:-lpcCoeff_e-1 -lpcCoeff_e-1 */

  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */

  FDK_vmov_i32(128, Q10, 0)            /* dummy product = 0 */

  /* Load coefficients into Q14,Q15, clear unused upper cells */
  /* Load state buffer into Q12,Q13                           */
FDK_label(CLpc_Synthesis_DBL_order_1to8_tst8)
  FDK_tst_imm(r2, 8)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_tst4)
    FDK_vld1_4d(32, D28, D29, D30, D31, r4)
    FDK_vld1_4d(32, D24, D25, D26, D27, r5)
    FDK_branch(AL, CLpc_Synthesis_DBL_order_1to8_loop)

FDK_label(CLpc_Synthesis_DBL_order_1to8_tst4)
  FDK_vmov_i32(128, Q14, 0)
  FDK_vmov_i32(128, Q15, 0)
  FDK_tst_imm(r2, 4)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_tst2)
    FDK_vld1_2d_ia(32, D30, D31, r4)
    FDK_vld1_2d_ia(32, D26, D27, r5)

FDK_label(CLpc_Synthesis_DBL_order_1to8_tst2)
  FDK_tst_imm(r2, 2)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_tst1)
    FDK_vld1_1d_ia(32, D29, r4)
    FDK_vld1_1d_ia(32, D25, r5)

FDK_label(CLpc_Synthesis_DBL_order_1to8_tst1)
  FDK_tst_imm(r2, 1)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_tst2_0)
    FDK_vld1_ia(32, S56, r4)
    FDK_vld1_ia(32, S48, r5)

FDK_label(CLpc_Synthesis_DBL_order_1to8_tst2_0)
  FDK_tst_imm(r2, 2)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_tst4_0)
    FDK_vswp(64, D29, D28)
    FDK_vswp(64, D25, D24)

FDK_label(CLpc_Synthesis_DBL_order_1to8_tst4_0)
  FDK_tst_imm(r2, 4)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_loop)
    FDK_vswp(128, Q15, Q14)
    FDK_vswp(128, Q13, Q12)

FDK_label(CLpc_Synthesis_DBL_order_1to8_loop)
  FDK_vld1(32, S3, r0)                      // Q0:  signal[i] ------   scale    scale
  FDK_subs_imm(r1, r1, 1)
  FDK_vshl_s32_d(D1, D1, D0)                // Q0:   x        ------   scale    scale
  FDK_vqdmulh_s32_qq(Q9, Q13, Q15)          // Q9:   c7*s7    c6*s6    c5*s5    c4*s4
  FDK_vqdmulh_s32_qq(Q8, Q12, Q14)          // Q8:   c3*s3    c2*s2    c1*s1    c0*s0
  FDK_vhadd_s32_q(Q8,  Q8,  Q9)             // Q8:  cs3+cs7  cs2+cs6  cs1+cs5  cs0+cs4  / 2
  FDK_vpadd_s32(D16, D16, D17)              // D16:                   cs1357   cs0246
  FDK_vpadd_s32(D16, D16, D16)              // D16:                   cs0..7   cs0..7
  FDK_vsub_s32_d(D1, D1, D16)               // Q0:  x-cs0..7
  FDK_vqshl_s32(64, D1, D1, D23)            // S3:  x=SATURATE_SHIFT(x, -lpcCoeff_e-1, DFRACT_BITS)
  FDK_vext_q(32, Q13, Q12, Q13, 3)          // Q13: state6    state5   state4   state3
  FDK_vext_q(32, Q12, Q0, Q12, 3)           // Q12: state2    state1   state0       x
  FDK_vst1_pu(32, S3, r0, r3)               // pSignal[0] = x, pSignal += inc;
  FDK_branch(NE, CLpc_Synthesis_DBL_order_1to8_loop)


  FDK_ldr(r5, sp, 0x10, state)         /* r5: state                       */

  /* Update state buffer from Q12..Q13 */
  FDK_tst_imm(r2, 8)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_chk4)
    FDK_vst1_4d(32, D24, D25, D26, D27, r5)
    FDK_branch(AL, CLpc_Synthesis_DBL_order_1to8_end)

FDK_label(CLpc_Synthesis_DBL_order_1to8_chk4)
  FDK_tst_imm(r2, 4)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_chk2)
    FDK_vst1_2d_ia(32, D24, D25, r5)
    FDK_vmov_q(Q12, Q13)

FDK_label(CLpc_Synthesis_DBL_order_1to8_chk2)
  FDK_tst_imm(r2, 2)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_chk1)
    FDK_vst1_1d_ia(32, D24, r5)
    FDK_vmov_d(D24, D25)

FDK_label(CLpc_Synthesis_DBL_order_1to8_chk1)
  FDK_tst_imm(r2, 1)
  FDK_branch(EQ, CLpc_Synthesis_DBL_order_1to8_end)
    FDK_vst1_ia(32, S48, r5)

FDK_label(CLpc_Synthesis_DBL_order_1to8_end)
  FDK_mpop(r4,r5)
  FDK_return()
FDK_ASM_ROUTINE_END()

#endif  /* FUNCTION_CLpc_Synthesis_DBL_order_1to8 */

#ifdef FUNCTION_CLpc_Synthesis_DBL
void CLpc_Synthesis(
        FIXP_DBL *signal,
        const int signal_size,
        const int inc,
        const FIXP_LPC_TNS *lpcCoeff_m,  /* coeffs in FIXP_DBL format */
        const int lpcCoeff_e,
        int order,
        FIXP_DBL *state,
        int *pStateIndex
        )
{
  FIXP_DBL *pSignal;
  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

#if defined(FUNCTION_CLpc_Synthesis_DBL_order_13to16)
 if ((order >= 13) && (order <= 16))
 {
   CLpc_Synthesis_DBL_order_13to16(pSignal,signal_size, order, inc, lpcCoeff_m, lpcCoeff_e, state, pStateIndex);
 }
 else
#endif
#if defined(FUNCTION_CLpc_Synthesis_DBL_order_9to12)
  if ((order >= 9) && (order <= 12))
 {
   CLpc_Synthesis_DBL_order_9to12(pSignal,signal_size, order, inc, lpcCoeff_m, lpcCoeff_e, state, pStateIndex);
 }
 else
#endif
#if defined(FUNCTION_CLpc_Synthesis_DBL_order_1to8)
 if ((order >= 1) && (order <= 8))
 {
   CLpc_Synthesis_DBL_order_1to8(pSignal,signal_size, order, inc, lpcCoeff_m, lpcCoeff_e, state, pStateIndex);
 }
 else
#endif
 {
  int i,j;
  int stateIndex = *pStateIndex;
  int lpcCoeffShift = lpcCoeff_e+1;

  FIXP_LPC_TNS  coeff[2*LPC_MAX_ORDER];
  FDKmemcpy(&coeff[0],     lpcCoeff_m, order*sizeof(FIXP_LPC_TNS));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order*sizeof(FIXP_LPC_TNS));

  FDK_ASSERT(lpcCoeffShift >= 0);
  /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */
  for (i = 0; i < signal_size ; i++)
  {
    FIXP_DBL x;
    const FIXP_LPC_TNS *pCoeff = coeff + order-stateIndex;

    x = (*pSignal) >> lpcCoeffShift;
    for (j = 0; j<order; j++)
    {
      x = fMultSubDiv2(x, state[j], pCoeff[j]);
    }
    x = SATURATE_LEFT_SHIFT(x, lpcCoeffShift, DFRACT_BITS);

    /* Update states */
    stateIndex =((stateIndex-1)<0) ? (order-1) : (stateIndex-1);
    state[stateIndex] = x;

    *pSignal = x;
    pSignal += inc;
  }
  *pStateIndex = stateIndex;
 }
}
#endif /* #ifdef FUNCTION_CLpc_Synthesis */

#if defined(__ARM_AARCH64_NEON__)

#include "arm/FDK_aarch64_neon_funcs.h"


#define FUNCTION_CLpc_Synthesis_DBL
#if defined(FUNCTION_CLpc_Synthesis_DBL)
#define FUNCTION_CLpc_Synthesis_DBL_ARMneonV8
#define FUNCTION_CLpc_Synthesis_DBL_order_1to8_ARMneonV8
#define FUNCTION_CLpc_Synthesis_DBL_order_9to12_ARMneonV8
#define FUNCTION_CLpc_Synthesis_DBL_order_13to16_ARMneonV8
#define CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8
#endif /* defined(FUNCTION_CLpc_Synthesis_DBL) */

#endif /* defined(__ARM_AARCH64_NEON__) */

#ifdef FUNCTION_CLpc_Synthesis_DBL_order_13to16_ARMneonV8
/**
 * \brief LPC synthesis filter for filter order 13 to 16 for ARMv8 AArch64
 *
 *        Note: The variable pStateIndex which is used in the default code is untouched in
 *              this version. It is assumed that the state index is zero (*pStateIndex=0).
 *              If the macro CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 is defined,
 *              the states are updated at the end of the lpc synthesis filter otherwise not.
 *
 * \param[in,out] signal        pointer to input/output values
 * \param[in]     signal_size   number of input/output values
 * \param[in]     inc           increment of input/output values
 * \param[in]     lpcCoeff_m    pointer to mantissa of lpc coefficients of type FIXP_DBL
 * \param[in]     lpcCoeff_e    exponent of lpc coefficients
 * \param[in]     order         filter order
 * \param[in,out] state         pointer to states buffer
 */
A64_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_13to16_ARMneonV8,
        (FIXP_DBL *signal,
         const INT signal_size,
         const INT64 inc,
         const FIXP_LPC_TNS *lpcCoeff_m,
         const INT lpcCoeff_e,
         const INT order,
         FIXP_DBL *state ) )

  /* register contents:
       X0: signal           format: <pointer to Q1.31>
       W1: signal_size      format: int, signal_size must be greater than 0
       W2: inc              format: int, in range [-1,1], represents filter direction
       X3: lpcCoeff_m       format: <pointer to Q1.31>
       W4: lpcCoeff_e       format: int, in range [-30..30]
       W5: order            format: int, in range [13..16]
       X6: state            format: <pointer to Q1.31>

     NEON registers (example for order=16):
       lane     3             2             1             0
       -------------------------------------------------------
       V0   -------       -------       -------       -------
       :    -------       -------       -------       -------
       V15  -------       -------       -------       -------

       V16:  coeff03       coeff02       coeff01       coeff00
       V17:  coeff07       coeff06       coeff05       coeff04
       V18:  coeff11       coeff10       coeff09       coeff08
       V19:  coeff15       coeff14       coeff13       coeff12

       V20:  state03       state02       state01       state00
       V21:  state07       state06       state05       state04
       V22:  state11       state10       state09       state08
       V23:  state15       state14       state13       state12

       V24:  c03*s03       c02*s02       c01*s01       c00*s00  <-- x
       V25:  c07*s07       c06*s06       c05*s05       c04*s04
       V26:  c11*s11       c10*s10       c09*s09       c08*s08
       V27:  c15*s15       c14*s14       c13*s13       c12*s12

       V28:  -------       -------       -------       signal[i]
       V29:  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1
       V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1

       V31:  -------       -------       -------       -------
  */

#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  X0 = (INT64)signal;
  W1 = signal_size;
  X2 = inc;
  X3 = (INT64)lpcCoeff_m;
  W4 = lpcCoeff_e;
  W5 = order;
  X6 = (INT64)state;
#endif

  A64_mov_Xt(X7, X6)                   /*  X7: save state pointer */

  A64_lsl_Xt_imm(X2, X2, 2)            /*  X2: Convert inc=1,-1 into inc=4,-4 */
  A64_add_Wt_imm(W4, W4, 1)            /*  W4: lpcCoeff_e+1 */
  A64_dup_Wt(32, 128, V29_4S, W4)      /* V29: lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 */

  A64_sub_Wt(W4, WZR, W4)              /*  W4: -lpcCoeff_e-1 */
  A64_dup_Wt(32, 128, V30_4S, W4)      /* V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 */

  /* Load coefficients into V16, V17, V18, V19, clear unused upper cells in V19 */
  /* Load state buffer into V20, V21, V22, V23, clear unused upper cells in V23 */
  A64_ld1x3_IA(32, 128, V16_4S, V17_4S, V18_4S, X3, 48)
  A64_dup_Wt(32, 128, V19_4S, WZR)
  A64_ld1x3_IA(32, 128, V20_4S, V21_4S, V22_4S, X6, 48)
  A64_dup_Wt(32, 128, V23_4S, WZR)

  A64_tbnz_Wt(W5, 0, CLpc_Synthesis_DBL_order_13to16_13_15)

A64_label(CLpc_Synthesis_DBL_order_13to16_14_16)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_13to16_14)

A64_label(CLpc_Synthesis_DBL_order_13to16_16)
  /* case (order & 3) == 0 */
  A64_ld1x1_IA(32, 128, V19_4S, X3, 16)
  A64_ld1x1_IA(32, 128, V23_4S, X6, 16)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

A64_label(CLpc_Synthesis_DBL_order_13to16_14)
  /* case (order & 3) == 2 */
  A64_ld1x1_IA(32, 64, V19_2S, X3, 8)
  A64_ld1x1_IA(32, 64, V23_2S, X6, 8)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

A64_label(CLpc_Synthesis_DBL_order_13to16_13_15)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_13to16_15)

A64_label(CLpc_Synthesis_DBL_order_13to16_13)
  /* case (order & 3) == 1 */
  A64_ld1_lane_IA(32, V19_S, 0, X3, 4)
  A64_ld1_lane_IA(32, V23_S, 0, X6, 4)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_loop)

A64_label(CLpc_Synthesis_DBL_order_13to16_15)
  /* case (order & 3) == 3 */
  A64_ld1x1_IA(32, 64, V19_2S, X3, 8)
  A64_ld1x1_IA(32, 64, V23_2S, X6, 8)
  A64_ld1_lane_IA(32, V19_S, 2, X3, 4)
  A64_ld1_lane_IA(32, V23_S, 2, X6, 4)

A64_label(CLpc_Synthesis_DBL_order_13to16_loop)

  A64_ld1_lane(32, V28_S, 0, X0)                    /* V28:  -------------------  -------------------  -------------------      signal[i]               */
  A64_sqshl(32, 128, V28_4S, V28_4S, V30_4S)        /* V28:  -------------------  -------------------  -------------------  x = signal[i] >> lpcCoeff_e */

  A64_sqdmulh(32, 128, V24_4S, V16_4S, V20_4S)      /* V24:        c03*s03              c02*s02              c01*s01              c00*s00               */
  A64_sqdmulh(32, 128, V25_4S, V17_4S, V21_4S)      /* V25:        c07*s07              c06*s06              c05*s05              c04*s04               */
  A64_sqdmulh(32, 128, V26_4S, V18_4S, V22_4S)      /* V26:        c11*s11              c10*s10              c09*s09              c08*s08               */
  A64_sqdmulh(32, 128, V27_4S, V19_4S, V23_4S)      /* V27:        c15*s15              c14*s14              c13*s13              c12*s12               */

  A64_shadd(32, 128, V24_4S, V24_4S, V25_4S)        /* V24:       cs03+cs07            cs02+cs06            cs01+cs05            cs00+cs04       /2     */
  A64_shadd(32, 128, V26_4S, V26_4S, V27_4S)        /* V26:       cs11+cs15            cs10+cs14            cs09+cs13            cs08+cs12       /2     */
  A64_add(32, 128, V24_4S, V24_4S, V26_4S)          /* V24:  cs03+cs07+cs11+cs15  cs02+cs06+cs10+cs14  cs01+cs05+cs09+cs13  cs00+cs04+cs08+cs12         */

  A64_addv(32, 128, S24, V24_4S)                    /* V24:  -------------------  -------------------  -------------------   y = cs00 + ... + cs15      */
  A64_sub(32, 128, V24_4S, V28_4S, V24_4S)          /* V24:  -------------------  -------------------  -------------------   x = x - y                  */
  A64_sqshl(32, 128, V24_4S, V24_4S, V29_4S)        /* V24:  -------------------  -------------------  -------------------   x = x << lpcCoeff_e        */
  A64_st1_lane_PU(32, V24_S, 0, X0, X2)             /* V24:  -------------------  -------------------  -------------------   x                          */

  A64_ext(8, 128, V24_16B, V24_16B, V24_16B, 4)     /* V24:           x           -------------------  -------------------  -------------------         */
  A64_ext(8, 128, V23_16B, V22_16B, V23_16B, 12)    /* V23:          s14                  s13                  s12                  s11                 */
  A64_ext(8, 128, V22_16B, V21_16B, V22_16B, 12)    /* V22:          s10                  s09                  s08                  s07                 */
  A64_ext(8, 128, V21_16B, V20_16B, V21_16B, 12)    /* V21:          s06                  s05                  s04                  s03                 */
  A64_ext(8, 128, V20_16B, V24_16B, V20_16B, 12)    /* V20:          s02                  s01                  s00                   x                  */

  A64_subs_imm(W1, W1, 1)
  A64_branch(NE, CLpc_Synthesis_DBL_order_13to16_loop)

#ifdef CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8

  /* Update state buffer from V20..V23 */
  A64_st1x3_IA(32, 128, V20_4S, V21_4S, V22_4S, X7, 48)

  A64_tbnz_Wt(W5, 0, CLpc_Synthesis_DBL_order_13to16_13_15_2)

A64_label(CLpc_Synthesis_DBL_order_13to16_14_16_2)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_13to16_14_2)

A64_label(CLpc_Synthesis_DBL_order_13to16_16_2)
  /* case (order & 3) == 0 */
  A64_st1x1_IA(32, 128, V23_4S, X7, 16)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_end)

A64_label(CLpc_Synthesis_DBL_order_13to16_14_2)
  /* case (order & 3) == 2 */
  A64_st1x1_IA(32, 64, V23_2S, X7, 8)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_end)

A64_label(CLpc_Synthesis_DBL_order_13to16_13_15_2)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_13to16_15_2)

A64_label(CLpc_Synthesis_DBL_order_13to16_13_2)
  /* case (order & 3) == 1 */
  A64_st1_lane_IA(32, V23_S, 0, X7, 4)
  A64_branch(AL, CLpc_Synthesis_DBL_order_13to16_end)

A64_label(CLpc_Synthesis_DBL_order_13to16_15_2)
  /* case (order & 3) == 3 */
  A64_st1x1_IA(32, 64, V23_2S, X7, 8)
  A64_st1_lane_IA(32, V23_S, 2, X7, 4)

A64_label(CLpc_Synthesis_DBL_order_13to16_end)

  A64_mov_Xt(X7, X7)      /* dummy instruction, needed for branching */

#endif /* FUNCTION_CLpc_Synthesis_DBL_enable_state_buffer_update_ARMneonV8 */

A64_ASM_ROUTINE_END()

#endif  /* CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 */


#ifdef FUNCTION_CLpc_Synthesis_DBL_order_9to12_ARMneonV8
/**
 * \brief LPC synthesis filter for filter order 9 to 12 for ARMv8 AArch64
 *
 *        Note: The variable pStateIndex which is used in the default code is untouched in
 *              this version. It is assumed that the state index is zero (*pStateIndex=0).
 *              If the macro CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 is defined,
 *              the states are updated at the end of the lpc synthesis filter otherwise not.
 *
 * \param[in,out] signal        pointer to input/output values
 * \param[in]     signal_size   number of input/output values
 * \param[in]     inc           increment of input/output values
 * \param[in]     lpcCoeff_m    pointer to mantissa of lpc coefficients of type FIXP_DBL
 * \param[in]     lpcCoeff_e    exponent of lpc coefficients
 * \param[in]     order         filter order
 * \param[in,out] state         pointer to states buffer
 */
A64_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_9to12_ARMneonV8,
        (FIXP_DBL *signal,
         const INT signal_size,
         const INT64 inc,
         const FIXP_LPC_TNS *lpcCoeff_m,
         const INT lpcCoeff_e,
         const INT order,
         FIXP_DBL *state ) )

  /* register contents:
       X0: signal           format: <pointer to Q1.31>
       W1: signal_size      format: int, signal_size must be greater than 0
       W2: inc              format: int, in range [-1,1], represents filter direction
       X3: lpcCoeff_m       format: <pointer to Q1.31>
       W4: lpcCoeff_e       format: int, in range [-30..30]
       W5: order            format: int, in range [13..16]
       X6: state            format: <pointer to Q1.31>

     NEON registers (example for order=12):
       lane     3             2             1             0
       -------------------------------------------------------
       V0   -------       -------       -------       -------
       :    -------       -------       -------       -------
       V15  -------       -------       -------       -------

       V16:  coeff03       coeff02       coeff01       coeff00
       V17:  coeff07       coeff06       coeff05       coeff04
       V18:  coeff11       coeff10       coeff09       coeff08
       V19:  -------       -------       -------       -------

       V20:  state03       state02       state01       state00
       V21:  state07       state06       state05       state04
       V22:  state11       state10       state09       state08
       V23:  -------       -------       -------       -------

       V24:  c03*s03       c02*s02       c01*s01       c00*s00  <-- x
       V25:  c07*s07       c06*s06       c05*s05       c04*s04
       V26:  c11*s11       c10*s10       c09*s09       c08*s08
       V27:     0             0              0            0

       V28:  -------       -------       -------       signal[i]
       V29:  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1
       V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1

       V31:  -------       -------       -------       -------
  */

#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  X0 = (INT64)signal;
  W1 = signal_size;
  X2 = inc;
  X3 = (INT64)lpcCoeff_m;
  W4 = lpcCoeff_e;
  W5 = order;
  X6 = (INT64)state;
#endif

  A64_mov_Xt(X7, X6)                   /*  X7: save state pointer */

  A64_lsl_Xt_imm(X2, X2, 2)            /*  X2: Convert inc=1,-1 into inc=4,-4 */
  A64_add_Wt_imm(W4, W4, 1)            /*  W4: lpcCoeff_e+1 */
  A64_dup_Wt(32, 128, V29_4S, W4)      /* V29: lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 */

  A64_sub_Wt(W4, WZR, W4)              /*  W4: -lpcCoeff_e-1 */
  A64_dup_Wt(32, 128, V30_4S, W4)      /* V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 */

  /* Load coefficients into V16, V17, V18, clear unused upper cells in V18 */
  /* Load state buffer into V20, V21, V22, clear unused upper cells in V22 */
  A64_ld1x2_IA(32, 128, V16_4S, V17_4S, X3, 32)
  A64_dup_Wt(32, 128, V18_4S, WZR)
  A64_ld1x2_IA(32, 128, V20_4S, V21_4S, X6, 32)
  A64_dup_Wt(32, 128, V22_4S, WZR)

  /* Clear cells in V27 */
  A64_dup_Wt(32, 128, V27_4S, WZR)

  A64_tbnz_Wt(W5, 0, CLpc_Synthesis_DBL_order_9to12_9_11)

A64_label(CLpc_Synthesis_DBL_order_9to12_10_12)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_9to12_10)

A64_label(CLpc_Synthesis_DBL_order_9to12_12)
  /* case (order & 3) == 0 */
  A64_ld1x1_IA(32, 128, V18_4S, X3, 16)
  A64_ld1x1_IA(32, 128, V22_4S, X6, 16)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

A64_label(CLpc_Synthesis_DBL_order_9to12_10)
  /* case (order & 3) == 2 */
  A64_ld1x1_IA(32, 64, V18_2S, X3, 8)
  A64_ld1x1_IA(32, 64, V22_2S, X6, 8)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

A64_label(CLpc_Synthesis_DBL_order_9to12_9_11)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_9to12_11)

A64_label(CLpc_Synthesis_DBL_order_9to12_9)
  /* case (order & 3) == 1 */
  A64_ld1_lane_IA(32, V18_S, 0, X3, 4)
  A64_ld1_lane_IA(32, V22_S, 0, X6, 4)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_loop)

A64_label(CLpc_Synthesis_DBL_order_9to12_11)
  /* case (order & 3) == 3 */
  A64_ld1x1_IA(32, 64, V18_2S, X3, 8)
  A64_ld1x1_IA(32, 64, V22_2S, X6, 8)
  A64_ld1_lane_IA(32, V18_S, 2, X3, 4)
  A64_ld1_lane_IA(32, V22_S, 2, X6, 4)

A64_label(CLpc_Synthesis_DBL_order_9to12_loop)

  A64_ld1_lane(32, V28_S, 0, X0)                    /* V28:  -------------------  -------------------  -------------------      signal[i]               */
  A64_sqshl(32, 128, V28_4S, V28_4S, V30_4S)        /* V28:  -------------------  -------------------  -------------------  x = signal[i] >> lpcCoeff_e */

  A64_sqdmulh(32, 128, V24_4S, V16_4S, V20_4S)      /* V24:        c03*s03              c02*s02              c01*s01              c00*s00               */
  A64_sqdmulh(32, 128, V25_4S, V17_4S, V21_4S)      /* V25:        c07*s07              c06*s06              c05*s05              c04*s04               */
  A64_sqdmulh(32, 128, V26_4S, V18_4S, V22_4S)      /* V26:        c11*s11              c10*s10              c09*s09              c08*s08               */

  A64_shadd(32, 128, V24_4S, V24_4S, V25_4S)        /* V24:       cs03+cs07            cs02+cs06            cs01+cs05            cs00+cs04       /2     */
  A64_shadd(32, 128, V26_4S, V26_4S, V27_4S)        /* V26:       cs11+0               cs10+0               cs09+0               cs08+0          /2     */
  A64_add(32, 128, V24_4S, V24_4S, V26_4S)          /* V24:  cs03+cs07+cs11+0     cs02+cs06+cs10+0     cs01+cs05+cs09+0     cs00+cs04+cs08+0            */

  A64_addv(32, 128, S24, V24_4S)                    /* V24:  -------------------  -------------------  -------------------   y = cs00 + ... + cs11      */
  A64_sub(32, 128, V24_4S, V28_4S, V24_4S)          /* V24:  -------------------  -------------------  -------------------   x = x - y                  */
  A64_sqshl(32, 128, V24_4S, V24_4S, V29_4S)        /* V24:  -------------------  -------------------  -------------------   x = x << lpcCoeff_e        */
  A64_st1_lane_PU(32, V24_S, 0, X0, X2)             /* V24:  -------------------  -------------------  -------------------   x                          */

  A64_ext(8, 128, V24_16B, V24_16B, V24_16B, 4)     /* V24:           x           -------------------  -------------------  -------------------         */
  A64_ext(8, 128, V22_16B, V21_16B, V22_16B, 12)    /* V22:          s10                  s09                  s08                  s07                 */
  A64_ext(8, 128, V21_16B, V20_16B, V21_16B, 12)    /* V21:          s06                  s05                  s04                  s03                 */
  A64_ext(8, 128, V20_16B, V24_16B, V20_16B, 12)    /* V20:          s02                  s01                  s00                   x                  */

  A64_subs_imm(W1, W1, 1)
  A64_branch(NE, CLpc_Synthesis_DBL_order_9to12_loop)

#ifdef CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8

  /* Update state buffer from V20..V22 */
  A64_st1x2_IA(32, 128, V20_4S, V21_4S, X7, 32)

  A64_tbnz_Wt(W5, 0, CLpc_Synthesis_DBL_order_9to12_9_11_2)

A64_label(CLpc_Synthesis_DBL_order_9to12_10_12_2)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_9to12_10_2)

A64_label(CLpc_Synthesis_DBL_order_9to12_12_2)
  /* case (order & 3) == 0 */
  A64_st1x1_IA(32, 128, V22_4S, X7, 16)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_end)

A64_label(CLpc_Synthesis_DBL_order_9to12_10_2)
  /* case (order & 3) == 2 */
  A64_st1x1_IA(32, 64, V22_2S, X7, 8)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_end)

A64_label(CLpc_Synthesis_DBL_order_9to12_9_11_2)
  A64_tbnz_Wt(W5, 1, CLpc_Synthesis_DBL_order_9to12_11_2)

A64_label(CLpc_Synthesis_DBL_order_9to12_9_2)
  /* case (order & 3) == 1 */
  A64_st1_lane_IA(32, V22_S, 0, X7, 4)
  A64_branch(AL, CLpc_Synthesis_DBL_order_9to12_end)

A64_label(CLpc_Synthesis_DBL_order_9to12_11_2)
  /* case (order & 3) == 3 */
  A64_st1x1_IA(32, 64, V22_2S, X7, 8)
  A64_st1_lane_IA(32, V22_S, 2, X7, 4)

A64_label(CLpc_Synthesis_DBL_order_9to12_end)

  A64_mov_Xt(X7, X7)      /* dummy instruction, needed for branching */

#endif /* CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 */

A64_ASM_ROUTINE_END()

#endif  /* FUNCTION_CLpc_Synthesis_DBL_order_9to12_ARMneonV8 */

#ifdef FUNCTION_CLpc_Synthesis_DBL_order_1to8_ARMneonV8
/**
 * \brief LPC synthesis filter for filter order 1 to 8 for ARMv8 AArch64
 *
 *        Note: The variable pStateIndex which is used in the default code is untouched in
 *              this version. It is assumed that the state index is zero (*pStateIndex=0).
 *              If the macro CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 is defined,
 *              the states are updated at the end of the lpc synthesis filter otherwise not.
 *
 * \param[in,out] signal        pointer to input/output values
 * \param[in]     signal_size   number of input/output values
 * \param[in]     inc           increment of input/output values
 * \param[in]     lpcCoeff_m    pointer to mantissa of lpc coefficients of type FIXP_DBL
 * \param[in]     lpcCoeff_e    exponent of lpc coefficients
 * \param[in]     order         filter order
 * \param[in,out] state         pointer to states buffer
 */
A64_ASM_ROUTINE_START(void, CLpc_Synthesis_DBL_order_1to8_ARMneonV8,
        (FIXP_DBL *signal,
          const INT signal_size,
          const INT64 inc,
          const FIXP_LPC_TNS *lpcCoeff_m,
          const INT lpcCoeff_e,
          const INT order,
          FIXP_DBL *state) )

  /* register contents:
       X0: signal           format: <pointer to Q1.31>
       W1: signal_size      format: int, signal_size must be greater than 0
       W2: inc              format: int, in range [-1,1], represents filter direction
       X3: lpcCoeff_m       format: <pointer to Q1.31>
       W4: lpcCoeff_e       format: int, in range [-30..30]
       W5: order            format: int, in range [13..16]
       X6: state            format: <pointer to Q1.31>

     NEON registers (example for order=8):
       lane     3             2             1             0
       -------------------------------------------------------
       V0   -------       -------       -------       -------
       :    -------       -------       -------       -------
       V15  -------       -------       -------       -------

       V16:  coeff03       coeff02       coeff01       coeff00
       V17:  coeff07       coeff06       coeff05       coeff04
       V18:  -------       -------       -------       -------
       V19:  -------       -------       -------       -------

       V20:  state03       state02       state01       state00
       V21:  state07       state06       state05       state04
       V22:  -------       -------       -------       -------
       V23:  -------       -------       -------       -------

       V24:  c03*s03       c02*s02       c01*s01       c00*s00  <-- x
       V25:  c07*s07       c06*s06       c05*s05       c04*s04
       V26:  -------       -------       -------       -------
       V27:  -------       -------       -------       -------

       V28:  -------       -------       -------       signal[i]
       V29:  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1  lpcCoeff_e+1
       V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1

       V31:  -------       -------       -------       -------
  */
#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  X0 = (INT64)signal;
  W1 = signal_size;
  X2 = inc;
  X3 = (INT64)lpcCoeff_m;
  W4 = lpcCoeff_e;
  W5 = order;
  X6 = (INT64)state;
#endif

  A64_mov_Xt(X7, X6)                   /*  X7: save state pointer */

  A64_lsl_Xt_imm(X2, X2, 2)            /*  X2: Convert inc=1,-1 into inc=4,-4 */
  A64_add_Wt_imm(W4, W4, 1)            /*  W4: lpcCoeff_e+1 */
  A64_dup_Wt(32, 128, V29_4S, W4)      /* V29: lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 lpcCoeff_e+1 */

  A64_sub_Wt(W4, WZR, W4)              /*  W4: -lpcCoeff_e-1 */
  A64_dup_Wt(32, 128, V30_4S, W4)      /* V30: -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 -lpcCoeff_e-1 */

  /* Load coefficients into V16, V17, clear unused upper cells in V16 and V17 */
  /* Load state buffer into V20, V21 */
A64_label(CLpc_Synthesis_DBL_order_1to8_tst8)
  A64_tbz_Wt(W5, 3, CLpc_Synthesis_DBL_order_1to8_tst4)
  A64_ld1x2_IA(32, 128, V16_4S, V17_4S, X3, 32)
  A64_ld1x2_IA(32, 128, V20_4S, V21_4S, X6, 32)
  A64_branch(AL, CLpc_Synthesis_DBL_order_1to8_loop)

A64_label(CLpc_Synthesis_DBL_order_1to8_tst4)
  A64_dup_Wt(32, 128, V16_4S, WZR)
  A64_dup_Wt(32, 128, V17_4S, WZR)
  A64_tbz_Wt(W5, 2, CLpc_Synthesis_DBL_order_1to8_tst2)
  A64_ld1x1_IA(32, 128, V17_4S, X3, 16)
  A64_ld1x1_IA(32, 128, V21_4S, X6, 16)

A64_label(CLpc_Synthesis_DBL_order_1to8_tst2)
  A64_tbz_Wt(W5, 1, CLpc_Synthesis_DBL_order_1to8_tst1)
  A64_ld1_lane_IA(32, V16_S, 2, X3, 4)
  A64_ld1_lane_IA(32, V16_S, 3, X3, 4)
  A64_ld1_lane_IA(32, V20_S, 2, X6, 4)
  A64_ld1_lane_IA(32, V20_S, 3, X6, 4)

A64_label(CLpc_Synthesis_DBL_order_1to8_tst1)
  A64_tbz_Wt(W5, 0, CLpc_Synthesis_DBL_order_1to8_tst2_0)
  A64_ld1_lane(32, V16_S, 0, X3)
  A64_ld1_lane(32, V20_S, 0, X6)

A64_label(CLpc_Synthesis_DBL_order_1to8_tst2_0)
  A64_tbz_Wt(W5, 1, CLpc_Synthesis_DBL_order_1to8_tst4_0)
  A64_ext(8, 128, V16_16B, V16_16B, V16_16B, 8)
  A64_ext(8, 128, V20_16B, V20_16B, V20_16B, 8)

 A64_label(CLpc_Synthesis_DBL_order_1to8_tst4_0)
  A64_tbz_Wt(W5, 2, CLpc_Synthesis_DBL_order_1to8_loop)
  A64_mov(8, 128, V18_16B, V16_16B)
  A64_mov(8, 128, V16_16B, V17_16B)
  A64_mov(8, 128, V17_16B, V18_16B)
  A64_mov(8, 128, V22_16B, V20_16B)
  A64_mov(8, 128, V20_16B, V21_16B)
  A64_mov(8, 128, V21_16B, V22_16B)

A64_label(CLpc_Synthesis_DBL_order_1to8_loop)

  A64_ld1_lane(32, V28_S, 0, X0)                    /* V28:  -------------------  -------------------  -------------------      signal[i]               */
  A64_sqshl(32, 128, V28_4S, V28_4S, V30_4S)        /* V28:  -------------------  -------------------  -------------------  x = signal[i] >> lpcCoeff_e */

  A64_sqdmulh(32, 128, V24_4S, V16_4S, V20_4S)      /* V24:        c03*s03              c02*s02              c01*s01              c00*s00               */
  A64_sqdmulh(32, 128, V25_4S, V17_4S, V21_4S)      /* V25:        c07*s07              c06*s06              c05*s05              c04*s04               */

  A64_shadd(32, 128, V24_4S, V24_4S, V25_4S)        /* V24:       cs03+cs07            cs02+cs06            cs01+cs05            cs00+cs04       /2     */
  A64_addv(32, 128, S24, V24_4S)                    /* V24:  -------------------  -------------------  -------------------   y = cs00 + ... + cs7       */
  A64_sub(32, 128, V24_4S, V28_4S, V24_4S)          /* V24:  -------------------  -------------------  -------------------   x = x - y                  */
  A64_sqshl(32, 128, V24_4S, V24_4S, V29_4S)        /* V24:  -------------------  -------------------  -------------------   x = x << lpcCoeff_e        */
  A64_st1_lane_PU(32, V24_S, 0, X0, X2)             /* V24:  -------------------  -------------------  -------------------   x                          */

  A64_ext(8, 128, V24_16B, V24_16B, V24_16B, 4)     /* V24:           x           -------------------  -------------------  -------------------         */
  A64_ext(8, 128, V21_16B, V20_16B, V21_16B, 12)    /* V21:          s06                  s05                  s04                  s03                 */
  A64_ext(8, 128, V20_16B, V24_16B, V20_16B, 12)    /* V20:          s02                  s01                  s00                   x                  */

  A64_subs_imm(W1, W1, 1)
  A64_branch(NE, CLpc_Synthesis_DBL_order_1to8_loop)

#ifdef CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8

  /* Update state buffer from v20 and V21 */
  A64_tbz_Wt(W5, 3, CLpc_Synthesis_DBL_order_1to8_chk4)
  A64_st1x2_IA(32, 128, V20_4S, V21_4S, X7, 32)
  A64_branch(AL, CLpc_Synthesis_DBL_order_1to8_end)

A64_label(CLpc_Synthesis_DBL_order_1to8_chk4)
  A64_tbz_Wt(W5, 2, CLpc_Synthesis_DBL_order_1to8_chk2)
  A64_st1x1_IA(32, 128, V20_4S, X7, 16)
  A64_mov(8, 128, V20_16B, V21_16B)

A64_label(CLpc_Synthesis_DBL_order_1to8_chk2)
  A64_tbz_Wt(W5, 1, CLpc_Synthesis_DBL_order_1to8_chk1)
  A64_st1x1_IA(32, 64, V20_2S, X7, 8)
  A64_ext(8, 128, V20_16B, V20_16B, V20_16B, 8)

A64_label(CLpc_Synthesis_DBL_order_1to8_chk1)
  A64_tbz_Wt(W5, 0, CLpc_Synthesis_DBL_order_1to8_end)
  A64_st1_lane_IA(32, V20_S, 0, X7, 4)

A64_label(CLpc_Synthesis_DBL_order_1to8_end)

  A64_mov_Xt(X7, X7)      /* dummy instruction, needed for branching */

#endif /* CLPC_SYNTHESIS_DBL_STATES_UPDATE_ENABLE_ARM_NEON_V8 */

A64_ASM_ROUTINE_END()

#endif  /* FUNCTION_CLpc_Synthesis_DBL_order_1to8_ARMneonV8 */

#ifdef FUNCTION_CLpc_Synthesis_DBL_ARMneonV8
void CLpc_Synthesis(
        FIXP_DBL *signal,
        const int signal_size,
        const int inc,
        const FIXP_LPC_TNS *lpcCoeff_m,  /* coeffs in FIXP_DBL format */
        const int lpcCoeff_e,
        int order,
        FIXP_DBL *state,
        int *pStateIndex
        )
{
  FIXP_DBL *pSignal;

  if (inc == -1)
  {
    pSignal = &signal[signal_size - 1];
  }
  else
  {
    pSignal = &signal[0];
  }

#if defined(FUNCTION_CLpc_Synthesis_DBL_order_13to16_ARMneonV8)
  if ((order >= 13) && (order <= 16))
  {
    CLpc_Synthesis_DBL_order_13to16_ARMneonV8(pSignal, signal_size, inc, lpcCoeff_m, lpcCoeff_e, order, state);
  }
  else
#endif
#if defined(FUNCTION_CLpc_Synthesis_DBL_order_9to12_ARMneonV8)
  if ((order >= 9) && (order <= 12))
  {
    CLpc_Synthesis_DBL_order_9to12_ARMneonV8(pSignal, signal_size, inc, lpcCoeff_m, lpcCoeff_e, order, state);
  }
  else
#endif
#if defined(FUNCTION_CLpc_Synthesis_DBL_order_1to8_ARMneonV8)
  if ((order >= 1) && (order <= 8))
  {
    CLpc_Synthesis_DBL_order_1to8_ARMneonV8(pSignal, signal_size, inc, lpcCoeff_m, lpcCoeff_e, order, state);
  }
 else
#endif
  {
    int i;
    int j;
    int stateIndex = *pStateIndex;
    int lpcCoeffShift = lpcCoeff_e + 1;
    FIXP_LPC_TNS coeff[2*LPC_MAX_ORDER];

    FDKmemcpy(&coeff[0],     lpcCoeff_m, order*sizeof(FIXP_LPC_TNS));
    FDKmemcpy(&coeff[order], lpcCoeff_m, order*sizeof(FIXP_LPC_TNS));

    FDK_ASSERT(lpcCoeffShift >= 0);

    /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */
    for (i=0; i < signal_size; i++)
    {
      FIXP_DBL x;
      const FIXP_LPC_TNS *pCoeff = coeff + order - stateIndex;

      x = (*pSignal) >> lpcCoeffShift;
      for (j = 0; j<order; j++)
      {
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
}
#endif /* #ifdef FUNCTION_CLpc_Synthesis_DBL_ARMneonV8 */
