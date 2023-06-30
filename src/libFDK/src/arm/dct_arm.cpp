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

   Author(s):

   Description:

*******************************************************************************/

/* clang-format off */
#include "arm/FDK_arm_funcs.h"
#if defined (__ARM_NEON__)
#include "arm/FDK_neon_funcs.h"
#endif


#if defined(__ARM_NEON__)
  #if defined(WINDOWTABLE_16BIT)
    #define FUNCTION_dst_IV_func1_ARMv7NEON
    #define FUNCTION_dst_IV_func2_ARMv7NEON
  #endif
#endif

#if defined (SINETABLE_16BIT) && defined(__ARM_NEON__)
#define FUNCTION_dct_IV_func1
#define FUNCTION_dct_IV_func2
#endif


#if defined(FUNCTION_dst_IV_func1_CORE) || defined(FUNCTION_dst_IV_func1_ARMv7NEON)
#define FUNCTION_dst_IV_func1
#endif
#if defined(FUNCTION_dst_IV_func2_CORE) || defined(FUNCTION_dst_IV_func2_ARMv7NEON)
  #define FUNCTION_dst_IV_func2
#endif

#ifdef FUNCTION_dst_IV_func2_ARMv7NEON

FDK_ASM_ROUTINE_START(void, dst_IV_func2,
    ( int             M,                    /* caller: M = L >> 3;   */
      const FIXP_SPK* twiddle,              /* packed FIXP_SGL values - caller: sin_twiddle + sin_step    */
      FIXP_DBL*       pDat_0,
      FIXP_DBL*       pDat_1,               /* caller: pDat[L - 1])           */
      int             sin_step
   ))

  /* register contents:
  r0: M
  r1: twiddle
  r2: pDat_0
  r3: pDat_1[L-1]

  NEON registers:
  Q0: twiddles
  Q1: input data
  Q2: input data
  Q3: twiddles
  Q4-7: reserved
  Q8: intermediate / output data
  Q9: intermediate / output data
  Q10:intermediate
  Q11:intermediate
  Q12: ---    ---   ---   i11 r11   initial
  Q13:
  Q14: (D28:) swap register for re/im twiddles
  Q15:  ---   ---   ---   -1/-1     (16 bit)


  stack content:
  0x08: sin_step
  0x04: r4
  0x00: r5
  */
#ifndef __ARM_NEON__
    int             r0  = M;
    const FIXP_SPK* r1  = twiddle;
    FIXP_DBL*       r2  = pDat_0;
    FIXP_DBL*       r3  = pDat_1;
    int r4;
    int r5;
#endif
  FDK_mpush(r4, r5)
    /*    : 0x00000000 (i11)    0xC0000000 (r11) in D24   */
  FDK_vmov_i64(64, D24, 0xFF)
  FDK_vshl_s32_d_imm(D24, D24, 30)                          // Q12:       ---       ---       i11      r11

  FDK_ldr(r4, sp, 0x08, sin_step)

  FDK_vmov_i32(64, D30, 0x80000000)
  FDK_vmov_i32(64, D31, 0x00008000)
  FDK_vadd_s32_d(D30, D30, D31)                             // Q15:       ---       ---     -1/-1     -1/-1


  FDK_lsl_imm(r4, r4, 2)
  FDK_sub_imm(r3, r3, 3<<2, 2)
  FDK_mov_imm(r5, 0xfffffff0)

FDK_label(dst_IV_func2_av7neon_loop)

    FDK_vrev64_d(32, D6, D24)                               // Q3:        ---       ---      i00      r00

    FDK_vld1_pu(32, S1, r1, r4)                             // Q0:      -----     -----     i10/r10   -------
    FDK_vld1_pu(32, S0 ,r1, r4)                             // Q0:      -----     -----     i10/r10   i11/r11

    FDK_vmull_s16(Q0, D0, D30)                              // Q0:        i10       r10      i11      r11       ("shifted right" by 1 and negated)
    FDK_vmov_d(D24, D0)                                     //                                                  backup i11, r11

    FDK_vrev64_d(32, D7, D1)                                // Q3:        i01       r01      i00      r00
    FDK_vswp(64, D0, D7)                                    // Q0:        i10       r10      i01      r01
                                                            // Q3:        i11       r11      i00      r00
    FDK_vtrn_q(32, Q3, Q0)                                  // Q0:        i10       i11      i01      i00
                                                            // Q3:        r10       r11      r01      r00

    FDK_vld2_2d(32, D3, D5, r3)
    FDK_vld2_2d(32, D2, D4, r2)                             // Q2:        2_0       2_1       4_1       4_0     Im
                                                            // Q1:        1_0       1_1       3_1       3_0     Re
    FDK_vshl_s32_q_imm(Q2, Q2, 1)
    FDK_vshl_s32_q_imm(Q1, Q1, 1)

    FDK_vqrdmulh_s32_qq(Q9, Q0, Q1)                         // Q9 :   a10*i10   a11*i11   a31*i01   a30*i00     Re*i
    FDK_vqrdmulh_s32_qq(Q8, Q0, Q2)                         // Q8 :   a20*i10   a21*i11   a41*i01   a40*i00     Im*i
    FDK_vqrdmulh_s32_qq(Q10, Q3, Q1)                        // Q10:   a10*r10   a11*r11   a31*r01   a30*r00     Re*r
    FDK_vqrdmulh_s32_qq(Q11, Q3, Q2)                        // Q11:   a20*r10   a21*r11   a41*r01   a40*r00     Im*r
    FDK_vadd_s32_q(Q9, Q9, Q11)                             // Q9 :   acc 2_0   acc 2_1   acc 4_1   acc 4_0     Re*i + Im*r
    FDK_vsub_s32_q(Q8, Q10, Q8)                             // Q8 :   acc 1_0   acc 1_1   acc 3_1   acc 3_0     Re*r - Im*i

    FDK_vrev64_q(32, Q9, Q9)                                // Q9 :   acc 2_1   acc 2_0   acc 4_0   acc 4_1
    FDK_vswp(64, D16, D17)                                  // Q8 :   acc 3_1   acc 3_0   acc 1_0   acc 1_1

    FDK_vst2_2d_ia(32, D17, D19, r2)
    FDK_vst2_2d_pu(32, D16, D18, r3, r5)

    FDK_subs_imm(r0, r0, 1)
  FDK_branch(NE, dst_IV_func2_av7neon_loop)
  FDK_mpop(r4, r5)

  FDK_return()
  FDK_ASM_ROUTINE_END()
