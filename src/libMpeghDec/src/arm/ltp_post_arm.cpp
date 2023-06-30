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

   Author(s):   Arthur Tritthart

   Description: ARMv7 optimized functions of TCX/AAC Long Term Prediction Postfilter

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_ltp_synth_filter_func1
#endif

#if defined(FUNCTION_ltp_synth_filter_func1)
FDK_ASM_ROUTINE_START(
void, ltp_synth_filter_func1,
     (FIXP_SGL *synth_ltp,
      FIXP_SGL *synth,
      FIXP_DBL *zir,
      INT       length,
      INT       pitch_res,
const FIXP_SGL *w0,
const FIXP_SGL *w1,
const FIXP_SGL *v0,
      INT       pitch_int,
      FIXP_DBL  alpha,
      FIXP_DBL  step,
      FIXP_SGL  gain,
      FIXP_DBL  Alpha) )   /* 0x7999999A = 0.95f */

    /* stack contents:
       0x34:     Alpha
       0x30:     gain
       0x2C:     step
       0x28:     alpha
       0x24:     pitch_int
       0x20:     v0
       0x1C:     w1
       0x18:     w0
       0x14:     pitch_res
       0x10:     r8
       0x0C:     r7
       0x08:     r6
       0x04:     r5
       0x00:     r4

     register usage:
        r0: synth_ltp ->x0,x1 reduced by (pitch_int+L=4) => &synth_ltp[-pitch_int-4]
        r1: synth       reduced by L2-1 => &synth[-6]
        r2: zir
        r3: length      multiple of 4, unequal 0, known range: [64,128,256,512,384,448,896]
        r4: w0, w1
        r5: pitch_int, pitch_res, const reg_step = 2
        r6: alpha, step
        r7: gain, Alpha
        r8: &synth_ltp[0]

      NEON register usage:
        Q15: used for computation of 4x s, s2
        Q14: used for computation of 4x s, s2
        Q13: zir(3)     zir(2)     zir(1)     zir(0)     preset with 0x0000.0000 (for zir == 0)
        Q12: s(3)       s(2)       s(1)       s(0)
        Q11: 4*step     4*step     4*step     4*step
        Q10: alpha-3s   alpha-2s   alpha-1s   alpha-0s
        Q9:  v00  v01   v02  v03   v04  v05   v06  ---
        Q8:  w13  w12   w11  w10   w03  w02   w01  w00
        Q7:  reserved   reserved   reserved   reserved
        Q6:  reserved   reserved   reserved   reserved
        Q5:  reserved   reserved   reserved   reserved
        Q4:  reserved   reserved   gain       ALPHA
        Q3:                       gain       ALPHA
        Q2:  sy-1 sy-2  sy-3 sy-4  sy-5 sy-6  ---- ---  <- synth[-6..-1]
        Q1:  s2(3)      s2(2)      s2(1)      s2(0)
        Q0:  sl2 sl1    sl0 sl-1   sl-2 sl-3  sl-4 ---  <- synth_ltp[-4..2]
    */

    FDK_mpush(r4, r8)

    /* Copy 4 w0 coefficients into D16, use D2 temporarily                   */
    /* Copy 4 w1 coefficients into D17 in reversed order, use D2 temporarily */
    /* Copy 7 v0 coefficients into Q9                                        */
    FDK_mov_reg(r8, r0)            // r8: synth_ltp
    //FDK_ldrd(r4, r5, sp, 0x14, pitch_res, w0)
    FDK_ldr(r4, sp, 0x14, pitch_res)
    FDK_ldr(r5, sp, 0x18, w0)
    FDK_lsl_imm(r4, r4, 1)         // pitch_res *= 2 (due to format FIXP_SGL)
                                   // lane   3               2               1               0
    FDK_vld1_pu(16, D3_0, r5, r4)  // D17:                                                  w0[0*pitch_res]
    FDK_vld1_pu(16, D3_1, r5, r4)  // D17:                                  w0[1*pitch_res] w0[0*pitch_res]
    FDK_vld1_pu(16, D3_2, r5, r4)  // D17:                  w0[2*pitch_res] w0[1*pitch_res] w0[0*pitch_res]
    FDK_vld1_pu(16, D3_3, r5, r4)  // D17:  w0[3*pitch_res] w0[2*pitch_res] w0[1*pitch_res] w0[0*pitch_res]

    FDK_ldrd(r6, r7, sp, 0x1C, w1, v0)
    FDK_vld1_pu(16, D2_3, r6, r4)  // D16:  w1[0*pitch_res]
    FDK_vld1_pu(16, D2_2, r6, r4)  // D16:  w1[0*pitch_res] w1[1*pitch_res]
    FDK_vld1_pu(16, D2_1, r6, r4)  // D16:  w1[0*pitch_res] w1[1*pitch_res] w1[2*pitch_res]
    FDK_vld1_pu(16, D2_0, r6, r4)  // D16:  w1[0*pitch_res] w1[1*pitch_res] w1[2*pitch_res] w1[3*pitch_res]
    FDK_vmov_q(Q8, Q1)

    /* Preload 7 v0 coefficients to be multiplied with synth[0..6] */
    FDK_vld1_1d_ia(16, D19, r7)     // D19:  v0[3]           v0[2]           v0[1]           v0[0]
    FDK_vrev64_d(16, D19, D19)      // D19:  v0[0]           v0[1]           v0[2]           v0[3]
    FDK_vmov_i32(64, D2, 0)         // D2:   0x0000          0x0000          0x0000          0x0000
    FDK_vld1_ia(16, D2_3, r7)       // D2:   v0[4]           0x0000          0x0000          0x0000
    FDK_vld1_ia(16, D2_2, r7)       // D2:   v0[4]           v0[5]           0x0000          0x0000
    FDK_vld1_ia(16, D2_1, r7)       // D2:   v0[4]           v0[5]           v0[6]           0x0000
    FDK_vmov_d(D18, D2)             // D18:  v0[4]           v0[5]           v0[6]           0x0000

    /* load 7 samples from synth_ltp[-pitch_int-4..+2] into Q0, D0[0] remains don't care */
    FDK_ldrd(r4, r5, sp, 0x24, pitch_int, alpha)
    FDK_sub_op_lsl(r0, r0, r4, 1, 1)// r0: &synth_ltp[-pitch_int)
    FDK_sub_imm(r0, r0, 8, 1)       // r0: &synth_ltp[-pitch_int-4)
    FDK_vld1_ia(16, D0_1, r0)       // D0:                                   sl[-4]          ------
    FDK_vld1_ia(16, D0_2, r0)       // D0:                   sl[-3]          sl[-4]          ------
    FDK_vld1_ia(16, D0_3, r0)       // D0:   sl[-2]          sl[-3]          sl[-4]          ------
    FDK_vld1_1d_ia(16, D1, r0)      // D1:   sl[ 2]          sl[ 1]          sl[ 0]          sl[-1]

    /* load 6 samples from synth[-6..-1] into D4,D5 */
    FDK_sub_imm(r1, r1, 12, 1)      // r1: synth[-6]
    FDK_vld1_ia(16, D4_2, r1)       // D4:   ------          sy[-6]          ------          ------
    FDK_vld1_ia(16, D4_3, r1)       // D4:   sy[-5]          sy[-6]          ------          ------
    FDK_vld1_1d_ia(16, D5, r1)      // D5:   sy[-1]          sy[-2]          sy[-3]          sy[-4]

    FDK_ldrd(r6, r7, sp, 0x2C, step, gain)

    FDK_lsl_imm(r7, r7, 16)         // r7: convert gain from Q1.15 to Q1.31
    FDK_vmov_sn(S13, r7)            // D6:                                  gain
    FDK_ldr(r7, sp, 0x34, Alpha)
    FDK_vmov_sn(S12, r7)            // D6:                                  gain            ALPHA

    FDK_vdup_q_reg(32, Q11, r6)     // Q11:   step           step           step            step
    FDK_sub(r6,r5,r6)               // r6: alpha-step
    FDK_vmov_sn(S4, r5)             // D2:                                                  alpha
    FDK_vmov_sn(S5, r6)             // D2:                                  alpha-s*1       alpha
    FDK_vmov_d(D20, D2)             // Q10:                                 alpha-s*1       alpha
    FDK_vshl_s32_q_imm(Q11, Q11, 1) // Q11:  2*step          2*step          2*step         2*step
    FDK_vsub_s32_d(D21, D20, D22)   // Q10:  alpha-s*3      alpha-s*2        alpha-s*1      alpha
    FDK_vshl_s32_q_imm(Q11, Q11, 1) // Q11:  4*step          4*step          4*step         4*step

