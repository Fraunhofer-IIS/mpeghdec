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

   Description: bitreversal of input data

*******************************************************************************/

/* clang-format off */
#if !defined(SCRAMBLE_ARM_H)
#define SCRAMBLE_ARM_H

#include "arm/FDK_arm_funcs.h"
#if defined(__ARM_NEON__)
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_scramble_256
#endif

#ifdef FUNCTION_scramble_256
FDK_ASM_ROUTINE_START(void, scramble_256_neonv7,
   (      FIXP_DBL * x,
    const UINT     * bitreverse,
    const UINT     * bits))

#ifndef __ARM_NEON__
#include "arm/FDK_neon_regs.h"
          FIXP_DBL * r0 = x;
    const SCHAR    * r1 = (const SCHAR *) bitreverse;
    const UINT     * r2 = bits;
          UINT       r3;
          UINT       r4;
          UINT       r5;
          UINT       r6;
          FIXP_DBL * r7;
          FIXP_DBL * r8;
          FIXP_DBL * r9;
          FIXP_DBL * r10;
#endif

    FDK_mpush(r4, r10)

    /* Load 16 bytes from bitreverse table and expand them into 32-bit words with shift left by 3+2 */
    FDK_vld1_2d(32, D16, D17, r1)                          // Q8:  0F070B03    0D050901    0E060A02    0C040800
    FDK_vshll_u8_imm(Q10,  D17, 4)                         // Q10: 00F00070    00B00030    00D00050    00900010
    FDK_vshll_u8_imm(Q8,   D16, 4)                         // Q8:  00E00060    00A00020    00C00040    00800000
    FDK_vshll_s16_imm(Q11, D21, 2)                         // Q11: 000003C0    000001C0    000002C0    000000C0
    FDK_vshll_s16_imm(Q10, D20, 2)                         // Q10: 00000340    00000140    00000240    00000040
    FDK_vshll_s16_imm(Q9,  D17, 2)                         // Q9:  00000380    00000180    00000280    00000080
    FDK_vshll_s16_imm(Q8,  D16, 2)                         // Q8:  00000300    00000100    00000200    00000000

    /* Duplicate buffer's base address x, x+128 into Q12 */
    FDK_add_imm(r7, r0, 0x400, 2)                          // r7:   &x[256]
    FDK_vdup_q_reg(32, Q12, r0)                            // Q12:                &x[0]
    FDK_vdup_q_reg(32, Q13, r7)                            // Q13:               &x[256]

    FDK_mov_imm(r5, 8)                                     // r5: Outer loop runs 8 times
FDK_label(scramble_256_neonv7_outer_loop)
      FDK_ldrb_ia(r4, r1, 1)                               // int bitreverse_lsb = (int) (bitreverse[i]>>1);
      FDK_ldr_ia(r3, r2, 4)                                // r3:  bits_256[0..7]
      FDK_lsl_imm(r4, r4, 2)                               // r4: (bitreverse_lsb >>1) << 3

      FDK_vdup_q_reg(32, Q15, r4)                          // Q15:     lsb         lsb      lsb         lsb
      FDK_vadd_s32_q(Q14, Q12, Q15)                        // Q14:  &x[lsb]     &x[lsb]    &x[lsb]    &x[lsb]
      FDK_vadd_s32_q(Q15, Q13, Q15)                        // Q15:  &x[lsb+256] &x[lsb+256]&x[lsb+256]&x[lsb+256]

      FDK_mov_imm(r6, 4)                                   // r6: Inner loop runs 4 times