#endif

#ifdef FUNCTION_dst_IV_func1_ARMv7NEON
FDK_ASM_ROUTINE_START(void, dst_IV_func1,
    ( int             M,                    /* caller: int M = L >> 1;   */
      const FIXP_SPK* twiddle,              /* packed FIXP_SGL values     */
      FIXP_DBL*       pDat_0,
      FIXP_DBL*       pDat_1                /* caller: pDat[L])           */
    ))

  /* register contents:
  r0: M
  r1: twiddle
  r2: pDat_0
  r3: pDat_1

  NEON registers:
  Q0: real in
  Q1: imag in
  Q2: twiddles: re3-re0
  Q3: twiddles: im3-im0
  Q8: intermediate / results (pDat_0 store)
  Q9: intermediate / results (pDat_1 store)
  Q10: -1   +1   -1   +1
  */
#ifndef __ARM_NEON__
    int             r0  = M;
    const FIXP_SPK* r1  = twiddle;
    FIXP_DBL*       r2  = pDat_0;
    FIXP_DBL*       r3  = pDat_1;
#endif

  FDK_vmvn_i32(64, D20, ~0xffffffff)
  FDK_vmov_i32(64, D21, 0x00000001)
  FDK_vzip_d(32, D20, D21)                                // Q10: +1   -1   +1   -1  (int)
  FDK_vneg_q(32, Q11, Q10)                                // Q11: -1   +1   -1   +1  (int)
  FDK_vshl_s32_q_imm(Q10, Q10, 30)                        // Q10: +1   -1   +1   -1  (frac)

FDK_label(dst_IV_func1_av7neon_loop)
  FDK_vld2_2d_ia(16, D4, D5, r1)                          // Q2:      i3/i2 i1/i0 r3/r2 r1/r0, twiddle += 4
  FDK_vldm2_db(64, D0, D1, r3)                            // Q0:       1_0   4_0   1_1   4_1  pDat_1 input (reversed order)
  FDK_vld1_2d(32, D2, D3, r2)                             // Q1:       3_1 | 2_1 | 3_0 | 2_0  pDat_0 input

  FDK_vshll_s16_imm(Q3, D4, 15)                           // Q3:       re3   re2   re1   re0 ("shifted right" by 1)
  FDK_vshll_s16_imm(Q2, D5, 15)                           // Q2:       im3   im2   im1   im0 ("shifted right" by 1)
  FDK_vqdmulh_s32_qq(Q0, Q0, Q10)
  FDK_vqdmulh_s32_qq(Q1, Q1, Q10)
  FDK_vswp(64, D0, D1)
  FDK_vrev64_q(32, Q0, Q0)                                // Q0:       4_1 |-1_1 | 4_0 |-1_0  pDat_0 input (right order)

  FDK_vqdmulh_s32_qq(Q8, Q0, Q3)                          // Q8: Re*tw.re
  FDK_vqdmulh_s32_qq(Q9, Q1, Q3)                          // Q9: Im*tw.re
  FDK_vqdmulh_s32_qq(Q3, Q1, Q2)                          // Q3: Im*tw.im
  FDK_vqdmulh_s32_qq(Q1, Q0, Q2)                          // Q1: Re*tw.im

  FDK_vsub_s32_q(Q8, Q8, Q3)                              // Q8: Re*tw.re - Im*tw.im   : 3_1   1_1   3_0   1_0
  FDK_vadd_s32_q(Q9, Q1, Q9)                              // Q9: Re*tw.im + Im*tw.re   : 4_1   2_1   4_0   2_0
  FDK_vtrn_q(32, Q9, Q8)                                  // Q8: 3_1   4_1   3_0   4_0
                                                          // Q9: 1_1   2_1   1_0   2_0
  FDK_vswp(64, D16, D17)                                  // Q8: 3_0   4_0   3_1   4_1
  FDK_vmul_s32_q(Q8, Q8, Q11)                             // Q8:-3_0   4_0  -3_1   4_1  (accu_3_x negated)

  FDK_vst1_2d_ia(32, D18, D19, r2)                        // Q9: pDat_0[i + 0..3]
  FDK_vst1_2d(32, D16, D17, r3)                           // Q8: pDat_1[i + 3..0]

  FDK_subs_imm(r0, r0, 4)
  FDK_branch(NE, dst_IV_func1_av7neon_loop)

  FDK_return()
  FDK_ASM_ROUTINE_END()
