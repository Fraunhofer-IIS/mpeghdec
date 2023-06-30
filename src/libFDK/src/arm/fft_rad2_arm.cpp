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

   Description: dit_fft ARM assembler replacements.

*******************************************************************************/

/* clang-format off */
#ifndef __FFT_RAD2_CPP__
#error "Do not compile this file separately. It is included on demand from fft_rad2.cpp"
#endif

#if defined(__ARM_AARCH64_NEON__) && defined(SINETABLE_16BIT)
#define FUNCTION_dit_fft

#ifdef  FUNCTION_dit_fft
#include "arm/FDK_aarch64_neon_funcs.h"

//                                                           re0=1.0f             im0=0.0f             re1=sqrt(0.5f)       im1=sqrt(0.5f)      -im0=0.0f             re0=1.0f            -im1=-sqrt(0.5f)       re1=sqrt(0.5f)
RAM_ALIGN static const FIXP_DBL dit_fft_w32_neonv8[8] = { FIXP_DBL(0x7FFFFFFF),FIXP_DBL(0x00000000),FIXP_DBL(0x5a82799a),FIXP_DBL(0x5a82799a),FIXP_DBL(0x00000000),FIXP_DBL(0x7FFFFFFF),FIXP_DBL(0xA57D8666), FIXP_DBL(0x5a82799a)};
A64_ASM_ROUTINE_START(
void ,dit_fft_neonv8,(FIXP_DBL *x, const INT64 ldn, const FIXP_STP *trigdata, INT64 trigDataSize, const FIXP_DBL *dit_fft_w32v8))

#ifndef __ARM_AARCH64_NEON__
  FIXP_DBL       *X0 = x;
  INT64           X1 = ldn;
  const FIXP_STP *X2 = trigdata;
  INT64           X3 = trigDataSize;
  const FIXP_DBL *X4 = dit_fft_w32v8;
#endif

  /* stack contents:
       0x00:

     register call parameter:
       X0: x                    format: 128-bit aligned array with interleaved Q1.31 real/imag samples
       X1: ldn                  format: INT64
       X2: trigdata             format: 128-bit aligned table with packed 2xQ1.15 real/imag coeffs
       X3: trigDataSize         format: INT64
       X4: dit_fft_w32v8        format: 128-bit aligned table with 2x2x2xQ1.31 real/imag coeffs

     register usage:
       X0: const FIXP_DBL pointer to 'x' input/output buffer
       X1: const INT64 ldn
       X2: const FIXP_STP pointer to 'trigdata' data
       X3: const INT trigDatasize
       X4: const FIXP_DBL *dit_fft_w32v8
       X5: INT middle loop counter
       X6: INT inner loop counter
       X7: FIXP_DBL read/write pointer to x
       X8: FIXP_DBL read/write pointer to x
       X9: INT outer loop counter
      X10: const FIXP_STP pointer to 'trigdata' data, used with ++trigstep
      X11: const FIXP_STP pointer to 'trigdata+trigDataSize/2' data, used with --trigstep
      X12: INT trigstep, starts with trigDataSize>>1, is shifted right by 1 per outer loop3
      X13: m
  */

  /* Push preserved SIMD register (bottom part) accoding to calling convention */
  A64_pushD(D8,  D9)
  A64_pushD(D10, D11)
  A64_pushD(D12, D13)
  A64_pushD(D14, D15)

  A64_mov_Xt_imm(X9, 1)           // X9: const 1

  /* Create factor +1/-1 to allow fast conjugation of twiddle factors */
  A64_movi_msl(32, 64, V16_2S, 0x01, 16)  // V16: ------  ------  0x0001,0xFFFF 0x0001,0xFFFF
  A64_rev32(16, 64, V16_4H, V16_4H)       // V16: ------  ------  0xFFFF,0x0001 0xFFFF,0x0001
  A64_movi(32, 64, V17_4H, 0x00)          // V17: ------  ------  0x0000,0x0000 0x0000,0x0000


  //-----------------------------------------------------------------------------
  A64_sub_Xt_imm(X5, X1, 4)       // X5: ldn - 4
  A64_lsl_Xt(X5, X9, X5)          // X5: loop counter for loop1
  A64_mov_Xt(X7, X0)              // X7: read pointer for x
  A64_mov_Xt(X8, X0)              // X8: write pointer for x

A64_label(dit_fft_loop1)
    A64_ld4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 0, X7, 16)  // x[0..3]
    A64_ld4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 0, X7, 16)  // x[4..7]
    A64_ld4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 1, X7, 16)  // ...
    A64_ld4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 1, X7, 16)  //
    A64_ld4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 2, X7, 16)  //
    A64_ld4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 2, X7, 16)  //
    A64_ld4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 3, X7, 16)  //
    A64_ld4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 3, X7, 16)  // [28..31]
    A64_shadd(32, 128, V18_4S, V0_4S, V2_4S)  // a00
    A64_shadd(32, 128, V19_4S, V4_4S, V6_4S)  // a10
    A64_shadd(32, 128, V20_4S, V1_4S, V3_4S)  // a20
    A64_shadd(32, 128, V21_4S, V5_4S, V7_4S)  // a30
    A64_shsub(32, 128, V22_4S, V0_4S, V2_4S)  // s00
    A64_shsub(32, 128, V23_4S, V4_4S, V6_4S)  // s20
    A64_shsub(32, 128, V24_4S, V1_4S, V3_4S)  // s30
    A64_shsub(32, 128, V25_4S, V5_4S, V7_4S)  // s10
    A64_add(32, 128, V0_4S, V18_4S, V19_4S)   // x[0] = a00 + a10 ;
    A64_add(32, 128, V1_4S, V20_4S, V21_4S)   // x[1] = a20 + a30 ;
    A64_add(32, 128, V2_4S, V22_4S, V25_4S)   // x[2] = s00 + s10 ;
    A64_sub(32, 128, V3_4S, V24_4S, V23_4S)   // x[3] = s30 - s20 ;
    A64_sub(32, 128, V4_4S, V18_4S, V19_4S)   // x[4] = a00 - a10 ;
    A64_sub(32, 128, V5_4S, V20_4S, V21_4S)   // x[5] = a20 - a30 ;
    A64_sub(32, 128, V6_4S, V22_4S, V25_4S)   // x[6] = s00 - s10 ;
    A64_add(32, 128, V7_4S, V24_4S, V23_4S)   // x[7] = s30 + s20 ;
    A64_st4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 0, X8, 16)
    A64_st4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 0, X8, 16)
    A64_st4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 1, X8, 16)
    A64_st4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 1, X8, 16)
    A64_st4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 2, X8, 16)
    A64_st4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 2, X8, 16)
    A64_st4_lane_IA(32, V0_S, V1_S, V2_S, V3_S, 3, X8, 16)
    A64_st4_lane_IA(32, V4_S, V5_S, V6_S, V7_S, 3, X8, 16)
    A64_subs_imm(X5, X5, 1)
    A64_branch(NE, dit_fft_loop1)
  //-----------------------------------------------------------------------------

  A64_sub_Xt_imm(X5, X1, 3)          // X5: ldn - 3
  A64_lsl_Xt(X5, X9, X5)             // X5: loop counter for loop2
  A64_mov_Xt(X7, X0)                 // X7: read pointer for x
  A64_mov_Xt(X8, X0)                 // X8: write pointer for x

  // Preload V0,V1 with const 32-bit twiddle factors:
  // re0: 1.0f         im0: 0.0f,
  // re1: sqrt(1/2)    im1: sqrt(1/2))
  // lane   3     2    1    0
  // V1:   re1  re0   im1  im0
  // V0:  -im1 -im0   re1  re0
  A64_ld2x2(32, 128, V0_4S, V1_4S, X4)

A64_label(dit_fft_loop2)
    // Load x[0..15] into V2..V5 with 2-interleaving
    A64_ld2x2_IA(32, 128, V2_4S, V3_4S, X7, 32)
    A64_ld2x2_IA(32, 128, V4_4S, V5_4S, X7, 32)
    // lane   3     2     1     0
    // Q5:  x15   x13   x11    x9
    // Q4:  x14   x12   x10    x8
    // Q3:   x7    x5    x3    x1    ui
    // Q2:   x6    x4    x2    x0    ur

    A64_sqdmulh(32, 128, V6_4S, V5_4S, V0_4S)  // V6: vi -> 1st part (doubled)
    A64_sqdmulh(32, 128, V7_4S, V4_4S, V1_4S)  // V7: vi -> 2nd part (doubled)
    A64_sqdmulh(32, 128, V8_4S, V4_4S, V0_4S)  // V6: vr -> 1st part (doubled)
    A64_sqdmulh(32, 128, V9_4S, V5_4S, V1_4S)  // V7: vr -> 2nd part (doubled)
    A64_sub(32, 128, V4_4S, V6_4S, V7_4S)      // V4: vi (doubled)
    A64_add(32, 128, V5_4S, V8_4S, V9_4S)      // V5; vr (doubled)
    A64_shadd(32, 128, V6_4S, V2_4S, V5_4S)    // V6: (ur+vr)>>1
    A64_shadd(32, 128, V7_4S, V3_4S, V4_4S)    // V7: (ui+vi)>>1
    A64_shsub(32, 128, V8_4S, V2_4S, V5_4S)    // V8: (ur-vr)>>1
    A64_shsub(32, 128, V9_4S, V3_4S, V4_4S)    // V9: (ui-vi)>>1
    // Store x[0..15] from Q6..Q9 with 2-interleaving
    A64_st2x2_IA(32, 128, V6_4S, V7_4S, X8, 32)
    A64_st2x2_IA(32, 128, V8_4S, V9_4S, X8, 32)

    A64_subs_imm(X5, X5, 1)
    A64_branch(NE, dit_fft_loop2)
  //-----------------------------------------------------------------------------
  A64_asr_Xt_imm(X3, X3, 1)             // X3:  trigDataSize >>= 1
  A64_mov_Xt(X12, X3)                   // X12: trigstep = trigDataSize >> 1
  A64_sub_Xt_imm(X9, X1, 3)             // X9:  number of outer loops: ldn-3
  A64_mov_Xt_imm(X13, 1)                // X13: const 1
  A64_lsl_Xt(X1, X13, X1)               // X1:  n = 1 << ldn
  A64_mov_Xt_imm(X13, 8)                // X13: m = 8, mq = 2, mo = 1

