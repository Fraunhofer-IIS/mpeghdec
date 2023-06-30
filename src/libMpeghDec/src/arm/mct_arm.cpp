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

   Description: multi-channel tool ARM optimized functions

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_applyMctRotationIdx_func1
#endif

#ifdef FUNCTION_applyMctRotationIdx_func1
FDK_ASM_ROUTINE_START(void, applyMctRotationIdx_func1,
  (FIXP_DBL * RESTRICT dmx,
   SHORT    * RESTRICT dmxExp,
   FIXP_DBL * RESTRICT res,
   SHORT    * RESTRICT resExp,
   INT                 OutExp,
   INT                 nSamples,   /* always a multiple of 4 */
   FIXP_SGL            SinAlpha,
   FIXP_SGL            CosAlpha,
   INT                 lScale,
   INT                 rScale))

  /* stack contents:
       0x24:     rScale
       0x20:     lScale
       0x1C:     CosAlpha   in Q1.15
       0x18:     SinAlpha   in Q1.15
       0x14:     nSamples
       0x10:     OutExp
       0x0C:     r4
       0x08:     r5
       0x04:     r6
       0x00:     r7

     register contents:
       r0: dmx       FIXP_DBL *
       r1: dmxExp    SHORT *
       r2: res       FIXP_DBL *
       r3: resExp    SHORT *

     register usage:
       r0: dmx
       r1: dmxExp
       r2: res
       r3: resExp
       r4: SinAlpha
       r5: CosAlpha
       r6: OutExp
       r7: nSamples

       Q0:        dmx[i]*SinAlpha
       Q1:        dmx[i]*CosAlpha
       Q2:        res[i]*SinAlpha
       Q3:        res[i]*CosAlpha
       Q4:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q8:       fMax(dmx[i]*CosAlpha>>lScale - res[i]*SinAlpha)>>rScale)
       Q9:       fMin(dmx[i]*CosAlpha>>lScale - res[i]*SinAlpha)>>rScale)
       Q10:      fMax(dmx[i]*SinAlpha>>lScale + res[i]*CosAlpha)>>rScale)
       Q11:      fMin(dmx[i]*SinAlpha>>lScale + res[i]*CosAlpha)>>rScale)
       Q12:      SinAlpha
       Q13:      CosAlpha
       Q14:      rScale / headroom_dmx
       Q15;      lScale / headroom_res
  */

#ifndef __ARM_NEON__
  FIXP_DBL *r0 = dmx;
  SHORT    *r1 = dmxExp;
  FIXP_DBL *r2 = res;
  SHORT    *r3 = resExp;
  FIXP_SGL  r4;
  FIXP_SGL  r5;
  INT       r6;
  INT       r7;
#endif
  FDK_mpush(r4, r7)

  FDK_ldrd(r4, r5, sp, 0x18, SinAlpha, CosAlpha)
  FDK_ldrd(r6, r7, sp, 0x20, lScale, rScale)

  FDK_vmov_i32(128, Q8,  0)
  FDK_vmov_i32(128, Q9,  0)
  FDK_vmov_i32(128, Q10, 0)
  FDK_vmov_i32(128, Q11, 0)
  FDK_vdup_d_reg(16, D24, r4)                              // D24:   SinAlpha in Q1.15 (duplicated(
  FDK_vdup_d_reg(16, D26, r5)                              // D26:   CosAlpha in Q1.15 (duplicated(
  FDK_vshll_s16_imm(Q12, D24, 16)                          // Q12:   SinAlpha in Q1.31 (duplicated(
  FDK_vshll_s16_imm(Q13, D26, 16)                          // Q13:   CosAlpha in Q1.31 (duplicated(
  FDK_vmvn_i32(128, Q0, 0)
  FDK_vdup_q_reg(32, Q14, r6)                              // Q14:   lScale (duplicated)
  FDK_vdup_q_reg(32, Q15, r7)                              // Q15:   rScale (duplicated)
  FDK_veor(128, Q14, Q14, Q0)                              // Q14:  -lScale-1 (duplicated)
  FDK_veor(128, Q15, Q15, Q0)                              // Q15:  -rScale-1 (duplicated)

  FDK_ldr(r7, sp, 0x14,  nSamples)
