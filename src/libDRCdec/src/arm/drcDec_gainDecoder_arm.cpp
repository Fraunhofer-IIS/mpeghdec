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

   Author(s):   Arthur Tritthart

   Description: Subroutines for DRC gain decoding optimized for ARMv7 NEON core.
                This file is an extension to the file: drcDec_gainDecoder.cpp.

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_drcDec_GainDecoder_SetChannelGains_func1
#endif

#ifdef FUNCTION_drcDec_GainDecoder_SetChannelGains_func1
FDK_ASM_ROUTINE_START(void, drcDec_GainDecoder_SetChannelGains_func1,
   (FIXP_DBL *audioBuffer,          /* r0  */
    FIXP_DBL  gain,                 /* r1  */
    FIXP_DBL  stepsize,             /* r2  */
    INT       n_min,                /* r3  */
    INT       frameSize))           /* [0]  must be a multiple of 4 */

  /* stack contents:
       0x00:  framesize

     register contents:
       r0:  audioBuffer      FIXP_DBL *
       r1:  gain             FIXP_DBL
       r2:  stepsize         FIXP_DBL
       r3:  n_min            INT

     register usage:
       r0:  audioBuffer      FIXP_DBL *
       r1:  gain             FIXP_DBL
       r2:  stepsize         FIXP_DBL
       r3:  n_min            INT
       r3:  frameSize        INT

       Q0:  audio[3]    audio[2]    audio[1]    audio[0]
       Q1:  gain+3*ss   gain+2*ss   gain+1*ss   gain
       Q2:  4*stepsize  4*stepsize  4*stepsize  4*stepsize
       Q3:   n_min-1     n_min-1     n_min-1     n_min-1
       Q4:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q8:  ------------------- unused -----------------
       Q9:  ------------------- unused -----------------
       Q10: ------------------- unused -----------------
       Q11: ------------------- unused -----------------
       Q12: ------------------- unused -----------------
       Q13: ------------------- unused -----------------
       Q14: ------------------- unused -----------------
       Q15: ------------------- unused -----------------
  */

#ifndef __ARM_NEON__
#include "arm/FDK_neon_regs.h"
    FIXP_DBL     *r0 = audioBuffer;
    FIXP_DBL      r1 = gain;
    FIXP_DBL      r2 = stepsize;
    INT           r3 = n_min;
#endif

    FDK_sub_imm(r3, r3, 1, 0)                              // r3:  n_min-1
    FDK_vdup_q_reg(32, Q3, r3)                             // Q3:  n_min-1      n_min-1     n_min-1     n_min-1
    FDK_add(r3, r1, r2)                                    // r3:  gain + stepsize
    FDK_vmov_dsn(s4, s5, r1, r3)                           // Q1:  --------     -------    gain+1*stsz gain+0*stsz
    FDK_add(r2, r2, r2)                                    // r2:  2*stepsize
    FDK_vdup_q_reg(32, Q2, r2)                             // Q2:    2*stsz      2*stsz      2*stsz      2*stsz
    FDK_vadd_s32_d(D3, D2, D4)                             // Q1:  gain+3*stsz gain+2*stsz gain+1*stsz gain+0*stsz
    FDK_vadd_s32_q(Q2, Q2, Q2)                             // Q2:  4*stepsize  4*stepsize  4*stepsize  4*stepsize
    FDK_ldr(r3, sp, 0x00, frameSize)                       // r3:  frameSize

FDK_label(drcDec_GainDecoder_SetChannelGains_func1_loop)
      FDK_vld1_2d(32, D0, D1, r0)                          // Q0:   audio[3]    audio[2]    audio[1]    audio[0]
      FDK_vqdmulh_s32_qq(Q0, Q0, Q1)                       // Q0:   fMult(audioBuffer[i], gain)
      FDK_vadd_s32_q(Q1, Q1, Q2)                           // Q1:  {gain+3*stsz gain+2*stsz gain+1*stsz gain+0*stsz} += 4*stepsize
      FDK_vshl_s32_q(Q0, Q0, Q3)                           // Q0:   fMult(audioBuffer[i], gain) << (n_min-1)
      FDK_vst1_2d_ia(32, D0, D1, r0)                       // Q0:   audio[3]    audio[2]    audio[1]    audio[0]  <-- store updated samples
      FDK_subs_imm(r3, r3, 4)
      FDK_branch(NE, drcDec_GainDecoder_SetChannelGains_func1_loop)
    FDK_return()
FDK_ASM_ROUTINE_END()

#endif