A64_label(dit_fft_loop3_outer)
    A64_lsl_Xt_imm(X13, X13, 1)                // X13:  m <<= 1, m=16,32,... mq=4,8,.. mo=2,4,...
    A64_asr_Xt_imm(X12, X12, 1)                // X12: trigstep = trigstep>>1
    A64_add_Xt_lsl_imm(X10, X2, X12, 2)        // X10: trigdata[trigstep]
    A64_add_Xt_lsl_imm(X11, X2, X3,  2)        // X11: trigdata[trigDataSize/2]
    A64_sub_Xt_lsl_imm(X11, X11,X12, 2)        // X11: trigdata[trigDataSize/2-trigstep]
    A64_mov_Xt(X7, X0)                         // X7: &x[0]
    A64_mov_Xt(X5, X13)                        // X5: j=[0..m[ in steps of 16 => for(j=0; j<m/8; j+=2)

    // Preload re0,im0...re3,im3 and mirror/conjugate each pair
    A64_ld2x2(32, 128, V0_4S, V1_4S, X4)       // V0:     -im1       -im0        re1        re0
                                               // V1:      re1        re0        im1        im0
    A64_ld1_lane(32, V2_S, 0, X10)             // V2:  ---------  ---------  ---------  (+im2/re2)
    A64_ld1_lane(32, V3_S, 0, X11)             // V3:  ---------  ---------  ---------  (re3/+im3)  <- to be reversed in order
    A64_rev32(16, 64, V3_4H, V3_4H)            // V3:  ---------  ---------  ---------  (+im3/re3)
    A64_trn1(32, 64, V2_2S, V2_2S, V3_2S)      // V2:  ---------  ---------  (+im3/re3) (+im2/re2)
    A64_mul(16, 64, V4_4H, V2_4H, V16_4H)      // V4:  ---------  ---------  (-im3/re3) (-im2/re2)
    A64_rev32(16, 64, V4_4H, V4_4H)            // V4:  ---------  ---------  (re3/-im3) (re2/-im2)
    A64_uzp1(16, 64, V3_4H, V2_4H, V4_4H)      // V3:  ---------  ---------  -im3 -im2   re3  re2
    A64_uzp2(16, 64, V4_4H, V2_4H, V4_4H)      // V4:  ---------  ---------   re3  re2   im3  im2
    A64_zip1(16, 128, V2_8H, V17_8H, V3_8H)    // V2:    -im3       -im2        re3        re2
    A64_zip1(16, 128, V3_8H, V17_8H, V4_8H)    // V3:     re3        re2        im3        im2
    // lane  3    2    1    0
    // V3: +re3 +re2 +im3 +im2
    // V2: -im3 -im2 +re3 +re2
    // V1: +re1 +re0 +im1 +im0
    // V0: -im1 -im0 +re1 +re0
    // Update trigdata pointers X10/X11 to next entries
    A64_add_Xt_lsl_imm(X10, X10, X12, 2) // X10: trigdata[2*trigstep]
    A64_sub_Xt_lsl_imm(X11, X11, X12, 2) // X11: trigdata[trigDataSize/2-2*trigstep]

A64_label(dit_fft_loop3_mid)
      A64_mov_Xt(X6, X1)                       // X6:  r=[0..n[ in steps of m
A64_label(dit_fft_loop3_inner)                 // comment is mostly valid only for lane 0 !!!
        A64_add_Xt_lsl_imm(X7,X7,X13,2)        // X7: &x[t0+4*mq]
        A64_ld4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 0, X7, X13)  // x[t0+4*mq+0,1,2,3]
        A64_ld4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 1, X7, X13)  // x[t0+5*mq+0,1,2,3]
        A64_ld4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 2, X7, X13)  // x[t0+6*mq+0,1,2,3]
        A64_ld4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 3, X7, X13)  // x[t0+7*mq+0,1,2,3]
        A64_sub_Xt_lsl_imm(X7,X7,X13,3)              // X7: &x[t0+8*mq] -> &x[t0+0*mq]
        A64_sqdmulh(32, 128, V8_4S,  V4_4S, V0_4S)   // V8:  vr -> 2nd part (x0*re0,doubled)
        A64_sqdmulh(32, 128, V11_4S, V5_4S, V1_4S)   // V11: vr -> 2st part (x1*im0,doubled)
        A64_sqdmulh(32, 128, V9_4S,  V4_4S, V1_4S)   // V9:  vi -> 2nd part (x0*im0,doubled)
        A64_sqdmulh(32, 128, V10_4S, V5_4S, V0_4S)   // V10: vi -> 1st part (x1*re0,doubled)
        A64_add(32, 128, V12_4S, V11_4S, V8_4S)      // V12: vr0 (doubled)
        A64_sub(32, 128, V13_4S, V10_4S, V9_4S)      // V13; vi0 (doubled)
        A64_sqdmulh(32, 128,  V8_4S, V6_4S, V2_4S)   // V8:  vr -> 2nd part (x2*re2,doubled)
        A64_sqdmulh(32, 128, V11_4S, V7_4S, V3_4S)   // V11: vr -> 2st part (x3*im2,doubled)
        A64_sqdmulh(32, 128,  V9_4S, V6_4S, V3_4S)   // V9:  vi -> 2nd part (x2*im2,doubled)
        A64_sqdmulh(32, 128, V10_4S, V7_4S, V2_4S)   // V10: vi -> 1st part (x3*re2,doubled)
        A64_add(32, 128, V14_4S, V11_4S, V8_4S)      // V14: vr2 (doubled)
        A64_sub(32, 128, V15_4S, V10_4S, V9_4S)      // V15; vi2 (doubled)
        A64_ld4_lane_PU(32, V8_S, V9_S,V10_S,V11_S, 0, X7, X13)  // x[t0+0*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> V8: ur0
        A64_ld4_lane_PU(32, V8_S, V9_S,V10_S,V11_S, 1, X7, X13)  // x[t0+1*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> V9: ui0
        A64_ld4_lane_PU(32, V8_S, V9_S,V10_S,V11_S, 2, X7, X13)  // x[t0+2*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> V10 ur2
        A64_ld4_lane_PU(32, V8_S, V9_S,V10_S,V11_S, 3, X7, X13)  // x[t0+3*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> V11 ui2
        A64_sub_Xt_lsl_imm(X7,X7,X13,2)              // X7: &x[t0+4*mq] -> &x[t0+0*mq]
        A64_shadd(32, 128, V4_4S, V8_4S, V12_4S)     // V4: (ur0+vr0)>>1
        A64_shadd(32, 128, V5_4S, V9_4S, V13_4S)     // V5: (ui0+vi0)>>1
        A64_shadd(32, 128, V6_4S,V10_4S, V14_4S)     // V6: (ur2+vr2)>>1
        A64_shadd(32, 128, V7_4S,V11_4S, V15_4S)     // V7: (ui2+vi2)>>1
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 0, X7, X13)  // x[t0+0*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 1, X7, X13)  // x[t0+1*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 2, X7, X13)  // x[t0+2*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 3, X7, X13)  // x[t0+3*mq+0,1,2,3]
        A64_shsub(32, 128, V4_4S, V8_4S, V12_4S)     // V4: (ur0-vr0)>>1
        A64_shsub(32, 128, V5_4S, V9_4S, V13_4S)     // V5: (ui0-vi0)>>1
        A64_shsub(32, 128, V6_4S,V10_4S, V14_4S)     // V6: (ur2-vr2)>>1
        A64_shsub(32, 128, V7_4S,V11_4S, V15_4S)     // V7: (ui2-vi2)>>1
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 0, X7, X13)  // x[t0+4*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 1, X7, X13)  // x[t0+5*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 2, X7, X13)  // x[t0+6*mq+0,1,2,3]
        A64_st4_lane_PU(32, V4_S, V5_S, V6_S, V7_S, 3, X7, X13)  // x[t0+7*mq+0,1,2,3]
        A64_subs(X6, X6, X13)
        A64_branch(NE, dit_fft_loop3_inner)

      A64_subs_imm(X5, X5, 16)
      A64_branch(EQ, dit_fft_loop3_outer_cont)
      A64_sub_Xt_lsl_imm(X7,X7,X1,3)           // X7: &x[2*j+2*n] -> &x[2*j]
      A64_add_Xt_imm(X7,X7,16)                 // t0 += 4

      A64_ld1_lane(32, V2_S, 0, X10)           // V2:  ---------  ---------  ---------  (+im0/re0)
      A64_ld1_lane(32, V3_S, 0, X11)           // V3:  ---------  ---------  ---------  (re1/+im1)
      A64_add_Xt_lsl_imm(X10, X10, X12, 2)
      A64_sub_Xt_lsl_imm(X11, X11, X12, 2)
      A64_rev32(16, 64, V3_4H, V3_4H)          // V3:  ---------  ---------  ---------  (+im1/re1)
      A64_trn1(32, 64, V2_2S, V2_2S, V3_2S)    // V2:  ---------  ---------  (+im1/re1) (+im0/re0)
      A64_mul(16, 64, V4_4H, V2_4H, V16_4H)    // V4:  ---------  ---------  (-im1/re1) (-im0/re0)
      A64_rev32(16, 64, V4_4H, V4_4H)          // V4:  ---------  ---------  (re1/-im1) (re0/-im0)
      A64_uzp1(16, 64, V3_4H, V2_4H, V4_4H)    // V3:  ---------  ---------  -im1 -im0   re1  re0
      A64_uzp2(16, 64, V4_4H, V2_4H, V4_4H)    // V4:  ---------  ---------   re1  re0   im1  im0
      A64_zip1(16, 128, V0_8H, V17_8H, V3_8H)  // V0:    -im1       -im0        re1        re0
      A64_zip1(16, 128, V1_8H, V17_8H, V4_8H)  // V1:     re1        re0        im1        im0

      A64_ld1_lane(32, V2_S, 0, X10)           // V2:  ---------  ---------  ---------  (+im2/re2)
      A64_ld1_lane(32, V3_S, 0, X11)           // V3:  ---------  ---------  ---------  (re3/+im3)  <- to be reversed in order
      A64_add_Xt_lsl_imm(X10, X10, X12, 2)
      A64_sub_Xt_lsl_imm(X11, X11, X12, 2)
      A64_rev32(16, 64, V3_4H, V3_4H)          // V3:  ---------  ---------  ---------  (+im3/re3)
      A64_trn1(32, 64, V2_2S, V2_2S, V3_2S)    // V2:  ---------  ---------  (+im3/re3) (+im2/re2)
      A64_mul(16, 64, V4_4H, V2_4H, V16_4H)    // V4:  ---------  ---------  (-im3/re3) (-im2/re2)
      A64_rev32(16, 64, V4_4H, V4_4H)          // V4:  ---------  ---------  (re3/-im3) (re2/-im2)
      A64_uzp1(16, 64, V3_4H, V2_4H, V4_4H)    // V3:  ---------  ---------  -im3 -im2   re3  re2
      A64_uzp2(16, 64, V4_4H, V2_4H, V4_4H)    // V4:  ---------  ---------   re3  re2   im3  im2
      A64_zip1(16, 128, V2_8H, V17_8H, V3_8H)  // V2:    -im3       -im2        re3        re2
      A64_zip1(16, 128, V3_8H, V17_8H, V4_8H)  // V3:     re3        re2        im3        im2

      A64_branch(NE, dit_fft_loop3_mid)