FDK_label(scramble_256_neonv7_loop)
      FDK_vadd_s32_q(Q2, Q14, Q8)                          // Q2:  &x[revn]         &x[revn]     &x[revn]     &x[revn]
      FDK_vadd_s32_q(Q3, Q15, Q8)                          // Q3:  &x[revn+128] &x[revn+128] &x[revn+128] &x[revn+128]
      FDK_vswp(128,  Q8,  Q9)
      FDK_vswp(128,  Q9, Q10)
      FDK_vswp(128, Q10, Q11)

      FDK_vld1_4d(32, D0, D1, D2, D3, r0)                  // Q0:  x[2*n+3]    x[2*n+2]    x[2*n+1]    x[2*n+0]
                                                           // Q1:  x[2*n+7]    x[2*n+6]    x[2*n+5]    x[2*n+4]
      FDK_vmov_dreg(r7, r8, s8, s9)                        // r7: &x[rev_n0]      r8: &x[rev_n1]
      FDK_vmov_dreg(r9, r10,s12, s13)                      // r9: &x[rev_n0+128] r10: &x[rev_n1+128]

      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst1)
        FDK_vld1_1d(32, D4, r7)
        FDK_vst1_1d(32, D0, r7)
        FDK_vmov_d(D0, D4)

FDK_label(scramble_256_neonv7_tst1)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst2)
        FDK_vld1_1d(32, D4, r9)
        FDK_vst1_1d(32, D1, r9)
        FDK_vmov_d(D1, D4)

FDK_label(scramble_256_neonv7_tst2)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst3)
        FDK_vld1_1d(32, D4, r8)
        FDK_vst1_1d(32, D2, r8)
        FDK_vmov_d(D2, D4)

FDK_label(scramble_256_neonv7_tst3)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst4)
        FDK_vld1_1d(32, D4, r10)
        FDK_vst1_1d(32, D3, r10)
        FDK_vmov_d(D3, D4)

FDK_label(scramble_256_neonv7_tst4)
      FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)

      FDK_vmov_dreg(r7, r8, s10, s11)                      // r7: &x[rev_n2]      r8: &x[rev_n3]
      FDK_vmov_dreg(r9, r10,s14, s15)                      // r9: &x[rev_n2+128] r10: &x[rev_n3+128]

      FDK_vld1_4d(32, D0, D1, D2, D3, r0)                  // Q0:  x[2*n+11]  x[2*n+10]   x[2*n+ 9]  x[2*n+ 8]
                                                           // Q1:  x[2*n+15]  x[2*n+14]   x[2*n+13]  x[2*n+12]
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst5)
        FDK_vld1_1d(32, D4, r7)
        FDK_vst1_1d(32, D0, r7)
        FDK_vmov_d(D0, D4)

FDK_label(scramble_256_neonv7_tst5)
      FDK_vmov_reg(r7, S10)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst6)
        FDK_vld1_1d(32, D4, r9)
        FDK_vst1_1d(32, D1, r9)
        FDK_vmov_d(D1, D4)

FDK_label(scramble_256_neonv7_tst6)
      FDK_vmov_reg(r8, S11)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_tst7)
        FDK_vld1_1d(32, D4, r8)
        FDK_vst1_1d(32, D2, r8)
        FDK_vmov_d(D2, D4)

FDK_label(scramble_256_neonv7_tst7)
      FDK_asrs_imm(r3, r3, 1)
      FDK_branch(CC, scramble_256_neonv7_done)
        FDK_vld1_1d(32, D4, r10)
        FDK_vst1_1d(32, D3, r10)
        FDK_vmov_d(D3, D4)

FDK_label(scramble_256_neonv7_done)
      FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)
      FDK_subs_imm(r6, r6, 1)
      FDK_branch(NE, scramble_256_neonv7_loop)

    FDK_subs_imm(r5, r5, 1)
    FDK_branch(NE, scramble_256_neonv7_outer_loop)

    FDK_mpop(r4, r10)
    FDK_return()
FDK_ASM_ROUTINE_END()

void scramble_256(FIXP_DBL *x);