FDK_label(ltp_synth_filter_func1_loop)
    /* Load 4 new samples from synth_ltp[3..6] into D2, update Q0 and compute 4x s(0) into Q14 */
    FDK_vld1_1d_ia(16, D2, r0)      // D2:   sl[ 6]          sl[ 5]          sl[ 4]          sl[ 3]

    FDK_vext_q(16, Q0, Q0, Q1, 1)   // Q0:  sl[ 3] sl[ 2]  sl[ 1] sl[ 0]  sl[-1] sl[-2]  sl[-3] sl[-4]
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sl[ 3]          sl[ 6]          sl[ 5]          sl[ 4]
    FDK_vmull_s16(Q14, D0, D16)     // Q14:  fMultDiv2(w1[k=0,1,2,3*pitch_res], x0[-i=-1,-2,-3,-4])  !! x1=x0-1
    FDK_vmlal_s16(Q14, D1, D17)     // Q14: +fMultDiv2(w0[k=0,1,2,3*pitch_res], x0[ i= 0, 1, 2, 3])  = s(0)

    FDK_vext_q(16, Q0, Q0, Q1, 1)   // Q0:  sl[ 4] sl[ 3]  sl[ 2] sl[ 1]  sl[ 0] sl[-1]  sl[-2] sl[-3]
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sl[ 4]          sl[ 3]          sl[ 6]          sl[ 5]
    FDK_vmull_s16(Q15, D0, D16)     // Q15:
    FDK_vmlal_s16(Q15, D1, D17)     // Q15:  s(1)

    /* pairwise add s(0), s(1) components and store results in D24 */
    FDK_vpadd_s32(D28, D28, D29)    // D28:                             s(0,2)+s(0,3)  s(0,0)+s(0,1)
    FDK_vpadd_s32(D29, D30, D31)    // D29: s2(0,2)+s(0,3)  s2(0,0)+s(0,1)
    FDK_vpadd_s32(D24, D28, D29)    // D24:                                  s2(0)           s(0)

    FDK_vext_q(16, Q0, Q0, Q1, 1)   // Q0:  sl[ 5] sl[ 4]  sl[ 3] sl[ 2]  sl[ 1] sl[ 0]  sl[-1] sl[-2]
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sl[ 5]          sl[ 4]          sl[ 3]          sl[ 6]
    FDK_vmull_s16(Q14, D0, D16)     // Q14:
    FDK_vmlal_s16(Q14, D1, D17)     // Q14:  s(2)

    FDK_vext_q(16, Q0, Q0, Q1, 1)   // Q0:  sl[ 6] sl[ 5]  sl[ 4] sl[ 3]  sl[ 2] sl[ 1]  sl[ 0] sl[-1]
    FDK_vmull_s16(Q15, D0, D16)     // Q15:
    FDK_vmlal_s16(Q15, D1, D17)     // Q15:  s(3)

    /* pairwise add s(2), s(3) components and store results in D25 */
    FDK_vpadd_s32(D28, D28, D29)
    FDK_vpadd_s32(D29, D30, D31)
    FDK_vpadd_s32(D25, D28, D29)    // Q12:  s(3)            s(2)            s(1)            s(0)

    /* Load 4 new samples from synth[0..3] into D2, update Q2 and compute 4x s2(0) into Q14 */
    FDK_vld1_1d_ia(16, D2, r1)      // D2:   sy[ 3]          sy[ 2]          sy[ 1]          sy[ 0]

    FDK_vext_q(16, Q2, Q2, Q1, 1)   // Q0:  sy[ 0] sy[-1]   sy[-2] sy[-3]   sy[-4] sy[-5]   sy[-6] ------
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sy[ 0]          sy[ 3]          sy[ 2]          sy[ 1]
    FDK_vmull_s16(Q14, D4, D18)     // Q14:  fMultDiv2(v0[i=4,5,6,-], y0[-i= -4,-5,-6,-7])
    FDK_vmlal_s16(Q14, D5, D19)     // Q14: +fMultDiv2(v0[i=0,1,2,3], y0[-i= 0,-1,-2,-3])     = s2(0)

    FDK_vext_q(16, Q2, Q2, Q1, 1)   // Q0:   sy[ 1] sy[ 0]   sy[-1] sy[-2]   sy[-3] sy[-4]   sy[-5] sy[-6]
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sy[ 1]          sy[ 0]          sy[ 3]          sy[ 2]
    FDK_vmull_s16(Q15, D4, D18)
    FDK_vmlal_s16(Q15, D5, D19)     // Q15:  s2(1)

    /* pairwise add s2(0), s2(1) components and store results in D2 */
    FDK_vpadd_s32(D28, D28, D29)    // D28:
    FDK_vpadd_s32(D29, D30, D31)
    FDK_vpadd_s32(D3,  D28, D29)    // D3:   s2(1)           s2(0)

    FDK_vext_q(16, Q2, Q2, Q1, 1)   // Q0:  sy[ 2] sy[ 1]   sy[ 0] sy[-1]   sy[-2] sy[-3]   sy[-4] sy[-5]
    FDK_vext_d(16, D2, D2, D2, 1)   // D2:   sy[ 2]          sy[ 1]          sy[ 0]          sy[ 3]
    FDK_vmull_s16(Q14, D4, D18)
    FDK_vmlal_s16(Q14, D5, D19)     // Q14:  s2(2)

    FDK_vext_q(16, Q2, Q2, Q1, 1)   // Q0:  sy[ 3] sy[ 2]   sy[ 1] sy[ 0]   sy[-1] sy[-2]   sy[-3] sy[-4]
    FDK_vmull_s16(Q15, D4, D18)
    FDK_vmlal_s16(Q15, D5, D19)     // Q15: = s2(3)

    FDK_vpadd_s32(D28, D28, D29)
    FDK_vpadd_s32(D29, D30, D31)
    FDK_vmov_d(D2, D3)              // Q1:   s2(1)           s2(0)    -->    s2(1)           s2(0)
    FDK_vpadd_s32(D3,  D28, D29)    // Q1:   s2(3)           s2(2)           s2(1)           s2(0)

    FDK_vqdmulh_s32_qs(Q1,Q1,S12)   // Q1:   s2(3)*A         s2(2)*A         s2(1)*A         s2(0)*A    <- s2 = fMult(s2,ALPHA)

    FDK_vhsub_s32_q(Q12, Q12, Q1)   // Q12:  tmp(3)          tmp(2)          tmp(1)          tmp(0)     <- tmp = (s >> 1) - (s2 >> 1)

    FDK_vqdmulh_s32_qs(Q1,Q10,S13)  // Q1:  gain*alpha(3)   gain*alpha(2)   gain*alpha(1)   gain*alpha(0)
    FDK_vsub_s32_q(Q10, Q10, Q11)   // Q10: alpha-s*7       alpha-s*6       alpha-s*5       alpha-s*4   <- alpha -= step;
    FDK_vqdmulh_s32_qq(Q12,Q12,Q1)  // Q12:  tmp(3)          tmp(2)          tmp(1)          tmp(0)     <- tmp=(LONG)fMult(fMult(gain, alpha), (FIXP_DBL)tmp);
    FDK_vshll_s16_imm(Q1, D5, 14)   // Q1:   sy[3]<<14       sy[2]<<14       sy[1]<<14       sy[0]<<14  <- ((LONG)synth[j] << 14)
    FDK_vadd_s32_q(Q1, Q12, Q1)     // Q1:   tmp(3)          tmp(2)          tmp(1)          tmp(0)     <- tmp=((LONG)synth[j] << 14) + (LONG)fMult(fMult(gain, alpha), (FIXP_DBL)tmp);
    FDK_cmp_imm(r2, 0)