A64_label(dit_fft_loop3_outer_cont)
    A64_subs_imm(X9, X9, 1)
    A64_branch(NE, dit_fft_loop3_outer)
  //-----------------------------------------------------------------------------

  /* Pop preserved SIMD register (bottom part) */
  A64_popD(D14, D15)
  A64_popD(D12, D13)
  A64_popD(D10, D11)
  A64_popD(D8,  D9)

A64_ASM_ROUTINE_END()

A64_ASM_ROUTINE_START(void ,scramble_ARMv8,(INT64 *x, const INT64 ldn))
#ifndef __ARM_AARCH64_NEON__
  INT64 *X0 = x;
  INT64  X1 = ldn;
  INT64  X2, X3, X4, X5, X6, X7, X10, X11, X12, X13;
#endif
  /* Quartered version, for speed-up reason */
  /* Bit-reversed index for element #1: brev(#4n+1) = brev(#4n)+size*2/4 */
  /* Bit-reversed index for element #2: brev(#4n+2) = brev(#4n)+size*1/4 */
  /* Bit-reversed index for element #3: brev(#4n+3) = brev(#4n)+size*3/4 */
  /* In Q1, for each 4 elements,                  #0 needs checking, #1,#2,#3 is always bit-reversed */
  /* In Q2, for each 4 elements, #0 is skipped,   #2 needs checking, #1,   #3 is always bit-reversed */
  /* In Q3, for each 4 elements, #0,2 is skipped, #1 needs checking,       #3 is always bit-reversed */
  /* In Q4, for each 4 elements, #0-2 is skipped, #3 needs checking, break at index X2==X4 */

  A64_mov_Xt_imm(X2, 8)             // X2: 8 = First address offset to bit-reverse
  A64_lsl_Xt(X4, X2, X1)            // X4: (8 << ldn)

  A64_add_Xt_lsr_imm(X11,  X0, X4, 1)   // X11: pointer to X + size*2/4
  A64_add_Xt_lsr_imm(X12,  X0, X4, 2)   // X12: pointer to X + size*1/4
  A64_add_Xt_lsr_imm(X13, X12, X4, 1)   // X13: pointer to X + size*3/4

  A64_mov_Xt_imm(X5, 58)            // X5: 64 -2*3
  A64_sub_Xt(X5, X5, X1)            // X5: 64 -2*3 - ldn <- LSR for X3

  A64_add_Xt_imm(X3, X1, 1)         // X1: ldn+1
  A64_asr_Xt_imm(X3, X3, 1)         // X1: (ldn+1)/2
  A64_lsl_Xt(X3, X2, X3)            // X3: 8 << ((ldn+1)/2)

  A64_lsr_Xt_imm(X10, X4, 2)        // X10: limit = size/4 for 1st quarter
  A64_sub_Xt(X4, X4, X3)            // X4: Loop break limit, reached by X2

  /* First quarter */
  A64_mov_Xt_imm(X2, 0)             // X2: 8 = First address offset to bit-reverse
A64_label(scramble_ARMv8_loopQ1)
  A64_rbit_Xt(X3, X2)               // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)            // X3: normalized to X2
  A64_cmp_Xt(X3, X2)
  A64_branch(LE, scramble_ARMv8_loopQ1_cont)
    /* Swap index #0, if X3 > X2 */
    A64_ldr_Xt_X_LSL(X6, X0, X2, 0)           // X4 = *(X0+X2)
    A64_ldr_Xt_X_LSL(X7, X0, X3, 0)           // X5 = *(X0+X3)
    A64_str_Xt_X_LSL(X6, X0, X3, 0)           // *(X0+X3) = X4
    A64_str_Xt_X_LSL(X7, X0, X2, 0)           // *(X0+X2) = X5

A64_label(scramble_ARMv8_loopQ1_cont)
  /* Swap index #1,#2,3 always */
  A64_add_Xt_imm(X2, X2, 8)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X11, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X11, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  A64_add_Xt_imm(X2, X2, 8)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X12, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X12, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  A64_add_Xt_imm(X2, X2, 8)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X13, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X13, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  A64_add_Xt_imm(X2, X2, 8)
  A64_cmp_Xt(X2, X10)
  A64_branch(NE, scramble_ARMv8_loopQ1)

  /* Second quarter */
  A64_lsl_Xt_imm(X10, X10, 1)        // X10: limit = size/2 for 1st quarter
A64_label(scramble_ARMv8_loopQ2)
  A64_rbit_Xt(X3, X2)               // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)            // X3: normalized to X2

  /* Swap index #1,3 always */
  A64_add_Xt_imm(X2, X2, 8)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X11, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X11, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  A64_add_Xt_imm(X2, X2, 16)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X13, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X13, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  A64_sub_Xt_imm(X2, X2, 8)
  A64_rbit_Xt(X3, X2)               // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)            // X3: normalized to X2
  A64_cmp_Xt(X3, X2)
  A64_branch(LE, scramble_ARMv8_loopQ2_cont)
    A64_ldr_Xt_X_LSL(X6, X0, X2, 0)           // X4 = *(X0+X2)
    A64_ldr_Xt_X_LSL(X7, X0, X3, 0)           // X5 = *(X0+X3)
    A64_str_Xt_X_LSL(X6, X0, X3, 0)           // *(X0+X3) = X4
    A64_str_Xt_X_LSL(X7, X0, X2, 0)           // *(X0+X2) = X5
A64_label(scramble_ARMv8_loopQ2_cont)

  A64_add_Xt_imm(X2, X2, 16)
  A64_cmp_Xt(X2, X10)
  A64_branch(NE, scramble_ARMv8_loopQ2)

  /* Third quarter */
  A64_add_Xt_lsr_imm(X10, X10, X10, 1)  // X10: 3/2 compared to Q2
A64_label(scramble_ARMv8_loopQ3)
  A64_rbit_Xt(X3, X2)                  // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)               // X3: normalized to X2

  /* Skip index #0,2 always */
  /* Swap index #3 always */
  A64_add_Xt_imm(X2, X2, 24)
  A64_ldr_Xt_X_LSL(X6, X0,  X2, 0)           // X4 = *(X0+X2)
  A64_ldr_Xt_X_LSL(X7, X13, X3, 0)           // X5 = *(X0+X3)
  A64_str_Xt_X_LSL(X6, X13, X3, 0)           // *(X0+X3) = X4
  A64_str_Xt_X_LSL(X7, X0,  X2, 0)           // *(X0+X2) = X5

  /* Check index #1 */
  A64_sub_Xt_imm(X2, X2, 16)
  A64_rbit_Xt(X3, X2)               // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)            // X3: normalized to X2
  A64_cmp_Xt(X3, X2)
  A64_branch(LE, scramble_ARMv8_loopQ3_cont)
    A64_ldr_Xt_X_LSL(X6, X0, X2, 0)           // X4 = *(X0+X2)
    A64_ldr_Xt_X_LSL(X7, X0, X3, 0)           // X5 = *(X0+X3)
    A64_str_Xt_X_LSL(X6, X0, X3, 0)           // *(X0+X3) = X4
    A64_str_Xt_X_LSL(X7, X0, X2, 0)           // *(X0+X2) = X5
A64_label(scramble_ARMv8_loopQ3_cont)

  A64_add_Xt_imm(X2, X2, 24)
  A64_cmp_Xt(X2, X10)
  A64_branch(NE, scramble_ARMv8_loopQ3)

  /* Fourth quarter */
  A64_add_Xt_imm(X2, X2, 24)           // Increment index to index #3
A64_label(scramble_ARMv8_loopQ4)
  /* Skip index #0,1,2 always */
  /* Check index #3 */
  A64_rbit_Xt(X3, X2)               // X3: bitreversed X2
  A64_lsr_Xt(X3, X3, X5)            // X3: normalized to X2
  A64_cmp_Xt(X3, X2)
  A64_branch(LE, scramble_ARMv8_loopQ4_cont)
    A64_ldr_Xt_X_LSL(X6, X0, X2, 0)           // X4 = *(X0+X2)
    A64_ldr_Xt_X_LSL(X7, X0, X3, 0)           // X5 = *(X0+X3)
    A64_str_Xt_X_LSL(X6, X0, X3, 0)           // *(X0+X3) = X4
    A64_str_Xt_X_LSL(X7, X0, X2, 0)           // *(X0+X2) = X5
A64_label(scramble_ARMv8_loopQ4_cont)

  A64_add_Xt_imm(X2, X2, 32)
  A64_cmp_Xt(X2, X4)
  A64_branch(LT, scramble_ARMv8_loopQ4)

A64_ASM_ROUTINE_END()

