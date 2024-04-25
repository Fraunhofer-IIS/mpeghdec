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

   Description: upsampling LP-interpolator utilizing biquad sections - ARM
                version

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_AARCH64_NEON__)
#include "arm/FDK_aarch64_neon_funcs.h"

#define noFUNCTION_TD_upsampler_2_1

#define FUNCTION_TD_upsampler_3_1

#if ( !defined(FUNCTION_TD_upsampler_3_1) || !defined(FUNCTION_TD_upsampler_2_1) || !defined(FUNCTION_TD_upsampler_3_1) )
#define noFUNCTION_TD_applyFilter
#endif


#endif /* defined(__ARM_AARCH64_NEON__) && (7 == 7) */

#if (TD_RS_DELAY == 30) && (defined (__ARM_NEON__))
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#endif

#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_TD_upsampler_3_1
#endif


#ifdef FUNCTION_TD_upsampler_3_1
#if defined(__ARM_AARCH64_NEON__)
A64_ASM_ROUTINE_START(
void ,TD_upsampler_3_1_neonv8,(const FIXP_DBL * RESTRICT sigIn, FIXP_DBL * RESTRICT sigOut, FIXP_DBL * RESTRICT states, INT64 lenIn, INT64 facUpsample, INT64 *coeff, INT64 *gain, INT scaleFacMantissa))
#ifndef __ARM_AARCH64_NEON__
  const FIXP_DBL * RESTRICT X0 = sigIn;                    // input buffer:  may not be 64-bit aligned
        FIXP_DBL * RESTRICT X1 = sigOut;                   // output buffer: may not be 64-bit aligned
        FIXP_DBL * RESTRICT X2 = states;                   // states buffer: 64-bit aligned
                            X3 = lenIn;                    // input length:  must be a multiple of 2 in case of facUpsample=3_2
                            X4 = facUpsample;              // enum in range [TD_FAC_UPSAMPLE_3_2 = 1, TD_FAC_UPSAMPLE_3_1 = 3]
        INT64    * RESTRICT X5 = coeff;                    // array of 7x4 coeffs, each ordered [B1, B2, A1, A2]
        INT64    * RESTRICT X6 = gain;                     // array of 7 gain factors, each 16-bit
                            W7 = scaleFacMantissa;         // scale factor mantissa, 32-bit
#endif

  /* NEON Register layout:

     V31:  00000000  00000000  00000000  00000000
     V30:  --------  --------  --------  00000002
     V29:  --------  --------     B2_6      B1_6
     V28:  --------  --------     A2_6      A1_6
     V27:  --------  --------     B2_5      B1_5
     V26:  --------  --------     A2_5      A1_5
     V25:  --------  --------     B2_4      B1_4
     V24:  --------  --------     A2_4      A1_4
     V23:  --------  --------     B2_3      B1_3
     V22:  --------  --------     A2_3      A1_3
     V21:  --------  --------     B2_2      B1_2
     V20:  --------  --------     A2_2      A1_2
     V19:  --------  --------     B2_1      B1_1
     V18:  --------  --------     A2_1      A1_1
     V17:  --------  --------     B2_0      B1_0
     V16:  --------  --------     A2_0      A1_0
     V15:  --------  --------  --------  --------
     V14:  --------  --------  --------  --------
     V13:  --------  --------  --------  --------
     V12:  --------  --------  --------  --------
     V11:  --------  --------  --------   output2
     V10:  --------  --------  --------   output1
     V9:   --------  --------  --------   output0
     V8:     sFMant    gain[6]  gain[5]   gain[4]
     V7:    gain[3]    gain[2]  gain[1]   gain[0]
     V6:   --------  --------  state1_6  state0_6
     V5:   --------  --------  state1_5  state0_5
     V4:   --------  --------  state1_4  state0_4
     V3:   --------  --------  state1_3  state0_3
     V2:   --------  --------  state1_2  state0_2
     V1:   --------  --------  state1_1  state0_1
     V0:   --------  --------  state1_0  state0_0
  */

  /* Push preserved SIMD register (bottom part) accoding to calling convention */
  A64_pushD(D8,  D9)
  A64_pushD(D10, D11)
  A64_pushD(D12, D13)
  A64_pushD(D14, D15)

  // Preload V31 with zero in all elements
  A64_movi(32, 128, V31_4S, 0x00)                                  // V31:  00000000  00000000  00000000  00000000
  A64_mov_Xt_imm(X8, 2)
  A64_mov_Xt_to_lane(64, 128, V30_D, 0, X8)                        // V30:  00000000  00000000  00000000..00000002

  // Load 7 sets of 2 states each 32-bit: state0, state1
  A64_mov_Xt(X8, X2)
  A64_ld1x4_IA(32, 64, V0_2S, V1_2S, V2_2S, V3_2S, X8, 32)
  A64_ld1x3_IA(32, 64, V4_2S, V5_2S, V6_2S,        X8, 24)

  // Load 7 sets of 4 coeffs each 16-bit: B1, B2, A1, A2
  A64_ld1x4_IA(16, 64, V16_4H, V17_4H, V18_4H, V19_4H, X5, 32)
  A64_ld1x3_IA(16, 64, V24_4H, V25_4H, V26_4H,         X5, 24)
  // convert 7 sets of 4 coeffs each 16-bit into 32-bit format, stored in lower register parts
  A64_zip1(16, 64, V29_4H, V31_4H, V26_4H)                        // V29:  --------  --------     B2_6      B1_6
  A64_zip2(16, 64, V28_4H, V31_4H, V26_4H)                        // V28:  --------  --------     A2_6      A1_6
  A64_zip1(16, 64, V27_4H, V31_4H, V25_4H)                        // V27:  --------  --------     B2_5      B1_5
  A64_zip2(16, 64, V26_4H, V31_4H, V25_4H)                        // V26:  --------  --------     A2_5      A1_5
  A64_zip1(16, 64, V25_4H, V31_4H, V24_4H)                        // V25:  --------  --------     B2_4      B1_4
  A64_zip2(16, 64, V24_4H, V31_4H, V24_4H)                        // V24:  --------  --------     A2_4      A1_4
  A64_zip1(16, 64, V23_4H, V31_4H, V19_4H)                        // V23:  --------  --------     B2_3      B1_3
  A64_zip2(16, 64, V22_4H, V31_4H, V19_4H)                        // V22:  --------  --------     A2_3      A1_3
  A64_zip1(16, 64, V21_4H, V31_4H, V18_4H)                        // V21:  --------  --------     B2_2      B1_2
  A64_zip2(16, 64, V20_4H, V31_4H, V18_4H)                        // V20:  --------  --------     A2_2      A1_2
  A64_zip1(16, 64, V19_4H, V31_4H, V17_4H)                        // V19:  --------  --------     B2_1      B1_1
  A64_zip2(16, 64, V18_4H, V31_4H, V17_4H)                        // V18:  --------  --------     A2_1      A1_1
  A64_zip1(16, 64, V17_4H, V31_4H, V16_4H)                        // V17:  --------  --------     B2_0      B1_0
  A64_zip2(16, 64, V16_4H, V31_4H, V16_4H)                        // V16:  --------  --------     A2_0      A1_0


  // Load 7 sos_gain values, each 16-bits
  A64_ld1x1(16,128, V7_8H, X6)                                     // V7:    xxx gn6   gn5 gn4   gn3 gn2   gn1 gn0
  A64_zip2(16, 128, V8_8H, V31_8H, V7_8H)                          // V8:   --------   gain[6]   gain[5]   gain[4]
  A64_zip1(16, 128, V7_8H, V31_8H, V7_8H)                          // V7:    gain[3]   gain[2]   gain[1]   gain[0]

  // Copy scaleFacMantissa (32-bit) into V9[3]:
  A64_mov_Wt_to_lane(32, 128, V8_S, 3, W7)                         // V8:     sfMant   gain[6]   gain[5]   gain[4]