#ifdef FDK_vld1_2d_ia_cond
    FDK_vld1_2d_ia_cond(NE, 32, D26, D27, r2)// Q13: zir[3]           zir[2]          zir[1]          zir[0]
    FDK_vsub_s32_q(Q1, Q1, Q13)     // Q1:  tmp(3)-zir[3]    tmp(2)-zir[2]   tmp(1)-zir[1]   tmp(0)-zir[0]
#else
    FDK_branch(EQ, ltp_synth_filter_func1_zir_end)
    FDK_vld1_2d_ia(32, D26, D27, r2)// Q13: zir[3]           zir[2]          zir[1]          zir[0]
    FDK_vsub_s32_q(Q1, Q1, Q13)     // Q1:  tmp(3)-zir[3]    tmp(2)-zir[2]   tmp(1)-zir[1]   tmp(0)-zir[0]
FDK_label(ltp_synth_filter_func1_zir_end)
#endif
    FDK_subs_imm(r3, r3, 4)
    FDK_vqshrn_s32_imm(D2, Q1, 14)
    FDK_vst1_1d_ia(32, D2, r8)      // Q8:  synth_ltp[3]     synth_ltp[2]    synth_ltp[1]    synth_ltp[0]

    FDK_branch(NE, ltp_synth_filter_func1_loop)

    FDK_mpop(r4, r8)
    FDK_return()

FDK_ASM_ROUTINE_END()
#endif