void dit_fft(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, const INT trigDataSize)
{
  scramble_ARMv8((INT64*)x, (INT64) ldn);
  dit_fft_neonv8(x, ldn, trigdata, trigDataSize, dit_fft_w32_neonv8);
}
#endif /* #ifdef  FUNCTION_dit_fft */
#endif  /* #if defined(__ARM_AARCH64_NEON__) && defined(SINETABLE_16BIT) && defined(FDKTOOLS_PACKED_TABLES) */

#if (defined (__ARM_NEON__) && defined(SINETABLE_16BIT) && !defined(FUNCTION_dit_fft))
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#if defined(__x86__)
#include "arm/FDK_neon_regs.h"
//#define FUNCTION_dit_ifft
#endif
// Do not compile with 3.1 armcc (version=310862)
#define FUNCTION_dit_fft
//#define FUNCTION_dit_ifft



#ifdef FUNCTION_dit_fft

//                                                         1.0f               0.0f               sqrt(0.5f)          sqrt(0.5f)
RAM_ALIGN
static const FIXP_DBL dit_fft_w32_neon[4] = { FIXP_DBL(0x7FFFFFFF),FIXP_DBL(0x00000000),FIXP_DBL(0x5a82799a),FIXP_DBL(0x5a82799a)};
FDK_ASM_ROUTINE_START(void, dit_fft_neon,(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, INT trigDataSize, const FIXP_DBL *dit_fft_w32))
#ifndef __ARM_NEON__
      FIXP_DBL *r0 = x;
      INT       r1 = ldn;
const FIXP_STP *r2 = trigdata;
      INT       r3 = trigDataSize;
      INT r4, r5, r6;
      FIXP_DBL *r7, *r8;
      const FIXP_DBL *r9;
      const FIXP_STP *r10;
      const FIXP_STP *r11;
      INT r12, lr;
#endif

  /* stack contents:
       0x28:     dit_fft_w32_neon
       0x24:     r4
       0x20:     r5
       0x1C:     r6
       0x18:     r7
       0x14:     r8
       0x10:     r9
       0x0C:     r10
       0x08:     r11
       0x04:     r12
       0x00:     lr

     register contents:
       r0: x                    format: Q1.31
       r1: ldn                  format: INT
       r2: trigdata             format: Q1.15
       r3: trigDataSize         format: Q1.31

     register usage:
       r0: const FIXP_DBL pointer to 'x' input/output buffer
       r1: const INT ldn
       r2: const FIXP_STP pointer to 'trigdata' data
       r3: const INT trigDatasize
       r4: INT outer loop counter
       r5: INT middle loop counter
       r6: INT inner loop counter
       r7: FIXP_DBL read/write pointer to x
       r8: FIXP_DBL read/write pointer to x
       r9: const FIXP_DBL *dit_fft_w32
      r10: const FIXP_STP pointer to 'trigdata' data, used with ++trigstep
      r11: const FIXP_STP pointer to 'trigdata+trigDataSize/2' data, used with --trigstep
      r12: INT trigstep, starts with trigDataSize>>1, is shifted right by 1 per outer loop3
       lr: m
  */

    FDK_mpush_lr(r4,r12)            // save all registers incl. lr
    FDK_mov_imm(r4, 1)              // r4: const 1

    //-----------------------------------------------------------------------------
    FDK_sub_imm(r5, r1, 4, 0)       // r5: ldn - 4
    FDK_lsl(r5, r4, r5)             // r5: loop counter for loop1
    FDK_mov_reg(r7, r0)             // r7: read pointer for x
    FDK_mov_reg(r8, r0)             // r8: write pointer for x
    FDK_ldr(r9,sp,0x28,dit_fft_w32) // r9: pointer to twiddle factors (used in 2nd loop)
    FDK_mvpush(Q4,Q7)

FDK_label(dit_fft_loop1)
      FDK_vld4_ia_align(32, S0, S4, S8,S12, r7, 64)  // x[0..3]
      FDK_vld4_ia_align(32,S16,S20,S24,S28, r7, 64)  // x[4..7]
      FDK_vld4_ia_align(32, S1, S5, S9,S13, r7, 64)  // ...
      FDK_vld4_ia_align(32,S17,S21,S25,S29, r7, 64)
      FDK_vld4_ia_align(32, S2, S6,S10,S14, r7, 64)
      FDK_vld4_ia_align(32,S18,S22,S26,S30, r7, 64)
      FDK_vld4_ia_align(32, S3, S7,S11,S15, r7, 64)  // ...
      FDK_vld4_ia_align(32,S19,S23,S27,S31, r7, 64)  // x[28..31]
      FDK_vrhadd_s32_q(Q8,  Q0, Q2)  // a00
      FDK_vrhadd_s32_q(Q9,  Q4, Q6)  // a10
      FDK_vrhadd_s32_q(Q10, Q1, Q3)  // a20
      FDK_vrhadd_s32_q(Q11, Q5, Q7)  // a30
      FDK_vhsub_s32_q(Q12, Q0, Q2)   // s00
      FDK_vhsub_s32_q(Q13, Q4, Q6)   // s20
      FDK_vhsub_s32_q(Q14, Q1, Q3)   // s30
      FDK_vhsub_s32_q(Q15, Q5, Q7)   // s10
      FDK_vadd_s32_q(Q0, Q8, Q9)     // x[0] = a00 + a10 ;
      FDK_vadd_s32_q(Q1,Q10,Q11)     // x[1] = a20 + a30 ;
      FDK_vadd_s32_q(Q2,Q12,Q15)     // x[2] = s00 + s10 ;
      FDK_vsub_s32_q(Q3,Q14,Q13)     // x[3] = s30 - s20 ;
      FDK_vsub_s32_q(Q4, Q8, Q9)     // x[4] = a00 - a10 ;
      FDK_vsub_s32_q(Q5,Q10,Q11)     // x[5] = a20 - a30 ;
      FDK_vsub_s32_q(Q6,Q12,Q15)     // x[6] = s00 - s10 ;
      FDK_vadd_s32_q(Q7,Q14,Q13)     // x[7] = s30 + s20 ;
      FDK_vst4_ia_align(32, S0, S4, S8,S12, r8, 64)
      FDK_vst4_ia_align(32,S16,S20,S24,S28, r8, 64)
      FDK_vst4_ia_align(32, S1, S5, S9,S13, r8, 64)
      FDK_vst4_ia_align(32,S17,S21,S25,S29, r8, 64)
      FDK_vst4_ia_align(32, S2, S6,S10,S14, r8, 64)
      FDK_vst4_ia_align(32,S18,S22,S26,S30, r8, 64)
      FDK_vst4_ia_align(32, S3, S7,S11,S15, r8, 64)
      FDK_vst4_ia_align(32,S19,S23,S27,S31, r8, 64)
      FDK_subs_imm(r5, r5, 1)
      FDK_branch(NE, dit_fft_loop1)

    //-----------------------------------------------------------------------------
    FDK_sub_imm(r5, r1, 3, 0)       // r5: ldn - 3
    FDK_lsl(r5, r4, r5)             // r5: loop counter for loop2
    FDK_mov_reg(r7, r0)             // r7: read pointer for x
    FDK_mov_reg(r8, r0)             // r8: write pointer for x

    // Preload Q0,Q1 with const 32-bit twiddle factors:
    // re0: 1.0f         im0: 0.0f,
    // re1: sqrt(1/2)    im1: sqrt(1/2))
    // lane   3     2    1    0
    // Q1:   re1  re0   im1  im0
    // Q0:  -im1 -im0   re1  re0
    FDK_vld2_2d_align(32, D0, D2, r9, 64)
    FDK_vneg_d(32, D1, D2)
    FDK_vmov_d(D3, D0)

FDK_label(dit_fft_loop2)
      // Load x[0..15] into Q2..Q5 with 2-interleaving
      FDK_vld2_4d_ia(32, D4, D5, D6, D7, r7)
      FDK_vld2_4d_ia(32, D8, D9,D10,D11, r7)
      // lane   3     2     1     0
      // Q5:  x15   x13   x11    x9    vi
      // Q4:  x14   x12   x10    x8    vr
      // Q3:   x7    x5    x3    x1    ui
      // Q2:   x6    x4    x2    x0    ur

      FDK_vqdmulh_s32_qq(Q6,Q5,Q0)  // Q6: vi -> 1st part (doubled)
      FDK_vqdmulh_s32_qq(Q7,Q4,Q1)  // Q7: vi -> 2nd part (doubled)
      FDK_vqdmulh_s32_qq(Q8,Q4,Q0)  // Q8: vr -> 1st part (doubled)
      FDK_vqdmulh_s32_qq(Q9,Q5,Q1)  // Q9: vr -> 2nd part (doubled)
      FDK_vsub_s32_q(Q4,Q6,Q7)      // Q4: vi (doubled)
      FDK_vadd_s32_q(Q5,Q8,Q9)      // Q5; vr (doubled)
      FDK_vrhadd_s32_q(Q6, Q2, Q5)  // Q6: (ur+vr)>>1
      FDK_vrhadd_s32_q(Q7, Q3, Q4)  // Q7: (ui+vi)>>1
      FDK_vhsub_s32_q(Q8, Q2, Q5)   // Q8: (ur-vr)>>1
      FDK_vhsub_s32_q(Q9, Q3, Q4)   // Q9: (ui-vi)>>1

      // Store x[0..15] from Q6..Q9 with 2-interleaving
      FDK_vst2_4d_ia_align(32, D12, D13, D14, D15, r8, 64)
      FDK_vst2_4d_ia_align(32, D16, D17, D18, D19, r8, 64)

      FDK_subs_imm(r5, r5, 1)
      FDK_branch(NE, dit_fft_loop2)
    //-----------------------------------------------------------------------------
    FDK_asr_imm(r3, r3, 1)                // r3:  trigDataSize >>= 1
    FDK_mov_reg(r12, r3)                  // r12: trigstep = trigDataSize >> 1
    FDK_sub_imm(r4, r1, 3, 0)             // r4:  number of outer loops: ldn-3
    FDK_mov_imm(lr, 1)                    // lr:  const 1
    FDK_lsl(r1, lr, r1)                   // r1:  n = 1 << ldn
    FDK_mov_imm(lr, 8)                    // lr:  m = 8, mq = 2, mo = 1