A64_label(TD_upsampler_3_1_neonv8_loop)
    A64_ld1_lane_IA(32, V9_S, 0, X0, 4)                            // V9:   --------  --------  --------  output0 = input[i]
                                                                   // V10:  --------  --------  --------  output1 = ????????
                                                                   // V11:  --------  --------  --------  output2 = ????????
    // --- s = 0 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V7_S, 0)            // V9:   --------  --------  --------  out0 = out0*gain[0]
    A64_uzp2(32, 64, V13_2S, V0_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_0
    A64_sqdmulh_scalar(32, 64, V14_2S, V17_2S, V9_S, 0)            // V14:  --------  --------  B2_0*out0 B1_0*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_0*out0 B1_0*out0+state1_0
    A64_mla( 32, 64,  V9_2S,  V0_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_0<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V16_2S, V9_S, 0)            // V14:  --------  --------  A2_0*out0 A1_0*out0
    A64_sub( 32, 64, V0_2S, V13_2S, V14_2S)                        // V0:   --------  --------  state1_0  state0_0
    // In first iteration (s = 0), input1, input2 = 0, no need to multiply by gain[0]
    A64_mul( 32, 64, V10_2S, V0_2S, V30_2S)                        // V10:  --------  --------  00000000  output1=state0_0<<1
    A64_uzp2(32, 64, V13_2S, V0_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_0
    A64_sqdmulh_scalar(32, 64, V14_2S, V16_2S, V10_S, 0)           // V14:  --------  --------  A2_0*out1 A1_0*out1
    A64_sub( 32, 64, V0_2S, V13_2S, V14_2S)                        // V0:   --------  --------  state1_0  state0_0

    A64_mul( 32, 64, V11_2S, V0_2S, V30_2S)                        // V11:  --------  --------  00000000  output2=state0_0<<1
    A64_uzp2(32, 64, V13_2S, V0_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_0
    A64_sqdmulh_scalar(32, 64, V14_2S, V16_2S, V11_S, 0)           // V14:  --------  --------  A2_0*out2 A1_0*out2
    A64_sub( 32, 64, V0_2S, V13_2S, V14_2S)                        // V0:   --------  --------  state1_0  state0_0
    // --- s = 1 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V7_S, 1)            // V9:   --------  --------  --------  output0 = out0*gain[1]
    A64_uzp2(32, 64, V13_2S, V1_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_1
    A64_sqdmulh_scalar(32, 64, V14_2S, V19_2S, V9_S, 0)            // V14:  --------  --------  B2_1*out0 B1_1*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_1*out0 B1_1*out0+state1_1
    A64_mla( 32, 64,  V9_2S,  V1_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_1<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V18_2S, V9_S, 0)            // V14:  --------  --------  A2_1*out0 A1_1*out0
    A64_sub( 32, 64, V1_2S, V13_2S, V14_2S)                        // V1:   --------  --------  state1_1  state0_1

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V7_S, 1)            // V10:  --------  --------  --------  output1 = out1*gain[1]
    A64_uzp2(32, 64, V13_2S, V1_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_1
    A64_sqdmulh_scalar(32, 64, V14_2S, V19_2S, V10_S, 0)           // V14:  --------  --------  B2_1*out1 B1_1*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_1*out1 B1_1*out1+state1_1
    A64_mla( 32, 64, V10_2S,  V1_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_1<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V18_2S, V10_S, 0)           // V14:  --------  --------  A2_1*out1 A1_1*out1
    A64_sub( 32, 64, V1_2S, V13_2S, V14_2S)                        // V1:   --------  --------  state1_1  state0_1

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V7_S, 1)            // V11:  --------  --------  --------  output2 = out2*gain[1]
    A64_uzp2(32, 64, V13_2S, V1_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_1
    A64_sqdmulh_scalar(32, 64, V14_2S, V19_2S, V11_S, 0)           // V14:  --------  --------  B2_1*out2 B1_1*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_1*out2 B1_1*out2+state1_1
    A64_mla( 32, 64, V11_2S,  V1_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_1<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V18_2S, V11_S, 0)           // V14:  --------  --------  A2_1*out2   A1_1*out2
    A64_sub( 32, 64, V1_2S, V13_2S, V14_2S)                        // V1:   --------  --------  state1_1  state0_1
    // --- s = 2 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V7_S, 2)            // V9:   --------  --------  --------  output0 = out0*gain[2]
    A64_uzp2(32, 64, V13_2S, V2_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_2
    A64_sqdmulh_scalar(32, 64, V14_2S, V21_2S, V9_S, 0)            // V14:  --------  --------  B2_2*out0 B1_2*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_2*out0 B1_2*out0+state1_2
    A64_mla( 32, 64,  V9_2S,  V2_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_2<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V20_2S, V9_S, 0)            // V14:  --------  --------  A2_2*out0 A1_2*out0
    A64_sub( 32, 64, V2_2S, V13_2S, V14_2S)                        // V2:   --------  --------  state1_2  state0_2

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V7_S, 2)            // V10:  --------  --------  --------  output1 = out1*gain[2]
    A64_uzp2(32, 64, V13_2S, V2_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_2
    A64_sqdmulh_scalar(32, 64, V14_2S, V21_2S, V10_S, 0)           // V14:  --------  --------  B2_2*out1 B1_2*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_2*out1 B1_2*out1+state1_2
    A64_mla( 32, 64, V10_2S,  V2_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_2<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V20_2S, V10_S, 0)           // V14:  --------  --------  A2_2*out1 A1_2*out1
    A64_sub( 32, 64, V2_2S, V13_2S, V14_2S)                        // V2:   --------  --------  state1_2  state0_2

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V7_S, 2)            // V11:  --------  --------  --------  output2 = out2*gain[2]
    A64_uzp2(32, 64, V13_2S, V2_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_2
    A64_sqdmulh_scalar(32, 64, V14_2S, V21_2S, V11_S, 0)           // V14:  --------  --------  B2_2*out2 B1_2*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_2*out2 B1_2*out2+state1_2
    A64_mla( 32, 64, V11_2S,  V2_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_2<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V20_2S, V11_S, 0)           // V14:  --------  --------  A2_2*out2   A1_2*out2
    A64_sub( 32, 64, V2_2S, V13_2S, V14_2S)                        // V2:   --------  --------  state1_2  state0_2
    // --- s = 3 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V7_S, 3)            // V9:   --------  --------  --------  output0 = out0*gain[3]
    A64_uzp2(32, 64, V13_2S, V3_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_3
    A64_sqdmulh_scalar(32, 64, V14_2S, V23_2S, V9_S, 0)            // V14:  --------  --------  B2_3*out0 B1_3*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_3*out0 B1_3*out0+state1_3
    A64_mla( 32, 64,  V9_2S,  V3_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_3<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V22_2S, V9_S, 0)            // V14:  --------  --------  A2_3*out0 A1_3*out0
    A64_sub( 32, 64, V3_2S, V13_2S, V14_2S)                        // V3:   --------  --------  state1_3  state0_3

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V7_S, 3)            // V10:  --------  --------  --------  output1 = out1*gain[3]
    A64_uzp2(32, 64, V13_2S, V3_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_3
    A64_sqdmulh_scalar(32, 64, V14_2S, V23_2S, V10_S, 0)           // V14:  --------  --------  B2_3*out1 B1_3*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_3*out1 B1_3*out1+state1_3
    A64_mla( 32, 64, V10_2S,  V3_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_3<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V22_2S, V10_S, 0)           // V14:  --------  --------  A2_3*out1 A1_3*out1
    A64_sub( 32, 64, V3_2S, V13_2S, V14_2S)                        // V3:   --------  --------  state1_3  state0_3

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V7_S, 3)            // V11:  --------  --------  --------  output2 = out2*gain[3]
    A64_uzp2(32, 64, V13_2S, V3_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_3
    A64_sqdmulh_scalar(32, 64, V14_2S, V23_2S, V11_S, 0)           // V14:  --------  --------  B2_3*out2 B1_3*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_3*out2 B1_3*out2+state1_3
    A64_mla( 32, 64, V11_2S,  V3_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_3<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V22_2S, V11_S, 0)           // V14:  --------  --------  A2_3*out2 A1_3*out2
    A64_sub( 32, 64, V3_2S, V13_2S, V14_2S)                        // V3:   --------  --------  state1_3  state0_3
    // --- s = 4 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V8_S, 0)            // V9:   --------  --------  --------  output0 = out0*gain[4]
    A64_uzp2(32, 64, V13_2S, V4_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_4
    A64_sqdmulh_scalar(32, 64, V14_2S, V25_2S, V9_S, 0)            // V14:  --------  --------  B2_4*out0 B1_4*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_4*out0 B1_4*out0+state1_4
    A64_mla( 32, 64,  V9_2S,  V4_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_4<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V24_2S, V9_S, 0)            // V14:  --------  --------  A2_4*out0 A1_4*out0
    A64_sub( 32, 64, V4_2S, V13_2S, V14_2S)                        // V4:   --------  --------  state1_4  state0_4

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V8_S, 0)            // V10:  --------  --------  --------  output1 = out1*gain[4]
    A64_uzp2(32, 64, V13_2S, V4_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_4
    A64_sqdmulh_scalar(32, 64, V14_2S, V25_2S, V10_S, 0)           // V14:  --------  --------  B2_4*out1 B1_4*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_4*out1 B1_4*out1+state1_4
    A64_mla( 32, 64, V10_2S,  V4_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_4<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V24_2S, V10_S, 0)           // V14:  --------  --------  A2_4*out1 A1_4*out1
    A64_sub( 32, 64, V4_2S, V13_2S, V14_2S)                        // V4:   --------  --------  state1_4  state0_4

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V8_S, 0)            // V11:  --------  --------  --------  output2 = out2*gain[4]
    A64_uzp2(32, 64, V13_2S, V4_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_4
    A64_sqdmulh_scalar(32, 64, V14_2S, V25_2S, V11_S, 0)           // V14:  --------  --------  B2_4*out2 B1_4*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_4*out2 B1_4*out2+state1_4
    A64_mla( 32, 64, V11_2S,  V4_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_4<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V24_2S, V11_S, 0)           // V14:  --------  --------  A2_4*out2 A1_4*out2
    A64_sub( 32, 64, V4_2S, V13_2S, V14_2S)                        // V4:   --------  --------  state1_4  state0_4
    // --- s = 5 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V8_S, 1)            // V9:   --------  --------  --------  output0 = out0*gain[5]
    A64_uzp2(32, 64, V13_2S, V5_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_5
    A64_sqdmulh_scalar(32, 64, V14_2S, V27_2S, V9_S, 0)            // V14:  --------  --------  B2_5*out0 B1_5*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_5*out0 B1_5*out0+state1_5
    A64_mla( 32, 64,  V9_2S,  V5_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_5<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V26_2S, V9_S, 0)            // V14:  --------  --------  A2_5*out0 A1_5*out0
    A64_sub( 32, 64, V5_2S, V13_2S, V14_2S)                        // V5:   --------  --------  state1_5  state0_5

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V8_S, 1)            // V10:  --------  --------  --------  output1 = out1*gain[5]
    A64_uzp2(32, 64, V13_2S, V5_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_5
    A64_sqdmulh_scalar(32, 64, V14_2S, V27_2S, V10_S, 0)           // V14:  --------  --------  B2_5*out1 B1_5*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_5*out1 B1_5*out1+state1_5
    A64_mla( 32, 64, V10_2S,  V5_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_5<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V26_2S, V10_S, 0)           // V14:  --------  --------  A2_5*out1 A1_5*out1
    A64_sub( 32, 64, V5_2S, V13_2S, V14_2S)                        // V5:   --------  --------  state1_5  state0_5

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V8_S, 1)            // V11:  --------  --------  --------  output2 = out2*gain[5]
    A64_uzp2(32, 64, V13_2S, V5_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_5
    A64_sqdmulh_scalar(32, 64, V14_2S, V27_2S, V11_S, 0)           // V14:  --------  --------  B2_5*out2 B1_5*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_5*out2 B1_5*out2+state1_5
    A64_mla( 32, 64, V11_2S,  V5_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_5<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V26_2S, V11_S, 0)           // V14:  --------  --------  A2_5*out2 A1_5*out2
    A64_sub( 32, 64, V5_2S, V13_2S, V14_2S)                        // V5:   --------  --------  state1_5  state0_5
    // --- s = 6 -------------------------------------------------------------------------------------------------------------
    A64_sqdmulh_scalar(32, 64,  V9_2S,  V9_2S, V8_S, 2)            // V9:   --------  --------  --------  output0 = out0*gain[6]
    A64_uzp2(32, 64, V13_2S, V6_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_6
    A64_sqdmulh_scalar(32, 64, V14_2S, V29_2S, V9_S, 0)            // V14:  --------  --------  B2_6*out0 B1_6*out0
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_6*out0 B1_6*out0+state1_6
    A64_mla( 32, 64,  V9_2S,  V6_2S, V30_2S)                       // V9:   --------  --------  00000000  out0=out0+state0_6<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V28_2S, V9_S, 0)            // V14:  --------  --------  A2_6*out0 A1_6*out0
    A64_sub( 32, 64, V6_2S, V13_2S, V14_2S)                        // V6:   --------  --------  state1_6  state0_6

    A64_sqdmulh_scalar(32, 64, V10_2S, V10_2S, V8_S, 2)            // V10:  --------  --------  --------  output1 = out1*gain[6]
    A64_uzp2(32, 64, V13_2S, V6_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_6
    A64_sqdmulh_scalar(32, 64, V14_2S, V29_2S, V10_S, 0)           // V14:  --------  --------  B2_6*out1 B1_6*out1
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_6*out1 B1_6*out1+state1_6
    A64_mla( 32, 64, V10_2S,  V6_2S, V30_2S)                       // V10:  --------  --------  00000000  out1=out1+state0_6<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V28_2S, V10_S, 0)           // V14:  --------  --------  A2_6*out1 A1_6*out1
    A64_sub( 32, 64, V6_2S, V13_2S, V14_2S)                        // V6:   --------  --------  state1_6  state0_6

    A64_sqdmulh_scalar(32, 64, V11_2S, V11_2S, V8_S, 2)            // V11:  --------  --------  --------  output2 = out2*gain[6]
    A64_uzp2(32, 64, V13_2S, V6_2S, V31_2S)                        // V13:  --------  --------  00000000  state1_6
    A64_sqdmulh_scalar(32, 64, V14_2S, V29_2S, V11_S, 0)           // V14:  --------  --------  B2_6*out2 B1_6*out2
    A64_add( 32, 64, V13_2S, V13_2S, V14_2S)                       // V13:  --------  --------  B2_6*out2 B1_6*out2+state1_6
    A64_mla( 32, 64, V11_2S,  V6_2S, V30_2S)                       // V11:  --------  --------  00000000  out2=out2+state0_6<<1
    A64_sqdmulh_scalar(32, 64, V14_2S, V28_2S, V11_S, 0)           // V14:  --------  --------  A2_6*out2 A1_6*out2
    A64_sub( 32, 64, V6_2S, V13_2S, V14_2S)                        // V6:   --------  --------  state1_6  state0_6

    // --- Prepare output  ---------------------------------------------------------------------------------------------
    A64_zip1(32,  64, V12_2S, V9_2S, V11_2S)                       // V12:  --------  --------  output2   output0
    A64_zip1(32, 128, V12_4S,V12_4S, V10_4S)                       // V12:  --------  output2   output1   output0
    A64_sqdmulh_scalar(32, 128, V13_4S, V12_4S, V8_S, 3)           // V13: (--------  output2   output1   output0)*scaleFacMantissa
    A64_sqshl_imm(32, 128, V9_4S, V13_4S, 2)                       // V9:  (--------  output2   output1   output0)<< 2

    A64_cmp_Xt_imm(X4, 1)                                          // compare to: TD_FAC_UPSAMPLE_3_2
    A64_branch(EQ, TD_upsampler_3_2_store)
//A64_label(TD_upsampler_3_1_store)
      A64_st1_lane_IA(32, V9_S, 0, X1, 4)
      A64_st1_lane_IA(32, V9_S, 1, X1, 4)
      A64_st1_lane_IA(32, V9_S, 2, X1, 4)
      A64_branch(AL, TD_upsampler_3_1_cont)

A64_label(TD_upsampler_3_2_store)
    A64_ands_Xt_imm(XZR, X3, 1)                                    // check, if LSBit is set
    A64_branch(NE, TD_upsampler_3_2_store_odd)
//A64_label(TD_upsampler_3_2_store_even)
      A64_st1_lane_IA(32, V9_S, 0, X1, 4)
      A64_st1_lane_IA(32, V9_S, 2, X1, 4)
      A64_branch(AL, TD_upsampler_3_1_cont)

A64_label(TD_upsampler_3_2_store_odd)
      A64_st1_lane_IA(32, V9_S, 1, X1, 4)

A64_label(TD_upsampler_3_1_cont)
    A64_subs_imm(X3, X3, 1)
    A64_branch(NE, TD_upsampler_3_1_neonv8_loop)

  // Store 7x2 states
  A64_st1x4_IA(32, 64, V0_2S, V1_2S, V2_2S, V3_2S, X2, 32)
  A64_st1x3_IA(32, 64, V4_2S, V5_2S, V6_2S,        X2, 24)

  /* Pop preserved SIMD register (bottom part) */
  A64_popD(D14, D15)
  A64_popD(D12, D13)
  A64_popD(D10, D11)
  A64_popD(D8,  D9)

A64_ASM_ROUTINE_END()


short TD_upsampler_3_1(
      const FIXP_DBL * RESTRICT sigIn,
      FIXP_DBL * RESTRICT sigOut,
      FIXP_DBL * RESTRICT states,
      short lenIn,
      TD_FAC_UPSAMPLE facUpsample)
{
   if (lenIn > 0) {
     TD_upsampler_3_1_neonv8(sigIn, sigOut, states, (INT64) lenIn, (INT64) facUpsample, (INT64 *) sos_3.coeff, (INT64 *) sos_3.sos_gain, (INT) sos_3.scaleFacMantissa);
   }
   return (lenIn*3) >> ((facUpsample == TD_FAC_UPSAMPLE_3_2) ? 1 : 0);
}
#else  /* #if defined(__ARM_AARCH64_NEON__) && (7 == 7) */


FDK_ASM_ROUTINE_START(
void ,TD_upsampler_3_1_neonv7,(const FIXP_DBL * RESTRICT sigIn, FIXP_DBL * RESTRICT sigOut, FIXP_DBL * RESTRICT states, INT lenIn, INT facUpsample, const FIXP_DBL *coeff, const FIXP_DBL *gain, INT scaleFacMantissa))
#ifndef __ARM_NEON__
  const FIXP_DBL * RESTRICT r0 = sigIn;                    // input buffer:  may not be 64-bit aligned
        FIXP_DBL * RESTRICT r1 = sigOut;                   // output buffer: may not be 64-bit aligned
        FIXP_DBL * RESTRICT r2 = states;                   // states buffer: 64-bit aligned
        INT                 r3 = lenIn;                    // input length:  must be a multiple of 2 in case of facUpsample=3_2
        INT                 r4 = facUpsample;              // enum in range [TD_FAC_UPSAMPLE_3_2 = 1, TD_FAC_UPSAMPLE_3_1 = 3]
        FIXP_DBL * RESTRICT r5 = coeff;                    // array of 7x4 coeffs, each ordered [B1, B2, A1, A2], 32-bit aligned
        FIXP_DBL * RESTRICT r6 = gain;                     // array of 7 gain factors, each 16-bit, array is 32-bit aligned
        FIXP_DBL            r7 = scaleFacMantissa;         // scale factor mantissa, 32-bit
        INT                 r8;                            // 
          //stackpointer simulation
        FIXP_DBL sp[8] = { 0 };
        INT i, i_sp = 0;
#endif

  /*
     NEON register layout:
     Q15:     A1 for sections 3-6  
     Q14:     A2 for sections 3-6   
     Q13:     B1 for sections 3-6
     Q12:     B2 for sections 3-6
     Q11:  A1 for sections 0-2 | x
     Q10:  A2 for sections 0-2 | x
     Q9:   B1 for sections 0-2 | x
     Q8:   B2 for sections 0-2 | x
     Q7: states s1 for sections 3-6                                           
     Q6: states s0 for sections 3-6
     Q5: states s1 for sections 0-2
     Q4: states s0 for sections 0-2
     Q3: out(sec 0-2)
     Q2: in (sec 3-6)
     Q1: gains (loaded on demand)
     Q0: in(sec0-2) / out(sec3-6)

     Note: All 7 sections are filled with input before the first output sample is calculated (preload).
     After all input samples are used as input (including preload), every section is calculated one last time consecutively. 
     States are saved after each sections last run.

     Core register usage:
     r0: sigIn                    // input buffer:  may not be 64-bit aligned
     r1: sigOut                   // output buffer: may not be 64-bit aligned
     r2: states                   // states buffer: 64-bit aligned (state_0_0  state_1_0  state_0_1  state_1_1 ...)
     r3: lenIn                    // input length:  must be a multiple of 2 in case of facUpsample=3_2, used with decrement   - later: loopcounter
     r4: facUpsample              // enum in range [TD_FAC_UPSAMPLE_3_2 = 1, TD_FAC_UPSAMPLE_3_1 = 3]                         - later: XOR factor for store
     r5: coeff                    // array of 7x4 coeffs, each ordered [B1, B2, A1, A2]                                       - later: gains
     r6: gain                     // array of 7 gain factors, each 16-bit                                                     - later: gains
     r7: scaleFacMantissa         // scale factor to align output data just before storage (=const 2)                         - later: step for store
     r8: lenIn_backup

     Stack contents:
       0x20:     scaleFacMantissa
       0x1C:     gain
       0x18:     coeff
       0x14:     facUpsample
       0x10:     r4
       0x0C:     r5
       0x08:     r6
       0x04:     r7
       0x00:     r8
  */

    FDK_mpush(r4, r8)
    FDK_mov_reg(r8, r3)

    /* Load stack parameters 4,5,6,7 into registers r4-r7 */
    FDK_ldrd(r4, r5, sp, 0x14, facUpsample, coeff)      // r4: facUpsample   r5: coeff
    FDK_ldrd(r6, r7, sp, 0x1C, gain, scaleFacMantissa)  // r6: gain          r7: scaleFacMantissa

    FDK_mvpush(Q4, Q7)                                  // push preserved NEON registers
    
      /* preload 7 sets of coeffs B1,B2,A1,A2 (each 4x16-bit) */
    FDK_vld1_4d_ia(16, D1, D2, D3, D4, r5)        //Q0:     A2_0/A1_0   B2_0/B1_0   XXXX/XXXX   XXXX/XXXX
                                                  //Q1:     A2_2/A1_2   B2_2/B1_2   A2_1/A1_1   B2_1/B1_1
    FDK_vld1_3d_ia(16, D5, D6, D7, r5)            //Q2:     A2_4/A1_4   B2_4/B1_4   A2_3/A1_3   B2_3/B1_3
                                                  //Q3:     A2_6/A1_6   B2_6/B1_6   A2_5/A1_5   B2_5/B1_5

    FDK_vmov_i32(64, D0, 0x0)                     //Q0:     A2_0/A1_0   B2_0/B1_0           0           0

    /* Expand 7 sets of coeffs to 32-bit format and zip-swap'em to their appropriate location */
    FDK_vswp(64, D1, D2)                          //Q0:     A2_1/A1_1   B2_1/B1_1           0           0
                                                  //Q1:     A2_2/A1_2   B2_2/B1_2   A2_0/A1_0   B2_0/B1_0
    FDK_vswp(64, D5, D6)                          //Q2:     A2_5/A1_5   B2_5/B1_5   A2_3/A1_3   B2_3/B1_3
                                                  //Q3:     A2_6/A1_6   B2_6/B1_6   A2_4/A1_4   B2_4/B1_4
    FDK_vzip_q(16, Q0, Q1)                        //Q0:     A2_0/   0   A1_0/   0   B2_0/   0   B1_0/   0
                                                  //Q1:     A2_2/A2_1   A1_2/A1_1   B2_2/B2_1   B1_2/B1_1            
    FDK_vzip_q(16, Q2, Q3)                        //Q2:     A2_4/A2_3   A1_4/A1_3   B2_4/B2_3   B1_4/B1_3
                                                  //Q3:     A2_6/A2_5   A1_6/A1_5   B2_6/B2_5   B1_6/B1_5
    FDK_vzip_q(32, Q0, Q2)                        //Q0:     B2_4/B2_3   B2_0/   0   B1_4/B1_3   B1_0/   0
                                                  //Q2:     A2_4/A2_3   A2_0/   0   A1_4/A1_3   A1_0/   0
    FDK_vzip_q(32, Q1, Q3)                        //Q1:     B2_6/B2_5   B2_2/B2_1   B1_6/B1_5   B1_2/B1_1
                                                  //Q3:     A2_6/A2_5   A2_2/A2_1   A1_6/A1_5   A1_2/A1_1         
    FDK_vzip_q(32, Q0, Q1)                        //Q0:     B1_6/B1_5   B1_4/B1_3   B1_2/B1_1   B1_0/   0
                                                  //Q1:     B2_6/B2_5   B2_4/B2_3   B2_2/B2_1   B2_0/   0
    FDK_vzip_q(32, Q2, Q3)                        //Q2:     A1_6/A1_5   A1_4/A1_3   A1_2/A1_1   A1_0/   0
                                                  //Q3:     A2_6/A2_5   A2_4/A2_3   A2_2/A2_1   A2_0/   0   

    FDK_vshll_s16_imm(Q15, D5, 16)                //Q15:    A1_6        A1_5        A1_4        A1_3
    FDK_vshll_s16_imm(Q14, D7, 16)                //Q14:    A2_6        A2_5        A2_4        A2_3
    FDK_vshll_s16_imm(Q13, D1, 16)                //Q13:    B1_6        B1_5        B1_4        B1_3
    FDK_vshll_s16_imm(Q12, D3, 16)                //Q12:    B2_6        B2_5        B2_4        B2_3

    FDK_vshll_s16_imm(Q11, D4, 16)                //Q11:    A1_2        A1_1        A1_0           0
    FDK_vshll_s16_imm(Q10, D6, 16)                //Q10:    A2_2        A2_1        A2_0           0 
    FDK_vshll_s16_imm(Q9,  D0, 16)                //Q9 :    B1_2        B1_1        B1_0           0
    FDK_vshll_s16_imm(Q8,  D2, 16)                
    FDK_vmov_sn(S32, r7)                          //Q8 :    B2_2        B2_1        B2_0   scaleFacMantissa

    FDK_cmp_imm(r4, 1)                            // compare to TD_FAC_UPSAMPLE_3_2 
    FDK_mov_cond_imm(EQ, r4, 4)
    FDK_mov_cond_imm(NE, r4, 0)
    FDK_mov_imm(r7, 4)

    /* preload 7 gain values (each 16-bit), here we load 8 (last is don't care) */
    FDK_vld1_2d_ia(16, D6, D7, r6)                // Q3:     X g6       g5 g4       g3 g2       g1 g0
    FDK_vmov_i32(64, D1, 0)                       // Q0:   0x00000000  0x00000000 
    FDK_vext_q(16, Q3, Q0, Q3, 7)                 // Q3:    g6 g5       g4 g3       g2 g1       g0 0

                                                          /* Expand 7 gains to 32-bit format */
    FDK_vshll_s16_imm(Q4, D7, 16)                 // Q4:    g6          g5          g4          g3
    FDK_vshll_s16_imm(Q3, D6, 16)                 // Q3:    g2          g1          g0          0

   /* r5, r6 not used anymore - push gains to stack and save pointers */
#ifdef __x86__
    r5 = &sp[i_sp];
    for (i = 0; i < 4; i++, i_sp++)
      sp[i_sp] = Q3[i];
    r6 = &sp[i_sp];
    for (i = 0; i < 4; i++, i_sp++)
      sp[i_sp] = Q4[i];
#else
    FDK_vpush(Q3)
    FDK_mov_reg(r5, sp)                           // r5: pointer to gains: g2          g1          g0          x
    FDK_vpush(Q4)
    FDK_mov_reg(r6, sp)                           // r6: pointer to gains: g6          g5          g4          g3
#endif

    FDK_vmov_i32(128, Q0, 0)                 // Q0:   0x00000000  0x00000000      0x00000000  0x00000000
    FDK_vmov_i32(128, Q4, 0)                 // Q4:   0x00000000  0x00000000      0x00000000  0x00000000
    FDK_vmov_i32(128, Q5, 0)                 // Q5:   0x00000000  0x00000000      0x00000000  0x00000000
    FDK_vmov_i32(128, Q6, 0)                 // Q6:   0x00000000  0x00000000      0x00000000  0x00000000
    FDK_vmov_i32(128, Q7, 0)                 // Q7:   0x00000000  0x00000000      0x00000000  0x00000000
      //PRELOAD SEC0  -------------------------------------------------------------------------------------------------------
    FDK_vld1_ia(32, S1, r0)                  // Q0:   0x00000000  0x00000000      in0=sigIn[i]0x00000000  
    FDK_vld1_ia(32, S17, r2)                 // Q4:   XXXXXXXXXX  XXXXXXXXXX      s0_0        XXXXXXXXXX  --> load state0_0
    FDK_vld1_ia(32, S21, r2)                 // Q5:   XXXXXXXXXX  XXXXXXXXXX      s1_0        XXXXXXXXXX  --> load state1_0
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000  --> load gains
    FDK_vqdmulh_s32_dd(D0, D0, D2)           // Q0:   XXXXXXXXXX  XXXXXXXXXX      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D2, D0, D18)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      in*B1       0000000000
    FDK_vadd_s32_d(D6, D0, D8)
    FDK_vadd_s32_d(D6, D6, D8)               // Q3:   XXXXXXXXXX  XXXXXXXXXX      out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_d(D8, D10, D2)              // Q4:   XXXXXXXXXX  XXXXXXXXXX      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D10, D0, D16)         // Q5:   XXXXXXXXXX  XXXXXXXXXX      in*B2       0000000000  
    FDK_vqdmulh_s32_dd(D0, D6, D22)          // Q0:   XXXXXXXXXX  XXXXXXXXXX      out*A1      0000000000  
    FDK_vqdmulh_s32_dd(D2, D6, D20)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      out*A2      0000000000
    FDK_vsub_s32_d(D8, D8, D0)               // Q4:   XXXXXXXXXX  XXXXXXXXXX      s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_d(D10, D10, D2)             // Q5:   XXXXXXXXXX  XXXXXXXXXX      s1 - out*A2 0000000000  --> save new state1

      //PRELOAD SEC1  -------------------------------------------------------------------------------------------------------
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   XXXXXXXXXX  out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   XXXXXXXXXX  in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
    FDK_vld1_ia(32, S18, r2)                 // Q4:   XXXXXXXXXX  s0_1            s0_0        XXXXXXXXXX  --> load state0_1
    FDK_vld1_ia(32, S22, r2)                 // Q5:   XXXXXXXXXX  s0_1            s1_0        XXXXXXXXXX  --> load state1_1
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000  --> load gains
    FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   XXXXXXXXXX  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   XXXXXXXXXX  in*B1           in*B1       0000000000
    FDK_vadd_s32_q(Q3, Q0, Q4)
    FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   XXXXXXXXXX  out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   XXXXXXXXXX  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   XXXXXXXXXX  in*B2           in*B2       0000000000  
    FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   XXXXXXXXXX  out*A1          out*A1      0000000000  
    FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   XXXXXXXXXX  out*A2          out*A2      0000000000
    FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   XXXXXXXXXX  s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   XXXXXXXXXX  s1 - out*A2     s1 - out*A2 0000000000  --> save new state1

      //PRELOAD SEC2  -------------------------------------------------------------------------------------------------------
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_sec1    out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   in_sec2     in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
    FDK_vld1_ia(32, S19, r2)                 // Q4:   s0_2        s0_1            s0_0        XXXXXXXXXX  --> load state0_2
    FDK_vld1_ia(32, S23, r2)                 // Q5:   s1_2        s1_1            s1_0        XXXXXXXXXX  --> load state1_2
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000  --> load gains
    FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
    FDK_vadd_s32_q(Q3, Q0, Q4)
    FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
    FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
    FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
    FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1

      //PRELOAD SEC3 (process sec0-2, then sec3-6)  -------------------------------------------------------------------------------------------------------
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  in_sec3     --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_sec1    out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   in_sec2     in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
    FDK_vld1_ia(32, S1 , r0)                 // Q0:   in_sec2     in_sec1         in_sec0     XXXXXXXXXX  --> Load new sample (in[i] - 0 - 0)
    FDK_vld1_ia(32, S24, r2)                 // Q6:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  s0_3        --> load state0_3
    FDK_vld1_ia(32, S28, r2)                 // Q7:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  s1_3        --> load state1_3
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000  --> load gains
    FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
    FDK_vadd_s32_q(Q3, Q0, Q4)
    FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
    FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
    FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
    FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
                                             //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_dd(D4, D4, D2)           // Q2:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D2, D4, D26)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  in*B1     
    FDK_vadd_s32_d(D0, D4, D12)
    FDK_vadd_s32_d(D0, D0, D12)              // Q0:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_d(D12, D14, D2)             // Q6:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D14, D4, D24)         // Q7:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  in*B2       
    FDK_vqdmulh_s32_dd(D4, D0, D30)          // Q2:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  out*A1      
    FDK_vqdmulh_s32_dd(D2, D0, D28)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  out*A2      
    FDK_vsub_s32_d(D12, D12, D4)             // Q6:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  s_0-out*A1  --> save new state0
    FDK_vsub_s32_d(D14, D14, D2)             // Q7:   XXXXXXXXXX  XXXXXXXXXX      XXXXXXXXXX  s1-out*A2   --> save new state1

      //PRELOAD SEC4 (process sec0-2, then sec3-6)  -------------------------------------------------------------------------------------------------------
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   XXXXXXXXXX  XXXXXXXXXX      in_sec4     in_sec3     --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_sec1    out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   in_sec2     in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
    FDK_vld1_ia(32, S25, r2)                 // Q6:   XXXXXXXXXX  XXXXXXXXXX      s0_4        s0_3        --> load state0_4
    FDK_vld1_ia(32, S29, r2)                 // Q7:   XXXXXXXXXX  XXXXXXXXXX      s1_4        s1_3        --> load state1_4
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
    FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
    FDK_vadd_s32_q(Q3, Q0, Q4)
    FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
    FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
    FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
    FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
                                             //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_dd(D4, D4, D2)           // Q2:   XXXXXXXXXX  XXXXXXXXXX      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D2, D4, D26)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      in*B1       in*B1     
    FDK_vadd_s32_d(D0, D4, D12)
    FDK_vadd_s32_d(D0, D0, D12)              // Q0:   XXXXXXXXXX  XXXXXXXXXX      out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_d(D12, D14, D2)             // Q6:   XXXXXXXXXX  XXXXXXXXXX      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D14, D4, D24)         // Q7:   XXXXXXXXXX  XXXXXXXXXX      in*B2       in*B2       
    FDK_vqdmulh_s32_dd(D4, D0, D30)          // Q2:   XXXXXXXXXX  XXXXXXXXXX      out*A1      out*A1      
    FDK_vqdmulh_s32_dd(D2, D0, D28)          // Q1:   XXXXXXXXXX  XXXXXXXXXX      out*A2      out*A2      
    FDK_vsub_s32_d(D12, D12, D4)             // Q6:   XXXXXXXXXX  XXXXXXXXXX      s_0-out*A1  s_0-out*A1  --> save new state0
    FDK_vsub_s32_d(D14, D14, D2)             // Q7:   XXXXXXXXXX  XXXXXXXXXX      s1-out*A2   s1-out*A2   --> save new state1

      //PRELOAD SEC5 (process sec0-2, then sec3-6)  -------------------------------------------------------------------------------------------------------
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   XXXXXXXXXX  in_sec5         in_sec4     in_sec3     --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_sec1    out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   in_sec2     in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
    FDK_vld1_ia(32, S26, r2)                 // Q6:   XXXXXXXXXX  s0_5            s0_4        s0_3        --> load state0_5
    FDK_vld1_ia(32, S30, r2)                 // Q7:   XXXXXXXXXX  s1_5            s1_4        s1_3        --> load state1_5
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
    FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
    FDK_vadd_s32_q(Q3, Q0, Q4)
    FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
    FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
    FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
    FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
    FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
                                             //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   XXXXXXXXXX  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   XXXXXXXXXX  in*B1           in*B1       in*B1     
    FDK_vadd_s32_q(Q0, Q2, Q6)
    FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   XXXXXXXXXX  out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   XXXXXXXXXX  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   XXXXXXXXXX  in*B2           in*B2       in*B2       
    FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   XXXXXXXXXX  out*A1          out*A1      out*A1      
    FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   XXXXXXXXXX  out*A2          out*A2      out*A2      
    FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   XXXXXXXXXX  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
    FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   XXXXXXXXXX  s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1

      //preprocess sec 6 gains ----------------------------------------------------------------------------------------------------------------------------------
    FDK_vld1_ia(32, S27, r2)                 // Q6:   s0_6        s0_5            s0_4        s0_3        --> load state0_6
    FDK_vld1_ia(32, S31, r2)                 // Q7:   s1_6        s1_5            s1_4        s_1_3       --> load state1_6
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   in_sec      in_sec5         in_sec4     in_sec3     --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_sec1    out_sec0        0000000000  XXXXXXXXXX  
                                             // Q0:   in_sec2     in_sec1         0000000000  XXXXXXXXXX  --> out-register is new in-register
                                             //
    FDK_subs_imm(r3, r3, 2)
    FDK_branch(EQ, TD_upsampler_3_1_loopEND) // Skip loop, if lenIn was only 2 (lenIn is always even)

      // Loop: do 3 iterations(each 7 sections) per loop:  load new sample - set input 0 - set input 0  ------------------------------------------------
FDK_label(TD_upsampler_3_1_loop)
      FDK_vld1_ia(32, S1, r0)                  // Q0:   in_sec2     in_sec1         in_sec0     XXXXXXXXXX  --> Load new sample (in[i] - 0 - 0)
        // --- (process sec0 - 2, then sec3 - 6)
      FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
      FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
      FDK_vadd_s32_q(Q3, Q0, Q4)
      FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
      FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
      FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
      FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
      FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
      FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
        //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
      FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
      FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
      FDK_vadd_s32_q(Q0, Q2, Q6)
      FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
      FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
      FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
      FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
      FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
      FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
        //--- reorder samples
      FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    out2=in3    --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
      FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_1=in2   out_0=in_1      0000000000  out6     --> out-regs[sec0-1] is EXTed in-regs[sec1-2] 
                                               // Q0:   in_sec2     in_sec1         0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
        //--- store result sample
      FDK_vst1_pu(32, S0, r1, r7)
      FDK_eor(r7, r7, r4)

      //--------------------------------------------------------------------------------------------------------------------------------------------------
      //--------------------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec0 - 2, then sec3 - 6)
      FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
      FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
      FDK_vadd_s32_q(Q3, Q0, Q4)
      FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
      FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
      FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
      FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
      FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
      FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
                                               //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
      FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
      FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
      FDK_vadd_s32_q(Q0, Q2, Q6)
      FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
      FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
      FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
      FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
      FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
      FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
        //--- reorder samples
      FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    out2=in3    --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
      FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_1=in2   out_0=in_1      0000000000  out6     --> out-regs[sec0-1] is EXTed in-regs[sec1-2] 
                                               // Q0:   in_sec2     in_sec1         0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
        //--- store result sample
      FDK_vst1_pu(32, S0, r1, r7)
      FDK_eor(r7, r7, r4)
      //--------------------------------------------------------------------------------------------------------------------------------------------------
      //--------------------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec0 - 2, then sec3 - 6)
      FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
      FDK_vqdmulh_s32_qq(Q0, Q0, Q1)           // Q0:   g2*in_sec2  g1*in_sec1      g0*in_sec0  0000000000  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q0, Q9)           // Q1:   in*B1       in*B1           in*B1       0000000000
      FDK_vadd_s32_q(Q3, Q0, Q4)
      FDK_vadd_s32_q(Q3, Q3, Q4)               // Q3:   out=in+s0*2 out=in+s0*2     out=in+s0*2 0000000000  --> out = in+state_0<<1
      FDK_vadd_s32_q(Q4, Q5, Q1)               // Q4:   s1 + in*B1  s1 + in*B1      s1 + in*B1  0000000000  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q5, Q0, Q8)           // Q5:   in*B2       in*B2           in*B2       0000000000  
      FDK_vqdmulh_s32_qq(Q0, Q3, Q11)          // Q0:   out*A1      out*A1          out*A1      0000000000  
      FDK_vqdmulh_s32_qq(Q1, Q3, Q10)          // Q1:   out*A2      out*A2          out*A2      0000000000
      FDK_vsub_s32_q(Q4, Q4, Q0)               // Q4:   s0 - out*A1 s0 - out*A1     s_0 -out*A1 0000000000  --> save new state0
      FDK_vsub_s32_q(Q5, Q5, Q1)               // Q5:   s1 - out*A2 s1 - out*A2     s1 - out*A2 0000000000  --> save new state1
                                               //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
      FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
      FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
      FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
      FDK_vadd_s32_q(Q0, Q2, Q6)
      FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
      FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
      FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
      FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
      FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
      FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
      FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
       //--- reorder samples
      FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    out2=in3    --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
      FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out_1=in2   out_0=in_1      0000000000  out6     --> out-regs[sec0-1] is EXTed in-regs[sec1-2] 
                                               // Q0:   in_sec2     in_sec1         0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
        //--- store result sample
      FDK_vst1_pu(32, S0, r1, r7)
      FDK_eor(r7, r7, r4)
      FDK_subs_imm(r3, r3, 1)
      FDK_branch(NE,TD_upsampler_3_1_loop)

FDK_label(TD_upsampler_3_1_loopEND)
    FDK_sub_imm(r2, r2, 14<<2, 2)
      // SAVE  SEC0 -----------------------------------------------------------------------------------------------------------------------------------
    FDK_vst1_ia(32, S17, r2)                 // store state0_0
    FDK_vst1_ia(32, S21, r2)                 // store state1_0
      // SAVE  SEC1 -----------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec0 - 2, then sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
    FDK_vqdmulh_s32_dd(D1, D1,D3)            // Q0:   g2*in_sec2  g1*in_sec1      XXXXXXXXXX  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D3, D1, D19)          // Q1:   in*B1       in*B1           XXXXXXXXXX  0000000000
    FDK_vadd_s32_d(D7, D1, D9)
    FDK_vadd_s32_d(D7, D7, D9)               // Q3:   out=in+s0*2 out=in+s0*2     XXXXXXXXXX  0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_d(D9, D11, D3)              // Q4:   s1 + in*B1  s1 + in*B1      XXXXXXXXXX  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D11, D1, D17)         // Q5:   in*B2       in*B2           XXXXXXXXXX  0000000000  
    FDK_vqdmulh_s32_dd(D1, D7, D23)          // Q0:   out*A1      out*A1          XXXXXXXXXX  0000000000  
    FDK_vqdmulh_s32_dd(D3, D7, D21)          // Q1:   out*A2      out*A2          XXXXXXXXXX  0000000000
    FDK_vsub_s32_d(D9, D9, D1)               // Q4:   s0 - out*A1 s0 - out*A1     XXXXXXXXXX  0000000000  --> save new state0
    FDK_vsub_s32_d(D11, D11, D3)             // Q5:   s1 - out*A2 s1 - out*A2     XXXXXXXXXX  0000000000  --> save new state1
        //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
    FDK_vadd_s32_q(Q0, Q2, Q6)
    FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
    FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
    FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
    FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
    FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
      //--- reorder samples
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    out2=in3    --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   out1=in2    XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2] 
                                             // Q0:   in_sec2     XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
      //--- store result sample
    FDK_vst1_pu(32, S0, r1, r7)
    FDK_eor(r7, r7, r4)
    FDK_vst1_ia(32, S18, r2)                 // store state0_1
    FDK_vst1_ia(32, S22, r2)                 // store state1_1
      // SAVE  SEC2 -------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec0 - 2, then sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r5)              // Q1:   g2          g1              g0          0000000000
    FDK_vqdmulh_s32_dd(D1, D1, D3)           // Q0:   g2*in_sec2  XXXXXXXXXX      XXXXXXXXXX  0000000000  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D3, D1, D19)          // Q1:   in*B1       XXXXXXXXXX      XXXXXXXXXX  0000000000
    FDK_vadd_s32_d(D7, D1, D9)
    FDK_vadd_s32_d(D7, D7, D9)               // Q3:   out=in+s0*2 XXXXXXXXXX      XXXXXXXXXX  0000000000  --> out = in+state_0<<1
    FDK_vadd_s32_d(D9, D11, D3)              // Q4:   s1 + in*B1  XXXXXXXXXX      XXXXXXXXXX  0000000000  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D11, D1, D17)         // Q5:   in*B2       XXXXXXXXXX      XXXXXXXXXX  0000000000  
    FDK_vqdmulh_s32_dd(D1, D7, D23)          // Q0:   out*A1      XXXXXXXXXX      XXXXXXXXXX  0000000000  
    FDK_vqdmulh_s32_dd(D3, D7, D21)          // Q1:   out*A2      XXXXXXXXXX      XXXXXXXXXX  0000000000
    FDK_vsub_s32_d(D9, D9, D1)               // Q4:   s0 - out*A1 XXXXXXXXXX      XXXXXXXXXX  0000000000  --> save new state0
    FDK_vsub_s32_d(D11, D11, D3)             // Q5:   s1 - out*A2 XXXXXXXXXX      XXXXXXXXXX  0000000000  --> save new state1
      //--- end of sec0-2, start of sec3-6 [Q3 is locked!]
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
    FDK_vadd_s32_q(Q0, Q2, Q6)
    FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
    FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
    FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
    FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
    FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
      //--- reorder samples
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    out2=in3    --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2] 
                                             // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
     //--- store result sample
    FDK_vst1_pu(32, S0, r1, r7)
    FDK_eor(r7, r7, r4)
    FDK_vst1_ia(32, S19, r2)                 // store state0_2
    FDK_vst1_ia(32, S23, r2)                 // store state1_2    
      // SAVE SEC3 -------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  g3*in_sec3  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       in*B1     
    FDK_vadd_s32_q(Q0, Q2, Q6)
    FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 out=in+s0*2 --> out = in+state_0<<1
    FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  s1 + in*B1  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       in*B2       
    FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      out*A1      
    FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      out*A2      
    FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  s_0-out*A1  --> save new state0
    FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   s1-out*A2   --> save new state1
      //--- reorder samples
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        out3=in4    XXXXXXXXXX  --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
                                             // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
                                             //--- store result sample
    FDK_vst1_pu(32, S0, r1, r7)
    FDK_eor(r7, r7, r4)
    FDK_vst1_ia(32, S24, r2)                 // store state0_3
    FDK_vst1_ia(32, S28, r2)                 // store state1_3
      // SAVE SEC4 -------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)           // Q2:   g6*in_sec6  g5*in_sec5      g4*in_sec4  XXXXXXXXXX  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_qq(Q1, Q2, Q13)          // Q1:   in*B1       in*B1           in*B1       XXXXXXXXXX
    FDK_vadd_s32_q(Q0, Q2, Q6)
    FDK_vadd_s32_q(Q0, Q0, Q6)               // Q0:   out=in+s0*2 out=in+s0*2     out=in+s0*2 XXXXXXXXXX  --> out = in+state_0<<1
    FDK_vadd_s32_q(Q6, Q7, Q1)               // Q6:   s1 + in*B1  s1 + in*B1      s1 + in*B1  XXXXXXXXXX  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_qq(Q7, Q2, Q12)          // Q7:   in*B2       in*B2           in*B2       XXXXXXXXXX  
    FDK_vqdmulh_s32_qq(Q2, Q0, Q15)          // Q2:   out*A1      out*A1          out*A1      XXXXXXXXXX  
    FDK_vqdmulh_s32_qq(Q1, Q0, Q14)          // Q1:   out*A2      out*A2          out*A2      XXXXXXXXXX  
    FDK_vsub_s32_q(Q6, Q6, Q2)               // Q6:   s_0-out*A1  s_0-out*A1      s_0-out*A1  XXXXXXXXXX  --> save new state0
    FDK_vsub_s32_q(Q7, Q7, Q1)               // Q7:   s1-out*A2   s1-out*A2       s1-out*A2   XXXXXXXXXX  --> save new state1
      //--- reorder samples
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    out4=in5        XXXXXXXXXX  XXXXXXXXXX  --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
                                             // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
      //--- store result sample
    FDK_vst1_pu(32, S0, r1, r7)
    FDK_eor(r7, r7, r4)
    FDK_vst1_ia(32, S25, r2)                 // store state0_4
    FDK_vst1_ia(32, S29, r2)                 // store state1_4
      // SAVE SEC5 -------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_dd(D5, D5, D3)           // Q2:   g6*in_sec6  g5*in_sec5      XXXXXXXXXX  XXXXXXXXXX  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D3, D5, D27)          // Q1:   in*B1       in*B1           XXXXXXXXXX  XXXXXXXXXX  
    FDK_vadd_s32_d(D1, D5, D13)
    FDK_vadd_s32_d(D1, D1, D13)              // Q0:   out=in+s0*2 out=in+s0*2     XXXXXXXXXX  XXXXXXXXXX  --> out = in+state_0<<1
    FDK_vadd_s32_d(D13, D15, D3)             // Q6:   s1 + in*B1  s1 + in*B1      XXXXXXXXXX  XXXXXXXXXX  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D15, D5, D25)         // Q7:   in*B2       in*B2           XXXXXXXXXX  XXXXXXXXXX  
    FDK_vqdmulh_s32_dd(D5, D1, D31)          // Q2:   out*A1      out*A1          XXXXXXXXXX  XXXXXXXXXX  
    FDK_vqdmulh_s32_dd(D3, D1, D29)          // Q1:   out*A2      out*A2          XXXXXXXXXX  XXXXXXXXXX  
    FDK_vsub_s32_d(D13, D13, D5)             // Q6:   s_0-out*A1  s_0-out*A1      XXXXXXXXXX  XXXXXXXXXX  --> save new state0
    FDK_vsub_s32_d(D15, D15, D3)             // Q7:   s1-out*A2   s1-out*A2       XXXXXXXXXX  XXXXXXXXXX  --> save new state1
      //--- reorder samples
    FDK_vext_q(32, Q2, Q3, Q0, 3)            // Q2:   out5=in6    XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> out-regs[sec2] is EXTed in-regs[sec3-6] (backup out2)
    FDK_vext_q(32, Q0, Q0, Q3, 3)            // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]
                                             // Q0:   XXXXXXXXXX  XXXXXXXXXX      0000000000  out6        --> out-regs[sec0-1] is EXTed in-regs[sec1-2]

      //--- store result sample
    FDK_vst1_pu(32, S0, r1, r7)
    FDK_eor(r7, r7, r4)

    FDK_vst1_ia(32, S26, r2)                 // store state0_5
    FDK_vst1_ia(32, S30, r2)                 // store state1_5
      // SAVE SEC6 -------------------------------------------------------------------------------------------------------------------------------------
      // --- (process sec3 - 6)
    FDK_vld1_2d(32, D2, D3, r6)              // Q1:   g6          g5              g4          g3
    FDK_vqdmulh_s32_dd(D5, D5, D3)           // Q2:   g6*in_sec6  XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> inx = in_secx*gainx
    FDK_vqdmulh_s32_dd(D3, D5, D27)          // Q1:   in*B1       XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  
    FDK_vadd_s32_d(D1, D5, D13)
    FDK_vadd_s32_d(D1, D1, D13)              // Q0:   out=in+s0*2 XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> out = in+state_0<<1
    FDK_vadd_s32_d(D13, D15, D3)             // Q6:   s1 + in*B1  XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> s0 = s1 + in * B1
    FDK_vqdmulh_s32_dd(D15, D5, D25)         // Q7:   in*B2       XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  
    FDK_vqdmulh_s32_dd(D5, D1, D31)          // Q2:   out*A1      XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  
    FDK_vqdmulh_s32_dd(D3, D1, D29)          // Q1:   out*A2      XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  
    FDK_vsub_s32_d(D13, D13, D5)             // Q6:   s_0-out*A1  XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> save new state0
    FDK_vsub_s32_d(D15, D15, D3)             // Q7:   s1-out*A2   XXXXXXXXXX      XXXXXXXXXX  XXXXXXXXXX  --> save new state1

      //--- store last result sample only, if pointer update (r7) is unequal 0 to prevent writing beyond buffer
    FDK_cmp_imm(r7, 0)
    FDK_branch(EQ, TD_upsampler_3_1_store_state6)
      FDK_vst1_pu(32, S0, r1, r7)

FDK_label(TD_upsampler_3_1_store_state6)
    FDK_vst1_ia(32, S27, r2)                 // store state0_6
    FDK_vst1_ia(32, S31, r2)                 // store state1_6

      // ------------------------------------------------------------------------------------------------------------------------------------------------------------
      // Scale a posteriori according to scalingmantissa and exponent -----------------------------------------------------------------------------------------------
      
    FDK_vmov_d(D15, D16)                     // D15: xxxxxxxx    scaleFacMantissa
      
    FDK_cmp_imm(r4, 4)                       // 4: UPSAMPLE_3_2 / 0: UPSAMPLE_3_1
    FDK_mov_imm(r3, 1)
    FDK_asr_cond(EQ, r8, r8, r3)
    FDK_mov_imm(r3, 3)
    FDK_mul(r8, r8, r3)                      // r8_ lenOut = lenIn*3/2 (for UPSAMPLE_3_2) or lenIn*3 (UPSAMPLE_3_1)
    FDK_sub_op_lsl(r1, r1, r8, 2, 2)         // r1: sigOut[0] used as read pointer
    FDK_mov_reg(r2, r1)                      // r2: &sigOut[0] used as write pointer

//FDK_label(TD_upsampler_3_1_tst1)
    FDK_asrs_imm(r8, r8, 1)                  // r8: lenOut >> 1, carry set, if lenOut & 1
    FDK_branch(CC, TD_upsampler_3_1_tst2)
      FDK_vld1_ia(32, S0, r1)                // Q0: ----------------------- XXXXXXXXXXX sigOut[i+0]
      FDK_vqdmulh_s32_ds(D0,  D0, S30)       // Q0: SigOut * scalefacmantissa
      FDK_vqshl_s32_imm(64,   D0, D0, 2)     // Q0: sigOut << scalefacExponent
      FDK_vst1_ia(32, S0, r2)                // Q0: store 1x output

FDK_label(TD_upsampler_3_1_tst2)
    FDK_asrs_imm(r8, r8, 1)                  // r8:  lenOut >> 2, carry set, if lenOut & 2
    FDK_branch(CC, TD_upsampler_3_1_tst4)
      FDK_vld1_1d_ia(32, D0, r1)             // Q0: ----------------------- sigOut[i+1] sigOut[i+0]
      FDK_vqdmulh_s32_ds(D0, D0, S30)        // Q0: SigOut * scalefacmantissa
      FDK_vqshl_s32_imm(64, D0, D0, 2)       // Q0: sigOut << scalefacExponent
      FDK_vst1_1d_ia(32, D0, r2)             // Q0: store 2x output

FDK_label(TD_upsampler_3_1_tst4)
    FDK_asrs_imm(r8, r8, 1)                  // r8:  lenOut >> 3, carry set, if lenOut & 4
    FDK_branch(CC, TD_upsampler_3_1_tst8)
      FDK_vld1_2d_ia(32, D0, D1, r1)         // Q0: sigOut[i+3] sigOut[i+2] sigOut[i+1] sigOut[i+0]
      FDK_vqdmulh_s32_qs(Q0, Q0, S30)        // Q0: SigOut * scalefacmantissa
      FDK_vqshl_s32_imm(128, Q0, Q0, 2)      // Q0: sigOut << scalefacExponent
      FDK_vst1_2d_ia(32, D0, D1, r2)         // Q0: store 4x output
 

FDK_label(TD_upsampler_3_1_tst8)
    FDK_asrs_imm(r8, r8, 1)                  // r8:  lenOut >> 4, carry set, if lenOut & 8
    FDK_branch(CC, TD_upsampler_3_1_tst16)
      FDK_vld1_4d_ia(32, D0,D1,D2,D3, r1)    // Q1: sigOut[i+7] sigOut[i+6] sigOut[i+5] sigOut[i+4]
                                             // Q0: sigOut[i+3] sigOut[i+2] sigOut[i+1] sigOut[i+0]
      FDK_vqdmulh_s32_qs(Q0, Q0, S30)        // Q0: SigOut * scalefacmantissa
      FDK_vqdmulh_s32_qs(Q1, Q1, S30)        // Q1: SigOut * scalefacmantissa
      FDK_vqshl_s32_imm(128, Q0, Q0, 2)      // Q0: sigOut << scalefacExponent
      FDK_vqshl_s32_imm(128, Q1, Q1, 2)      // Q1: sigOut << scalefacExponent
      FDK_vst1_4d_ia(32, D0, D1, D2, D3, r2) // Q0,Q1: store 8x output


FDK_label(TD_upsampler_3_1_tst16)
    FDK_branch(EQ, TD_upsampler_3_1_end)
FDK_label(TD_upsampler_3_1_loop16)
      FDK_vldm4_ia(128, Q0, Q1, Q2, Q3, r1)  // Q3: sigOut[i+15] sigOut[i+14] sigOut[i+13] sigOut[i+12]
                                             // Q2: sigOut[i+11] sigOut[i+10] sigOut[i+ 9] sigOut[i+ 8]
                                             // Q1: sigOut[i+ 7] sigOut[i+ 6] sigOut[i+ 5] sigOut[i+ 4]
                                             // Q0: sigOut[i+3] sigOut[i+2] sigOut[i+1] sigOut[i+0]
      FDK_subs_imm(r8, r8, 1)                // r8: loop counter decrement
      FDK_vqdmulh_s32_qs(Q0, Q0, S30)        // Q0: SigOut * scalefacmantissa
      FDK_vqdmulh_s32_qs(Q1, Q1, S30)        // Q1: SigOut * scalefacmantissa
      FDK_vqdmulh_s32_qs(Q2, Q2, S30)        // Q2: SigOut * scalefacmantissa
      FDK_vqdmulh_s32_qs(Q3, Q3, S30)        // Q3: SigOut * scalefacmantissa
      FDK_vqshl_s32_imm(128, Q0, Q0, 2)      // Q0: sigOut << scalefacExponent
      FDK_vqshl_s32_imm(128, Q1, Q1, 2)      // Q1: sigOut << scalefacExponent
      FDK_vqshl_s32_imm(128, Q2, Q2, 2)      // Q2: sigOut << scalefacExponent
      FDK_vqshl_s32_imm(128, Q3, Q3, 2)      // Q3: sigOut << scalefacExponent
      FDK_vstm4_ia(128, Q0, Q1, Q2, Q3, r2)  // Q0,Q1,Q2,Q3: store 16x output
      FDK_branch(NE, TD_upsampler_3_1_loop16)
//---------------------------------------------------------------------------------------- End of scaling

FDK_label(TD_upsampler_3_1_end)
    FDK_mvpop(Q3, Q4)
    FDK_mvpop(Q4, Q7)
    FDK_mpop(r4, r8)
    FDK_return()

FDK_ASM_ROUTINE_END()

short TD_upsampler_3_1(
      const FIXP_DBL * RESTRICT sigIn,
      FIXP_DBL * RESTRICT sigOut,
      FIXP_DBL * RESTRICT states,
      short lenIn,
      TD_FAC_UPSAMPLE facUpsample)
{
   if (lenIn > 0)
     /* The reinterpret_cast is used to suppress a compiler warning. We know that sos_3.sos_gain and sos_3.scaleFacMantissa are sufficiently aligned, so the cast is safe */
     TD_upsampler_3_1_neonv7(sigIn, sigOut, states, (INT) lenIn, (INT) facUpsample, reinterpret_cast<const FIXP_DBL *>(reinterpret_cast<const void *>(sos_3.coeff)), reinterpret_cast<const FIXP_DBL *>(reinterpret_cast<const void *>(sos_3.sos_gain)), (INT) sos_3.scaleFacMantissa);
   return (lenIn*3) >> ((facUpsample == TD_FAC_UPSAMPLE_3_2) ? 1 : 0);
}


#endif /* #if defined(__ARM_NEON__) && (7 == 7) */
#endif /* FUNCTION_TD_upsampler_3_1 */