FDK_label(applyMctRotationIdx_func1_loop1)
    FDK_vld1_2d(32, D0, D1, r0)                            // Q0:  dmx[3]     dmx[2]     dmx[1]     dmx[0]
    FDK_vld1_2d(32, D4, D5, r2)                            // Q2:  res[3]     res[2]     res[1]     res[0]
    FDK_vqdmulh_s32_qq(Q1, Q0, Q13)                        // Q1: dmx[3]*cos dmx[2]*cos dmx[1]*cos dmx[0]*cos
    FDK_vqdmulh_s32_qq(Q0, Q0, Q12)                        // Q0: dmx[3]*sin dmx[2]*sin dmx[1]*sin dmx[0]*sin
    FDK_vqdmulh_s32_qq(Q3, Q2, Q13)                        // Q3: res[3]*cos res[2]*cos res[1]*cos res[0]*cos
    FDK_vqdmulh_s32_qq(Q2, Q2, Q12)                        // Q2: res[3]*sin res[2]*sin res[1]*sin res[0]*sin
    FDK_vshl_s32_q(Q0, Q0, Q14)                            // Q0:{dmx[3]*sin dmx[2]*sin dmx[1]*sin dmx[0]*sin} >> lScale
    FDK_vshl_s32_q(Q1, Q1, Q14)                            // Q1:{dmx[3]*cos dmx[2]*cos dmx[1]*cos dmx[0]*cos} >> lScale
    FDK_vshl_s32_q(Q2, Q2, Q15)                            // Q2:{res[3]*sin res[2]*sin res[1]*sin res[0]*sin} >> rScale
    FDK_vshl_s32_q(Q3, Q3, Q15)                            // Q3:{res[3]*cos res[2]*cos res[1]*cos res[0]*cos} >> rScale
    FDK_vsub_s32_q(Q1, Q1, Q2)                             // Q1: dmx[i]*CosAlpha>>lScale - res[i]*SinAlpha)>>rScale  <-- dmx output
    FDK_vadd_s32_q(Q3, Q0, Q3)                             // Q3: dmx[i]*SinAlpha>>lScale + res[i]*CosAlpha)>>rScale  <-- res output
    FDK_vst1_2d_ia(32, D2, D3, r0)                         // Q1:  dmx[3]     dmx[2]     dmx[1]     dmx[0]
    FDK_vmax_s32(128, Q8, Q1, Q8)                          // Q8: fMax(dmx[i]*CosAlpha>>lScale - res[i]*SinAlpha)>>rScale)
    FDK_vmin_s32(128, Q9, Q1, Q9)                          // Q9: fMin(dmx[i]*CosAlpha>>lScale - res[i]*SinAlpha)>>rScale)
    FDK_vst1_2d_ia(32, D6, D7, r2)                         // Q3:  res[3]     res[2]     res[1]     res[0]
    FDK_vmax_s32(128, Q10, Q3, Q10)                        // Q10: fMax(dmx[i]*SinAlpha>>lScale + res[i]*CosAlpha)>>rScale)
    FDK_vmin_s32(128, Q11, Q3, Q11)                        // Q11: fMin(dmx[i]*SinAlpha>>lScale + res[i]*CosAlpha)>>rScale)
    FDK_subs_imm(r7, r7, 4)
    FDK_branch(GT, applyMctRotationIdx_func1_loop1)

  /* Compute scale factor for dmx from Q8/Q9 and for res from Q10/Q11 */
  /* Note: Example code is taken from scale_arm_neon.cpp */
  FDK_ldrd(r6, r7, sp, 0x10, OutExp,  nSamples)
  FDK_vshr_s32_q_imm(Q0, Q9, 31)                           // Q0:  dmx: negative minimum >> 31
  FDK_vshr_s32_q_imm(Q1, Q11, 31)                          // Q1:  res: negative minimum >> 31
  FDK_veor(128, Q9, Q9, Q0)                                // Q9:  dmx: absolute maximum derived from minimum
  FDK_veor(128, Q11, Q11, Q1)                              // Q11: res: absolute maximum derived from minimum
  FDK_vmax_s32(128, Q8, Q8, Q9)                            // Q8:  dmx: maximum
  FDK_vmax_s32(128, Q10, Q10, Q11)                         // Q10: res: maximum
  FDK_vpmax_s32(64, D0, D16, D17)                          // D0:  dmx: maximum of  Q8[3,2] and  Q8[1,0]
  FDK_vpmax_s32(64, D1, D20, D21)                          // D1:  dmx: maximum of Q10[3,2] and Q10[1,0]
  FDK_vpmax_s32(64, D2, D0, D1)                            // D2:  S1: max of res     S0: max of dmx3
  FDK_vclz_d(32, D3, D2)                                   // D3: clz(S1)             clz(S0)
  FDK_vmov_i32(64, D5, 2)
  FDK_vsub_s32_d(D4, D3, D5)                               // D4: clz(S1)-1-1        clz(S0)-1-1
  FDK_vdup_q_32(Q14, S8)                                   // Q14: dmx: -headroom (duplicated)
  FDK_vdup_q_32(Q15, S9)                                   // Q15: res: -headroom (duplicated)

  FDK_vdup_d_reg(32, D5, r6)                               // D5: OutExp              OutExp
  FDK_vsub_s32_d(D5, D5, D4)                               // D5: OutExp-hdr(res)     OutExp-hdr(dmx)

  FDK_vst1(16, D5[0], r1)                                    // *dmxExp = OutExp - headroom;
  FDK_vst1(16, D5[2], r3)                                    // *resExp = OutExp - headroom;

  FDK_sub_op_lsl(r0, r0, r7, 2, 2)                         // r0: &dmx[nSamples] -> &dmx[09
  FDK_sub_op_lsl(r2, r2, r7, 2, 2)                         // r2: &res[nSamples] -> &res[09

  /* Scale dmx[] and res[] with their specific headroom */
FDK_label(applyMctRotationIdx_func1_loop2)
    FDK_vld1_2d(32, D0, D1, r0)                            // Q0:  dmx[3]     dmx[2]     dmx[1]     dmx[0]
    FDK_vld1_2d(32, D4, D5, r2)                            // Q2:  res[3]     res[2]     res[1]     res[0]
    FDK_vshl_s32_q(Q1, Q0, Q14)
    FDK_vshl_s32_q(Q3, Q2, Q15)
    FDK_vst1_2d_ia(32, D2, D3, r0)                         // Q1:  dmx[3]     dmx[2]     dmx[1]     dmx[0]
    FDK_vst1_2d_ia(32, D6, D7, r2)                         // Q3:  res[3]     res[2]     res[1]     res[0]
    FDK_subs_imm(r7, r7, 4)
    FDK_branch(GT, applyMctRotationIdx_func1_loop2)

  FDK_mpop(r4, r7)

  FDK_return()

FDK_ASM_ROUTINE_END()

#endif