FDK_label(dit_fft_loop3_outer)
      FDK_lsl_imm(lr, lr, 1)              // lr:  m <<= 1, m=16,32,... mq=4,8,.. mo=2,4,...
      FDK_asr_imm(r12, r12, 1)            // r12: trigstep = trigstep>>1
      FDK_add_op_lsl(r10, r2, r12, 2, 2)  // r10: trigdata[trigstep]
      FDK_add_op_lsl(r11, r2, r3,  2, 2)  // r11: trigdata[trigDataSize/2]
      FDK_sub_op_lsl(r11, r11,r12, 2, 2)  // r11: trigdata[trigDataSize/2-trigstep]
      FDK_mov_reg(r7, r0)                 // r7: &x[0]
      FDK_mov_reg(r5, lr)                 // r5: j=[0..m[ in steps of 16 => for(j=0; j<m/8; j+=2)

      // Preload re0,im0...re3,im3 and mirror/conjugate each pair
      FDK_vld2_2d_align(32, D0, D2, r9, 64)
      FDK_vneg_d(32, D1, D2)
      FDK_vmov_d(D3, D0)
      FDK_vld1(32, S8, r10)
      FDK_vld1(32, S9, r11)               // D4: re3  im3  im2  re2 <- need to reverse order of re3/im3
      FDK_vshll_s16_imm(Q2, D4, 16)       // Q2: re3  im3  im2  re2
      FDK_vrev64_d(32, D6, D5)            // D6: im3  re3
                                          // D4: im2  re2
      FDK_vzip_d(32, D4, D6)              // D6: im3  im2            D4: re3 re2
      FDK_vmov_d(D7, D4)                  // Q3: re3  re2  im3  im2
      FDK_vneg_d(32, D5, D6)              // Q2:-im3 -im2  re3  re2
      // lane  3    2    1    0
      // Q3: +re3 +re2 +im3 +im2
      // Q2: -im3 -im2 +re3 +re2
      // Q1: +re1 +re0 +im1 +im0
      // Q0: -im1 -im0 +re1 +re0
      // Update trigdata pointers r10/r11 to next entries
      FDK_add_op_lsl(r10, r10, r12, 2, 2) // r10: trigdata[2*trigstep]
      FDK_sub_op_lsl(r11, r11, r12, 2, 2) // r11: trigdata[trigDataSize/2-2*trigstep]

FDK_label(dit_fft_loop3_mid)
        FDK_mov_reg(r6, r1)               // r6:  r=[0..n[ in steps of m
FDK_label(dit_fft_loop3_inner)            // comment is mostly valid only for lane 0 !!!
          FDK_add_op_lsl(r7,r7,lr,2,2)    // r7: &x[t0+4*mq]
          FDK_vld4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+4*mq+0,1,2,3]
          FDK_vld4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+5*mq+0,1,2,3]
          FDK_vld4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+6*mq+0,1,2,3]
          FDK_vld4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+7*mq+0,1,2,3]
          FDK_sub_op_lsl(r7,r7,lr,3,2)    // r7: &x[t0+8*mq] -> &x[t0+0*mq]
          FDK_vqdmulh_s32_qq( Q8, Q4, Q0) // Q8:  vr -> 2nd part (x0*re0,doubled)
          FDK_vqdmulh_s32_qq(Q11, Q5, Q1) // Q11: vr -> 1st part (x1*im0,doubled)
          FDK_vqdmulh_s32_qq( Q9, Q4, Q1) // Q9:  vi -> 2nd part (x0*im0,doubled)
          FDK_vqdmulh_s32_qq(Q10, Q5, Q0) // Q10: vi -> 1st part (x1*re0,doubled)
          FDK_vadd_s32_q(Q12, Q11, Q8)    // Q12: vr0 (doubled)
          FDK_vsub_s32_q(Q13, Q10, Q9)    // Q13; vi0 (doubled)
          FDK_vqdmulh_s32_qq( Q8, Q6, Q2) // Q8:  vr -> 2nd part (x2*re2,doubled)
          FDK_vqdmulh_s32_qq(Q11, Q7, Q3) // Q11: vr -> 1st part (x3*im2,doubled)
          FDK_vqdmulh_s32_qq( Q9, Q6, Q3) // Q9:  vi -> 2nd part (x2*im2,doubled)
          FDK_vqdmulh_s32_qq(Q10, Q7, Q2) // Q10: vi -> 1st part (x3*re2,doubled)
          FDK_vadd_s32_q(Q14, Q11, Q8)    // Q14: vr2 (doubled)
          FDK_vsub_s32_q(Q15, Q10, Q9)    // Q15; vi2 (doubled)
          FDK_vld4_pu_align(32, S32, S36, S40, S44, r7, 64, lr)  // x[t0+0*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q8: ur0
          FDK_vld4_pu_align(32, S33, S37, S41, S45, r7, 64, lr)  // x[t0+1*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q9: ui0
          FDK_vld4_pu_align(32, S34, S38, S42, S46, r7, 64, lr)  // x[t0+2*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q10 ur2
          FDK_vld4_pu_align(32, S35, S39, S43, S47, r7, 64, lr)  // x[t0+3*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q11 ui2
          FDK_sub_op_lsl(r7,r7,lr,2,2)    // r7: &x[t0+4*mq] -> &x[t0+0*mq]
          FDK_vrhadd_s32_q(Q4,  Q8, Q12)   // Q4: (ur0+vr0)>>1
          FDK_vrhadd_s32_q(Q5,  Q9, Q13)   // Q5: (ui0+vi0)>>1
          FDK_vrhadd_s32_q(Q6, Q10, Q14)   // Q6: (ur2+vr2)>>1
          FDK_vrhadd_s32_q(Q7, Q11, Q15)   // Q7: (ui2+vi2)>>1
          FDK_vst4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+0*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+1*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+2*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+3*mq+0,1,2,3]
          FDK_vhsub_s32_q(Q4,  Q8, Q12)   // Q4: (ur0-vr0)>>1
          FDK_vhsub_s32_q(Q5,  Q9, Q13)   // Q5: (ui0-vi0)>>1
          FDK_vhsub_s32_q(Q6, Q10, Q14)   // Q6: (ur2-vr2)>>1
          FDK_vhsub_s32_q(Q7, Q11, Q15)   // Q7: (ui2-vi2)>>1
          FDK_vst4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+4*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+5*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+6*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+7*mq+0,1,2,3]
          FDK_subs(r6, r6, lr)
          FDK_branch(NE, dit_fft_loop3_inner)

        FDK_subs_imm(r5, r5, 16)
        FDK_branch(EQ, dit_fft_loop3_outer_cont)
        FDK_sub_op_lsl(r7,r7,r1,3,2)      // r7: &x[2*j+2*n] -> &x[2*j]
        FDK_add_imm(r7,r7,16,2)           // t0 += 4

        FDK_vld1(32, S0, r10)             // S0: im0,re0
        FDK_vld1(32, S1, r11)             // S1: re1,im1
        FDK_add_op_lsl(r10, r10, r12, 2, 2)
        FDK_sub_op_lsl(r11, r11, r12, 2, 2)
        FDK_vshll_s16_imm(Q0, D0, 16)     // Q0: re1  im1  im0  re0
        FDK_vrev64_d(32, D2, D1)          // D2: im1  re1
                                          // D0: im0  re0
        FDK_vzip_d(32, D0, D2)            // D2: im1  im0              D0: re1 re0
        FDK_vmov_d(D3, D0)                // Q1: re1  re0  im1  im0
        FDK_vneg_d(32, D1, D2)            // Q0:-im1 -im0  re1  re0

        FDK_vld1(32, S8, r10)
        FDK_vld1(32, S9, r11)             // D4: re3  im3  im2  re2 <- need to reverse order of re3/im3
        FDK_add_op_lsl(r10, r10, r12, 2, 2)
        FDK_sub_op_lsl(r11, r11, r12, 2, 2)
        FDK_vshll_s16_imm(Q2, D4, 16)     // Q2: re3  im3  im2  re2
        FDK_vrev64_d(32, D6, D5)          // D6: im3  re3
                                          // D4: im2  re2
        FDK_vzip_d(32, D4, D6)            // D6: im3  im2            D4: re3 re2
        FDK_vmov_d(D7, D4)                // Q3: re3  re2  im3  im2
        FDK_vneg_d(32, D5, D6)            // Q2:-im3 -im2  re3  re2
        // lane  3    2    1    0
        // Q3: +re3 +re2 +im3 +im2
        // Q2: -im3 -im2 +re3 +re2
        // Q1: +re1 +re0 +im1 +im0
        // Q0: -im1 -im0 +re1 +re0
        FDK_branch(NE, dit_fft_loop3_mid)

FDK_label(dit_fft_loop3_outer_cont)
      FDK_subs_imm(r4, r4, 1)
      FDK_branch(NE, dit_fft_loop3_outer)
    //-----------------------------------------------------------------------------
    FDK_mvpop(Q4,Q7)
    FDK_mpop_pc(r4,r12)
FDK_ASM_ROUTINE_END()