void scramble_256(FIXP_DBL *x)
{
    static const UINT bitreverse[4] = { 0x0C040800, 0x0E060A02, 0x0D050901, 0x0F070B03 };
    static const UINT bits_256[8]    = { 0xFEFEFFFE, 0xEEEEFEEE, 0xEAEAEEEA, 0xAAAAEAAA, 0xA8A8AAA8, 0x8888A888, 0x80808880, 0x00008000 };
    scramble_256_neonv7(x, bitreverse, bits_256);
}

#endif  /* #ifdef FUNCTION_scramble_256 */

#if defined(FUNCTION_scramble)
#if defined(__GNUC__)

#define FUNCTION_scramble

#if defined(__ARM_ARCH_5TE__)
#define USE_LDRD_STRD   /* LDRD requires 8 byte data alignment. */
#endif

inline void scramble(FIXP_DBL x [], INT n) {
  FDK_ASSERT(!(((INT)x)&(ALIGNMENT_DEFAULT-1)));
  asm("mov     r2, #1;\n"               /* r2(m) = 1;           */
      "sub     r3, %1, #1;\n"           /* r3 = n-1;            */
      "mov     r4, #0;\n"               /* r4(j) = 0;           */

"scramble_m_loop%=:\n"                  /* {                    */
      "mov     r5, %1;\n"               /*  r5(k) = 1;          */

"scramble_k_loop%=:\n"                  /*  {                   */
      "mov     r5, r5, lsr #1;\n"       /*   k >>= 1;           */
      "eor     r4, r4, r5;\n"           /*   j ^=k;             */
      "ands    r10, r4, r5;\n"           /*   r10 = r4 & r5;      */
      "beq     scramble_k_loop%=;\n"      /*  } while (r10 == 0);  */

      "cmp     r4, r2;\n"               /*   if (r4 < r2) break;        */
      "bcc     scramble_m_loop_end%=;\n"

#ifdef USE_LDRD_STRD
      "mov     r5, r2, lsl #3;\n"       /* m(r5) = r2*4*2               */
      "ldrd    r10, [%0, r5];\n"         /* r10 = x[r5], x7 = x[r5+1]     */
      "mov     r6, r4, lsl #3;\n"      /* j(r6) = r4*4*2              */
      "ldrd    r8, [%0, r6];\n"        /* r8 = x[r6], r9 = x[r6+1];  */
      "strd    r10, [%0, r6];\n"        /* x[r6,r6+1] = r10,r11;        */
      "strd    r8, [%0, r5];\n"         /* x[r5,r5+1] = r8,r9;          */
#else
      "mov      r5, r2, lsl #3;\n"       /* m(r5) = r2*4*2               */
      "ldr      r10, [%0, r5];\n"
      "mov      r6, r4, lsl #3;\n"      /* j(r6) = r4*4*2              */
      "ldr      r11, [%0, r6];\n"

      "str      r10, [%0, r6];\n"
      "str      r11, [%0, r5];\n"

      "add      r5, r5, #4;"
      "ldr      r10, [%0, r5];\n"
      "add      r6, r6, #4;"
      "ldr      r11, [%0, r6];\n"
      "str      r10, [%0, r6];\n"
      "str      r11, [%0, r5];\n"
#endif
"scramble_m_loop_end%=:\n"
      "add     r2, r2, #1;\n"           /* r2++;                        */
      "cmp     r2, r3;\n"
      "bcc     scramble_m_loop%=;\n"      /* } while (r2(m) < r3(n-1));   */
       :
       : "r"(x), "r"(n)
#ifdef USE_LDRD_STRD
       : "r2","r3", "r4","r5", "r10","r11", "r8","r9", "r6" );
#else
       : "r2","r3", "r4","r5", "r10","r11", "r6" );
#endif
}
#else
/* Force C implementation if no assembler version available. */
#undef FUNCTION_scramble
#endif  /* Toolchain selection. */

#endif  /* defined(FUNCTION_scramble) */
#endif /* !defined(SCRAMBLE_ARM_H) */