#endif

#ifdef FUNCTION_dct_IV_func1

#if defined(__ARM_NEON__)

FDK_ASM_ROUTINE_START(void, dct_IV_func1,
  ( int             M,                    /* caller:  M >> 2 (= L >> 3)   */
    const FIXP_SPK* twiddle,              /* packed FIXP_SGL values       */
    FIXP_DBL*       pDat_0,
    FIXP_DBL*       pDat_1                /* caller: pDat[L - 1])         */
    ))
  /* register contents:
  r0: M                  -> M >> 2
  r1: twiddle
  r2: pDat_0
  r3: pDat_1             -> pDat_0[L-1]
  r4: step for pDat1

  NEON registers:
  Q0: pDat_1 Samples
  Q1: pDat_0 samples
  Q2: twiddle_Im
  Q3: twiddle_Re
  Q8: result pDat_1
  Q9: result pDat_0
  Q10: -1  +1  -1  +1
  */

#ifndef __ARM_NEON__
#include "arm/FDK_neon_regs.h"
  int             r0 = M;
  const FIXP_SPK* r1 = twiddle;
  FIXP_DBL*       r2 = pDat_0;
  FIXP_DBL*       r3 = pDat_1;
  FIXP_DBL        r4;
#endif

  FDK_push(r4)
  FDK_mov_imm(r4, -16)                                     // r4: -4 (FIXP_DBL)

  FDK_sub_imm(r3, r3, 12, 2)                               // r3: pDat_1 = pDat[L-4]
  FDK_vmvn_i32(64, D21,~0xffffffff)
  FDK_vmov_i32(64, D20, 0x00000001)
  FDK_vzip_d(32,D20,D21)                                   // Q10: -1   +1   -1   +1


FDK_label(dct_IV_func1_loop)
    FDK_vld1_2d(32, D0, D1, r3)                            // Q0:  accu 1_0 | 4_0 | 1_1 | 4_1  real input (reverse order)
    FDK_vld2_2d_ia(16, D4, D5, r1)                         // Q2:      i3/i2 i1/i0 r3/r2 r1/r0   Q1.15
    FDK_subs_imm(r0, r0 , 1)
    FDK_vswp(64, D0, D1)                                   // Q0:       1_1 | 4_1 | 1_0 | 4_0
    FDK_vrev64_q(32, Q0, Q0)                               // Q0:       4_1 | 1_1 | 4_0 | 1_0  real input (right order)
    FDK_vld1_2d(32, D2, D3, r2)                            // Q1:       3_1 | 2_1 | 3_0 | 2_0  imag input (right order)
    FDK_vshll_s16_imm(Q3, D4, 15)                          // Q3:       re3   re2   re1   re0    Q30
    FDK_vshll_s16_imm(Q2, D5, 15)                          // Q2:       im3   im2   im1   im0    Q30

    FDK_vqdmulh_s32_qq(Q8, Q0, Q3)                         // Q8: Re*re
    FDK_vqdmulh_s32_qq(Q9, Q1, Q3)                         // Q9: Im*re
    FDK_vqdmulh_s32_qq(Q3, Q1, Q2)                         // Q3: Im*im
    FDK_vqdmulh_s32_qq(Q1, Q0, Q2)                         // Q1: Re*im
    FDK_vhsub_s32_q(Q8, Q8, Q3)                            // Q8:       3_1 | 1_1 | 3_0 | 1_0  real output
    FDK_vmul_s32_q(Q8, Q8, Q10)                            // Q8:      -3_1 | 1_1 |-3_0 | 1_0  real output (odd ones negated)
    FDK_vhadd_s32_q(Q9, Q9, Q1)                            // Q9:       4_1 | 2_1 | 4_0 | 2_0  imag output
    FDK_vtrn_q(32, Q9, Q8)                                 // Q8:      -3_1 | 4_1 |-3_0 | 4_0
                                                           // Q9        1_1 | 2_1 | 1_0 | 2_0 --> pDat_0
    FDK_vswp(64, D16, D17)                                 // Q8:      -3_0 | 4_0 |-3_1 | 4_1 --> pDat_1

    FDK_vst1_2d_ia(32, D18, D19, r2)                       // Q9: store via pDat_0 with ++4
    FDK_vst1_2d_pu(32, D16, D17, r3, r4)                   // Q8: store via pDat_1 with --4
    FDK_branch(NE, dct_IV_func1_loop)

  FDK_pop(r4)
  FDK_return()
  FDK_ASM_ROUTINE_END()

#else
/*
   Note: This assembler routine is here, because the ARM926 compiler does
         not encode the inline assembler with optimal speed.
         With this version, we save 2 cycles per loop iteration.
*/