#ifdef FUNCTION_scramble_32
FDK_ASM_ROUTINE_START(void, scramble_32,(FIXP_DBL *r0))
    FDK_mvpush(Q4,Q7)
    FDK_vldm8_ia(128,Q0,Q1, Q2, Q3, Q4, Q5, Q6, Q7,r0)
    FDK_vldm8_ia(128,Q8,Q9,Q10,Q11,Q12,Q13,Q14,Q15,r0)
    FDK_sub_imm(r0, r0, 256, 2)
    //SCRAMBLE(x, 1,16)    SCRAMBLE(x, 2, 8)    SCRAMBLE(x, 3,24)    SCRAMBLE(x, 5,20)
    //SCRAMBLE(x, 6,12)    SCRAMBLE(x, 7,28)    SCRAMBLE(x, 9,18)    SCRAMBLE(x,11,26)
    //SCRAMBLE(x,13,22)    SCRAMBLE(x,15,30)    SCRAMBLE(x,19,25)    SCRAMBLE(x,23,29)
    FDK_vswp(64, D1, D16)
    FDK_vswp(64, D2, D8)
    FDK_vswp(64, D3, D24)
    FDK_vswp(64, D5, D20)
    FDK_vswp(64, D6, D12)
    FDK_vswp(64, D7, D28)
    FDK_vswp(64, D9, D18)
    FDK_vswp(64,D11, D26)
    FDK_vswp(64,D13, D22)
    FDK_vswp(64,D15, D30)
    FDK_vswp(64,D19, D25)
    FDK_vswp(64,D23, D29)
    FDK_vstm8_ia(128,Q0,Q1, Q2, Q3, Q4, Q5, Q6, Q7,r0)
    FDK_vstm8_ia(128,Q8,Q9,Q10,Q11,Q12,Q13,Q14,Q15,r0)
    FDK_mvpop(Q4,Q7)
    FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* FUNCTION_scramble_32 */

void dit_fft(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, const INT trigDataSize)
{
#ifdef FUNCTION_FDK_preload_L2_cache
    FDK_preload_L2_cache((const SHORT *) x, (INT) (8 << ldn));
#endif
#ifdef FUNCTION_scramble_32
    if (ldn == 5)
    {
        scramble_32(x);
    }
    else
#endif
    {
        scramble(x,1<<ldn);
    }
    /*
     * 1+2 stage radix 4
     */

#ifdef FUNCTION_FDK_preload_L2_cache
    FDK_preload_L2_cache((const SHORT *) trigdata, trigDataSize<<1);
#endif
    dit_fft_neon(x, ldn, trigdata, trigDataSize, dit_fft_w32_neon);
}
#endif  /* #ifdef FUNCTION_dit_fft */

#endif /* #if defined (__ARM_NEON__) && defined(SINETABLE_16BIT) && defined(FDKTOOLS_PACKED_TABLES) && !defined(FUNCTION_dit_fft) */



#ifndef FUNCTION_dit_fft

/* If dit_fft was not yet defined by ARM-Cortex ... */

#if defined(SINETABLE_16BIT)

#define FUNCTION_dit_fft

/*****************************************************************************

   date:   28.07.2005   srl

   Contents/description: dit-tukey-FFT-algorithm

******************************************************************************/

#if defined(FUNCTION_dit_fft)


void dit_fft(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, const INT trigDataSize)
{
    const INT n=1<<ldn;
    INT i;

    scramble(x,n);
    /*
     * 1+2 stage radix 4
     */

    for (i=0;i<n*2;i+=8)
    {
      FIXP_DBL a00, a10, a20, a30;
      a00 = (x[i + 0] + x[i + 2])>>1;  /* Re A + Re B */
      a10 = (x[i + 4] + x[i + 6])>>1;  /* Re C + Re D */
      a20 = (x[i + 1] + x[i + 3])>>1;  /* Im A + Im B */
      a30 = (x[i + 5] + x[i + 7])>>1;  /* Im C + Im D */

      x[i + 0] = a00 + a10;       /* Re A' = Re A + Re B + Re C + Re D */
      x[i + 4] = a00 - a10;       /* Re C' = Re A + Re B - Re C - Re D */
      x[i + 1] = a20 + a30;       /* Im A' = Im A + Im B + Im C + Im D */
      x[i + 5] = a20 - a30;       /* Im C' = Im A + Im B - Im C - Im D */

      a00 = a00 - x[i + 2];       /* Re A - Re B */
      a10 = a10 - x[i + 6];       /* Re C - Re D */
      a20 = a20 - x[i + 3];       /* Im A - Im B */
      a30 = a30 - x[i + 7];       /* Im C - Im D */

      x[i + 2] = a00 + a30;       /* Re B' = Re A - Re B + Im C - Im D */
      x[i + 6] = a00 - a30;       /* Re D' = Re A - Re B - Im C + Im D */
      x[i + 3] = a20 - a10;       /* Im B' = Im A - Im B - Re C + Re D */
      x[i + 7] = a20 + a10;       /* Im D' = Im A - Im B + Re C - Re D */
    }

    INT mh = 1 << 1;
    INT ldm = ldn - 2;
    INT trigstep = trigDataSize;

    do
    {
        const FIXP_STP *pTrigData = trigdata;
        INT j;

        mh <<= 1;
        trigstep >>= 1;

        FDK_ASSERT(trigstep > 0);

        /* Do first iteration with c=1.0 and s=0.0 separately to avoid loosing to much precision.
           Beware: The impact on the overal FFT precision is rather large. */
        {
            FIXP_DBL *xt1 = x;
            int r = n;

            do {
                FIXP_DBL *xt2 = xt1 + (mh<<1);
                /*
                FIXP_DBL *xt1 = x+ ((r)<<1);
                FIXP_DBL *xt2 = xt1 + (mh<<1);
                */
                FIXP_DBL vr,vi,ur,ui;

                //cplxMultDiv2(&vi, &vr, x[t2+1], x[t2], (FIXP_SGL)1.0, (FIXP_SGL)0.0);
                vi = xt2[1]>>1;
                vr = xt2[0]>>1;

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui+vi;

                xt2[0] = ur-vr;
                xt2[1] = ui-vi;

                xt1 += mh;
                xt2 += mh;

                //cplxMultDiv2(&vr, &vi, x[t2+1], x[t2], (FIXP_SGL)1.0, (FIXP_SGL)0.0);
                vr = xt2[1]>>1;
                vi = xt2[0]>>1;

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui-vi;

                xt2[0] = ur-vr;
                xt2[1] = ui+vi;

                xt1 = xt2 + mh;
            } while ((r=r-(mh<<1)) != 0);
        }
        for(j=4; j<mh; j+=4)
        {
            FIXP_DBL *xt1 = x + (j>>1);
            FIXP_SPK cs;
            int r = n;

            pTrigData += trigstep;
            cs = *pTrigData;

            do
            {
                FIXP_DBL *xt2 = xt1 + (mh<<1);
                FIXP_DBL vr,vi,ur,ui;

                cplxMultDiv2(&vi, &vr, xt2[1], xt2[0], cs);

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui+vi;

                xt2[0] = ur-vr;
                xt2[1] = ui-vi;

                xt1 += mh;
                xt2 += mh;

                cplxMultDiv2(&vr, &vi, xt2[1], xt2[0], cs);

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui-vi;

                xt2[0] = ur-vr;
                xt2[1] = ui+vi;

                /* Same as above but for t1,t2 with j>mh/4 and thus cs swapped */
                xt1 = xt1 - (j);
                xt2 = xt1 + (mh<<1);

                cplxMultDiv2(&vi, &vr, xt2[0], xt2[1], cs);

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui-vi;

                xt2[0] = ur-vr;
                xt2[1] = ui+vi;

                xt1 += mh;
                xt2 += mh;

                cplxMultDiv2(&vr, &vi, xt2[0], xt2[1], cs);

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur-vr;
                xt1[1] = ui-vi;

                xt2[0] = ur+vr;
                xt2[1] = ui+vi;

                xt1 = xt2 + (j);
            }  while ((r=r-(mh<<1)) != 0);
        }
        {
            FIXP_DBL *xt1 = x + (mh>>1);
            int r = n;

            do
            {
                FIXP_DBL *xt2 = xt1 + (mh<<1);
                FIXP_DBL vr,vi,ur,ui;

                cplxMultDiv2(&vi, &vr, xt2[1], xt2[0], STC(0x5a82799a), STC(0x5a82799a));

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui+vi;

                xt2[0] = ur-vr;
                xt2[1] = ui-vi;

                xt1 += mh;
                xt2 += mh;

                cplxMultDiv2(&vr, &vi, xt2[1], xt2[0], STC(0x5a82799a), STC(0x5a82799a));

                ur = xt1[0]>>1;
                ui = xt1[1]>>1;

                xt1[0] = ur+vr;
                xt1[1] = ui-vi;

                xt2[0] = ur-vr;
                xt2[1] = ui+vi;

                xt1 = xt2 + mh;
            }  while ((r=r-(mh<<1)) != 0);
        }
    } while (--ldm != 0);
}

#endif /* if defined(FUNCTION_dit_fft)  */

#endif /* if defined(SINETABLE_16BIT) */

#endif /* ifndef FUNCTION_dit_fft */


#ifdef FUNCTION_dit_ifft

//                                                         1.0f               0.0f               sqrt(0.5f)          sqrt(0.5f)
RAM_ALIGN
static const FIXP_DBL dit_ifft_w32_neon[4] = { FIXP_DBL(0x7FFFFFFF),FIXP_DBL(0x00000000),FIXP_DBL(0x5a82799a),FIXP_DBL(0x5a82799a)};
FDK_ASM_ROUTINE_START(void, dit_ifft_neon,(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, INT trigDataSize, const FIXP_DBL *dit_ifft_w32))
#ifndef __ARM_NEON__
      FIXP_DBL *r0 = x;
      INT       r1 = ldn;
const FIXP_STP *r2 = trigdata;
      INT       r3 = trigDataSize;
      INT r4, r5, r6;
      FIXP_DBL *r7, *r8;
      const FIXP_DBL *r9;
      const FIXP_STP *r10;
      const FIXP_STP *r11;
      INT r12, lr;
