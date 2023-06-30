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

   Author(s):   Arthur Tritthart

   Description: MDCT subroutines tuned for ARMv7-NEON

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_imdct_block_func1
#endif


#ifdef FUNCTION_imdct_block_func1

FDK_ASM_ROUTINE_START(void, imdct_block_func1,
   (FIXP_DBL *pCurr,
    FIXP_DBL *pOvl,
    const FIXP_SPK *pWindow,
    FIXP_DBL *pOut0,
    FIXP_DBL *pOut1,
    INT fl))

  /* stack contents:
       0x10      fl
       0x0C      pOut1
       0x08:     r4
       0x04:     r5
       0x00:     lr

     register contents:
       r0: pCurr                format: pointer to Q1.31
       r1: pOvl                 format: pointer to Q1.31
       r2: pWindow              format: pointer to 2xQ1.15 (packed)
       r3: pOut0                format: pointer to Q1.31

     register usage:
       r0: pCurr                used with post-increment ++4
       r1: pOvl                 used with post-decrement --4
       r2: pWindow              used with post-increment ++4
       r3: pOut0                used with post-increment ++4
       r4: pOut1                used with post-decrement --4
       r5: step_m4              step register for --4
       lr: fl                   loop counter

       Q0:  pCurr[0..3]
       Q1:  pOvl[0..3]
       Q2:  pWindow[0..3].re     format: Q1.31
       Q3:  pWindow[0..3].im     format: Q1.31
       Q8:  pCurr[0..3] x pWindow[0..3].re
       Q9:  pCurr[0..3] x pWindow[0..3].im
       Q10: pOvl[0..3]  x pWindow[0..3].re
       Q11: pOvl[0..3]  x pWindow[0..3].im
       Q12: im0..im3, pOut0[0..3]
       Q13; re0,,re3, pOut1[0..3]
  */
#ifndef __ARM_NEON__
       FIXP_DBL *r0 = pCurr;
       FIXP_DBL *r1 = pOvl;
const  FIXP_SPK *r2 = pWindow;
       FIXP_DBL *r3 = pOut0;
       FIXP_DBL *r4;
       INT       r5;
       INT       lr;
#endif

    FDK_mpush_lr(r4, r5)

    FDK_mov_imm(r5, -16)
    FDK_ldr(r4, sp, 0x0C, pOut1)
    FDK_ldr(lr, sp, 0x10, fl)
    FDK_sub_imm(r1, r1, 12, 2)
    FDK_sub_imm(r4, r4, 12, 2)

FDK_label(mdct_block_func1_loop)
      FDK_vld1_2d_ia(32, D0, D1, r0)               // Q0:  pCurr[3]    pCurr[2]    pCurr[1]    pCurr[0]
      FDK_vld1_2d_pu(32, D2, D3, r1, r5)           // Q1:   pOvl[3]     pOvl[2]     pOvl[1]     pOvl[0]
      FDK_vld1_2d_ia(32, D4, D5, r2)               // Q2:pWindow[3]  pWindow[2]  pWindowr[1] pWindow[0] <- Q1.15 | Q1.15
      FDK_vshll_s16_imm(Q3, D5, 16)                // Q3:    w3.im       w3.re        w2.im      w2.re  <- Q1.31
      FDK_vshll_s16_imm(Q2, D4, 16)                // Q2:    w1.im       w1.re        w0.im      w0.re  <- Q1.31
      FDK_vrev64_q(32, Q1, Q1)                     // Q1:   pOvl[2]     pOvl[3]     pOvl[0]     pOvl[1]
      FDK_vswp(64, D2, D3)                         // Q1:   pOvl[0]     pOvl[1]     pOvl[2]     pOvl[3]
      FDK_vuzp_q(32, Q2, Q3)                       // Q2:   w3.re       w2.re        w1.re      w0.re
                                                   // Q3:   w3.im       w2.im        w1.im      w0.im

      //   *c_Re = -(fMult(a_Re,w.v.re) + fMult(a_Im,w.v.im));
      //   *c_Im =   fMult(a_Re,w.v.im) - fMult(a_Im,w.v.re);
      FDK_vqdmulh_s32_qq(Q8, Q0, Q2)               // Q8:  pC[3]*w3.re pC[2]*w2.re pC[1]*w1.re pC[0]w0.re  <- fMult(a_Re,w.v.re)
      FDK_vqdmulh_s32_qq(Q9, Q0, Q3)               // Q9:  pC[3]*w3.im pC[2]*w2.im pC[1]*w1.im pC[0]w0.im  <- fMult(a_Re,w.v.im)
      FDK_vqdmulh_s32_qq(Q10, Q1, Q2)              // Q10: pO[0]*w3.re pO[1]*w2.re pO[2]*w1.re pO[3]w0.re  <- fMult(a_Im,w.v.re)
      FDK_vqdmulh_s32_qq(Q11, Q1, Q3)              // Q11: pO[3]*w3.im pO[2]*w2.im pO[1]*w1.im pO[0]w0.im  <- fMult(a_Im,w.v.im)
      FDK_subs_imm(lr, lr, 8)
      FDK_vsub_s32_q(Q12, Q9, Q10)                 // Q12:  im3         im2          im1        im0
      FDK_vadd_s32_q(Q13, Q11, Q8)                 // Q13: -re3        -re2         -re1       -re0
      FDK_vneg_q(32, Q13, Q13)                     // Q13:  re3         re2          re1        re0
      FDK_vrev64_q(32, Q13, Q13)                   // Q13:  re2         re3          re0        re1
      FDK_vswp(64, D26, D27)                       // Q13:  re0         re1          re2        re3

      FDK_vst1_2d_ia(32, D24, D25, r3)             // Q12: store pOut0[0..3] with post-increment ++4
      FDK_vst1_2d_pu(32, D26, D27, r4, r5)         // Q13: store pOut1[0..3] with post-decrement --4
      FDK_branch(NE, mdct_block_func1_loop)

    FDK_mpop_pc(r4, r5)
FDK_ASM_ROUTINE_END()


#endif  /* #ifdef FUNCTION_imdct_block_func1 */