__asm  void dct_IV_func1(
    int i,
    const FIXP_SPK *twiddle,
    FIXP_DBL *RESTRICT pDat_0,
    FIXP_DBL *RESTRICT pDat_1)
{
    /* Register map:
       r0   i
       r1   twiddle
       r2   pDat_0
       r3   pDat_1
       r4   accu1
       r5   accu2
       r6   accu3
       r7   accu4
       r8   val_tw
       r9   accuX
    */
    PUSH    {r4-r9}

     /* 44 cycles for 2 iterations = 22 cycles/iteration */
dct_IV_loop1_start
/*  First iteration */
    LDR     r8, [r1], #4    // val_tw = *twiddle++;
    LDR     r5, [r2, #0]    // accu2 = pDat_0[0]
    LDR     r4, [r3, #0]    // accu1 = pDat_1[0]

    SMULWT  r9, r5, r8      // accuX = accu2*val_tw.l
    SMULWB  r5, r5, r8      // accu2 = accu2*val_tw.h
    RSB     r9, r9, #0      // accuX =-accu2*val_tw.l
    SMLAWT  r5, r4, r8, r5  // accu2 = accu2*val_tw.h + accu1*val_tw.l
    SMLAWB  r4, r4, r8, r9  // accu1 = accu1*val_tw.h - accu2*val_tw.l

    LDR     r8, [r1], #4    // val_tw = *twiddle++;
    LDR     r7, [r3, #-4]   // accu4 = pDat_1[-1]
    LDR     r6, [r2, #4]    // accu3 = pDat_0[1]

    SMULWB  r9, r7, r8      // accuX = accu4*val_tw.h
    SMULWT  r7, r7, r8      // accu4 = accu4*val_tw.l
    RSB     r9, r9, #0      // accuX =-accu4*val_tw.h
    SMLAWB  r7, r6, r8, r7  // accu4 = accu4*val_tw.l+accu3*val_tw.h
    SMLAWT  r6, r6, r8, r9  // accu3 = accu3*val_tw.l-accu4*val_tw.h

    ASR     r5, r5, #1
    ASR     r4, r4, #1
    ASR     r7, r7, #1
    ASR     r6, r6, #1

    STR     r5, [r2], #4    // *pDat_0++ = accu2
    STR     r4, [r2], #4    // *pDat_0++ = accu1
    STR     r6, [r3], #-4   // *pDat_1-- = accu3
    STR     r7, [r3], #-4   // *pDat_1-- = accu4

/*  Second iteration */
    LDR     r8, [r1], #4    // val_tw = *twiddle++;
    LDR     r5, [r2, #0]    // accu2 = pDat_0[0]
    LDR     r4, [r3, #0]    // accu1 = pDat_1[0]

    SMULWT  r9, r5, r8      // accuX = accu2*val_tw.l
    SMULWB  r5, r5, r8      // accu2 = accu2*val_tw.h
    RSB     r9, r9, #0      // accuX =-accu2*val_tw.l
    SMLAWT  r5, r4, r8, r5  // accu2 = accu2*val_tw.h + accu1*val_tw.l
    SMLAWB  r4, r4, r8, r9  // accu1 = accu1*val_tw.h - accu2*val_tw.l

    LDR     r8, [r1], #4    // val_tw = *twiddle++;
    LDR     r7, [r3, #-4]   // accu4 = pDat_1[-1]
    LDR     r6, [r2, #4]    // accu3 = pDat_0[1]

    SMULWB  r9, r7, r8      // accuX = accu4*val_tw.h
    SMULWT  r7, r7, r8      // accu4 = accu4*val_tw.l
    RSB     r9, r9, #0      // accuX =-accu4*val_tw.h
    SMLAWB  r7, r6, r8, r7  // accu4 = accu4*val_tw.l+accu3*val_tw.h
    SMLAWT  r6, r6, r8, r9  // accu3 = accu3*val_tw.l-accu4*val_tw.h

    ASR     r5, r5, #1
    ASR     r4, r4, #1
    ASR     r7, r7, #1
    ASR     r6, r6, #1

    STR     r5, [r2], #4    // *pDat_0++ = accu2
    STR     r4, [r2], #4    // *pDat_0++ = accu1
    STR     r6, [r3], #-4   // *pDat_1-- = accu3
    STR     r7, [r3], #-4   // *pDat_1-- = accu4

    SUBS    r0, r0, #1
    BNE     dct_IV_loop1_start

    POP     {r4-r9}

    BX      lr
}

#endif
#endif /* FUNCTION_dct_IV_func1 */

#ifdef FUNCTION_dct_IV_func2

#if defined(__ARM_NEON__)
FDK_ASM_ROUTINE_START(void, dct_IV_func2,
  ( int             M,                    /* caller:  M >> 2 (= L >> 3)   */
    const FIXP_SPK* twiddle,              /* packed 16-bit twidlle coeffs */
    FIXP_DBL*       pDat_0,               /* caller: pDat[0]              */
    FIXP_DBL*       pDat_1,               /* caller: pDat[L]              */
    INT             sin_step))

  /* Core register contents:
      r0: M                  -> M >> 2
      r1: twiddle
      r2: pDat_0
      r3: pDat_1             -> pDat_0[L]
      r4: sin_step
      r5: -16 (pointer decrement for r3)

    Stack contents:
      0x08: sin_step
      0x04: r4
      0x00: r5

  NEON registers:
    Q0: pDat_0[0..3] I/O samples
    Q1: pDat_1[0..3] I/O samples
    Q2: re3    re2    re1    re0
    Q3: im3    im2    im1    im0
    Q8: Re*re
    Q9: Im*re
    Q10: Im*im
    Q11: Re*im
*/

#ifndef __ARM_NEON__
#include "arm/FDK_neon_regs.h"
  int             r0 = M;
  const FIXP_SPK* r1 = twiddle;
  FIXP_DBL*       r2 = pDat_0;
  FIXP_DBL*       r3 = pDat_1;
  INT             r4;
  INT             r5;
#endif
  FDK_mpush(r4, r5)
  FDK_mov_imm(r5, -16)                                     // r5: -4 (FIXP_DBL)
  FDK_ldr(r4, sp, 0x08, sin_step)
  FDK_add_op_lsl(r1, r1, r4, 2, 2)
  FDK_lsl_imm(r4, r4, 2)                                   // r4: sin_step<<2

  FDK_sub_imm(r3, r3, 16, 2)                               // r3: pDat_1 = pDat[L-4]
  FDK_vmvn_i32(128, Q2, 0x80000000)                        // Q2:  7FFFFFFF  7FFFFFFF  7FFFFFFF  7FFFFFFF  real coeffs
  FDK_vmov_i32(128, Q3, 0x00000000)                        // Q3:  00000000  00000000  00000000  00000000  imag coeffs

FDK_label(dct_IV_func2_loop)
    FDK_vld1_pu(32, S32, r1, r4)                           // Q8:                                im1/re1
    FDK_vld1_pu(32, S33, r1, r4)                           // Q8:                      im3,re3   im1,re1   in Q1.15
    FDK_vshll_s16_imm(Q8, D16, 16)                         // Q8:    im3       re3       im1       re1     in Q1.31
    FDK_vrev64_q(32, Q9, Q8)                               // Q9:    re3       im3       re1       im1     in Q1.31
    FDK_vswp(64, D5, D7)                                   // Q2:    im3       ...       ...       ...
                                                           // Q3:    re2       ...       ...       ...

    FDK_vext_q(32, Q2, Q2, Q8, 3)                          // Q2:    re3      re2=im1    re1     re0=im3
    FDK_vext_q(32, Q3, Q3, Q9, 3)                          // Q3:    im3      im2=re1    im1     im0=re3

    FDK_vld1_2d(32, D0, D1, r2)                            // Q0:    4_1       3_1       4_0       3_0
    FDK_vld1_2d(32, D2, D3, r3)                            // Q1:    2_0       1_0       2_1       1_1
    FDK_vswp(64, D2, D3)                                   // Q1:    2_1       1_1       2_0       1_0
                                                           // Q1:    2_1       4_1       2_0       4_0
    FDK_vtrn_q(32, Q0, Q1)                                 // Q0:    1_1       3_1       1_0       3_0  real input
                                                           // Q1:    2_1       4_1       2_0       4_0  imag input

    FDK_vqdmulh_s32_qq(Q8,  Q0, Q2)                        // Q8: Re*re
    FDK_vqdmulh_s32_qq(Q9,  Q0, Q3)                        // Q9: Re*im
    FDK_vqdmulh_s32_qq(Q10, Q1, Q3)                        // Q10:Im*im
    FDK_vqdmulh_s32_qq(Q11, Q1, Q2)                        // Q11:Im*re

    FDK_vsub_s32_q(Q1, Q8, Q10)                            // Q1:    1_1       3_1       1_0       3_0  real output of cplxMult
    FDK_vadd_s32_q(Q0, Q9, Q11)                            // Q0:    2_1       4_1       2_0       4_0  imag output of cplxMult

    FDK_vrev64_q(32, Q1, Q1)                               // Q1:    3_1       1_1       3_0       1_0
    FDK_vtrn_q(32, Q0, Q1)                                 // Q0:    1_1       4_1       1_0       4_0  <-- pDat_0[0..3]
                                                           // Q1:    3_1       2_1       3_0       2_0
    FDK_vswp(64, D2, D3)                                   // Q1:    3_0       2_0       3_1       2_1  <-- pDat_1[0..3]

    FDK_vst1_2d_ia(32, D0, D1, r2)                         // Q0: store output via pDat_0
    FDK_vst1_2d_pu(32, D2, D3, r3, r5)                     // Q1: store output via pDat_1
    FDK_subs_imm(r0, r0, 1)
    FDK_branch(NE, dct_IV_func2_loop)

  FDK_mpop(r4,r5)
  FDK_return()
FDK_ASM_ROUTINE_END()


#else

/* __attribute__((noinline)) */
static inline void dct_IV_func2(
    int i,
    const FIXP_SPK *twiddle,
    FIXP_DBL *pDat_0,
    FIXP_DBL *pDat_1,
    int inc)
{
  FIXP_DBL accu1, accu2, accu3, accu4, accuX;
  LONG val_tw;

  accu1 = pDat_1[-2];
  accu2 = pDat_1[-1];

  *--pDat_1 = -(pDat_0[1]);
  *pDat_0++ = (pDat_0[0]);

  twiddle += inc;

__asm
  {
    LDR     val_tw, [twiddle], inc, LSL #2    // val_tw = *twiddle; twiddle += inc
    B       dct_IV_loop2_2nd_part

    /* 42 cycles for 2 iterations = 21 cycles/iteration */
dct_IV_loop2:
    SMULWT  accuX, accu2, val_tw
    SMULWB  accu2, accu2, val_tw
    RSB     accuX, accuX, #0
    SMLAWB  accuX, accu1, val_tw, accuX
    SMLAWT  accu2, accu1, val_tw, accu2
    LSL     accuX, accuX, #1
    LSL     accu2, accu2, #1
    STR     accuX, [pDat_0], #4
    STR     accu2, [pDat_1, #-4] !

    LDR     accu4, [pDat_0, #4]
    LDR     accu3, [pDat_0]
    SMULWB  accuX, accu4, val_tw
    SMULWT  accu4, accu4, val_tw
    RSB     accuX, accuX, #0
    SMLAWT  accuX, accu3, val_tw, accuX
    SMLAWB  accu4, accu3, val_tw, accu4

    LDR     accu1, [pDat_1, #-8]
    LDR     accu2, [pDat_1, #-4]

    LDR     val_tw, [twiddle], inc, LSL #2    // val_tw = *twiddle; twiddle += inc

    LSL     accuX, accuX, #1
    LSL     accu4, accu4, #1
    STR     accuX, [pDat_1, #-4] !
    STR     accu4, [pDat_0], #4

dct_IV_loop2_2nd_part:
    SMULWT  accuX, accu2, val_tw
    SMULWB  accu2, accu2, val_tw
    RSB     accuX, accuX, #0
    SMLAWB  accuX, accu1, val_tw, accuX
    SMLAWT  accu2, accu1, val_tw, accu2
    LSL     accuX, accuX, #1
    LSL     accu2, accu2, #1
    STR     accuX, [pDat_0], #4
    STR     accu2, [pDat_1, #-4] !

    LDR     accu4, [pDat_0, #4]
    LDR     accu3, [pDat_0]
    SMULWB  accuX, accu4, val_tw
    SMULWT  accu4, accu4, val_tw
    RSB     accuX, accuX, #0
    SMLAWT  accuX, accu3, val_tw, accuX
    SMLAWB  accu4, accu3, val_tw, accu4

    LDR     accu1, [pDat_1, #-8]
    LDR     accu2, [pDat_1, #-4]

    LSL     accuX, accuX, #1
    LSL     accu4, accu4, #1
    STR     accuX, [pDat_1, #-4] !
    STR     accu4, [pDat_0], #4

    LDR     val_tw, [twiddle], inc, LSL #2    // val_tw = *twiddle; twiddle += inc

    SUBS    i, i, #1
    BNE     dct_IV_loop2
  }

  /* Last Sin and Cos value pair are the same */
  accu1 = fMult(accu1, WTC(0x5a82799a));
  accu2 = fMult(accu2, WTC(0x5a82799a));

  *--pDat_1 = accu1 + accu2;
  *pDat_0++ = accu1 - accu2;
}
#endif
#endif /* FUNCTION_dct_IV_func2 */


#ifdef FUNCTION_dst_IV_func1_CORE

__asm void dst_IV_func1(
    int i,
    const FIXP_SPK *twiddle,
    FIXP_DBL *pDat_0,
    FIXP_DBL *pDat_1)
{
    /* Register map:
       r0   i
       r1   twiddle
       r2   pDat_0
       r3   pDat_1
       r4   accu1
       r5   accu2
       r6   accu3
       r7   accu4
       r8   val_tw
       r9   accuX
    */
    PUSH    {r4-r9}

dst_IV_loop1
    LDR     r8, [r1], #4               // val_tw = *twiddle++
    LDR     r5, [r2]                   // accu2 = pDat_0[0]
    LDR     r6, [r2, #4]               // accu3 = pDat_0[1]
    ASR     r5, r5, #1
    ASR     r6, r6, #1
    RSB     r5, r5, #0                 // accu2 = -accu2
    SMULWT  r9, r5, r8                 // accuX = (-accu2)*val_tw.l
    LDR     r4, [r3, #-4]              // accu1 = pDat_1[-1]
    ASR     r4, r4, #1
    RSB     r9, r9, #0                 // accuX = -(-accu2)*val_tw.l
    SMLAWB  r9, r4, r8, r9             // accuX = accu1*val_tw.h-(-accu2)*val_tw.l
    SMULWT  r4, r4, r8                 // accu1 = accu1*val_tw.l
    LDR     r7, [r3, #-8]              // accu4 = pDat_1[-2]
    ASR     r7, r7, #1
    SMLAWB  r5, r5, r8, r4             // accu2 = (-accu2)*val_tw.t+accu1*val_tw.l
    LDR     r8, [r1], #4               // val_tw = *twiddle++
    STR     r5, [r2], #4               // *pDat_0++ = accu2
    STR     r9, [r2], #4               // *pDat_0++ = accu1 (accuX)
    RSB     r7, r7, #0                 // accu4 = -accu4
    SMULWB  r5, r7, r8                 // accu2 = (-accu4)*val_tw.h
    SMULWB  r4, r6, r8                 // accu1 = (-accu4)*val_tw.l
    RSB     r5, r5, #0                 // accu2 = -(-accu4)*val_tw.h
    SMLAWT  r6, r6, r8, r5             // accu3 = (-accu4)*val_tw.l-(-accu3)*val_tw.h
    SMLAWT  r7, r7, r8, r4             // accu4 = (-accu3)*val_tw.l+(-accu4)*val_tw.h
    STR     r6, [r3, #-4] !            // *--pDat_1 = accu3
    STR     r7, [r3, #-4] !            // *--pDat_1 = accu4

    LDR     r8, [r1], #4               // val_tw = *twiddle++
    LDR     r5, [r2]                   // accu2 = pDat_0[0]
    LDR     r6, [r2, #4]               // accu3 = pDat_0[1]
    ASR     r5, r5, #1
    ASR     r6, r6, #1
    RSB     r5, r5, #0                 // accu2 = -accu2
    SMULWT  r9, r5, r8                 // accuX = (-accu2)*val_tw.l
    LDR     r4, [r3, #-4]              // accu1 = pDat_1[-1]
    ASR     r4, r4, #1
    RSB     r9, r9, #0                 // accuX = -(-accu2)*val_tw.l
    SMLAWB  r9, r4, r8, r9             // accuX = accu1*val_tw.h-(-accu2)*val_tw.l
    SMULWT  r4, r4, r8                 // accu1 = accu1*val_tw.l
    LDR     r7, [r3, #-8]              // accu4 = pDat_1[-2]
    ASR     r7, r7, #1
    SMLAWB  r5, r5, r8, r4             // accu2 = (-accu2)*val_tw.t+accu1*val_tw.l
    LDR     r8, [r1], #4               // val_tw = *twiddle++
    STR     r5, [r2], #4               // *pDat_0++ = accu2
    STR     r9, [r2], #4               // *pDat_0++ = accu1 (accuX)
    RSB     r7, r7, #0                 // accu4 = -accu4
    SMULWB  r5, r7, r8                 // accu2 = (-accu4)*val_tw.h
    SMULWB  r4, r6, r8                 // accu1 = (-accu4)*val_tw.l
    RSB     r5, r5, #0                 // accu2 = -(-accu4)*val_tw.h
    SMLAWT  r6, r6, r8, r5             // accu3 = (-accu4)*val_tw.l-(-accu3)*val_tw.h
    SMLAWT  r7, r7, r8, r4             // accu4 = (-accu3)*val_tw.l+(-accu4)*val_tw.h
    STR     r6, [r3, #-4] !            // *--pDat_1 = accu3
    STR     r7, [r3, #-4] !            // *--pDat_1 = accu4

    SUBS    r0, r0, #4                 // i-= 4
    BNE     dst_IV_loop1

    POP     {r4-r9}
    BX      lr
}
#endif /* FUNCTION_dst_IV_func1_CORE */

#ifdef FUNCTION_dst_IV_func2_CORE

/* __attribute__((noinline)) */
static inline void dst_IV_func2(
    int i,
    const FIXP_SPK *twiddle,
    FIXP_DBL *RESTRICT pDat_0,
    FIXP_DBL *RESTRICT pDat_1,
    int inc)
{
  FIXP_DBL accu1,accu2,accu3,accu4;
  LONG val_tw;

  accu4 = pDat_0[0];
  accu3 = pDat_0[1];
  accu4 = -accu4;

  accu1 = pDat_1[-1];
  accu2 = pDat_1[0];

  *pDat_0++ = accu3;
  *pDat_1-- = accu4;


  __asm
  {
    B       dst_IV_loop2_2nd_part

    /* 50 cycles for 2 iterations = 25 cycles/iteration */

dst_IV_loop2:

    LDR     val_tw, [twiddle], inc, LSL #2    // val_tw = *twiddle; twiddle += inc

    RSB     accu2, accu2, #0                  // accu2 = -accu2
    RSB     accu1, accu1, #0                  // accu1 = -accu1
    SMULWT  accu3, accu2, val_tw              // accu3 = (-accu2)*val_tw.l
    SMULWT  accu4, accu1, val_tw              // accu4 = (-accu1)*val_tw.l
    RSB     accu3, accu3, #0                  // accu3 = -accu2*val_tw.l
    SMLAWB  accu1, accu1, val_tw, accu3       // accu1 = -accu1*val_tw.h-(-accu2)*val_tw.l
    SMLAWB  accu2, accu2, val_tw, accu4       // accu2 = (-accu1)*val_tw.l+(-accu2)*val_tw.h
    LSL     accu1, accu1, #1
    LSL     accu2, accu2, #1
    STR     accu1, [pDat_1], #-4              // *pDat_1-- = accu1
    STR     accu2, [pDat_0], #4               // *pDat_0++ = accu2

    LDR     accu4, [pDat_0]                   // accu4 = pDat_0[0]
    LDR     accu3, [pDat_0, #4]               // accu3 = pDat_0[1]

    RSB     accu4, accu4, #0                  // accu4 = -accu4
    RSB     accu3, accu3, #0                  // accu3 = -accu3

    SMULWB  accu1, accu3, val_tw              // accu1 = (-accu3)*val_tw.h
    SMULWT  accu2, accu3, val_tw              // accu2 = (-accu3)*val_tw.l
    RSB     accu1, accu1, #0                  // accu1 = -(-accu3)*val_tw.h
    SMLAWT  accu3, accu4, val_tw, accu1       // accu3 = (-accu4)*val_tw.l-(-accu3)*val_tw.h
    SMLAWB  accu4, accu4, val_tw, accu2       // accu4 = (-accu3)*val_tw.l+(-accu4)*val_tw.h
    LSL     accu3, accu3, #1
    LSL     accu4, accu4, #1

    LDR     accu1, [pDat_1, #-4]              // accu1 = pDat_1[-1]
    LDR     accu2, [pDat_1]                   // accu2 = pDat_1[0]

    STR     accu3, [pDat_0], #4               // *pDat_0++ = accu3
    STR     accu4, [pDat_1], #-4              // *pDat_1-- = accu4

dst_IV_loop2_2nd_part:

    LDR     val_tw, [twiddle], inc, LSL #2    // val_tw = *twiddle; twiddle += inc

    RSB     accu2, accu2, #0                  // accu2 = -accu2
    RSB     accu1, accu1, #0                  // accu1 = -accu1
    SMULWT  accu3, accu2, val_tw              // accu3 = (-accu2)*val_tw.l
    SMULWT  accu4, accu1, val_tw              // accu4 = (-accu1)*val_tw.l
    RSB     accu3, accu3, #0                  // accu3 = -accu2*val_tw.l
    SMLAWB  accu1, accu1, val_tw, accu3       // accu1 = -accu1*val_tw.h-(-accu2)*val_tw.l
    SMLAWB  accu2, accu2, val_tw, accu4       // accu2 = (-accu1)*val_tw.l+(-accu2)*val_tw.h
    LSL     accu1, accu1, #1
    LSL     accu2, accu2, #1
    STR     accu1, [pDat_1], #-4              // *pDat_1-- = accu1
    STR     accu2, [pDat_0], #4               // *pDat_0++ = accu2

    LDR     accu4, [pDat_0]                   // accu4 = pDat_0[0]
    LDR     accu3, [pDat_0, #4]               // accu3 = pDat_0[1]

    RSB     accu4, accu4, #0                  // accu4 = -accu4
    RSB     accu3, accu3, #0                  // accu3 = -accu3

    SMULWB  accu1, accu3, val_tw              // accu1 = (-accu3)*val_tw.h
    SMULWT  accu2, accu3, val_tw              // accu2 = (-accu3)*val_tw.l
    RSB     accu1, accu1, #0                  // accu1 = -(-accu3)*val_tw.h
    SMLAWT  accu3, accu4, val_tw, accu1       // accu3 = (-accu4)*val_tw.l-(-accu3)*val_tw.h
    SMLAWB  accu4, accu4, val_tw, accu2       // accu4 = (-accu3)*val_tw.l+(-accu4)*val_tw.h
    LSL     accu3, accu3, #1
    LSL     accu4, accu4, #1

    LDR     accu1, [pDat_1, #-4]              // accu1 = pDat_1[-1]
    LDR     accu2, [pDat_1]                   // accu2 = pDat_1[0]

    STR     accu3, [pDat_0], #4               // *pDat_0++ = accu3
    STR     accu4, [pDat_1], #-4              // *pDat_1-- = accu4

    SUBS    i, i, #1
    BNE     dst_IV_loop2
  }

  /* Last Sin and Cos value pair are the same */
  accu1 = fMultDiv2(-accu1, WTC(0x5a82799a));
  accu2 = fMultDiv2(-accu2, WTC(0x5a82799a));
  accu1<<=1;
  accu2<<=1;

  *pDat_0 = accu1 + accu2;
  *pDat_1 = accu1 - accu2;
}
#endif /* FUNCTION_dst_IV_func2_CORE */


#if defined(__ARM_NEON__)
#define FUNCTION_dct_II_func1
#endif

#ifdef FUNCTION_dct_II_func1

FDK_ASM_ROUTINE_START(
void, dct_II_func1,(
    FIXP_DBL * __restrict out,         /* r0 */
    FIXP_DBL * __restrict in,          /* r1 */
    INT len))                          /* r2 */
  FDK_add_op_lsl(r3, r0, r2, 3, 2)    // r3: &out[2*len]
  FDK_movs_asr_imm(r2, r2, 2)
  FDK_branch(EQ, dct_II_func1_end)
FDK_label(dct_II_func1_loop)
    FDK_vld2_4d_ia(32, D0, D1, D2, D3, r1)   // load 8 values interleaved into Q0, Q1
    FDK_subs_imm(r2, r2, 1)
    FDK_vshr_s32_q_imm(Q1, Q1, 2)            // Q1 >>= 2
    FDK_vshr_s32_q_imm(Q0, Q0, 2)            // Q0 >>= 2
    FDK_vrev64_q(32, Q1, Q1)                 // exchange pair-wise:   3-2-1-0 => 2-3-0-1
    FDK_vswp(64, D2, D3)                     // exchange D2 with D3:  2-3-0-1 => 0-1-2-3
    FDK_vstm1_ia(128, Q0, r0)                // store even numbered input with pointer post-increment to buffer start
    FDK_vstm1_db(128, Q1, r3)                // store odd  numbered input with pointer pre-decrement to buffer end
    FDK_branch(NE, dct_II_func1_loop)
FDK_label(dct_II_func1_end)
  FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* FUNCTION_dct_II_func1 */