#endif

  /* stack contents:
       0x28:     dit_ifft_w32_neon
       0x24:     r4
       0x20:     r5
       0x1C:     r6
       0x18:     r7
       0x14:     r8
       0x10:     r9
       0x0C:     r10
       0x08:     r11
       0x04:     r12
       0x00:     lr

     register contents:
       r0: x                    format: Q1.31
       r1: ldn                  format: INT
       r2: trigdata             format: Q1.15
       r3: trigDataSize         format: Q1.31

     register usage:
       r0: const FIXP_DBL pointer to 'x' input/output buffer
       r1: const INT ldn
       r2: const FIXP_STP pointer to 'trigdata' data
       r3: const INT trigDatasize
       r4: INT outer loop counter
       r5: INT middle loop counter
       r6: INT inner loop counter
       r7: FIXP_DBL read/write pointer to x
       r8: FIXP_DBL read/write pointer to x
       r9: const FIXP_DBL *dit_ifft_w32
      r10: const FIXP_STP pointer to 'trigdata' data, used with ++trigstep
      r11: const FIXP_STP pointer to 'trigdata+trigDataSize/2' data, used with --trigstep
      r12: INT trigstep, starts with trigDataSize>>1, is shifted right by 1 per outer loop3
       lr: m
  */

    FDK_mpush_lr(r4,r12)            // save all registers incl. lr
    FDK_mov_imm(r4, 1)              // r4: const 1

    //-----------------------------------------------------------------------------
    FDK_sub_imm(r5, r1, 4, 0)       // r5: ldn - 4
    FDK_lsl(r5, r4, r5)             // r5: loop counter for loop1
    FDK_mov_reg(r7, r0)             // r7: read pointer for x
    FDK_mov_reg(r8, r0)             // r8: write pointer for x
    FDK_ldr(r9,sp,0x28,dit_ifft_w32) // r9: pointer to twiddle factors (used in 2nd loop)
    FDK_mvpush(Q4,Q7)

FDK_label(dit_ifft_loop1)
      FDK_vld4_ia_align(32, S0, S4, S8,S12, r7, 64)  // x[0...3] into lane 0
      FDK_vld4_ia_align(32,S16,S20,S24,S28, r7, 64)  // x[4...7] into lane 0
      FDK_vld4_ia_align(32, S1, S5, S9,S13, r7, 64)  // x[8..11] into lane 1
      FDK_vld4_ia_align(32,S17,S21,S25,S29, r7, 64)  // x[12.15] into lane 1
      FDK_vld4_ia_align(32, S2, S6,S10,S14, r7, 64)  // x[16.19] into lane 2
      FDK_vld4_ia_align(32,S18,S22,S26,S30, r7, 64)  // x[20.23] into lane 2
      FDK_vld4_ia_align(32, S3, S7,S11,S15, r7, 64)  // x[24.27] into lane 3
      FDK_vld4_ia_align(32,S19,S23,S27,S31, r7, 64)  // x[28.31] into lane 3
      FDK_vrhadd_s32_q(Q8,  Q0, Q2)  // a00
      FDK_vrhadd_s32_q(Q9,  Q4, Q6)  // a10
      FDK_vrhadd_s32_q(Q10, Q1, Q3)  // a20
      FDK_vrhadd_s32_q(Q11, Q5, Q7)  // a30
      FDK_vhsub_s32_q(Q12, Q0, Q2)   // s00
      FDK_vhsub_s32_q(Q13, Q4, Q6)   // s20
      FDK_vhsub_s32_q(Q14, Q1, Q3)   // s30
      FDK_vhsub_s32_q(Q15, Q5, Q7)   // s10
      FDK_vadd_s32_q(Q0, Q8, Q9)     // x[0] = a00 + a10 ;
      FDK_vadd_s32_q(Q1,Q10,Q11)     // x[1] = a20 + a30 ;
      FDK_vsub_s32_q(Q2,Q12,Q15)     // x[2] = s00 - s10 ;
      FDK_vadd_s32_q(Q3,Q14,Q13)     // x[3] = s30 + s20 ;
      FDK_vsub_s32_q(Q4, Q8, Q9)     // x[4] = a00 - a10 ;
      FDK_vsub_s32_q(Q5,Q10,Q11)     // x[5] = a20 - a30 ;
      FDK_vadd_s32_q(Q6,Q12,Q15)     // x[6] = s00 + s10 ;
      FDK_vsub_s32_q(Q7,Q14,Q13)     // x[7] = s30 - s20 ;
      FDK_vst4_ia_align(32, S0, S4, S8,S12, r8, 64)  // x[0...3] from lane 0
      FDK_vst4_ia_align(32,S16,S20,S24,S28, r8, 64)  // x[4...7] from lane 0
      FDK_vst4_ia_align(32, S1, S5, S9,S13, r8, 64)  // x[8..11] from lane 1
      FDK_vst4_ia_align(32,S17,S21,S25,S29, r8, 64)  // x[12.15] from lane 1
      FDK_vst4_ia_align(32, S2, S6,S10,S14, r8, 64)  // x[16.19] from lane 2
      FDK_vst4_ia_align(32,S18,S22,S26,S30, r8, 64)  // x[20.23] from lane 2
      FDK_vst4_ia_align(32, S3, S7,S11,S15, r8, 64)  // x[24.27] from lane 3
      FDK_vst4_ia_align(32,S19,S23,S27,S31, r8, 64)  // x[28.31] from lane 3
      FDK_subs_imm(r5, r5, 1)
      FDK_branch(NE, dit_ifft_loop1)

#ifdef DEBUG_dit_ifft
      for (int i = 0; i < (1 << ldn); i+= 4)
      {
          FDKprintf("l1: 0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  %d..%d\n", x[2*i+0], x[2*i+1],  x[2*i+2], x[2*i+3],  x[2*i+4], x[2*i+5],  x[2*i+6], x[2*i+7], i, i+3);
      }
#endif
    //-----------------------------------------------------------------------------

    FDK_sub_imm(r5, r1, 3, 0)       // r5: ldn - 3
    FDK_lsl(r5, r4, r5)             // r5: loop counter for loop2
    FDK_mov_reg(r7, r0)             // r7: read pointer for x
    FDK_mov_reg(r8, r0)             // r8: write pointer for x

    // Preload Q0,Q1 with const 32-bit twiddle factors:
    // re0: 1.0f         im0: 0.0f,
    // re1: sqrt(1/2)    im1: sqrt(1/2))
    // lane   3     2    1    0
    // Q1:   im1  im0   im1  im0
    // Q0:   re1  re0   re1  re0
    FDK_vld2_2d_align(32, D0, D2, r9, 64)
    FDK_vneg_d(32, D3, D2)                       // Q1: re1  re0  im1  im0
    FDK_vmov_d(D3, D0)                           // Q0:-im1 -im0  re1  re0

FDK_label(dit_ifft_loop2)
      // Load x[0..15] into Q2..Q5 with 2-interleaving
      FDK_vld2_4d_ia(32, D4, D5, D6, D7, r7)
      FDK_vld2_4d_ia(32, D8, D9,D10,D11, r7)
      // lane   3     2     1     0
      // Q5:  x15   x13   x11    x9         x[t2+1]
      // Q4:  x14   x12   x10    x8         x[t2+0]
      // Q3:   x7    x5    x3    x1    ui  x[t1+1]
      // Q2:   x6    x4    x2    x0    ur  x[t1+0]

      FDK_vqdmulh_s32_qq(Q6,Q4,Q0)  // Q6: vr -> x{t2+0,2,4,6]*re
      FDK_vqdmulh_s32_qq(Q7,Q5,Q1)  // Q7: vr -> x[t2+1,3,5,7]*im
      FDK_vqdmulh_s32_qq(Q8,Q4,Q1)  // Q8: vi -> x{t2+0,2,4,6]*im
      FDK_vqdmulh_s32_qq(Q9,Q5,Q0)  // Q9: vi -> x[t2+1,3,5,7]*re
      FDK_vsub_s32_q(Q4,Q6,Q7)      // Q4: vi3  vi2  vr1  vr0
      FDK_vadd_s32_q(Q5,Q8,Q9)      // Q5: vr3  vr2  vi1  vi0
      FDK_vrhadd_s32_q(Q6, Q2, Q4)  // Q6: (ur+vr)>>1
      FDK_vrhadd_s32_q(Q7, Q3, Q5)  // Q7: (ui+vi)>>1
      FDK_vhsub_s32_q(Q8, Q2, Q4)   // Q8: (ur-vr)>>1
      FDK_vhsub_s32_q(Q9, Q3, Q5)   // Q9: (ui-vi)>>1

      // Store x[0..15] from Q6..Q9 with 2-interleaving
      FDK_vst2_4d_ia_align(32, D12, D13, D14, D15, r8, 64)
      FDK_vst2_4d_ia_align(32, D16, D17, D18, D19, r8, 64)

      FDK_subs_imm(r5, r5, 1)
      FDK_branch(NE, dit_ifft_loop2)
#ifdef DEBUG_dit_ifft
      for (int i = 0; i < (1 << ldn); i+= 4)
      {
          FDKprintf("l2: 0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  %d..%d\n", x[2*i+0], x[2*i+1],  x[2*i+2], x[2*i+3],  x[2*i+4], x[2*i+5],  x[2*i+6], x[2*i+7], i, i+3);
      }
#endif    //-----------------------------------------------------------------------------
    FDK_asr_imm(r3, r3, 1)                // r3:  trigDataSize >>= 1
    FDK_mov_reg(r12, r3)                  // r12: trigstep = trigDataSize >> 1
    FDK_sub_imm(r4, r1, 3, 0)             // r4:  number of outer loops: ldn-3
    FDK_mov_imm(lr, 1)                    // lr:  const 1
    FDK_lsl(r1, lr, r1)                   // r1:  n = 1 << ldn
    FDK_mov_imm(lr, 8)                    // lr:  m = 8, mq = 2, mo = 1

FDK_label(dit_ifft_loop3_outer)
      FDK_lsl_imm(lr, lr, 1)              // lr:  m <<= 1, m=16,32,... mq=4,8,.. mo=2,4,...
      FDK_asr_imm(r12, r12, 1)            // r12: trigstep = trigstep>>1
      FDK_add_op_lsl(r10, r2, r12, 2, 2)  // r10: trigdata[trigstep]
      FDK_add_op_lsl(r11, r2, r3,  2, 2)  // r11: trigdata[trigDataSize/2]
      FDK_sub_op_lsl(r11, r11,r12, 2, 2)  // r11: trigdata[trigDataSize/2-trigstep]
      FDK_mov_reg(r7, r0)                 // r7: &x[0]
      FDK_mov_reg(r5, lr)                 // r5: j=[0..m[ in steps of 16 => for(j=0; j<m/8; j+=2)

      // Preload re0,im0...re3,im3 and duplicate each pair
      FDK_vld2_2d_align(32, D0, D2, r9, 64)
      FDK_vmov_d(D3, D2)                  // Q1: im1  im0  im1  im0  in Q1.31
      FDK_vmov_d(D1, D0)                  // Q0: re1  re0  re1  re0  in Q1.31
      FDK_vld1(32, S8, r10)               // D4:           im2  re2  in Q1.15
      FDK_vld1(32, S9, r11)               // D4: re3  im3  im2  re2  in Q1.15

      // Update trigdata pointers r10/r11 to next entries
      FDK_add_op_lsl(r10, r10, r12, 2, 2) // r10: trigdata[2*trigstep]
      FDK_sub_op_lsl(r11, r11, r12, 2, 2) // r11: trigdata[trigDataSize/2-2*trigstep]

      FDK_vshll_s16_imm(Q2, D4, 16)       // Q2: re3  im3  im2  re2  in Q1.31
      FDK_vrev64_d(32, D5, D5)            // Q2: im3  re3  im2  re2  in Q1.31
      FDK_vuzp_d(32, D4, D5)              // Q2: im3  im2  re3  re2  in Q1.31
      FDK_vmov_q(Q3, Q2)                  // Q3: im3  im2  re3  re2
      FDK_vswp(64, D5, D6)                // Q3: im3  im2  im3  im2
                                          // Q2: re3  re2  re3  re2

FDK_label(dit_ifft_loop3_mid)
        FDK_mov_reg(r6, r1)               // r6:  r=[0..n[ in steps of m
FDK_label(dit_ifft_loop3_inner)           // comment is mostly valid only for lane 0 !!!
          FDK_add_op_lsl(r7,r7,lr,2,2)    // r7: &x[t0+4*mq]
          FDK_vld4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+4*mq+0,1,2,3] into lane 0
          FDK_vld4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+5*mq+0,1,2,3] into lane 1
          FDK_vld4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+6*mq+0,1,2,3] into lane 2
          FDK_vld4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+7*mq+0,1,2,3] into lane 3
          FDK_sub_op_lsl(r7,r7,lr,3,2)    // r7: &x[t0+8*mq] -> &x[t0+0*mq]
          FDK_vqdmulh_s32_qq( Q8, Q4, Q0) // Q8:  vr -> 1st part (x0*re0,doubled)
          FDK_vqdmulh_s32_qq(Q11, Q5, Q1) // Q11: vr -> 2nd part (x1*im0,doubled)
          FDK_vqdmulh_s32_qq( Q9, Q4, Q1) // Q9:  vi -> 1st part (x0*im0,doubled)
          FDK_vqdmulh_s32_qq(Q10, Q5, Q0) // Q10: vi -> 2nd part (x1*re0,doubled)
          FDK_vsub_s32_q(Q12, Q8, Q11)    // Q12: vr0 (doubled)
          FDK_vadd_s32_q(Q13, Q9, Q10)    // Q13; vi0 (doubled)
          FDK_vswp(64, D25, D27)          // Q12: vr3  vr2  vr1  vr0
                                          // Q13: vi3  vi2  vi1  vi0
          FDK_vneg_d(32, D25, D25)        // Q12:-vr3 -vr2  vr1  vr0

          FDK_vqdmulh_s32_qq( Q8, Q7, Q2) // Q8:  vr -> 1st part (x2*re2,doubled)
          FDK_vqdmulh_s32_qq(Q11, Q6, Q3) // Q11: vr -> 2st part (x3*im2,doubled)
          FDK_vqdmulh_s32_qq( Q9, Q6, Q3) // Q9:  vi -> 2nd part (x2*im2,doubled)
          FDK_vqdmulh_s32_qq(Q10, Q7, Q2) // Q10: vi -> 1st part (x3*re2,doubled)
          FDK_vsub_s32_q(Q14, Q8, Q11)    // Q14: vr2 (doubled)
          FDK_vadd_s32_q(Q15, Q10, Q9)    // Q15; vi2 (doubled)
          FDK_vswp(64, D29, D31)          // Q14: vr3  vr2  vr1  vr0
                                          // Q15: vi3  vi2  vi1  vi0
          FDK_vneg_d(32, D29, D29)        // Q14:-vr3 -vr2  vr1  vr0
          FDK_vld4_pu_align(32, S32, S36, S40, S44, r7, 64, lr)  // x[t0+0*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q8: ur0
          FDK_vld4_pu_align(32, S33, S37, S41, S45, r7, 64, lr)  // x[t0+1*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q9: ui0
          FDK_vld4_pu_align(32, S34, S38, S42, S46, r7, 64, lr)  // x[t0+2*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q10 ur2
          FDK_vld4_pu_align(32, S35, S39, S43, S47, r7, 64, lr)  // x[t0+3*mq+0,1,2,3] ur0,ui0,ur2,ui0 -> Q11 ui2
          FDK_sub_op_lsl(r7,r7,lr,2,2)    // r7: &x[t0+4*mq] -> &x[t0+0*mq]
          FDK_vrhadd_s32_q(Q4,  Q8, Q12)  // Q4: (ur0+vr0)>>1
          FDK_vrhadd_s32_q(Q5,  Q9, Q13)  // Q5: (ui0+vi0)>>1
          FDK_vhsub_s32_q(Q6, Q10, Q14)   // Q6: (ur2-vr2)>>1
          FDK_vhsub_s32_q(Q7, Q11, Q15)   // Q7: (ui2-vi2)>>1
          FDK_vst4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+0*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+1*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+2*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+3*mq+0,1,2,3]
          FDK_vhsub_s32_q( Q4, Q8,  Q12)  // Q4: (ur0-vr0)>>1
          FDK_vrhadd_s32_q(Q5, Q9,  Q13)  // Q5: (ui0+vi0)>>1
          FDK_vrhadd_s32_q(Q6, Q10, Q14)  // Q6: (ur2+vr2)>>1
          FDK_vhsub_s32_q( Q7, Q11, Q15)  // Q7: (ui2-vi2)>>1
          FDK_vst4_pu_align(32, S16, S20, S24, S28, r7, 64, lr)  // x[t0+4*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S17, S21, S25, S29, r7, 64, lr)  // x[t0+5*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S18, S22, S26, S30, r7, 64, lr)  // x[t0+6*mq+0,1,2,3]
          FDK_vst4_pu_align(32, S19, S23, S27, S31, r7, 64, lr)  // x[t0+7*mq+0,1,2,3]
          FDK_subs(r6, r6, lr)
          FDK_branch(NE, dit_ifft_loop3_inner)

        FDK_subs_imm(r5, r5, 16)
        FDK_branch(EQ, dit_ifft_loop3_outer_cont)
        FDK_sub_op_lsl(r7,r7,r1,3,2)      // r7: &x[2*j+2*n] -> &x[2*j]
        FDK_add_imm(r7,r7,16,2)           // t0 += 4

        FDK_vld1(32, S0, r10)             // S0: im0,re0  in Q1.15
        FDK_vld1(32, S1, r11)             // S1: re1,im1  in Q1.15
        FDK_add_op_lsl(r10, r10, r12, 2, 2)
        FDK_sub_op_lsl(r11, r11, r12, 2, 2)
        FDK_vshll_s16_imm(Q0, D0, 16)     // Q0: re1  im1  im0  re0  in Q1.31
        FDK_vrev64_d(32, D1, D1)          // Q0: im1  re1  im0  re0  in Q1.31
        FDK_vuzp_d(32, D0, D1)            // Q0: im1  im0  re1  re0
        FDK_vmov_q(Q1, Q0)                // Q1: im1  im0  re1  re0
        FDK_vswp(64, D1, D2)              // Q1: im1  im0  im1  im0
                                          // Q0: re1  re0  re1  re0

        FDK_vld1(32, S8, r10)
        FDK_vld1(32, S9, r11)             // D4: re3  im3  im2  re2  in Q1.15

        FDK_add_op_lsl(r10, r10, r12, 2, 2)
        FDK_sub_op_lsl(r11, r11, r12, 2, 2)

        FDK_vshll_s16_imm(Q2, D4, 16)     // Q2: re3  im3  im2  re2  in Q1.31
        FDK_vrev64_d(32, D5, D5)          // Q2: im3  re3  im2  re2  in Q1.31
        FDK_vuzp_d(32, D4, D5)            // Q2: im3  im2  re3  re2
        FDK_vmov_q(Q3, Q2)                // Q3: im3  im2  re3  re2
        FDK_vswp(64, D5, D6)              // Q3: im3  im2  im3  im2
                                          // Q2: re3  re2  re3  re2
        FDK_branch(NE, dit_ifft_loop3_mid)

FDK_label(dit_ifft_loop3_outer_cont)
      FDK_subs_imm(r4, r4, 1)
      FDK_branch(NE, dit_ifft_loop3_outer)
    //-----------------------------------------------------------------------------
#ifdef DEBUG_dit_ifft
      for (int i = 0; i < (1 << ldn); i+= 4)
      {
          FDKprintf("l4: 0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  0x%08X 0x%08X  %d..%d\n", x[2*i+0], x[2*i+1],  x[2*i+2], x[2*i+3],  x[2*i+4], x[2*i+5],  x[2*i+6], x[2*i+7], i, i+3);
      }
#endif 
    FDK_mvpop(Q4,Q7)
    FDK_mpop_pc(r4,r12)
FDK_ASM_ROUTINE_END()

void dit_ifft(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata, const INT trigDataSize)
{
#ifdef FUNCTION_FDK_preload_L2_cache
    FDK_preload_L2_cache((const SHORT *) x, (INT) (8 << ldn));
#endif
#ifdef FUNCTION_scramble_32
    if (ldn == 5)
    {
        scramble_32(x);
    }
    else
#endif
    {
        scramble(x,1<<ldn);
    }
    /*
     * 1+2 stage radix 4
     */

#ifdef FUNCTION_FDK_preload_L2_cache
    FDK_preload_L2_cache((const SHORT *) trigdata, trigDataSize<<1);
#endif
    dit_ifft_neon(x, ldn, trigdata, trigDataSize, dit_ifft_w32_neon);
}
#endif  /* #ifdef FUNCTION_dit_fft */
