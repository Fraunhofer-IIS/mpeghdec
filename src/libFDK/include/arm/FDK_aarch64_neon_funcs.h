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

   Description: Functions replacing NEON operations for ARMv8/64-bit

*******************************************************************************/

/* clang-format off */
#ifndef FDK_AARCH64_NEON_FUNCS_H
#define FDK_AARCH64_NEON_FUNCS_H

#ifdef __ARM_AARCH64_NEON__
  #define HASHSIGN #
  #define PLUSSIGN +
  #define FDK_TEXT(x) x
  #define A64_V int128_t
  #define A64_D int64_t
  #define A64_S int32_t
  #define A64_H int16_t

#define V0_16B  V0.16B
#define V0_8B   V0.8B
#define V0_8H   V0.8H
#define V0_4H   V0.4H
#define V0_4S   V0.4S
#define V0_2S   V0.2S
#define V0_2D   V0.2D
#define V0_1D   V0.1D
#define V0_D    V0.D
#define V0_S    V0.S
#define V0_H    V0.H
#define V0_B    V0.B

#define V1_16B  V1.16B
#define V1_8B   V1.8B
#define V1_8H   V1.8H
#define V1_4H   V1.4H
#define V1_4S   V1.4S
#define V1_2S   V1.2S
#define V1_2D   V1.2D
#define V1_1D   V1.1D
#define V1_D    V1.D
#define V1_S    V1.S
#define V1_H    V1.H
#define V1_B    V1.B

#define V2_16B  V2.16B
#define V2_8B   V2.8B
#define V2_8H   V2.8H
#define V2_4H   V2.4H
#define V2_4S   V2.4S
#define V2_2S   V2.2S
#define V2_2D   V2.2D
#define V2_1D   V2.1D
#define V2_D    V2.D
#define V2_S    V2.S
#define V2_H    V2.H
#define V2_B    V2.B

#define V3_16B  V3.16B
#define V3_8B   V3.8B
#define V3_8H   V3.8H
#define V3_4H   V3.4H
#define V3_4S   V3.4S
#define V3_2S   V3.2S
#define V3_2D   V3.2D
#define V3_1D   V3.1D
#define V3_D    V3.D
#define V3_S    V3.S
#define V3_H    V3.H
#define V3_B    V3.B

#define V4_16B  V4.16B
#define V4_8B   V4.8B
#define V4_8H   V4.8H
#define V4_4H   V4.4H
#define V4_4S   V4.4S
#define V4_2S   V4.2S
#define V4_2D   V4.2D
#define V4_1D   V4.1D
#define V4_D    V4.D
#define V4_S    V4.S
#define V4_H    V4.H
#define V4_B    V4.B

#define V5_16B  V5.16B
#define V5_8B   V5.8B
#define V5_8H   V5.8H
#define V5_4H   V5.4H
#define V5_4S   V5.4S
#define V5_2S   V5.2S
#define V5_2D   V5.2D
#define V5_1D   V5.1D
#define V5_D    V5.D
#define V5_S    V5.S
#define V5_H    V5.H
#define V5_B    V5.B

#define V6_16B  V6.16B
#define V6_8B   V6.8B
#define V6_8H   V6.8H
#define V6_4H   V6.4H
#define V6_4S   V6.4S
#define V6_2S   V6.2S
#define V6_2D   V6.2D
#define V6_1D   V6.1D
#define V6_D    V6.D
#define V6_S    V6.S
#define V6_H    V6.H
#define V6_B    V6.B

#define V7_16B  V7.16B
#define V7_8B   V7.8B
#define V7_8H   V7.8H
#define V7_4H   V7.4H
#define V7_4S   V7.4S
#define V7_2S   V7.2S
#define V7_2D   V7.2D
#define V7_1D   V7.1D
#define V7_D    V7.D
#define V7_S    V7.S
#define V7_H    V7.H
#define V7_B    V7.B

#define V8_16B  V8.16B
#define V8_8B   V8.8B
#define V8_8H   V8.8H
#define V8_4H   V8.4H
#define V8_4S   V8.4S
#define V8_2S   V8.2S
#define V8_2D   V8.2D
#define V8_1D   V8.1D
#define V8_D    V8.D
#define V8_S    V8.S
#define V8_H    V8.H
#define V8_B    V8.B

#define V9_16B  V9.16B
#define V9_8B   V9.8B
#define V9_8H   V9.8H
#define V9_4H   V9.4H
#define V9_4S   V9.4S
#define V9_2S   V9.2S
#define V9_2D   V9.2D
#define V9_1D   V9.1D
#define V9_D    V9.D
#define V9_S    V9.S
#define V9_H    V9.H
#define V9_B    V9.B

#define V10_16B  V10.16B
#define V10_8B   V10.8B
#define V10_8H   V10.8H
#define V10_4H   V10.4H
#define V10_4S   V10.4S
#define V10_2S   V10.2S
#define V10_2D   V10.2D
#define V10_1D   V10.1D
#define V10_D    V10.D
#define V10_S    V10.S
#define V10_H    V10.H
#define V10_B    V10.B

#define V11_16B  V11.16B
#define V11_8B   V11.8B
#define V11_8H   V11.8H
#define V11_4H   V11.4H
#define V11_4S   V11.4S
#define V11_2S   V11.2S
#define V11_2D   V11.2D
#define V11_1D   V11.1D
#define V11_D    V11.D
#define V11_S    V11.S
#define V11_H    V11.H
#define V11_B    V11.B

#define V12_16B  V12.16B
#define V12_8B   V12.8B
#define V12_8H   V12.8H
#define V12_4H   V12.4H
#define V12_4S   V12.4S
#define V12_2S   V12.2S
#define V12_2D   V12.2D
#define V12_1D   V12.1D
#define V12_D    V12.D
#define V12_S    V12.S
#define V12_H    V12.H
#define V12_B    V12.B

#define V13_16B  V13.16B
#define V13_8B   V13.8B
#define V13_8H   V13.8H
#define V13_4H   V13.4H
#define V13_4S   V13.4S
#define V13_2S   V13.2S
#define V13_2D   V13.2D
#define V13_1D   V13.1D
#define V13_D    V13.D
#define V13_S    V13.S
#define V13_H    V13.H
#define V13_B    V13.B

#define V14_16B  V14.16B
#define V14_8B   V14.8B
#define V14_8H   V14.8H
#define V14_4H   V14.4H
#define V14_4S   V14.4S
#define V14_2S   V14.2S
#define V14_2D   V14.2D
#define V14_1D   V14.1D
#define V14_D    V14.D
#define V14_S    V14.S
#define V14_H    V14.H
#define V14_B    V14.B

#define V15_16B  V15.16B
#define V15_8B   V15.8B
#define V15_8H   V15.8H
#define V15_4H   V15.4H
#define V15_4S   V15.4S
#define V15_2S   V15.2S
#define V15_2D   V15.2D
#define V15_1D   V15.1D
#define V15_D    V15.D
#define V15_S    V15.S
#define V15_H    V15.H
#define V15_B    V15.B

#define V16_16B  V16.16B
#define V16_8B   V16.8B
#define V16_8H   V16.8H
#define V16_4H   V16.4H
#define V16_4S   V16.4S
#define V16_2S   V16.2S
#define V16_2D   V16.2D
#define V16_1D   V16.1D
#define V16_D    V16.D
#define V16_S    V16.S
#define V16_H    V16.H
#define V16_B    V16.B

#define V17_16B  V17.16B
#define V17_8B   V17.8B
#define V17_8H   V17.8H
#define V17_4H   V17.4H
#define V17_4S   V17.4S
#define V17_2S   V17.2S
#define V17_2D   V17.2D
#define V17_1D   V17.1D
#define V17_D    V17.D
#define V17_S    V17.S
#define V17_H    V17.H
#define V17_B    V17.B

#define V18_16B  V18.16B
#define V18_8B   V18.8B
#define V18_8H   V18.8H
#define V18_4H   V18.4H
#define V18_4S   V18.4S
#define V18_2S   V18.2S
#define V18_2D   V18.2D
#define V18_1D   V18.1D
#define V18_D    V18.D
#define V18_S    V18.S
#define V18_H    V18.H
#define V18_B    V18.B

#define V19_16B  V19.16B
#define V19_8B   V19.8B
#define V19_8H   V19.8H
#define V19_4H   V19.4H
#define V19_4S   V19.4S
#define V19_2S   V19.2S
#define V19_2D   V19.2D
#define V19_1D   V19.1D
#define V19_D    V19.D
#define V19_S    V19.S
#define V19_H    V19.H
#define V19_B    V19.B

#define V20_16B  V20.16B
#define V20_8B   V20.8B
#define V20_8H   V20.8H
#define V20_4H   V20.4H
#define V20_4S   V20.4S
#define V20_2S   V20.2S
#define V20_2D   V20.2D
#define V20_1D   V20.1D
#define V20_D    V20.D
#define V20_S    V20.S
#define V20_H    V20.H
#define V20_B    V20.B

#define V21_16B  V21.16B
#define V21_8B   V21.8B
#define V21_8H   V21.8H
#define V21_4H   V21.4H
#define V21_4S   V21.4S
#define V21_2S   V21.2S
#define V21_2D   V21.2D
#define V21_1D   V21.1D
#define V21_D    V21.D
#define V21_S    V21.S
#define V21_H    V21.H
#define V21_B    V21.B

#define V22_16B  V22.16B
#define V22_8B   V22.8B
#define V22_8H   V22.8H
#define V22_4H   V22.4H
#define V22_4S   V22.4S
#define V22_2S   V22.2S
#define V22_2D   V22.2D
#define V22_1D   V22.1D
#define V22_D    V22.D
#define V22_S    V22.S
#define V22_H    V22.H
#define V22_B    V22.B

#define V23_16B  V23.16B
#define V23_8B   V23.8B
#define V23_8H   V23.8H
#define V23_4H   V23.4H
#define V23_4S   V23.4S
#define V23_2S   V23.2S
#define V23_2D   V23.2D
#define V23_1D   V23.1D
#define V23_D    V23.D
#define V23_S    V23.S
#define V23_H    V23.H
#define V23_B    V23.B

#define V24_16B  V24.16B
#define V24_8B   V24.8B
#define V24_8H   V24.8H
#define V24_4H   V24.4H
#define V24_4S   V24.4S
#define V24_2S   V24.2S
#define V24_2D   V24.2D
#define V24_1D   V24.1D
#define V24_D    V24.D
#define V24_S    V24.S
#define V24_H    V24.H
#define V24_B    V24.B

#define V25_16B  V25.16B
#define V25_8B   V25.8B
#define V25_8H   V25.8H
#define V25_4H   V25.4H
#define V25_4S   V25.4S
#define V25_2S   V25.2S
#define V25_2D   V25.2D
#define V25_1D   V25.1D
#define V25_D    V25.D
#define V25_S    V25.S
#define V25_H    V25.H
#define V25_B    V25.B

#define V26_16B  V26.16B
#define V26_8B   V26.8B
#define V26_8H   V26.8H
#define V26_4H   V26.4H
#define V26_4S   V26.4S
#define V26_2S   V26.2S
#define V26_2D   V26.2D
#define V26_1D   V26.1D
#define V26_D    V26.D
#define V26_S    V26.S
#define V26_H    V26.H
#define V26_B    V26.B

#define V27_16B  V27.16B
#define V27_8B   V27.8B
#define V27_8H   V27.8H
#define V27_4H   V27.4H
#define V27_4S   V27.4S
#define V27_2S   V27.2S
#define V27_2D   V27.2D
#define V27_1D   V27.1D
#define V27_D    V27.D
#define V27_S    V27.S
#define V27_H    V27.H
#define V27_B    V27.B

#define V28_16B  V28.16B
#define V28_8B   V28.8B
#define V28_8H   V28.8H
#define V28_4H   V28.4H
#define V28_4S   V28.4S
#define V28_2S   V28.2S
#define V28_2D   V28.2D
#define V28_1D   V28.1D
#define V28_D    V28.D
#define V28_S    V28.S
#define V28_H    V28.H
#define V28_B    V28.B

#define V29_16B  V29.16B
#define V29_8B   V29.8B
#define V29_8H   V29.8H
#define V29_4H   V29.4H
#define V29_4S   V29.4S
#define V29_2S   V29.2S
#define V29_2D   V29.2D
#define V29_1D   V29.1D
#define V29_D    V29.D
#define V29_S    V29.S
#define V29_H    V29.H
#define V29_B    V29.B

#define V30_16B  V30.16B
#define V30_8B   V30.8B
#define V30_8H   V30.8H
#define V30_4H   V30.4H
#define V30_4S   V30.4S
#define V30_2S   V30.2S
#define V30_2D   V30.2D
#define V30_1D   V30.1D
#define V30_D    V30.D
#define V30_S    V30.S
#define V30_H    V30.H
#define V30_B    V30.B

#define V31_16B  V31.16B
#define V31_8B   V31.8B
#define V31_8H   V31.8H
#define V31_4H   V31.4H
#define V31_4S   V31.4S
#define V31_2S   V31.2S
#define V31_2D   V31.2D
#define V31_1D   V31.1D
#define V31_D    V31.D
#define V31_S    V31.S
#define V31_H    V31.H
#define V31_B    V31.B

#else
  #define A64_X   INT64 *      /*  64x1/32x2/16x4/8x8 bits */
  #define A64_S   FIXP_DBL     /*  32x1/16x2/8x4 bits */
  #define A64_H   FIXP_SGL     /*  16x1/8x2 bits */
  #define A64_B   SCHAR *       /* 8x1 bits */
  #define A64_V   A64_S*       /*  64x2/32x4/16x8 bits */
  #define A64_D   A64_S*       /*  64x1/32x2/16x4 bits */

  /* help pointer definitions */
  #define A64_SP  float *       /*    32 bit */
  #define A64_DP  double *      /*    64 bit */
  #define fQAbs32(a) (a == MINVAL_DBL) ? MAXVAL_DBL : fAbs(a);
  #define fQAbs16(a) (a == MINVAL_SGL) ? MAXVAL_SGL : fAbs(a);
  #define fQNeg32(a) (a == MINVAL_DBL) ? MAXVAL_DBL : -a;
  #define fQNeg16(a) (a == MINVAL_SGL) ? MAXVAL_SGL : -a;
  #define MAXVAL_INT64        ((INT64)0x7FFFFFFFFFFFFFFF)
  #define MINVAL_INT64        ((INT64)0x8000000000000000)

#define V0_16B  V0
#define V0_8B   V0
#define V0_8H   V0
#define V0_4H   V0
#define V0_4S   V0
#define V0_2S   V0
#define V0_2D   V0
#define V0_1D   V0
#define V0_D    V0
#define V0_S    V0
#define V0_H    V0
#define V0_B    V0

#define V1_16B  V1
#define V1_8B   V1
#define V1_8H   V1
#define V1_4H   V1
#define V1_4S   V1
#define V1_2S   V1
#define V1_2D   V1
#define V1_1D   V1
#define V1_D    V1
#define V1_S    V1
#define V1_H    V1
#define V1_B    V1

#define V2_16B  V2
#define V2_8B   V2
#define V2_8H   V2
#define V2_4H   V2
#define V2_4S   V2
#define V2_2S   V2
#define V2_2D   V2
#define V2_1D   V2
#define V2_D    V2
#define V2_S    V2
#define V2_H    V2
#define V2_B    V2

#define V3_16B  V3
#define V3_8B   V3
#define V3_8H   V3
#define V3_4H   V3
#define V3_4S   V3
#define V3_2S   V3
#define V3_2D   V3
#define V3_1D   V3
#define V3_D    V3
#define V3_S    V3
#define V3_H    V3
#define V3_B    V3

#define V4_16B  V4
#define V4_8B   V4
#define V4_8H   V4
#define V4_4H   V4
#define V4_4S   V4
#define V4_2S   V4
#define V4_2D   V4
#define V4_1D   V4
#define V4_D    V4
#define V4_S    V4
#define V4_H    V4
#define V4_B    V4

#define V5_16B  V5
#define V5_8B   V5
#define V5_8H   V5
#define V5_4H   V5
#define V5_4S   V5
#define V5_2S   V5
#define V5_2D   V5
#define V5_1D   V5
#define V5_D    V5
#define V5_S    V5
#define V5_H    V5
#define V5_B    V5

#define V6_16B  V6
#define V6_8B   V6
#define V6_8H   V6
#define V6_4H   V6
#define V6_4S   V6
#define V6_2S   V6
#define V6_2D   V6
#define V6_1D   V6
#define V6_D    V6
#define V6_S    V6
#define V6_H    V6
#define V6_B    V6

#define V7_16B  V7
#define V7_8B   V7
#define V7_8H   V7
#define V7_4H   V7
#define V7_4S   V7
#define V7_2S   V7
#define V7_2D   V7
#define V7_1D   V7
#define V7_D    V7
#define V7_S    V7
#define V7_H    V7
#define V7_B    V7

#define V8_16B  V8
#define V8_8B   V8
#define V8_8H   V8
#define V8_4H   V8
#define V8_4S   V8
#define V8_2S   V8
#define V8_2D   V8
#define V8_1D   V8
#define V8_D    V8
#define V8_S    V8
#define V8_H    V8
#define V8_B    V8

#define V9_16B  V9
#define V9_8B   V9
#define V9_8H   V9
#define V9_4H   V9
#define V9_4S   V9
#define V9_2S   V9
#define V9_2D   V9
#define V9_1D   V9
#define V9_D    V9
#define V9_S    V9
#define V9_H    V9
#define V9_B    V9

#define V10_16B  V10
#define V10_8B   V10
#define V10_8H   V10
#define V10_4H   V10
#define V10_4S   V10
#define V10_2S   V10
#define V10_2D   V10
#define V10_1D   V10
#define V10_D    V10
#define V10_S    V10
#define V10_H    V10
#define V10_B    V10

#define V11_16B  V11
#define V11_8B   V11
#define V11_8H   V11
#define V11_4H   V11
#define V11_4S   V11
#define V11_2S   V11
#define V11_2D   V11
#define V11_1D   V11
#define V11_D    V11
#define V11_S    V11
#define V11_H    V11
#define V11_B    V11

#define V12_16B  V12
#define V12_8B   V12
#define V12_8H   V12
#define V12_4H   V12
#define V12_4S   V12
#define V12_2S   V12
#define V12_2D   V12
#define V12_1D   V12
#define V12_D    V12
#define V12_S    V12
#define V12_H    V12
#define V12_B    V12

#define V13_16B  V13
#define V13_8B   V13
#define V13_8H   V13
#define V13_4H   V13
#define V13_4S   V13
#define V13_2S   V13
#define V13_2D   V13
#define V13_1D   V13
#define V13_D    V13
#define V13_S    V13
#define V13_H    V13
#define V13_B    V13

#define V14_16B  V14
#define V14_8B   V14
#define V14_8H   V14
#define V14_4H   V14
#define V14_4S   V14
#define V14_2S   V14
#define V14_2D   V14
#define V14_1D   V14
#define V14_D    V14
#define V14_S    V14
#define V14_H    V14
#define V14_B    V14

#define V15_16B  V15
#define V15_8B   V15
#define V15_8H   V15
#define V15_4H   V15
#define V15_4S   V15
#define V15_2S   V15
#define V15_2D   V15
#define V15_1D   V15
#define V15_D    V15
#define V15_S    V15
#define V15_H    V15
#define V15_B    V15

#define V16_16B  V16
#define V16_8B   V16
#define V16_8H   V16
#define V16_4H   V16
#define V16_4S   V16
#define V16_2S   V16
#define V16_2D   V16
#define V16_1D   V16
#define V16_D    V16
#define V16_S    V16
#define V16_H    V16
#define V16_B    V16

#define V17_16B  V17
#define V17_8B   V17
#define V17_8H   V17
#define V17_4H   V17
#define V17_4S   V17
#define V17_2S   V17
#define V17_2D   V17
#define V17_1D   V17
#define V17_D    V17
#define V17_S    V17
#define V17_H    V17
#define V17_B    V17

#define V18_16B  V18
#define V18_8B   V18
#define V18_8H   V18
#define V18_4H   V18
#define V18_4S   V18
#define V18_2S   V18
#define V18_2D   V18
#define V18_1D   V18
#define V18_D    V18
#define V18_S    V18
#define V18_H    V18
#define V18_B    V18

#define V19_16B  V19
#define V19_8B   V19
#define V19_8H   V19
#define V19_4H   V19
#define V19_4S   V19
#define V19_2S   V19
#define V19_2D   V19
#define V19_1D   V19
#define V19_D    V19
#define V19_S    V19
#define V19_H    V19
#define V19_B    V19

#define V20_16B  V20
#define V20_8B   V20
#define V20_8H   V20
#define V20_4H   V20
#define V20_4S   V20
#define V20_2S   V20
#define V20_2D   V20
#define V20_1D   V20
#define V20_D    V20
#define V20_S    V20
#define V20_H    V20
#define V20_B    V20

#define V21_16B  V21
#define V21_8B   V21
#define V21_8H   V21
#define V21_4H   V21
#define V21_4S   V21
#define V21_2S   V21
#define V21_2D   V21
#define V21_1D   V21
#define V21_D    V21
#define V21_S    V21
#define V21_H    V21
#define V21_B    V21

#define V22_16B  V22
#define V22_8B   V22
#define V22_8H   V22
#define V22_4H   V22
#define V22_4S   V22
#define V22_2S   V22
#define V22_2D   V22
#define V22_1D   V22
#define V22_D    V22
#define V22_S    V22
#define V22_H    V22
#define V22_B    V22

#define V23_16B  V23
#define V23_8B   V23
#define V23_8H   V23
#define V23_4H   V23
#define V23_4S   V23
#define V23_2S   V23
#define V23_2D   V23
#define V23_1D   V23
#define V23_D    V23
#define V23_S    V23
#define V23_H    V23
#define V23_B    V23

#define V24_16B  V24
#define V24_8B   V24
#define V24_8H   V24
#define V24_4H   V24
#define V24_4S   V24
#define V24_2S   V24
#define V24_2D   V24
#define V24_1D   V24
#define V24_D    V24
#define V24_S    V24
#define V24_H    V24
#define V24_B    V24

#define V25_16B  V25
#define V25_8B   V25
#define V25_8H   V25
#define V25_4H   V25
#define V25_4S   V25
#define V25_2S   V25
#define V25_2D   V25
#define V25_1D   V25
#define V25_D    V25
#define V25_S    V25
#define V25_H    V25
#define V25_B    V25

#define V26_16B  V26
#define V26_8B   V26
#define V26_8H   V26
#define V26_4H   V26
#define V26_4S   V26
#define V26_2S   V26
#define V26_2D   V26
#define V26_1D   V26
#define V26_D    V26
#define V26_S    V26
#define V26_H    V26
#define V26_B    V26

#define V27_16B  V27
#define V27_8B   V27
#define V27_8H   V27
#define V27_4H   V27
#define V27_4S   V27
#define V27_2S   V27
#define V27_2D   V27
#define V27_1D   V27
#define V27_D    V27
#define V27_S    V27
#define V27_H    V27
#define V27_B    V27

#define V28_16B  V28
#define V28_8B   V28
#define V28_8H   V28
#define V28_4H   V28
#define V28_4S   V28
#define V28_2S   V28
#define V28_2D   V28
#define V28_1D   V28
#define V28_D    V28
#define V28_S    V28
#define V28_H    V28
#define V28_B    V28

#define V29_16B  V29
#define V29_8B   V29
#define V29_8H   V29
#define V29_4H   V29
#define V29_4S   V29
#define V29_2S   V29
#define V29_2D   V29
#define V29_1D   V29
#define V29_D    V29
#define V29_S    V29
#define V29_H    V29
#define V29_B    V29

#define V30_16B  V30
#define V30_8B   V30
#define V30_8H   V30
#define V30_4H   V30
#define V30_4S   V30
#define V30_2S   V30
#define V30_2D   V30
#define V30_1D   V30
#define V30_D    V30
#define V30_S    V30
#define V30_H    V30
#define V30_B    V30

#define V31_16B  V31
#define V31_8B   V31
#define V31_8H   V31
#define V31_4H   V31
#define V31_4S   V31
#define V31_2S   V31
#define V31_2D   V31
#define V31_1D   V31
#define V31_D    V31
#define V31_S    V31
#define V31_H    V31
#define V31_B    V31

/* ARM core flag conditions, set by operations like CMP, SUBS, ADCS, etc.    */
/* Reference: Thumb 16-bit Instruction Set, Quick Refrence Card, www.arm.com */

int __FDK_coreflags_ = 1;       /* No condition, branch always          */
int __FDK_coreflags_AL = 1;     /* No condition, branch always          */
int __FDK_coreflags_EQ;         /* equal [to zero]                      */
int __FDK_coreflags_NE;         /* not equal [to zero]                  */
int __FDK_coreflags_CS;         /* carry set / unsigned higher or same  */
int __FDK_coreflags_CC;         /* carry cleared / unsigned lower       */
int __FDK_coreflags_MI;         /* negative                             */
int __FDK_coreflags_PL;         /* positive or zero                     */

//int __FDK_coreflags_VS;         /* overflow set                         */
//int __FDK_coreflags_VC;         /* overflow cleared                     */
//int __FDK_coreflags_HI;         /* unsigned higher                      */
//int __FDK_coreflags_LS;         /* unsigned lower or same               */

int __FDK_coreflags_GE;         /* signed greater than or equal         */
int __FDK_coreflags_LT;         /* signed less than                     */
int __FDK_coreflags_GT;         /* signed greater than                  */
int __FDK_coreflags_LE;         /* signed less than or equal            */


#endif


/*#################################################################################*/
/*
   Description: Load a single 16/32-bit lane of a D register from memory (src)
   Parameter size: 16 or 32
   Parameter dst:  Dx[0,1] 64-bit NEON register
   Parameter src:  r core register used as a pointer to 16/32-Bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_AARCH64_NEON__
/* ARMv8 GCC */
#define A64_pushD(src1, src2)                   " STP   " #src1 ", " #src2 ", [SP, #-16]!\n\t"
#define A64_pushp(src1, src2)                   " STP   " #src1 ", " #src2 ", [SP, #-16]!\n\t"
#define A64_popp(dst1, dst2)                    " LDP   " #dst1 ", " #dst2 ", [SP], #16\n\t"
#define A64_popD(dst1, dst2)                    " LDP   " #dst1 ", " #dst2 ", [SP], #16\n\t"

#define A64_ldr_Xt(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_Wt(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_Qt(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_Dt(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_St(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_Ht(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"
#define A64_ldr_Bt(  dst1,       src)           " LDR   " #dst1 ", [" #src "]\n\t"

#define A64_ldp_Xt(  dst1, dst2, src)           " LDP   " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldpsw_Xt(dst1, dst2, src)           " LDPSW " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldp_Wt(  dst1, dst2, src)           " LDP   " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldp_Qt(  dst1, dst2, src)           " LDP   " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldp_Dt(  dst1, dst2, src)           " LDP   " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldp_St(  dst1, dst2, src)           " LDP   " #dst1 ", " #dst2 ", [" #src "]\n\t"
#define A64_ldrb_Wt( dst1,       src)           " LDRB  " #dst1 ",            [" #src "]\n\t"
#define A64_ldrh_Wt( dst1,       src)           " LDRH  " #dst1 ",            [" #src "]\n\t"
#define A64_ldrsb_Xt(dst1,       src)           " LDRSB " #dst1 ",            [" #src "]\n\t"
#define A64_ldrsb_Wt(dst1,       src)           " LDRSB " #dst1 ",            [" #src "]\n\t"
#define A64_ldrsh_Xt(dst1,       src)           " LDRSH " #dst1 ",            [" #src "]\n\t"
#define A64_ldrsh_Wt(dst1,       src)           " LDRSH " #dst1 ",            [" #src "]\n\t"
#define A64_ldrsw_Xt(dst1,       src)           " LDRSH " #dst1 ",            [" #src "]\n\t"

#define A64_str_Xt(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_Wt(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_Qt(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_Dt(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_St(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_Ht(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_str_Bt(  src1,       dst)           " STR   " #src1 ",            [" #dst "]\n\t"
#define A64_stp_Xt(  src1, src2, dst)           " STP   " #src1 ", " #src2 ", [" #dst "]\n\t"
#define A64_stp_Wt(  src1, src2, dst)           " STP   " #src1 ", " #src2 ", [" #dst "]\n\t"
#define A64_stp_Qt(  src1, src2, dst)           " STP   " #src1 ", " #src2 ", [" #dst "]\n\t"
#define A64_stp_Dt(  src1, src2, dst)           " STP   " #src1 ", " #src2 ", [" #dst "]\n\t"
#define A64_stp_St(  src1, src2, dst)           " STP   " #src1 ", " #src2 ", [" #dst "]\n\t"
#define A64_strb_Wt( src1,       dst)           " STRB  " #src1 ",       [" #dst "]\n\t"
#define A64_strh_Wt( src1,       dst)           " STRH  " #src1 ",       [" #dst "]\n\t"

/* load/store from/to the address in src/dst with immediate offset, no pointer modification */
/* Example: LDR X0, [X1, #8]                                                                */
#define A64_ldr_Xt_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_Wt_I(  dst1, src, offset, name)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_Qt_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_Dt_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_St_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_Ht_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldr_Bt_I(  dst1,       src, offset)    " LDR  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldp_Xt_I(  dst1, dst2, src, offset, name1, name2)   " LDP   " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldpsw_Xt_I(dst1, dst2, src, offset)   " LDPSW " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldp_Wt_I(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldp_Qt_I(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldp_Dt_I(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldp_St_I(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrb_Wt_I( dst1,       src, offset)   " LDRB  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrh_Wt_I( dst1,       src, offset)   " LDRH  " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrsb_Xt_I(dst1,       src, offset)   " LDRSB " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrsb_Wt_I(dst1,       src, offset)   " LDRSB " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrsh_Xt_I(dst1,       src, offset)   " LDRSH " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrsh_Wt_I(dst1,       src, offset)   " LDRSH " #dst1 ", [" #src ", # " #offset "]\n\t"
#define A64_ldrsw_Xt_I(dst1,       src, offset)   " LDRSW " #dst1 ", [" #src ", # " #offset "]\n\t"

#define A64_str_Xt_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_Wt_I(  src1, dst, offset, name)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_Qt_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_Dt_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_St_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_Ht_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_str_Bt_I(  src1,       dst, offset)   " STR  " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_stp_Xt_I(  src1, src2, dst, offset)   " STP  " #src1 ", " #src2 ", [" #dst ", # " #offset "]\n\t"
#define A64_stp_Wt_I(  src1, src2, dst, offset)   " STP  " #src1 ", " #src2 ", [" #dst ", # " #offset "]\n\t"
#define A64_stp_Qt_I(  src1, src2, dst, offset)   " STP  " #src1 ", " #src2 ", [" #dst ", # " #offset "]\n\t"
#define A64_stp_Dt_I(  src1, src2, dst, offset)   " STP  " #src1 ", " #src2 ", [" #dst ", # " #offset "]\n\t"
#define A64_stp_St_I(  src1, src2, dst, offset)   " STP  " #src1 ", " #src2 ", [" #dst ", # " #offset "]\n\t"
#define A64_strb_Wt_I( src1,       dst, offset)   " STRB " #src1 ",            [" #dst ", # " #offset "]\n\t"
#define A64_strh_Wt_I( src1,       dst, offset)   " STRH " #src1 ",            [" #dst ", # " #offset "]\n\t"

/* load/store from/to the address in src/dst with register offset, no pointer modification */
/* Example: LDR X0, [X1, X2]                                                               */
#define A64_ldr_Xt_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_Wt_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_Qt_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_Dt_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_St_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_Ht_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldr_Bt_X(  dst1,       src, offset)   " LDR   " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldp_Xt_X(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldpsw_Xt_X(dst1, dst2, src, offset)   " LDPSW " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldp_Wt_X(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldp_Qt_X(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldp_Dt_X(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldp_St_X(  dst1, dst2, src, offset)   " LDP   " #dst1 ", " #dst2 ", [" #src ", " #offset "]\n\t"
#define A64_ldrb_Wt_X( dst1,       src, offset)   " LDRB  " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrh_Wt_X( dst1,       src, offset)   " LDRH  " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrsb_Xt_X(dst1,       src, offset)   " LDRSB " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrsb_Wt_X(dst1,       src, offset)   " LDRSB " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrsh_Xt_X(dst1,       src, offset)   " LDRSH " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrsh_Wt_X(dst1,       src, offset)   " LDRSH " #dst1 ",            [" #src ", " #offset "]\n\t"
#define A64_ldrsw_Xt_X(dst1,       src, offset)   " LDRSW " #dst1 ",            [" #src ", " #offset "]\n\t"

#define A64_str_Xt_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_Wt_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_Qt_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_Dt_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_St_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_Ht_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_str_Bt_X(  src1,       dst, offset)   " STR   " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_stp_Xt_X(  src1, src2, dst, offset)   " STP   " #src1 ", " #src2 ", [" #dst ", " #offset "]\n\t"
#define A64_stp_Wt_X(  src1, src2, dst, offset)   " STP   " #src1 ", " #src2 ", [" #dst ", " #offset "]\n\t"
#define A64_stp_Qt_X(  src1, src2, dst, offset)   " STP   " #src1 ", " #src2 ", [" #dst ", " #offset "]\n\t"
#define A64_stp_Dt_X(  src1, src2, dst, offset)   " STP   " #src1 ", " #src2 ", [" #dst ", " #offset "]\n\t"
#define A64_stp_St_X(  src1, src2, dst, offset)   " STP   " #src1 ", " #src2 ", [" #dst ", " #offset "]\n\t"
#define A64_strb_Wt_X( src1,       dst, offset)   " STRB  " #src1 ",            [" #dst ", " #offset "]\n\t"
#define A64_strh_Wt_X( src1,       dst, offset)   " STRH  " #src1 ",            [" #dst ", " #offset "]\n\t"

/* load/store from/to the address in src/dst with left-shifted register offset, no pointer modification */
/* Example: LDR X0, [X1, X2, LSL, #3]                                                                        */
#define A64_ldr_Xt_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_Wt_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_Qt_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_Dt_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_St_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_Ht_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldr_Bt_X_LSL(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldp_Xt_X_LSL(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldpsw_Xt_X_LSL(dst1, dst2, src, offset, lsl)  "LDPSW " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldp_Wt_X_LSL(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldp_Qt_X_LSL(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldp_Dt_X_LSL(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldp_St_X_LSL(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrb_Wt_X_LSL( dst1,       src, offset, lsl)  "LDRB  " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrh_Wt_X_LSL( dst1,       src, offset, lsl)  "LDRH  " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrsb_Xt_X_LSL(dst1,       src, offset, lsl)  "LDRSB " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrsb_Wt_X_LSL(dst1,       src, offset, lsl)  "LDRSB " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrsh_Xt_X_LSL(dst1,       src, offset, lsl)  "LDRSH " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrsh_Wt_X_LSL(dst1,       src, offset, lsl)  "LDRSH " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"
#define A64_ldrsw_Xt_X_LSL(dst1,       src, offset, lsl)  "LDRSW " #dst1 ",           [" #src ", " #offset ", LSL # " #lsl"]\n\t"

#define A64_ldr_Xt_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_Wt_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_Qt_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_Dt_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_St_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_Ht_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldr_Bt_X_LSR(  dst1,       src, offset, lsl)  "LDR   " #dst1 ",           [" #src ", " #offset ", LSR " #lsl"]\n\t"
#define A64_ldp_Xt_X_LSR(  dst1, dst2, src, offset, lsl)  "LDP   " #dst1 ", " #dst2 ",[" #src ", " #offset ", LSR " #lsl"]\n\t"

#define A64_str_Xt_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_Wt_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_Qt_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_Dt_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_St_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_Ht_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_str_Bt_X_LSL(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_stp_Xt_X_LSL(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_stp_Wt_X_LSL(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_stp_Qt_X_LSL(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_stp_Dt_X_LSL(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_stp_St_X_LSL(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_strb_Wt_X_LSL( src1,       dst, offset, lsl)  "STRB " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"
#define A64_strh_Wt_X_LSL( src1,       dst, offset, lsl)  "STRH " #src1 ",           [" #dst ", " #offset ", LSL " #lsl "]\n\t"


#define A64_str_Xt_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_Wt_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_Qt_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_Dt_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_St_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_Ht_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_str_Bt_X_LSR(  src1,       dst, offset, lsl)  "STR  " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_stp_Xt_X_LSR(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_stp_Wt_X_LSR(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_stp_Qt_X_LSR(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_stp_Dt_X_LSR(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_stp_St_X_LSR(  src1, src2, dst, offset, lsl)  "STP  " #src1 ", " #src2 ",[" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_strb_Wt_X_LSR( src1,       dst, offset, lsl)  "STRB " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"
#define A64_strh_Wt_X_LSR( src1,       dst, offset, lsl)  "STRH " #src1 ",           [" #dst ", " #offset ", LSR # " #lsl "]\n\t"

/* load/store from/to the address in src/dst with signed-extended word-register offset, no pointer modification */
/* Example: LDR X0, [X1, W2, SXTW]                                                                        */
#define A64_ldr_Xt_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_Wt_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_Qt_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_Dt_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_St_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_Ht_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldr_Bt_X_SXTW(  dst1,       src, sxtw)   LDR   dst1,       [src, sxtw, SXTW]
#define A64_ldp_Xt_X_SXTW(  dst1, dst2, src, sxtw)   LDP   dst1, dst2, [src, sxtw, SXTW]
#define A64_ldpsw_Xt_X_SXTW(dst1, dst2, src, sxtw)   LDPSW dst1, dst2, [src, sxtw, SXTW]
#define A64_ldp_Wt_X_SXTW(  dst1, dst2, src, sxtw)   LDP   dst1, dst2, [src, sxtw, SXTW]
#define A64_ldp_Qt_X_SXTW(  dst1, dst2, src, sxtw)   LDP   dst1, dst2, [src, sxtw, SXTW]
#define A64_ldp_Dt_X_SXTW(  dst1, dst2, src, sxtw)   LDP   dst1, dst2, [src, sxtw, SXTW]
#define A64_ldp_St_X_SXTW(  dst1, dst2, src, sxtw)   LDP   dst1, dst2, [src, sxtw, SXTW]
#define A64_ldrb_Wt_X_SXTW( dst1,       src, sxtw)   LDRB  dst1,       [src, sxtw, SXTW]
#define A64_ldrh_Wt_X_SXTW( dst1,       src, sxtw)   LDRH  dst1,       [src, sxtw, SXTW]
#define A64_ldrsb_Xt_X_SXTW(dst1,       src, sxtw)   LDRSB dst1,       [src, sxtw, SXTW]
#define A64_ldrsb_Wt_X_SXTW(dst1,       src, sxtw)   LDRSB dst1,       [src, sxtw, SXTW]
#define A64_ldrsh_Xt_X_SXTW(dst1,       src, sxtw)   LDRSH dst1,       [src, sxtw, SXTW]
#define A64_ldrsh_Wt_X_SXTW(dst1,       src, sxtw)   LDRSH dst1,       [src, sxtw, SXTW]
#define A64_ldrsw_Xt_X_SXTW(dst1,       src, sxtw)   LDRSW dst1,       [src, sxtw, SXTW]

#define A64_str_Xt_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_Wt_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_Qt_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_Dt_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_St_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_Ht_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_str_Bt_X_SXTW(  src1,       dst, sxtw)   STR   src1,       [dst, sxtw, SXTW]
#define A64_stp_Xt_X_SXTW(  src1, src2, dst, sxtw)   STP   src1, src2, [dst, sxtw, SXTW]
#define A64_stp_Wt_X_SXTW(  src1, src2, dst, sxtw)   STP   src1, src2, [dst, sxtw, SXTW]
#define A64_stp_Qt_X_SXTW(  src1, src2, dst, sxtw)   STP   src1, src2, [dst, sxtw, SXTW]
#define A64_stp_Dt_X_SXTW(  src1, src2, dst, sxtw)   STP   src1, src2, [dst, sxtw, SXTW]
#define A64_stp_St_X_SXTW(  src1, src2, dst, sxtw)   STP   src1, src2, [dst, sxtw, SXTW]
#define A64_strb_Wt_X_SXTW( src1,       dst, sxtw)   STRB  src1,       [dst, sxtw, SXTW]
#define A64_strh_Wt_X_SXTW( src1,       dst, sxtw)   STRH  src1,       [dst, sxtw, SXTW]

/* load/store from/to the address in src/dst with signed-extended+left-shifted word-register offset, no pointer modification */
/* Example: LDR X0, [X1, W2, SXTW, #3]                                                                        */
#define A64_ldr_Xt_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_Wt_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_Qt_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_Dt_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_St_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_Ht_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldr_Bt_X_SXTW_LSL(  dst1,       src, sxtw, lsl)   LDR   dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldp_Xt_X_SXTW_LSL(  dst1, dst2, src, sxtw, lsl)   LDP   dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldpsw_Xt_X_SXTW_LSL(dst1, dst2, src, sxtw, lsl)   LDPSW dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldp_Wt_X_SXTW_LSL(  dst1, dst2, src, sxtw, lsl)   LDP   dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldp_Qt_X_SXTW_LSL(  dst1, dst2, src, sxtw, lsl)   LDP   dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldp_Dt_X_SXTW_LSL(  dst1, dst2, src, sxtw, lsl)   LDP   dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldp_St_X_SXTW_LSL(  dst1, dst2, src, sxtw, lsl)   LDP   dst1, dst2, [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrb_Wt_X_SXTW_LSL( dst1,       src, sxtw, lsl)   LDRB  dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrh_Wt_X_SXTW_LSL( dst1,       src, sxtw, lsl)   LDRH  dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrsb_Xt_X_SXTW_LSL(dst1,       src, sxtw, lsl)   LDRSB dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrsb_Wt_X_SXTW_LSL(dst1,       src, sxtw, lsl)   LDRSB dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrsh_Xt_X_SXTW_LSL(dst1,       src, sxtw, lsl)   LDRSH dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrsh_Wt_X_SXTW_LSL(dst1,       src, sxtw, lsl)   LDRSH dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_ldrsw_Xt_X_SXTW_LSL(dst1,       src, sxtw, lsl)   LDRSW dst1,       [src, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]

#define A64_str_Xt_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_Wt_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_Qt_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_Dt_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_St_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_Ht_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_str_Bt_X_SXTW_LSL(  src1,       dst, sxtw, lsl)   STR   src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_stp_Xt_X_SXTW_LSL(  src1, src2, dst, sxtw, lsl)   STP   src1, src2, [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_stp_Wt_X_SXTW_LSL(  src1, src2, dst, sxtw, lsl)   STP   src1, src2, [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_stp_Qt_X_SXTW_LSL(  src1, src2, dst, sxtw, lsl)   STP   src1, src2, [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_stp_Dt_X_SXTW_LSL(  src1, src2, dst, sxtw, lsl)   STP   src1, src2, [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_stp_St_X_SXTW_LSL(  src1, src2, dst, sxtw, lsl)   STP   src1, src2, [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_strb_Wt_X_SXTW_LSL( src1,       dst, sxtw, lsl)   STRB  src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]
#define A64_strh_Wt_X_SXTW_LSL( src1,       dst, sxtw, lsl)   STRH  src1,       [dst, sxtw, SXTW, FDK_TEXT(HASHSIGN) lsl]

/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter 6.3.4  Specify the address for a Load or Store instruction, table 6-9 Index addressing modes */
/* Pre-index: Update first source/destination pointer with offset, then load/store from/to new address */
/* Example: LDR X0, [X1, #8]! */
#define A64_ldr_Xt_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldr_Wt_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldr_Qt_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldr_St_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldr_Ht_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldr_Bt_IB(  dst1,       src, offset)   LDR   dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldp_Xt_IB(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldpsw_Xt_IB(dst1, dst2, src, offset)   LDPSW dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldp_Wt_IB(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldp_Qt_IB(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldp_Dt_IB(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldp_St_IB(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrb_Wt_IB( dst1,       src, offset)   LDRB  dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrh_Wt_IB( dst1,       src, offset)   LDRH  dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrsb_Xt_IB(dst1,       src, offset)   LDRSB dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrsb_Wt_IB(dst1,       src, offset)   LDRSB dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrsh_Xt_IB(dst1,       src, offset)   LDRSH dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrsh_Wt_IB(dst1,       src, offset)   LDRSH dst1,       [src, FDK_TEXT(HASHSIGN) offset]!
#define A64_ldrsw_Xt_IB(dst1,       src, offset)   LDRSW dst1,       [src, FDK_TEXT(HASHSIGN) offset]!

#define A64_str_Xt_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_Wt_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_Qt_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_Dt_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_St_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_Ht_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_str_Bt_IB(  src1,       dst, offset)   STR   src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_stp_Xt_IB(  src1, src2, dst, offset)   STP   src1, src2, [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_stp_Wt_IB(  src1, src2, dst, offset)   STP   src1, src2, [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_stp_Qt_IB(  src1, src2, dst, offset)   STP   src1, src2, [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_stp_Dt_IB(  src1, src2, dst, offset)   STP   src1, src2, [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_stp_St_IB(  src1, src2, dst, offset)   STP   src1, src2, [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_strb_Wt_IB( src1,       dst, offset)   STRB  src1,       [dst, FDK_TEXT(HASHSIGN) offset]!
#define A64_strh_Wt_IB( src1,       dst, offset)   STRH  src1,       [dst, FDK_TEXT(HASHSIGN) offset]!

/* Post-index: Load/store first from/to source/destination pointer, then update pointer with immediate offset */
/* Example: LDR X0, [X1] #8 */
#define A64_ldr_Xt_IA(  dst1,       src, offset)   LDR   dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldr_Wt_IA(dst1,src,offset)            "LDR   " #dst1 ",       [" #src "], # " #offset "\n\t"
#define A64_ldr_Qt_IA(  dst1,       src, offset)   LDR   dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldr_St_IA(  dst1,       src, offset)   LDR   dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldr_Ht_IA(  dst1,       src, offset)   LDR   dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldr_Bt_IA(  dst1,       src, offset)   LDR   dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldp_Xt_IA(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldpsw_Xt_IA(dst1, dst2, src, offset)   LDPSW dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldp_Wt_IA(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldp_Qt_IA(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldp_Dt_IA(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldp_St_IA(  dst1, dst2, src, offset)   LDP   dst1, dst2, [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrb_Wt_IA( dst1,       src, offset)   LDRB  dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrh_Wt_IA( dst1,       src, offset)   LDRH  dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrsb_Xt_IA(dst1,       src, offset)   LDRSB dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrsb_Wt_IA(dst1,       src, offset)   LDRSB dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrsh_Xt_IA(dst1,       src, offset)   LDRSH dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrsh_Wt_IA(dst1,       src, offset)   LDRSH dst1,       [src], FDK_TEXT(HASHSIGN) offset
#define A64_ldrsw_Xt_IA(dst1,       src, offset)   LDRSW dst1,       [src], FDK_TEXT(HASHSIGN) offset


#define A64_str_Xt_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_Wt_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_Qt_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_Dt_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_St_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_Ht_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_str_Bt_IA(  src1,       dst, offset)  " STR " #src1 ",            [" #dst "], # " #offset "\n\t"
#define A64_stp_Xt_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_Wt_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_Ht_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_Bt_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_Qt_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_Dt_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_stp_St_IA(  src1, src2, dst, offset)  " STP " #src1 ", " #src2 ", [" #dst "], # " #offset "\n\t"
#define A64_strb_Wt_IA( src1,       dst, offset)  " STRB " #src1 ",           [" #dst "], # " #offset "\n\t"
#define A64_strh_Wt_IA( src1,       dst, offset)  " STRH " #src1 ",           [" #dst "], # " #offset "\n\t"

#define __A64_ld1_lane(          dst1,                   lane, src)      " LD1 { " #dst1 "                                  } [" #lane "], [" #src "]\n\t"
#define __A64_ld2_lane(          dst1, dst2,             lane, src)      " LD2 { " #dst1 ", " #dst2 "                       } [" #lane "], [" #src "]\n\t"
#define __A64_ld3_lane(          dst1, dst2, dst3,       lane, src)      " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            } [" #lane "], [" #src "]\n\t"
#define __A64_ld4_lane(          dst1, dst2, dst3, dst4, lane, src)      " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } [" #lane "], [" #src "]\n\t"
#define   A64_ld1_lane(   width, dst1,                   lane, src)      __A64_ld1_lane(dst1,                   lane, src)
#define   A64_ld2_lane(   width, dst1, dst2,             lane, src)      __A64_ld2_lane(dst1, dst2,             lane, src)
#define   A64_ld3_lane(   width, dst1, dst2, dst3,       lane, src)      __A64_ld3_lane(dst1, dst2, dst3,       lane, src)
#define   A64_ld4_lane(   width, dst1, dst2, dst3, dst4, lane, src)      __A64_ld4_lane(dst1, dst2, dst3, dst4, lane, src)

#define __A64_ld1_lane_IA(       dst1,                   lane, src, imm) " LD1 { " #dst1 "                                  } [" #lane "], [" #src "], # " #imm "\n\t"
#define __A64_ld2_lane_IA(       dst1, dst2,             lane, src, imm) " LD2 { " #dst1 ", " #dst2 "                       } [" #lane "], [" #src "], # " #imm "\n\t"
#define __A64_ld3_lane_IA(       dst1, dst2, dst3,       lane, src, imm) " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            } [" #lane "], [" #src "], # " #imm "\n\t"
#define __A64_ld4_lane_IA(       dst1, dst2, dst3, dst4, lane, src, imm) " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } [" #lane "], [" #src "], # " #imm "\n\t"
#define   A64_ld1_lane_IA(width, dst1,                   lane, src, imm) __A64_ld1_lane_IA(dst1,                   lane, src, imm)
#define   A64_ld2_lane_IA(width, dst1, dst2,             lane, src, imm) __A64_ld2_lane_IA(dst1, dst2,             lane, src, imm)
#define   A64_ld3_lane_IA(width, dst1, dst2, dst3,       lane, src, imm) __A64_ld3_lane_IA(dst1, dst2, dst3,       lane, src, imm)
#define   A64_ld4_lane_IA(width, dst1, dst2, dst3, dst4, lane, src, imm) __A64_ld4_lane_IA(dst1, dst2, dst3, dst4, lane, src, imm)

#define __A64_ld1_lane_PU(       dst1,                   lane, src, upd) " LD1 { " #dst1 "                                  } [" #lane "], [" #src "], " #upd "\n\t"
#define __A64_ld2_lane_PU(       dst1, dst2,             lane, src, upd) " LD2 { " #dst1 ", " #dst2 "                       } [" #lane "], [" #src "], " #upd "\n\t"
#define __A64_ld3_lane_PU(       dst1, dst2, dst3,       lane, src, upd) " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            } [" #lane "], [" #src "], " #upd "\n\t"
#define __A64_ld4_lane_PU(       dst1, dst2, dst3, dst4, lane, src, upd) " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } [" #lane "], [" #src "], " #upd "\n\t"
#define   A64_ld1_lane_PU(width, dst1,                   lane, src, upd) __A64_ld1_lane_PU(dst1,                   lane, src, upd)
#define   A64_ld2_lane_PU(width, dst1, dst2,             lane, src, upd) __A64_ld2_lane_PU(dst1, dst2,             lane, src, upd)
#define   A64_ld3_lane_PU(width, dst1, dst2, dst3,       lane, src, upd) __A64_ld3_lane_PU(dst1, dst2, dst3,       lane, src, upd)
#define   A64_ld4_lane_PU(width, dst1, dst2, dst3, dst4, lane, src, upd) __A64_ld4_lane_PU(dst1, dst2, dst3, dst4, lane, src, upd)

#define __A64_ld1x1(dst1,                   src)                         " LD1 { " #dst1 "                                  }, [" #src "]\n\t"
#define __A64_ld1x2(dst1, dst2,             src)                         " LD1 { " #dst1 ", " #dst2 "                       }, [" #src "]\n\t"
#define __A64_ld1x3(dst1, dst2, dst3,       src)                         " LD1 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "]\n\t"
#define __A64_ld1x4(dst1, dst2, dst3, dst4, src)                         " LD1 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "]\n\t"
#define __A64_ld2x2(dst1, dst2,             src)                         " LD2 { " #dst1 ", " #dst2 "                       }, [" #src "]\n\t"
#define __A64_ld3x3(dst1, dst2, dst3,       src)                         " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "]\n\t"
#define __A64_ld4x4(dst1, dst2, dst3, dst4, src)                         " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "]\n\t"
#define   A64_ld1x1(width, size, dst1,                   src)            __A64_ld1x1(dst1,                   src)
#define   A64_ld1x2(width, size, dst1, dst2,             src)            __A64_ld1x2(dst1, dst2,             src)
#define   A64_ld1x3(width, size, dst1, dst2, dst3,       src)            __A64_ld1x3(dst1, dst2, dst3,       src)
#define   A64_ld1x4(width, size, dst1, dst2, dst3, dst4, src)            __A64_ld1x4(dst1, dst2, dst3, dst4, src)
#define   A64_ld2x2(width, size, dst1, dst2,             src)            __A64_ld2x2(dst1, dst2,             src)
#define   A64_ld3x3(width, size, dst1, dst2, dst3,       src)            __A64_ld3x3(dst1, dst2, dst3,       src)
#define   A64_ld4x4(width, size, dst1, dst2, dst3, dst4, src)            __A64_ld4x4(dst1, dst2, dst3, dst4, src)

#define __A64_ld1x1_IA(dst1,                   src, imm)                 " LD1 { " #dst1 "                                  }, [" #src "], # " #imm "\n\t"
#define __A64_ld1x2_IA(dst1, dst2,             src, imm)                 " LD1 { " #dst1 ", " #dst2 "                       }, [" #src "], # " #imm "\n\t"
#define __A64_ld1x3_IA(dst1, dst2, dst3,       src, imm)                 " LD1 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], # " #imm "\n\t"
#define __A64_ld1x4_IA(dst1, dst2, dst3, dst4, src, imm)                 " LD1 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], # " #imm "\n\t"
#define __A64_ld2x2_IA(dst1, dst2,             src, imm)                 " LD2 { " #dst1 ", " #dst2 "                       }, [" #src "], # " #imm "\n\t"
#define __A64_ld3x3_IA(dst1, dst2, dst3,       src, imm)                 " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], # " #imm "\n\t"
#define __A64_ld4x4_IA(dst1, dst2, dst3, dst4, src, imm)                 " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], # " #imm "\n\t"
#define   A64_ld1x1_IA(width, size, dst1,                   src, imm)    __A64_ld1x1_IA(dst1,                   src, imm)
#define   A64_ld1x2_IA(width, size, dst1, dst2,             src, imm)    __A64_ld1x2_IA(dst1, dst2,             src, imm)
#define   A64_ld1x3_IA(width, size, dst1, dst2, dst3,       src, imm)    __A64_ld1x3_IA(dst1, dst2, dst3,       src, imm)
#define   A64_ld1x4_IA(width, size, dst1, dst2, dst3, dst4, src, imm)    __A64_ld1x4_IA(dst1, dst2, dst3, dst4, src, imm)
#define   A64_ld2x2_IA(width, size, dst1, dst2,             src, imm)    __A64_ld2x2_IA(dst1, dst2,             src, imm)
#define   A64_ld3x3_IA(width, size, dst1, dst2, dst3,       src, imm)    __A64_ld3x3_IA(dst1, dst2, dst3,       src, imm)
#define   A64_ld4x4_IA(width, size, dst1, dst2, dst3, dst4, src, imm)    __A64_ld4x4_IA(dst1, dst2, dst3, dst4, src, imm)


#define __A64_ld1x1_PU(dst1,                   src, upd)                 " LD1 { " #dst1 "                                  }, [" #src "], " #upd "\n\t"
#define __A64_ld1x2_PU(dst1, dst2,             src, upd)                 " LD1 { " #dst1 ", " #dst2 "                       }, [" #src "], " #upd "\n\t"
#define __A64_ld1x3_PU(dst1, dst2, dst3,       src, upd)                 " LD1 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], " #upd "\n\t"
#define __A64_ld1x4_PU(dst1, dst2, dst3, dst4, src, upd)                 " LD1 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], " #upd "\n\t"
#define __A64_ld2x2_PU(dst1, dst2,             src, upd)                 " LD2 { " #dst1 ", " #dst2 "                       }, [" #src "], " #upd "\n\t"
#define __A64_ld3x3_PU(dst1, dst2, dst3,       src, upd)                 " LD3 { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], " #upd "\n\t"
#define __A64_ld4x4_PU(dst1, dst2, dst3, dst4, src, upd)                 " LD4 { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], " #upd "\n\t"
#define   A64_ld1x1_PU(width, size, dst1,                   src, upd)    __A64_ld1x1_PU(dst1,                   src, upd)
#define   A64_ld1x2_PU(width, size, dst1, dst2,             src, upd)    __A64_ld1x2_PU(dst1, dst2,             src, upd)
#define   A64_ld1x3_PU(width, size, dst1, dst2, dst3,       src, upd)    __A64_ld1x3_PU(dst1, dst2, dst3,       src, upd)
#define   A64_ld1x4_PU(width, size, dst1, dst2, dst3, dst4, src, upd)    __A64_ld1x4_PU(dst1, dst2, dst3, dst4, src, upd)
#define   A64_ld2x2_PU(width, size, dst1, dst2,             src, upd)    __A64_ld2x2_PU(dst1, dst2,             src, upd)
#define   A64_ld3x3_PU(width, size, dst1, dst2, dst3,       src, upd)    __A64_ld3x3_PU(dst1, dst2, dst3,       src, upd)
#define   A64_ld4x4_PU(width, size, dst1, dst2, dst3, dst4, src, upd)    __A64_ld4x4_PU(dst1, dst2, dst3, dst4, src, upd)

#define __A64_ld1rx1(dst1,                   src)                         " LD1R { " #dst1 "                                  }, [" #src "]\n\t"
#define __A64_ld1rx2(dst1, dst2,             src)                         " LD1R { " #dst1 ", " #dst2 "                       }, [" #src "]\n\t"
#define __A64_ld1rx3(dst1, dst2, dst3,       src)                         " LD1R { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "]\n\t"
#define __A64_ld1rx4(dst1, dst2, dst3, dst4, src)                         " LD1R { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "]\n\t"
#define   A64_ld1rx1(width, size, dst1,                   src)            __A64_ld1rx1(dst1,                   src)
#define   A64_ld1rx2(width, size, dst1, dst2,             src)            __A64_ld1rx2(dst1, dst2,             src)
#define   A64_ld1rx3(width, size, dst1, dst2, dst3,       src)            __A64_ld1rx3(dst1, dst2, dst3,       src)
#define   A64_ld1rx4(width, size, dst1, dst2, dst3, dst4, src)            __A64_ld1rx4(dst1, dst2, dst3, dst4, src)

#define __A64_ld1rx1_IA(dst1,                   src, imm)                 " LD1R { " #dst1 "                                  }, [" #src "], # " #imm "\n\t"
#define __A64_ld1rx2_IA(dst1, dst2,             src, imm)                 " LD1R { " #dst1 ", " #dst2 "                       }, [" #src "], # " #imm "\n\t"
#define __A64_ld1rx3_IA(dst1, dst2, dst3,       src, imm)                 " LD1R { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], # " #imm "\n\t"
#define __A64_ld1rx4_IA(dst1, dst2, dst3, dst4, src, imm)                 " LD1R { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], # " #imm "\n\t"
#define   A64_ld1rx1_IA(width, size, dst1,                   src, imm)    __A64_ld1rx1_IA(dst1,                   src, imm)
#define   A64_ld1rx2_IA(width, size, dst1, dst2,             src, imm)    __A64_ld1rx2_IA(dst1, dst2,             src, imm)
#define   A64_ld1rx3_IA(width, size, dst1, dst2, dst3,       src, imm)    __A64_ld1rx3_IA(dst1, dst2, dst3,       src, imm)
#define   A64_ld1rx4_IA(width, size, dst1, dst2, dst3, dst4, src, imm)    __A64_ld1rx4_IA(dst1, dst2, dst3, dst4, src, imm)


#define __A64_ld1rx1_PU(dst1,                   src, upd)                " LD1R { " #dst1 "                                  }, [" #src "], " #upd "\n\t"
#define __A64_ld1rx2_PU(dst1, dst2,             src, upd)                " LD1R { " #dst1 ", " #dst2 "                       }, [" #src "], " #upd "\n\t"
#define __A64_ld1rx3_PU(dst1, dst2, dst3,       src, upd)                " LD1R { " #dst1 ", " #dst2 ", " #dst3 "            }, [" #src "], " #upd "\n\t"
#define __A64_ld1rx4_PU(dst1, dst2, dst3, dst4, src, upd)                " LD1R { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " }, [" #src "], " #upd "\n\t"
#define   A64_ld1rx1_PU(width, size, dst1,                   src, upd)    __A64_ld1rx1_PU(dst1,                   src, upd)
#define   A64_ld1rx2_PU(width, size, dst1, dst2,             src, upd)    __A64_ld1rx2_PU(dst1, dst2,             src, upd)
#define   A64_ld1rx3_PU(width, size, dst1, dst2, dst3,       src, upd)    __A64_ld1rx3_PU(dst1, dst2, dst3,       src, upd)
#define   A64_ld1rx4_PU(width, size, dst1, dst2, dst3, dst4, src, upd)    __A64_ld1rx4_PU(dst1, dst2, dst3, dst4, src, upd)


#define __A64_st1_lane(src1,                   lane, dst)              " ST1 { " #src1 "                                  } [" #lane "], [" #dst "]\n\t"
#define __A64_st2_lane(src1, src2,             lane, dst)              " ST2 { " #src1 ", " #src2 "                       } [" #lane "], [" #dst "]\n\t"
#define __A64_st3_lane(src1, src2, src3,       lane, dst)              " ST3 { " #src1 ", " #src2 ", " #src3 "            } [" #lane "], [" #dst "]\n\t"
#define __A64_st4_lane(src1, src2, src3, src4, lane, dst)              " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " } [" #lane "], [" #dst "]\n\t"
#define A64_st1_lane(   width, src1,                   lane, dst)       __A64_st1_lane(src1,                   lane, dst)
#define A64_st2_lane(   width, src1, src2,             lane, dst)       __A64_st2_lane(src1, src2,             lane, dst)
#define A64_st3_lane(   width, src1, src2, src3,       lane, dst)       __A64_st3_lane(src1, src2, src3,       lane, dst)
#define A64_st4_lane(   width, src1, src2, src3, src4, lane, dst)       __A64_st4_lane(src1, src2, src3, src4, lane, dst)

#define __A64_st1_lane_IA(src1,                   lane, dst, imm)      " ST1 { " #src1 "                                  } [" #lane "], [" #dst "], # " #imm "\n\t"
#define __A64_st2_lane_IA(src1, src2,             lane, dst, imm)      " ST2 { " #src1 ", " #src2 "                       } [" #lane "], [" #dst "], # " #imm "\n\t"
#define __A64_st3_lane_IA(src1, src2, src3,       lane, dst, imm)      " ST3 { " #src1 ", " #src2 ", " #src3 "            } [" #lane "], [" #dst "], # " #imm "\n\t"
#define __A64_st4_lane_IA(src1, src2, src3, src4, lane, dst, imm)      " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " } [" #lane "], [" #dst "], # " #imm "\n\t"
#define A64_st1_lane_IA(width, src1,                   lane, dst, imm)  __A64_st1_lane_IA(src1,                   lane, dst, imm)
#define A64_st2_lane_IA(width, src1, src2,             lane, dst, imm)  __A64_st2_lane_IA(src1, src2,             lane, dst, imm)
#define A64_st3_lane_IA(width, src1, src2, src3,       lane, dst, imm)  __A64_st3_lane_IA(src1, src2, src3,       lane, dst, imm)
#define A64_st4_lane_IA(width, src1, src2, src3, src4, lane, dst, imm)  __A64_st4_lane_IA(src1, src2, src3, src4, lane, dst, imm)

#define __A64_st1_lane_PU(src1,                   lane, dst, upd)      " ST1 { " #src1 "                                  } [" #lane "], [" #dst "], " #upd "\n\t"
#define __A64_st2_lane_PU(src1, src2,             lane, dst, upd)      " ST2 { " #src1 ", " #src2 "                       } [" #lane "], [" #dst "], " #upd "\n\t"
#define __A64_st3_lane_PU(src1, src2, src3,       lane, dst, upd)      " ST3 { " #src1 ", " #src2 ", " #src3 "            } [" #lane "], [" #dst "], " #upd "\n\t"
#define __A64_st4_lane_PU(src1, src2, src3, src4, lane, dst, upd)      " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " } [" #lane "], [" #dst "], " #upd "\n\t"
#define A64_st1_lane_PU(width, src1,                   lane, dst, upd)  __A64_st1_lane_PU(src1,                   lane, dst, upd)
#define A64_st2_lane_PU(width, src1, src2,             lane, dst, upd)  __A64_st2_lane_PU(src1, src2,             lane, dst, upd)
#define A64_st3_lane_PU(width, src1, src2, src3,       lane, dst, upd)  __A64_st3_lane_PU(src1, src2, src3,       lane, dst, upd)
#define A64_st4_lane_PU(width, src1, src2, src3, src4, lane, dst, upd)  __A64_st4_lane_PU(src1, src2, src3, src4, lane, dst, upd)

#define __A64_st1x1(src1,                   dst)                 " ST1 { " #src1 "                                  }, [" #dst "]\n\t"
#define __A64_st1x2(src1, src2,             dst)                 " ST1 { " #src1 ", " #src2 "                       }, [" #dst "]\n\t"
#define __A64_st1x3(src1, src2, src3,       dst)                 " ST1 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "]\n\t"
#define __A64_st1x4(src1, src2, src3, src4, dst)                 " ST1 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "]\n\t"
#define __A64_st2x2(src1, src2,             dst)                 " ST2 { " #src1 ", " #src2 "                       }, [" #dst "]\n\t"
#define __A64_st3x3(src1, src2, src3,       dst)                 " ST3 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "]\n\t"
#define __A64_st4x4(src1, src2, src3, src4, dst)                 " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "]\n\t"
#define   A64_st1x1(width, size, src1,                   dst)    __A64_st1x1(src1,                   dst)
#define   A64_st1x2(width, size, src1, src2,             dst)    __A64_st1x2(src1, src2,             dst)
#define   A64_st1x3(width, size, src1, src2, src3,       dst)    __A64_st1x3(src1, src2, src3,       dst)
#define   A64_st1x4(width, size, src1, src2, src3, src4, dst)    __A64_st1x4(src1, src2, src3, src4, dst)
#define   A64_st2x2(width, size, src1, src2,             dst)    __A64_st2x2(src1, src2,             dst)
#define   A64_st3x3(width, size, src1, src2, src3,       dst)    __A64_st3x3(src1, src2, src3,       dst)
#define   A64_st4x4(width, size, src1, src2, src3, src4, dst)    __A64_st4x4(src1, src2, src3, src4, dst)

#define __A64_st1x1_IA(src1,                   dst, imm)                 " ST1 { " #src1 "                                  }, [" #dst "], # " #imm "\n\t"
#define __A64_st1x2_IA(src1, src2,             dst, imm)                 " ST1 { " #src1 ", " #src2 "                       }, [" #dst "], # " #imm "\n\t"
#define __A64_st1x3_IA(src1, src2, src3,       dst, imm)                 " ST1 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "], # " #imm "\n\t"
#define __A64_st1x4_IA(src1, src2, src3, src4, dst, imm)                 " ST1 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "], # " #imm "\n\t"
#define __A64_st2x2_IA(src1, src2,             dst, imm)                 " ST2 { " #src1 ", " #src2 "                       }, [" #dst "], # " #imm "\n\t"
#define __A64_st3x3_IA(src1, src2, src3,       dst, imm)                 " ST3 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "], # " #imm "\n\t"
#define __A64_st4x4_IA(src1, src2, src3, src4, dst, imm)                 " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "], # " #imm "\n\t"
#define   A64_st1x1_IA(width, size, src1,                   dst, imm)    __A64_st1x1_IA(src1,                   dst, imm)
#define   A64_st1x2_IA(width, size, src1, src2,             dst, imm)    __A64_st1x2_IA(src1, src2,             dst, imm)
#define   A64_st1x3_IA(width, size, src1, src2, src3,       dst, imm)    __A64_st1x3_IA(src1, src2, src3,       dst, imm)
#define   A64_st1x4_IA(width, size, src1, src2, src3, src4, dst, imm)    __A64_st1x4_IA(src1, src2, src3, src4, dst, imm)
#define   A64_st2x2_IA(width, size, src1, src2,             dst, imm)    __A64_st2x2_IA(src1, src2,             dst, imm)
#define   A64_st3x3_IA(width, size, src1, src2, src3,       dst, imm)    __A64_st3x3_IA(src1, src2, src3,       dst, imm)
#define   A64_st4x4_IA(width, size, src1, src2, src3, src4, dst, imm)    __A64_st4x4_IA(src1, src2, src3, src4, dst, imm)


#define __A64_st1x1_PU(src1,                   dst, upd)                 " ST1 { " #src1 "                                  }, [" #dst "], " #upd "\n\t"
#define __A64_st1x2_PU(src1, src2,             dst, upd)                 " ST1 { " #src1 ", " #src2 "                       }, [" #dst "], " #upd "\n\t"
#define __A64_st1x3_PU(src1, src2, src3,       dst, upd)                 " ST1 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "], " #upd "\n\t"
#define __A64_st1x4_PU(src1, src2, src3, src4, dst, upd)                 " ST1 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "], " #upd "\n\t"
#define __A64_st2x2_PU(src1, src2,             dst, upd)                 " ST2 { " #src1 ", " #src2 "                       }, [" #dst "], " #upd "\n\t"
#define __A64_st3x3_PU(src1, src2, src3,       dst, upd)                 " ST3 { " #src1 ", " #src2 ", " #src3 "            }, [" #dst "], " #upd "\n\t"
#define __A64_st4x4_PU(src1, src2, src3, src4, dst, upd)                 " ST4 { " #src1 ", " #src2 ", " #src3 ", " #src4 " }, [" #dst "], " #upd "\n\t"
#define   A64_st1x1_PU(width, size, src1,                   dst, upd)    __A64_st1x1_PU(src1,                   dst, upd)
#define   A64_st1x2_PU(width, size, src1, src2,             dst, upd)    __A64_st1x2_PU(src1, src2,             dst, upd)
#define   A64_st1x3_PU(width, size, src1, src2, src3,       dst, upd)    __A64_st1x3_PU(src1, src2, src3,       dst, upd)
#define   A64_st1x4_PU(width, size, src1, src2, src3, src4, dst, upd)    __A64_st1x4_PU(src1, src2, src3, src4, dst, upd)
#define   A64_st2x2_PU(width, size, src1, src2,             dst, upd)    __A64_st2x2_PU(src1, src2,             dst, upd)
#define   A64_st3x3_PU(width, size, src1, src2, src3,       dst, upd)    __A64_st3x3_PU(src1, src2, src3,       dst, upd)
#define   A64_st4x4_PU(width, size, src1, src2, src3, src4, dst, upd)    __A64_st4x4_PU(src1, src2, src3, src4, dst, upd)
#else
  /* Visual Studio / Linux gcc */
#define A64_stp_Xt_IA(src1, src2, dst, imm12)   __A64_stp_Xt_IA (src1, src2, (INT64 &) dst, imm12);
#define A64_stp_Xt_IB(src1, src2, dst, imm12)   __A64_stp_Xt_IB (src1, src2, (INT64 &) dst, imm12);
#define A64_str_Xt_IA(src1,       dst, imm12)   __A64_str_Xt_IA (src1,       (INT64 &) dst, imm12);
#define A64_str_Xt_IB(src1,       dst, imm12)   __A64_str_Xt_IB (src1,       (INT64 &) dst, imm12);

#define A64_stp_Wt_IA(src1, src2, dst, imm12)   __A64_stp_Wt_IA ((INT) src1, (INT)src2, (INT64 &) dst, imm12);
#define A64_stp_Wt_IB(src1, src2, dst, imm12)   __A64_stp_Wt_IB ((INT) src1, (INT)src2, (INT64 &) dst, imm12);
#define A64_str_Wt_IA(src1,       dst, imm12)   __A64_str_Wt_IA ((INT) src1,       (INT64 &) dst, imm12);
#define A64_str_Wt_IB(src1,       dst, imm12)   __A64_str_Wt_IB ((INT) src1,       (INT64 &) dst, imm12);

#define A64_strh_Wt_IA(src1,      dst, imm12)   __A64_strh_Wt_IA ((INT) src1,      (INT64 &) dst, imm12);
#define A64_strh_Wt_IB(src1,      dst, imm12)   __A64_strh_Wt_IB ((INT) src1,      (INT64 &) dst, imm12);

#define A64_strb_Wt_IA(src1,      dst, imm12)   __A64_strb_Wt_IA ((INT) src1,      (INT64 &) dst, imm12);
#define A64_strb_Wt_IB(src1,      dst, imm12)   __A64_strb_Wt_IB ((INT) src1,      (INT64 &) dst, imm12);

#define A64_ldr_Wt(dst, src )                     { __A64_ldr_Wt_IA((INT &) dst, (INT64 &) src, 0); }
#define A64_ldr_Wt_I(dst, src, offset, name)      { dst = name; }  /* valid for stack operation */
#define A64_ldr_Wt_IA(dst, src, imm12)            { __A64_ldr_Wt_IA((INT &) dst, (INT64 &) src, imm12); }

#define A64_str_Wt_I(src, dst, imm12, name)    { extern INT64 SP;  if ((INT64) (&dst) == (INT64) &SP)  name = src;  /* valid for stack operation */  \
                                                 else { INT64 __dst = (INT64)dst+((INT64)imm12); __A64_str_Wt_IA ((INT) src, (INT64 &) __dst, 0); }}

#define A64_ldr_Xt_X_LSL(  dst1,       src, offset, lsl)  { INT64 __src = (INT64)src+((INT64)offset<<lsl);  __A64_ldr_Xt_IA(dst1,       __src, 0); }
#define A64_ldp_Xt_X_LSL(  dst1, dst2, src, offset, lsl)  { INT64 __src = (INT64)src+((INT64)offset<<lsl);  __A64_ldp_Xt_IA(dst1, dst2, __src, 0); }

#define A64_str_Xt_X_LSL(  src1,       dst, offset, lsl)  { INT64 __dst = (INT64)dst+((INT64)offset<<lsl);  __A64_str_Xt_IA(src1,       __dst, 0); }
#define A64_stp_Xt_X_LSL(  src1, src2, dst, offset, lsl)  { INT64 __dst = (INT64)dst+((INT64)offset<<lsl);  __A64_stp_Xt_IA(src1, src2, __dst, 0); }

#define A64_ldr_Xt_X_LSR(  dst1,       src, offset, lsr)  { INT64 __src = (INT64)src+((UINT64)offset>>lsr);  __A64_ldr_Xt_IA(dst1,       __src, 0); }
#define A64_ldp_Xt_X_LSR(  dst1, dst2, src, offset, lsr)  { INT64 __src = (INT64)src+((UINT64)offset>>lsr);  __A64_ldp_Xt_IA(dst1, dst2, __src, 0); }

#define A64_str_Xt_X_LSR(  src1,       dst, offset, lsr)  { INT64 __dst = (INT64)dst+((UINT64)offset>>lsr);  __A64_str_Xt_IA(src1,       __dst, 0); }
#define A64_stp_Xt_X_LSR(  src1, src2, dst, offset, lsr)  { INT64 __dst = (INT64)dst+((UINT64)offset>>lsr);  __A64_stp_Xt_IA(src1, src2, __dst, 0); }

// Push on stack: stack must remain 16-byte aligned
#define A64_pushp(src1, src2)                   __A64_stp_Xt_IB ((INT64) src1, (INT64) src2, (INT64 &) SP,  -16);
#define A64_pushV(src)                          __A64_stp_Xt_IB ((INT64) src[0], (INT64) src[1], (INT64 &) SP,  -16);
#define A64_pushD(src1, src2)                   __A64_stp_Xt_IB ((INT64) src1, (INT64) src2, (INT64 &) SP,  -16);

// Pop from stack: stack must remain 16-byte aligned
#define A64_ldp_Xt_IA(dst1, dst2, src, imm12)   __A64_ldp_Xt_IA (&dst1, &dst2, &src, imm12);
#define A64_popp(     dst1, dst2)               __A64_ldp_Xt_IA ((INT64 &) dst1, (INT64 &) dst2, (INT64 &) SP,  16);
#define A64_popD(     dst1, dst2)               __A64_ldp_Xt_IA ((INT64 &) dst1, (INT64 &) dst2, (INT64 &) SP,  16);

#define A64_ldp_Xt_I( dst1, dst2, src, imm12, name1, name2) { dst1 = name1; dst2 = name2; } /*  __A64_ldp_Xt_I  (dst1, dst2, src, imm12, name1, name2); */
#define A64_ldp_Wt_I( dst1, dst2, src, imm12, name1, name2) { dst1 = name1; dst2 = name2; } /*  __A64_ldp_Wt_I  (dst1, dst2, src, imm12, name1, name2); */

#define A64_ld1x1(      width, size, dst1,                   src)       __A64_ld1x1_IA((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) 0);
#define A64_ld1x2(      width, size, dst1, dst2,             src)       __A64_ld1x2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) 0);
#define A64_ld1x3(      width, size, dst1, dst2, dst3,       src)       __A64_ld1x3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) 0);
#define A64_ld1x4(      width, size, dst1, dst2, dst3, dst4, src)       __A64_ld1x4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) 0);
#define A64_ld2x2(      width, size, dst1, dst2,             src)       __A64_ld2x2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) 0);
#define A64_ld3x3(      width, size, dst1, dst2, dst3,       src)       __A64_ld3x3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) 0);
#define A64_ld4x4(      width, size, dst1, dst2, dst3, dst4, src)       __A64_ld4x4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) 0);
#define A64_ld1x1_IA(   width, size, dst1,                   src, imm)  __A64_ld1x1_IA((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) imm);
#define A64_ld1x2_IA(   width, size, dst1, dst2,             src, imm)  __A64_ld1x2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) imm);
#define A64_ld1x3_IA(   width, size, dst1, dst2, dst3,       src, imm)  __A64_ld1x3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) imm);
#define A64_ld1x4_IA(   width, size, dst1, dst2, dst3, dst4, src, imm)  __A64_ld1x4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) imm);
#define A64_ld2x2_IA(   width, size, dst1, dst2,             src, imm)  __A64_ld2x2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) imm);
#define A64_ld3x3_IA(   width, size, dst1, dst2, dst3,       src, imm)  __A64_ld3x3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) imm);
#define A64_ld4x4_IA(   width, size, dst1, dst2, dst3, dst4, src, imm)  __A64_ld4x4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) imm);

#define A64_ld1x1_PU(   width, size, dst1,                   src, upd)  __A64_ld1x1_PU((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) upd);
#define A64_ld1x2_PU(   width, size, dst1, dst2,             src, upd)  __A64_ld1x2_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) upd);
#define A64_ld1x3_PU(   width, size, dst1, dst2, dst3,       src, upd)  __A64_ld1x3_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) upd);
#define A64_ld1x4_PU(   width, size, dst1, dst2, dst3, dst4, src, upd)  __A64_ld1x4_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) upd);
#define A64_ld2x2_PU(   width, size, dst1, dst2,             src, upd)  __A64_ld2x2_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) upd);
#define A64_ld3x3_PU(   width, size, dst1, dst2, dst3,       src, upd)  __A64_ld3x3_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) upd);
#define A64_ld4x4_PU(   width, size, dst1, dst2, dst3, dst4, src, upd)  __A64_ld4x4_PU((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) upd);



#define A64_ld1rx1(     width, size, dst1,                   src)       __A64_ld1rx1_IA((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) 0);
#define A64_ld1rx2(     width, size, dst1, dst2,             src)       __A64_ld1rx2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) 0);
#define A64_ld1rx3(     width, size, dst1, dst2, dst3,       src)       __A64_ld1rx3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) 0);
#define A64_ld1rx4(     width, size, dst1, dst2, dst3, dst4, src)       __A64_ld1rx4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) 0);
#define A64_ld1rx1_IA(  width, size, dst1,                   src, imm)  __A64_ld1rx1_IA((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) imm);
#define A64_ld1rx2_IA(  width, size, dst1, dst2,             src, imm)  __A64_ld1rx2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) imm);
#define A64_ld1rx3_IA(  width, size, dst1, dst2, dst3,       src, imm)  __A64_ld1rx3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) imm);
#define A64_ld1rx4_IA(  width, size, dst1, dst2, dst3, dst4, src, imm)  __A64_ld1rx4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) imm);
#define A64_ld1rx1_PU(  width, size, dst1,                   src, upd)  __A64_ld1rx1_IA((INT) width, (INT) size, (INT64 &) dst1,                                                 (INT64 &) src, (INT64) upd);
#define A64_ld1rx2_PU(  width, size, dst1, dst2,             src, upd)  __A64_ld1rx2_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2,                                 (INT64 &) src, (INT64) upd);
#define A64_ld1rx3_PU(  width, size, dst1, dst2, dst3,       src, upd)  __A64_ld1rx3_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT64 &) src, (INT64) upd);
#define A64_ld1rx4_PU(  width, size, dst1, dst2, dst3, dst4, src, upd)  __A64_ld1rx4_IA((INT) width, (INT) size, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT64 &) src, (INT64) upd);


#define A64_ld1_lane(   width, dst1,                   lane, src)       __A64_ld1_lane_IA((INT) width, (INT64 &) dst1,                                                 (INT) lane, (INT64 &) src, (INT64) 0);
#define A64_ld2_lane(   width, dst1, dst2,             lane, src)       __A64_ld2_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2,                                 (INT) lane, (INT64 &) src, (INT64) 0);
#define A64_ld3_lane(   width, dst1, dst2, dst3,       lane, src)       __A64_ld3_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT) lane, (INT64 &) src, (INT64) 0);
#define A64_ld4_lane(   width, dst1, dst2, dst3, dst4, lane, src)       __A64_ld4_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT) lane, (INT64 &) src, (INT64) 0);
#define A64_ld1_lane_IA(width, dst1,                   lane, src, imm)  __A64_ld1_lane_IA((INT) width, (INT64 &) dst1,                                                 (INT) lane, (INT64 &) src, (INT64 )imm);
#define A64_ld2_lane_IA(width, dst1, dst2,             lane, src, imm)  __A64_ld2_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2,                                 (INT) lane, (INT64 &) src, (INT64 )imm);
#define A64_ld3_lane_IA(width, dst1, dst2, dst3,       lane, src, imm)  __A64_ld3_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT) lane, (INT64 &) src, (INT64 )imm);
#define A64_ld4_lane_IA(width, dst1, dst2, dst3, dst4, lane, src, imm)  __A64_ld4_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT) lane, (INT64 &) src, (INT64 )imm);
#define A64_ld1_lane_PU(width, dst1,                   lane, src, upd)  __A64_ld1_lane_IA((INT) width, (INT64 &) dst1,                                                 (INT) lane, (INT64 &) src, (INT64 )upd);
#define A64_ld2_lane_PU(width, dst1, dst2,             lane, src, upd)  __A64_ld2_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2,                                 (INT) lane, (INT64 &) src, (INT64 )upd);
#define A64_ld3_lane_PU(width, dst1, dst2, dst3,       lane, src, upd)  __A64_ld3_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3,                 (INT) lane, (INT64 &) src, (INT64 )upd);
#define A64_ld4_lane_PU(width, dst1, dst2, dst3, dst4, lane, src, upd)  __A64_ld4_lane_IA((INT) width, (INT64 &) dst1, (INT64 &) dst2, (INT64 &) dst3, (INT64 &) dst4, (INT) lane, (INT64 &) src, (INT64 )upd);

#define A64_st1x1(      width, size, src1,                   dst)       __A64_st1x1_IA((INT) width, (INT) size, (INT64 &) src1,                                                    (INT64 &) dst, (INT64) 0);
#define A64_st1x2(      width, size, src1, src2,             dst)       __A64_st1x2_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) 0);
#define A64_st1x3(      width, size, src1, src2, src3,       dst)       __A64_st1x3_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) 0);
#define A64_st1x4(      width, size, src1, src2, src3, src4, dst)       __A64_st1x4_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) 0);
#define A64_st2x2(      width, size, src1, src2,             dst)       __A64_st2x2_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) 0);
#define A64_st3x3(      width, size, src1, src2, src3,       dst)       __A64_st3x3_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) 0);
#define A64_st4x4(      width, size, src1, src2, src3, src4, dst)       __A64_st4x4_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) 0);
#define A64_st1x1_IA(   width, size, src1,                   dst, imm)  __A64_st1x1_IA((INT) width, (INT) size, (INT64 &) src1,                                                    (INT64 &) dst, (INT64) imm);
#define A64_st1x2_IA(   width, size, src1, src2,             dst, imm)  __A64_st1x2_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) imm);
#define A64_st1x3_IA(   width, size, src1, src2, src3,       dst, imm)  __A64_st1x3_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) imm);
#define A64_st1x4_IA(   width, size, src1, src2, src3, src4, dst, imm)  __A64_st1x4_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) imm);
#define A64_st2x2_IA(   width, size, src1, src2,             dst, imm)  __A64_st2x2_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) imm);
#define A64_st3x3_IA(   width, size, src1, src2, src3,       dst, imm)  __A64_st3x3_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) imm);
#define A64_st4x4_IA(   width, size, src1, src2, src3, src4, dst, imm)  __A64_st4x4_IA((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) imm);

#define A64_st1x1_PU(   width, size, src1,                   dst, upd)  __A64_st1x1_PU((INT) width, (INT) size, (INT64 &) src1,                                                    (INT64 &) dst, (INT64) upd);
#define A64_st1x2_PU(   width, size, src1, src2,             dst, upd)  __A64_st1x2_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) upd);
#define A64_st1x3_PU(   width, size, src1, src2, src3,       dst, upd)  __A64_st1x3_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) upd);
#define A64_st1x4_PU(   width, size, src1, src2, src3, src4, dst, upd)  __A64_st1x4_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) upd);
#define A64_st2x2_PU(   width, size, src1, src2,             dst, upd)  __A64_st2x2_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2,                                    (INT64 &) dst, (INT64) upd);
#define A64_st3x3_PU(   width, size, src1, src2, src3,       dst, upd)  __A64_st3x3_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                    (INT64 &) dst, (INT64) upd);
#define A64_st4x4_PU(   width, size, src1, src2, src3, src4, dst, upd)  __A64_st4x4_PU((INT) width, (INT) size, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4,    (INT64 &) dst, (INT64) upd);

/* Immediate offset = 0 mode */
#define A64_st1_lane(   width, src1,                   lane, dst)       __A64_st1_lane_IA((INT) width, (INT64 &) src1,                                                 (INT) lane, (INT64 &) dst, (INT64) 0);
#define A64_st2_lane(   width, src1, src2,             lane, dst)       __A64_st2_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2,                                 (INT) lane, (INT64 &) dst, (INT64) 0);
#define A64_st3_lane(   width, src1, src2, src3,       lane, dst)       __A64_st3_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                 (INT) lane, (INT64 &) dst, (INT64) 0);
#define A64_st4_lane(   width, src1, src2, src3, src4, lane, dst)       __A64_st4_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4, (INT) lane, (INT64 &) dst, (INT64) 0);
/* Immediate offset unequal 0 mode */
#define A64_st1_lane_I( width, src1,                   lane, dst, imm)  __A64_st1_lane_IA((INT) width, (INT64 &) src1,                                                 (INT) lane, (INT64 &) (dst+imm), (INT64) 0);
#define A64_st2_lane_I( width, src1, src2,             lane, dst, imm)  __A64_st2_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2,                                 (INT) lane, (INT64 &) (dst+imm), (INT64) 0);
#define A64_st3_lane_I( width, src1, src2, src3,       lane, dst, imm)  __A64_st3_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                 (INT) lane, (INT64 &) (dst+imm), (INT64) 0);
#define A64_st4_lane_I( width, src1, src2, src3, src4, lane, dst, imm)  __A64_st4_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4, (INT) lane, (INT64 &) (dst+imm), (INT64) 0);
/* Post-indexed immediate addressing mode */
#define A64_st1_lane_IA(width, src1,                   lane, dst, imm)  __A64_st1_lane_IA((INT) width, (INT64 &) src1,                                                 (INT) lane, (INT64 &) dst, (INT64) imm);
#define A64_st2_lane_IA(width, src1, src2,             lane, dst, imm)  __A64_st2_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2,                                 (INT) lane, (INT64 &) dst, (INT64) imm);
#define A64_st3_lane_IA(width, src1, src2, src3,       lane, dst, imm)  __A64_st3_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                 (INT) lane, (INT64 &) dst, (INT64) imm);
#define A64_st4_lane_IA(width, src1, src2, src3, src4, lane, dst, imm)  __A64_st4_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4, (INT) lane, (INT64 &) dst, (INT64) imm);
/* Post-indexed register addressing mode */
#define A64_st1_lane_PU(width, src1,                   lane, dst, upd)  __A64_st1_lane_IA((INT) width, (INT64 &) src1,                                                 (INT) lane, (INT64 &) dst, (INT64) upd);
#define A64_st2_lane_PU(width, src1, src2,             lane, dst, upd)  __A64_st2_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2,                                 (INT) lane, (INT64 &) dst, (INT64) upd);
#define A64_st3_lane_PU(width, src1, src2, src3,       lane, dst, upd)  __A64_st3_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3,                 (INT) lane, (INT64 &) dst, (INT64) upd);
#define A64_st4_lane_PU(width, src1, src2, src3, src4, lane, dst, upd)  __A64_st4_lane_IA((INT) width, (INT64 &) src1, (INT64 &) src2, (INT64 &) src3, (INT64 &) src4, (INT) lane, (INT64 &) dst, (INT64) upd);


static void __A64_stp_Xt_IA(INT64 src1, INT64 src2, INT64 &dst, INT64 imm12)
{
    INT64 *Dst = (INT64 *) dst;
    Dst[0] = src1;
    Dst[1] = src2;
    dst += imm12;
}

static void __A64_stp_Xt_IB(INT64 src1, INT64 src2, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    INT64 *Dst = (INT64 *) dst;
    Dst[0] = src1;
    Dst[1] = src2;
}

static void __A64_str_Xt_IA(INT64 src1, INT64 &dst, INT64 imm12)
{
    INT64 *Dst = (INT64 *) dst;
    Dst[0] = src1;
    dst += imm12;
}

static void __A64_str_Xt_IB(INT64 src1, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    INT64 *Dst = (INT64 *) dst;
    Dst[0] = src1;
}

static void __A64_stp_Wt_IA(INT src1, INT src2, INT64 &dst, INT64 imm12)
{
    INT *Dst = (INT *) dst;
    Dst[0] = src1;
    Dst[1] = src2;
    dst += imm12;
}

static void __A64_stp_Wt_IB(INT src1, INT src2, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    INT *Dst = (INT *) dst;
    Dst[0] = src1;
    Dst[1] = src2;
}

static void __A64_str_Wt_IA(INT src1, INT64 &dst, INT64 imm12)
{
    INT *Dst = (INT *) dst;
    Dst[0] = src1;
    dst = (INT64) (&Dst[imm12/sizeof(INT)]);
}

static void __A64_str_Wt_IB(INT src1, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    INT *Dst = (INT *) dst;
    Dst[0] = src1;
}

static void __A64_strh_Wt_IA(INT src1, INT64 &dst, INT64 imm12)
{
    SHORT *Dst = (SHORT *) dst;
    Dst[0] = src1;
    dst += imm12;
}

static void __A64_strh_Wt_IB(INT src1, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    SHORT *Dst = (SHORT *) dst;
    Dst[0] = src1;
}

static void __A64_strb_Wt_IA(INT src1, INT64 &dst, INT64 imm12)
{
    SCHAR *Dst = (SCHAR *) dst;
    Dst[0] = src1;
    dst += imm12;
}

static void __A64_strb_Wt_IB(INT src1, INT64 &dst, INT64 imm12)
{
    dst += imm12;
    SCHAR *Dst = (SCHAR *) dst;
    Dst[0] = src1;
}

static void __A64_ldp_Xt_I(INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm12)
{
    INT64 *Src = (INT64 *) (src + imm12);
    dst1 = Src[0];
    dst2 = Src[1];
}

static void __A64_ldp_Wt_I(INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm12)
{
    INT *Src = (INT *) (src + imm12);
    dst1 = Src[0];
    dst2 = Src[1];
}



static void __A64_ldp_Xt_IA(INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm12)
{
    INT64 *Src = (INT64 *) src;
    dst1 = Src[0];
    dst2 = Src[1];
    src += imm12;
}
static void __A64_ldr_Xt_IA(INT64 &dst1, INT64 &src, INT64 imm12)
{
    INT64 *Src = (INT64 *) src;
    dst1 = Src[0];
    src += imm12;
}

static void __A64_ldr_Wt_IA(INT &dst, INT64 &src, INT64 imm12)
{
    dst = ((INT *) src)[0];
    src = (INT64) ((INT64) src + imm12);
}

static void __A64_ld1x1_IA(INT width, INT size, INT64 &dst1, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 1*(size>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        for (i=0; i < num; i++)
          Dst1[i] = Src[i];
        break;
      }
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        for (i=0; i < num; i++)
          Dst1[i] = Src[i];
        break;
      }
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        for (i=0; i < num; i++)
          Dst1[i] = Src[i];
        break;
      }
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        for (i=0; i < num; i++)
          Dst1[i] = Src[i];
        break;
      }
    }
    src += imm;
}
static void __A64_ld1x1_PU(INT width, INT size, INT64& dst1, INT64& src, INT64 upd)
{
    __A64_ld1x1_IA(width, size, dst1, src, (INT64)0);
    src += upd;
}


static void __A64_ld1x2_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 2*(size>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        for (i=0; i < num; i++)  Dst1[i] = Src[i];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num];
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        for (i=0; i < num; i++)  Dst1[i] = Src[i];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        for (i=0; i < num; i++)  Dst1[i] = Src[i];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        for (i=0; i < num; i++)  Dst1[i] = Src[i];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num];
      }
        break;
    }
    src += imm;
}
static void __A64_ld1x2_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& src, INT64 upd)
{
    __A64_ld1x2_IA(width, size, dst1, dst2, src, (INT64)0);
    src += upd;
}

static void __A64_ld1x3_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 3*(size>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        INT64 *Dst3 = (INT64 *) dst3;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
      }
        break;
    }
    src += imm;
}
static void __A64_ld1x3_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& dst3, INT64& src, INT64 upd)
{
    __A64_ld1x3_IA(width, size, dst1, dst2, dst3, src, (INT64)0);
    src += upd;
}


static void __A64_ld1x4_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &dst4, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 4*(size>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        INT64 *Dst3 = (INT64 *) dst3;
        INT64 *Dst4 = (INT64 *) dst4;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
        for (i=0; i < num; i++)  Dst4[i] = Src[i+num*3];
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        INT *Dst4 = (INT *) dst4;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
        for (i=0; i < num; i++)  Dst4[i] = Src[i+num*3];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        SHORT *Dst4 = (SHORT *) dst4;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
        for (i=0; i < num; i++)  Dst4[i] = Src[i+num*3];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        SCHAR *Dst4 = (SCHAR *) dst4;
        for (i=0; i < num; i++)  Dst1[i] = Src[i+num*0];
        for (i=0; i < num; i++)  Dst2[i] = Src[i+num*1];
        for (i=0; i < num; i++)  Dst3[i] = Src[i+num*2];
        for (i=0; i < num; i++)  Dst4[i] = Src[i+num*3];
      }
        break;
    }
    src += imm;
}
static void __A64_ld1x4_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& dst3, INT64& dst4, INT64& src, INT64 upd)
{
    __A64_ld1x4_IA(width, size, dst1, dst2, dst3, dst4, src, (INT64)0);
    src += upd;
}


static void __A64_ld2x2_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 2*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[2*i+0];
          Dst2[i] = Src[2*i+1];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[2*i+0];
          Dst2[i] = Src[2*i+1];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[2*i+0];
          Dst2[i] = Src[2*i+1];
        }
      }
        break;
    }
    src += imm;
}
static void __A64_ld2x2_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& src, INT64 upd)
{
    __A64_ld2x2_IA(width, size, dst1, dst2, src, (INT64)0);
    src += upd;
}

static void __A64_ld3x3_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 3*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[3*i+0];
          Dst2[i] = Src[3*i+1];
          Dst3[i] = Src[3*i+2];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[3*i+0];
          Dst2[i] = Src[3*i+1];
          Dst3[i] = Src[3*i+2];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[3*i+0];
          Dst2[i] = Src[3*i+1];
          Dst3[i] = Src[3*i+2];
        }
      }
        break;
    }
    src += imm;
}
static void __A64_ld3x3_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& dst3, INT64& src, INT64 upd)
{
    __A64_ld3x3_IA(width, size, dst1, dst2, dst3, src, (INT64)0);
    src += upd;
}

static void __A64_ld4x4_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &dst4, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 4*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        INT *Dst4 = (INT *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[4*i+0];
          Dst2[i] = Src[4*i+1];
          Dst3[i] = Src[4*i+2];
          Dst4[i] = Src[4*i+3];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        SHORT *Dst4 = (SHORT *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[4*i+0];
          Dst2[i] = Src[4*i+1];
          Dst3[i] = Src[4*i+2];
          Dst4[i] = Src[4*i+3];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        SCHAR *Dst4 = (SCHAR *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[4*i+0];
          Dst2[i] = Src[4*i+1];
          Dst3[i] = Src[4*i+2];
          Dst4[i] = Src[4*i+3];
        }
      }
        break;
    }
    src += imm;
}

static void __A64_ld4x4_PU(INT width, INT size, INT64& dst1, INT64& dst2, INT64& dst3, INT64& dst4, INT64& src, INT64 upd)
{
    __A64_ld4x4_IA(width, size, dst1, dst2, dst3, dst4, src, (INT64)0);
    src += upd;
}



static void __A64_ld1rx1_IA(INT width, INT size, INT64 &dst1, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 1*(width>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
        }
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
        }
      }
        break;
    }
    src += imm;
}

static void __A64_ld1rx2_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 2*(width>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
        }
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
        }
      }
        break;
    }
    src += imm;
}


static void __A64_ld1rx3_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 3*(width>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        INT64 *Dst3 = (INT64 *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
        }
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
        }
      }
        break;
    }
    src += imm;
}


static void __A64_ld1rx4_IA(INT width, INT size, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &dst4, INT64 &src, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 4*(width>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src  = (INT64 *) src;
        INT64 *Dst1 = (INT64 *) dst1;
        INT64 *Dst2 = (INT64 *) dst2;
        INT64 *Dst3 = (INT64 *) dst3;
        INT64 *Dst4 = (INT64 *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
          Dst4[i] = Src[3];
        }
      }
        break;
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        INT *Dst4 = (INT *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
          Dst4[i] = Src[3];
        }
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        SHORT *Dst4 = (SHORT *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
          Dst4[i] = Src[3];
        }
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        SCHAR *Dst4 = (SCHAR *) dst4;
        for (i=0; i < num; i++)
        {
          Dst1[i] = Src[0];
          Dst2[i] = Src[1];
          Dst3[i] = Src[2];
          Dst4[i] = Src[3];
        }
      }
        break;
    }
    src += imm;
}



static void __A64_ld1_lane_IA(INT width, INT64 &dst1, INT lane, INT64 &src, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 1*4));
        Dst1[lane] = Src[0];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 1*2));
        Dst1[lane] = Src[0];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 1*1));
        Dst1[lane] = Src[0];
      }
        break;
    }
    src += imm;
}


static void __A64_ld2_lane_IA(INT width, INT64 &dst1, INT64 &dst2, INT lane, INT64 &src, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 2*4));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 2*2));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 2*1));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
      }
        break;
    }
    src += imm;
}

static void __A64_ld3_lane_IA(INT width, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT lane, INT64 &src, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 3*4));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 3*2));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 3*1));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
      }
        break;
    }
    src += imm;
}

static void __A64_ld4_lane_IA(INT width, INT64 &dst1, INT64 &dst2, INT64 &dst3, INT64 &dst4, INT lane, INT64 &src, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Src  = (INT *) src;
        INT *Dst1 = (INT *) dst1;
        INT *Dst2 = (INT *) dst2;
        INT *Dst3 = (INT *) dst3;
        INT *Dst4 = (INT *) dst4;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 4*4));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
        Dst4[lane] = Src[3];
      }
        break;
      case 16:
      {
        SHORT *Src  = (SHORT *) src;
        SHORT *Dst1 = (SHORT *) dst1;
        SHORT *Dst2 = (SHORT *) dst2;
        SHORT *Dst3 = (SHORT *) dst3;
        SHORT *Dst4 = (SHORT *) dst4;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 4*2));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
        Dst4[lane] = Src[3];
      }
        break;
      case 8:
      {
        SCHAR *Src  = (SCHAR *) src;
        SCHAR *Dst1 = (SCHAR *) dst1;
        SCHAR *Dst2 = (SCHAR *) dst2;
        SCHAR *Dst3 = (SCHAR *) dst3;
        SCHAR *Dst4 = (SCHAR *) dst4;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 4*1));
        Dst1[lane] = Src[0];
        Dst2[lane] = Src[1];
        Dst3[lane] = Src[2];
        Dst4[lane] = Src[3];
      }
        break;
    }
    src += imm;
}

static void __A64_st1x4_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &src3, INT64 &src4, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 4*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        INT *Src4 = (INT *) src4;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src4[i];
      }
      break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        SHORT *Src4 = (SHORT *) src4;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src4[i];
      }
      break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        SCHAR *Src4 = (SCHAR *) src4;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src4[i];
      }
      break;
    }
    dst += imm;
}

static void __A64_st1x3_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &src3, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 3*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        for (i=0; i < num; i++)   Dst[i] = Src1[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src2[i];
        Dst += num;
        for (i=0; i < num; i++)   Dst[i] = Src3[i];
      }
        break;
    }
    dst += imm;
}
static void __A64_st1x3_PU(INT width, INT size, INT64& src1, INT64& src2, INT64& src3, INT64& dst, INT64 upd)
{
    __A64_st1x3_IA(width, size, src1, src2, src3, dst, (INT64) 0);
    dst += upd;
}


static void __A64_st1x1_IA(INT width, INT size, INT64 &src, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == (size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst = (INT *) dst;
        INT *Src = (INT *) src;
        for (i=0; i < num; i++)
        {
          Dst[i] = Src[i];
        }
      }
        break;
      case 16:
      {
        SHORT *Dst = (SHORT *) dst;
        SHORT *Src = (SHORT *) src;
        for (i=0; i < num; i++)
        {
          Dst[i] = Src[i];
        }
      }
        break;
      case 8:
      {
        SCHAR *Dst = (SCHAR *) dst;
        SCHAR *Src = (SCHAR *) src;
        for (i=0; i < num; i++)
        {
          Dst[i] = Src[i];
        }
      }
        break;
    }
    dst += imm;
}
static void __A64_st1x1_PU(INT width, INT size, INT64& src, INT64& dst, INT64 upd)
{
    __A64_st1x1_IA(width, size, src, dst, 0);
    dst += upd;
}


static void __A64_st1x2_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 2*(size>>3)));
    switch (width)
    {
      case 64:
      {
        INT64 *Dst  = (INT64 *) dst;
        INT64 *Src1 = (INT64 *) src1;
        INT64 *Src2 = (INT64 *) src2;
        for (i=0; i < num; i++)
        {
          Dst[0+i]   = Src1[i];
          Dst[num+i] = Src2[i];
        }
      }
        break;
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        for (i=0; i < num; i++)
        {
          Dst[0+i]   = Src1[i];
          Dst[num+i] = Src2[i];
        }
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        for (i=0; i < num; i++)
        {
          Dst[0+i]   = Src1[i];
          Dst[num+i] = Src2[i];
        }
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        for (i=0; i < num; i++)
        {
          Dst[0+i]   = Src1[i];
          Dst[num+i] = Src2[i];
        }
      }
        break;
    }
    dst += imm;
}
static void __A64_st1x2_PU(INT width, INT size, INT64& src1, INT64& src2, INT64& dst, INT64 upd)
{
    __A64_st1x2_IA(width, size, src1, src2, dst, 0);
    dst += upd;
}


static void __A64_st2x2_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 2*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        for (i=0; i < num; i++)
        {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
        }
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        for (i=0; i < num; i++)
        {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
        }
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        for (i=0; i < num; i++)
        {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
        }
      }
        break;
    }
    dst += imm;
}
static void __A64_st2x2_PU(INT width, INT size, INT64& src1, INT64& src2, INT64& dst, INT64 upd)
{
    __A64_st2x2_IA(width, size, src1, src2, dst, 0);
    dst += upd;
}


static void __A64_st3x3_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &src3, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 3*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        for (i=0; i < num; i++)
        {
          Dst[3*i+0] = Src1[i];
          Dst[3*i+1] = Src2[i];
          Dst[3*i+2] = Src3[i];
        }
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        for (i=0; i < num; i++)
        {
          Dst[3*i+0] = Src1[i];
          Dst[3*i+1] = Src2[i];
          Dst[3*i+2] = Src3[i];
        }
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        for (i=0; i < num; i++)
        {
          Dst[3*i+0] = Src1[i];
          Dst[3*i+1] = Src2[i];
          Dst[3*i+2] = Src3[i];
        }
      }
        break;
    }
    dst += imm;
}
static void __A64_st3x3_PU(INT width, INT size, INT64& src1, INT64& src2, INT64& src3, INT64& dst, INT64 upd)
{
    __A64_st3x3_IA(width, size, src1, src2, src3, dst, 0);
    dst += upd;
}


static void __A64_st4x4_IA(INT width, INT size, INT64 &src1, INT64 &src2, INT64 &src3, INT64 &src4, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((size == 128) || (size == 64));
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    INT i, num = size / width;
    FDK_ASSERT((imm == 0) || (imm == 4*(size>>3)));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        INT *Src4 = (INT *) src4;
        for (i=0; i < num; i++)
        {
          Dst[4*i+0] = Src1[i];
          Dst[4*i+1] = Src2[i];
          Dst[4*i+2] = Src3[i];
          Dst[4*i+3] = Src4[i];
        }
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        SHORT *Src4 = (SHORT *) src4;
        for (i=0; i < num; i++)
        {
          Dst[4*i+0] = Src1[i];
          Dst[4*i+1] = Src2[i];
          Dst[4*i+2] = Src3[i];
          Dst[4*i+3] = Src4[i];
        }
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        SCHAR *Src4 = (SCHAR *) src4;
        for (i=0; i < num; i++)
        {
          Dst[4*i+0] = Src1[i];
          Dst[4*i+1] = Src2[i];
          Dst[4*i+2] = Src3[i];
          Dst[4*i+3] = Src4[i];
        }
      }
        break;
    }
    dst += imm;
}
static void __A64_st4x4_PU(INT width, INT size, INT64& src1, INT64& src2, INT64& src3, INT64& src4, INT64& dst, INT64 upd)
{
    __A64_st4x4_IA(width, size, src1, src2, src3, src4, dst, 0);
    dst += upd;
}

static void __A64_st1_lane_IA(INT width, INT64 &src1, INT lane, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 1*4));
        Dst[0] = Src1[lane];
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 1*2));
        Dst[0] = Src1[lane];
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 1*1));
        Dst[0] = Src1[lane];
      }
        break;
    }
    dst += imm;
}

static void __A64_st2_lane_IA(INT width, INT64 &src1, INT64 &src2, INT lane, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 2*4));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 2*2));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 2*1));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
      }
        break;
    }
    dst += imm;
}

static void __A64_st3_lane_IA(INT width, INT64 &src1, INT64 &src2, INT64 &src3, INT lane, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 3*4));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 3*2));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 3*1));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
      }
        break;
    }
    dst += imm;
}

static void __A64_st4_lane_IA(INT width, INT64 &src1, INT64 &src2, INT64 &src3, INT64 &src4, INT lane, INT64 &dst, INT64 imm)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    switch (width)
    {
      case 32:
      {
        INT *Dst  = (INT *) dst;
        INT *Src1 = (INT *) src1;
        INT *Src2 = (INT *) src2;
        INT *Src3 = (INT *) src3;
        INT *Src4 = (INT *) src4;
        FDK_ASSERT((lane >=0) && (lane <= 3));
        // FDK_ASSERT((imm == 0) || (imm == 4*4));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
        Dst[3] = Src4[lane];
      }
        break;
      case 16:
      {
        SHORT *Dst  = (SHORT *) dst;
        SHORT *Src1 = (SHORT *) src1;
        SHORT *Src2 = (SHORT *) src2;
        SHORT *Src3 = (SHORT *) src3;
        SHORT *Src4 = (SHORT *) src4;
        FDK_ASSERT((lane >=0) && (lane <= 7));
        // FDK_ASSERT((imm == 0) || (imm == 4*2));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
        Dst[3] = Src4[lane];
      }
        break;
      case 8:
      {
        SCHAR *Dst  = (SCHAR *) dst;
        SCHAR *Src1 = (SCHAR *) src1;
        SCHAR *Src2 = (SCHAR *) src2;
        SCHAR *Src3 = (SCHAR *) src3;
        SCHAR *Src4 = (SCHAR *) src4;
        FDK_ASSERT((lane >=0) && (lane <= 15));
        // FDK_ASSERT((imm == 0) || (imm == 4*1));
        Dst[0] = Src1[lane];
        Dst[1] = Src2[lane];
        Dst[2] = Src3[lane];
        Dst[3] = Src4[lane];
      }
        break;
    }
    dst += imm;
}

#endif


#ifndef __ARM_AARCH64_NEON__

void FDK_check_s16_overflow(FIXP_DBL value)
{
  if (value > (FIXP_DBL) 0x7FFF)
  {
    FDKprintf("Error: 16-bit overflow: 0x%016X\n", value);
  }
  else if (value < (FIXP_DBL) 0x8000)
  {
    FDKprintf("Error: 16-bit underflow: 0x%016X\n", value);
  }
}


void FDK_check_s32_overflow(INT64 value)
{
  if (value > (INT64) 0x7FFFFFFF)
  {
    FDKprintf("Error: 32-bit overflow: 0x%016X\n", value);
  }
  else if (-value > (INT64) 2147483648 /* 0x8000.000 */)
  {
    FDKprintf("Error: 32-bit underflow: 0x%016X\n", value);
  }
}

void A64_check_move_immediate_sp(float immediate)
{
   int sign = 1;
   float imm = immediate;
   int exponent = 0;
   if (immediate < 0.f)
   {
       immediate = -immediate;
       sign = -1;
   }

   while (imm < 16.f)
   {
       imm *= 2.f;
       exponent--;
   }
   while (imm >= 32.f)
   {
       imm /= 2.f;
       exponent++;
   }
   if ( (imm > 31.f) || ((imm - (int) imm) != 0.f) )
   {
     /*  Mantissa exceeds */
     FDKprintf("Error in float immediate mantissa: %f = %f *2^%d\n", immediate, imm, exponent);
   }
   else if ( (exponent > 0) || (exponent < -7) )
   {
     /* Exponent exceeds */
     FDKprintf("Error in float immediate exponent: %f = %f *2^%d\n", immediate, imm, exponent);
   }
   else
     FDKprintf("Success in float immediate check: %f = %f *2^%d\n", immediate, imm, exponent);
}

void A64_check_move_immediate(int size, int datatype, int sign, INT64 immediate)
{
  FDK_ASSERT(size >= datatype);
  FDK_ASSERT (sign == 1 || sign == -1);
  if (immediate)
  {
    UCHAR byte0 = (UCHAR) (immediate>> 0);
    UCHAR byte1 = (UCHAR) (immediate>> 8);
    UCHAR byte2 = (UCHAR) (immediate>>16);
    UCHAR byte3 = (UCHAR) (immediate>>24);
    UCHAR byte4 = (UCHAR) (immediate>>32);
    UCHAR byte5 = (UCHAR) (immediate>>40);
    UCHAR byte6 = (UCHAR) (immediate>>48);
    UCHAR byte7 = (UCHAR) (immediate>>56);

    if (datatype == 8)
    {
      if (sign == -1)
        FDKprintf ("Error in MVN instruction: I8 datatype not available\n");

      else if (byte1 | byte2 | byte3 | byte4 | byte5 | byte6 | byte7)
      {
        FDKprintf ("Error in 8-bit immediate: 0x%016X - upper bytes must be 0x00\n", immediate);
      }
    }
    else if (datatype == 16)
    {
      if (byte2 | byte3 | byte4 | byte5 | byte6 | byte7)
      {
        FDKprintf ("Error in 16-bit immediate: 0x%016X - upper bits (>16) must be unique\n", immediate);
      }
      else
      {
        if ((sign == 1) && !( (byte0 == 0x00) || (byte1 == 0x00) ))
        {
          FDKprintf ("Error in 16-bit immediate: 0x%016X - one of the lower 2 bytes must be 0x00\n", immediate);
        }
        else if ((sign == -1) && !( (byte0 == 0xFF) || (byte1 == 0xFF) ))
        {
          FDKprintf ("Error in 16-bit immediate: 0x%016X - one of the lower 2 bytes must be 0xFF\n", immediate);
        }
      }
    }
    else if (datatype == 32)
    {
      if ( (sign == 1) && ((byte4 | byte5 | byte6 | byte7) != 0x00) )
      {
        FDKprintf ("Error in 32-bit immediate: 0x%016X - upper 4 bytes must be 0x00\n", immediate);
      }
      else if ( (sign == -1) && !( ((byte4 & byte5 & byte6 & byte7) != 0xFF) || ((byte4 | byte5 | byte6 | byte7) != 0x00)) )
      {
        FDKprintf ("Error in 32-bit immediate: 0x%016X - upper 4 bytes must be 0xFF/0x00\n", immediate);
      }
      if (sign == 1)
      {
        // check, if 3 bytes are 0x00 or byte0=0xFF,byte2=byte3=0x00 or byte1=byte0=FF,byte3=0x00
        if (!((byte0 && !(byte1 | byte2 | byte3))   || (byte1 && !(byte0 | byte2 | byte3)) ||
              (byte2 && !(byte0 | byte1 | byte3))   || (byte3 && !(byte0 | byte1 | byte2)) ||
              ((byte0 == 0xFF) && !(byte2 | byte3)) || (((byte0 & byte1) == 0xFF) && !byte3)))
        {
          FDKprintf ("Error in 32-bit immediate: 0x%016X - upper fillbytes must be 0x00, lower fillbytes must be 0x00/0xFF\n", immediate);
        }
      }
      else
      {
        // check, if 3 bytes are 0xFF or byte0=0x00,byte2=byte3=0xFF or byte1=byte0=00,byte3=0xFF
        if (!(((byte0 != 0xFF) && ((byte1 & byte2 & byte3) == 0xFF)) || ((byte1 != 0xFF) && ((byte0 & byte2 & byte3) == 0xFF)) ||
              ((byte2 != 0xFF) && ((byte1 & byte0 & byte3) == 0xFF)) || ((byte3 != 0xFF) && ((byte1 & byte2 & byte0) == 0xFF)) ||
              (!byte0 && ((byte2 & byte3) == 0xFF)) || (!(byte0 | byte1) && (byte3 == 0xFF)) ) )
        {
          FDKprintf ("Error in 32-bit immediate: 0x%016X - upper fillbytes must be 0xFF, lower fillbytes must be 0xFF/0x00\n", immediate);
        }
      }
    }
    else if (datatype == 64)
    {
      if (sign == -1)
        FDKprintf ("Error in MVN instruction: I64 datatype not available\n");
      // all bytes must match either 0x00 or 0xFF
      else if (!(( (byte0 == 0x00) || (byte0 == 0xFF) ) || ( (byte1 == 0x00) || (byte1 == 0xFF) ) ||
                 ( (byte2 == 0x00) || (byte2 == 0xFF) ) || ( (byte3 == 0x00) || (byte3 == 0xFF) ) ||
                 ( (byte4 == 0x00) || (byte4 == 0xFF) ) || ( (byte5 == 0x00) || (byte5 == 0xFF) ) ||
                 ( (byte6 == 0x00) || (byte6 == 0xFF) ) || ( (byte7 == 0x00) || (byte7 == 0xFF) ) ))
      {
        FDKprintf ("Error in 64-bit immediate: 0x%16X - any byte must be either 0x00/0xFF\n", immediate);
      }
    }
  }
}
#endif


/*#################################################################################*/
/*
*/
#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.13 Vector immediate */
/* ARMv8 GCC */
#define __A64_mvni_i16(size, dst, imm)            "MVNI  " #dst ", # " #imm "\n\t"
#define __A64_mvni_i32_msl(size, dst, imm, msl)   "MVNI  " #dst ", # " #imm " , MSL " #msl "\n\t"
#define __A64_mvni_i32_lsl(size, dst, imm, lsl)   "MVNI  " #dst ", # " #imm " , LSL " #lsl "\n\t"

#define __A64_movi(dst, imm)                      "MOVI  " #dst ", # " #imm "\n\t"
#define __A64_movi_lsl(dst, imm, lsl)             "MOVI  " #dst ", # " #imm ", LSL  " #lsl "\n\t"
#define __A64_movi_msl(dst, imm, msl)             "MOVI  " #dst ", # " #imm ", MSL  " #msl "\n\t"
#define __A64_fmov(dst, imm)                      "FMOV  " #dst ", # " #imm "\n\t"

#define __A64_mov(dst, src)                       "MOV   " #dst ", " #src "\n\t"

#define __A64_mov_Xt_imm(dst, imm)                "MOV   " #dst ", # " #imm "\n\t"
#define __A64_mov_Wt_imm(dst, imm)                "MOV   " #dst ", # " #imm "\n\t"
#define __A64_mov_Xt(dst, src)                    "MOV   " #dst ", " #src "\n\t"
#define __A64_mov_Wt(dst, src)                    "MOV   " #dst ", " #src "\n\t"
#define __A64_smov_Xt(dst, src, lane)             "SMOV  " #dst ", " #src " [ " #lane " ]\n\t"
#define __A64_smov_Wt(dst, src, lane)             "SMOV  " #dst ", " #src " [ " #lane " ]\n\t"
#define __A64_mov_Xt_to_lane(dst, lane, src)      "MOV   " #dst " [ " #lane " ], " #src "\n\t"
#define __A64_mov_Wt_to_lane(dst, lane, src)      "MOV   " #dst " [ " #lane " ], " #src "\n\t"
#define __A64_umov_Wt(dst, src, lane)             "UMOV  " #dst ", " #src "[ " #lane " ]\n\t"
#define __A64_umov_Xt(dst, src, lane)             "UMOV  " #dst ", " #src "[ " #lane " ]\n\t"
#define __A64_sxtw_Xt(Xd, Wn)                     "SXTW  " #Xd ", " #Wn "\n\t"

#define A64_mvni_i16(size, dst, imm)                     __A64_mvni_i16(size, dst, imm)
#define A64_mvni_i32_lsl(size, dst, imm, lsl)            __A64_mvni_i32_lsl(size, dst, imm, lsl)
#define A64_mvni_i32_msl(size, dst, imm, msl)            __A64_mvni_i32_lsl(size, dst, imm, msl)

#define A64_movi(    width, size, dst, imm)              __A64_movi(dst, imm)
#define A64_movi_lsl(width, size, dst, imm, lsl)         __A64_movi_lsl(dst, imm, lsl)
#define A64_movi_msl(width, size, dst, imm, msl)         __A64_movi_msl(dst, imm, msl)
#define A64_fmov(           size, dst, imm)              __A64_fmov(dst, imm)

#define A64_mov(width, size, dst, src)                   __A64_mov(dst, src)

#define A64_mov_Xt_imm(dst, imm)                         __A64_mov_Xt_imm(dst, imm)
#define A64_mov_Wt_imm(dst, imm)                         __A64_mov_Wt_imm(dst, imm)
#define A64_mov_Xt(dst, src)                             __A64_mov_Xt(dst, src)
#define A64_mov_Wt(dst, src)                             __A64_mov_Wt(dst, src)
#define A64_smov_Xt(width, size, dst, src, lane)         __A64_smov_Xt(dst, src, lane)
#define A64_smov_Wt(width, size, dst, src, lane)         __A64_smov_Wt(dst, src, lane)
#define A64_mov_Wt_to_lane(width, size, dst, lane, src)  __A64_mov_Wt_to_lane(dst, lane, src)
#define A64_mov_Xt_to_lane(width, size, dst, lane, src)  __A64_mov_Xt_to_lane(dst, lane, src)
#define A64_umov_Wt(dst, src, lane)                      __A64_umov_Wt(dst, src, lane)
#define A64_umov_Xt(width, size, dst, src, lane)         __A64_umov_Xt(dst, src, lane)
#define A64_sxtw_Xt(dst, src)                            __A64_sxtw_Xt(dst, src)

#else
/* Visual Studio / Linux gcc */
#define A64_mvni_i16(size, dst, imm)                     __A64_mvni_i16(size, (A64_H *) dst, imm);
#define A64_mvni_i32_msl(size, dst, imm, msl)            __A64_mvni_i32_msl(size, (A64_S *) dst, imm, msl);
#define A64_mvni_i32_lsl(size, dst, imm, lsl)            __A64_mvni_i32_lsl(size, (A64_S *) dst, imm, lsl);

#define A64_movi(    width, size, dst, imm)              __A64_movi((INT) width, (INT) size, (INT64 &) dst, (UINT64) imm);
#define A64_movi_lsl(width, size, dst, imm, lsl)         __A64_movi_lsl((INT) width, (INT) size, (INT64 &) dst, (UINT64) imm, (INT) lsl);
#define A64_movi_msl(width, size, dst, imm, msl)         __A64_movi_msl((INT) width, (INT) size, (INT64 &) dst, (UINT64) imm, (INT) msl);
#define A64_fmov(           size, dst, imm)              __A64_fmov(size, (A64_SP)  dst, (float) imm);

#define A64_mov(width, size, dst, src)                   __A64_mov((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src);

#define A64_mov_Xt_imm(dst, imm)                         { A64_check_move_immediate(64, 64, 1, imm); dst = imm; } /* __A64_mov_Xt_imm((INT64 &) dst, (INT64) imm); */
#define A64_mov_Wt_imm(dst, imm)                         __A64_mov_Wt_imm((FIXP_DBL *) &dst, (INT64) imm);
#define A64_mov_Xt(dst, src)                             __A64_mov_Xt((INT64 &) dst, (INT64 &) src);
#define A64_smov_Wt(width, size, dst, src, lane)         __A64_smov_Wt((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT) lane);
#define A64_smov_Xt(width, size, dst, src, lane)         __A64_smov_Xt((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT) lane);
#define A64_mov_Wt(dst, src)                             __A64_mov_Wt((INT &) dst, (INT &) src);
#define A64_mov_Wt_to_lane(width, size, dst, lane, src)  __A64_mov_Wt_to_lane((INT) width, (INT) size, (SCHAR *) dst, (INT) lane, (FIXP_DBL *) &src);
#define A64_mov_Xt_to_lane(width, size, dst, lane, src)  __A64_mov_Xt_to_lane((INT) width, (INT) size, (INT64 &) dst, (INT) lane, (INT64 &) src);
#define A64_umov_Wt(dst, src, lane)                      __A64_umov_Wt((INT) 32, (INT) 128, (INT64 &) dst, (INT64 &) src, (INT) lane);
#define A64_umov_Xt(width, size, dst, src, lane)         __A64_umov_Xt((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT) lane);
#define A64_sxtw_Xt(Xd, Wn)                              __A64_sxtw_Xt((INT64 &) Xd, (INT64 &) Wn);

/* Move immediate to vector register, but not to any core registers */
static inline void __A64_mvni_i16(int size, A64_H *dst, INT64 imm)
{
    A64_check_move_immediate(64, 16, -1, imm);
    for (int i = 0; i < (size >> 4); i++)
    {
        dst[i] = (A64_H) imm;
    }
}
static inline void __A64_mvni_i32_lsl(int size, A64_S *dst, INT64 imm, int lsl)
{
    A64_check_move_immediate(64, 8, 1, imm);
    if ((lsl == 0) || (lsl == 8) || (lsl == 16) || (lsl == 24))
    {
      for (int i = 0; i < (size >> 5); i++)
      {
        dst[i] = (A64_S) ~(imm<<lsl);
      }
    }
    else
       FDKprintf ("Error in MVNI instruction: LSL operand (%d) must be in range [0,8,16,24]\n", lsl);
}
static inline void __A64_mvni_i32_msl(int size, A64_S *dst, INT64 imm, int msl)
{
    A64_check_move_immediate(64, 8, 1, imm);
    if ((msl == 8) || (msl == 16))
    {
      INT64 lowbits = (msl == 8) ? 0x000000FF : 0x0000FFFF;
      for (int i = 0; i < (size >> 5); i++)
      {
        dst[i] = (A64_S) ~((imm<<msl) | lowbits);
      }
    }
    else
       FDKprintf ("Error in MVNI instruction: MSL operand (%d) must be in range [8,16]\n", msl);
}

/* MOVI Vn.<T>, #uimm8  <T> is 8B or 16B */
static inline void __A64_movi(INT width, INT size, INT64 &dst, UINT64 imm)
{
  FDK_ASSERT((width == 8) || (width == 16) || (width == 32));
  FDK_ASSERT((size == 64) || (size == 128));
  FDK_ASSERT(imm <= (UINT64) 0xFF);

  INT i, num = size / width;
  switch (width)
  {
    case 8:
    {
      UCHAR *Dst8 = (UCHAR *) dst;
      for (i = 0; i < num; i++)
      {
        Dst8[i] = (UCHAR) imm;
      }   
      break;
    }
    case 16:
    {
        USHORT* Dst16 = (USHORT*)dst;
        for (i = 0; i < num; i++)
        {
            Dst16[i] = (USHORT)imm;
        }
        break;
    }

    case 32:
    {
      INT * Dst32 = (INT*)dst;
      for (i = 0; i < num; i++)
      {
        Dst32[i] = (INT)imm;
      }
      break;
    }
  }
}

static inline void __A64_movi_lsl(INT width, INT size, INT64 &dst, UINT64 imm, INT lsl)
{
    /* Note: width is related to the created element output data */
    FDK_ASSERT((width == 8) || (width == 16) || (width == 32) || (width == 64));
    FDK_ASSERT((size == 64) || (size == 128) );

    INT i, num = size / width;

    switch (width)
    {
      case 8:  /* MOVI Vn.<T>, #uimm8  where <T> is 8B or 16B */
      {
        FDK_ASSERT(lsl == 0);
        UCHAR *Dst = (UCHAR *) dst;
        A64_check_move_immediate(64, 8, 1, imm);
        for (i = 0; i < num; i++)
        {
            Dst[i] = (UCHAR) imm;
        }
      }
      break;
      case 16:  /* MOVI Vn.<T>, #uimm8 {, LSL #shift}   where <T> is 4H or 8H and shift is 0 or 8 */
      {
        FDK_ASSERT((lsl == 0) || (lsl == 8));
        USHORT *Dst = (USHORT *) dst;
        A64_check_move_immediate(64, 8, 1, imm);
        for (i = 0; i < num; i++)
        {
            Dst[i] = (USHORT) imm << lsl;
        }
      }
      break;
      case 32:  /* MOVI Vn.<T>, #uimm8 {, LSL #shift}   where <T> is 2S or 4S and shift is 0,8,16,24 */
      {
        FDK_ASSERT((lsl == 0) || (lsl == 8) || (lsl == 16) || (lsl == 24));
        UINT *Dst = (UINT *) dst;
        A64_check_move_immediate(64, 8, 1, imm);
        for (i = 0; i < num; i++)
        {
            Dst[i] = (UINT) imm << lsl;
        }
      }
      break;
      case 64:  /* MOVI Vn.2D, #uimm64    or    MOVI Dn, #uimm64 */
      {
        FDK_ASSERT((lsl == 0));
        UINT64 *Dst = (UINT64 *) dst;
        A64_check_move_immediate(64, 64, 1, imm);
        for (i = 0; i < num; i++)
        {
            Dst[i] = (UINT64) imm;
        }
      }
      break;
    }
}

static inline void __A64_movi_msl(INT width, INT size, INT64 &dst, UINT64 imm, INT msl)
{
    /* Note: width is related to the created element output data */
    FDK_ASSERT(width == 32);
    FDK_ASSERT((size == 64) || (size == 128) );

    INT i, num = size / width;

    switch (width)
    {
      case 32:  /* MOVI Vn.<T>, #uimm8 {, MSL #shift}   where <T> is 2S or 4S and shift is 0,8,16,24 */
      {
        FDK_ASSERT((msl == 8) || (msl == 16));
        UINT *Dst = (UINT *) dst;
        UINT msl_bits = (msl == 16) ? (UINT) 0x0000FFFF : (UINT) 0x000000FF;
        A64_check_move_immediate(64, 8, 1, imm);
        for (i = 0; i < num; i++)
        {
            Dst[i] = ((UINT) imm << msl) | msl_bits;
        }
      }
      break;
    }
}

static inline void __A64_fmov( int size, A64_SP dst, float imm)
{
    A64_check_move_immediate_sp(imm);
    for (int i = 0; i < (size >> 5); i++)
    {
        dst[i] = imm;
    }
}

static inline void __A64_mov(INT width, INT size, INT64 &dst, INT64 &src)
{
  FDK_ASSERT(width == 8);
  FDK_ASSERT((size == 64) || (size == 128));

  INT i, num = size / width;

  switch (width)
  {
    case 8:
    {
      INT *Dst = (INT *) dst;
      INT *Src = (INT *) src;
      for (i=0; i < (num/4); i++)
      {
        Dst[i] = Src[i];
      }
    }
    break;
  }
}

/* Move immediate to any core register, but not to vector registers */
static inline void __A64_mov_Xt_imm(A64_X dst, INT64 imm)
{
    A64_check_move_immediate(64, 64, 1, imm);
    *dst = imm;
}

static inline void __A64_mov_Wt_imm(A64_S *dst, INT64 imm)
{
    A64_check_move_immediate(64, 32, 1, imm);
    *dst = (INT) imm;
}


static inline void __A64_mov_Xt(INT64 &dst, INT64 &src)
{
extern INT64 * __XZR;
    if (&dst != __XZR)
    {
      if (&src != __XZR)
      {
        dst = src;
      }
      else
      {
        /* Reading XZR results in zero */
        dst = (INT64) 0;
      }
    }
    else
    {
        ;  /* Write to XZR are ignored */
    }
}

static inline void __A64_mov_Xt_to_lane(INT width, INT size, INT64 &dst, INT lane, INT64 &src)
{
  extern INT64 * __XZR;
  extern INT   * __WZR;
  FDK_ASSERT((width == 64));
  FDK_ASSERT((size == 128));

  INT num = size / width;
  FDK_ASSERT((lane >= 0) && (lane < num));

  INT64 val = src;
  if (&src == __XZR)
  {
    /* Reading XZR results in zero */
    val = (INT64) 0;
  }
  INT64 *Dst = (INT64 *) dst;
  Dst[lane] = val;
}

static inline void __A64_mov_Wt(INT & dst, INT & src)
{
    extern INT * __WZR;
    if (&dst != __WZR)
    {
        if (&src != __WZR)
        {
            dst = src;
        }
        else
        {
            /* Reading WZR results in zero */
            dst = (INT)0;
        }
    }
    else
    {
        ;  /* Write to WZR are ignored */
    }
}


static inline void __A64_mov_Wt_to_lane(INT width, INT size, SCHAR *dst, INT lane, FIXP_DBL *src)
{
  extern INT64 * __XZR;
  extern INT   * __WZR;
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));

  INT num = size / width;
  FDK_ASSERT((lane >= 0) && (lane < num));

  INT val = *src;
  if (src == (FIXP_DBL *) __WZR)
  {
    /* Reading XZR results in zero */
    val = (INT) 0;
  }
  switch (width)
  {
    case 32:
    {
      INT *Dst = (INT *) dst;
      Dst[lane] = val;
    }
    break;
    case 16:
    {
      SHORT *Dst = (SHORT *) dst;
      Dst[lane] = (SHORT) val;
    }
    break;
    case 8:
    {
      SCHAR *Dst = (SCHAR *) dst;
      Dst[lane] = (SCHAR) val;
    }
    break;
  }
}


/* Move a vector lane zero-extended to Wt register, if not WZR */
static inline void __A64_umov_Wt(INT width, INT size, INT64 &dst, INT64 &src, INT lane)
{
  extern INT64 * __XZR;
  extern INT   * __WZR;
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));

  INT num = size / width;
  FDK_ASSERT((lane >= 0) && (lane < num));

  UINT val = (UINT) 0;

  switch (width)
  {
    case 32:
    {
      UINT *Src = (UINT *) src;
      val = Src[lane];
    }
    break;
    case 16:
    {
      USHORT *Src = (USHORT *) src;
      val = (UINT) Src[lane];
    }
    break;
    case 8:
    {
      UCHAR *Src = (UCHAR *) src;
      val = (UINT) Src[lane];
    }
    break;
  }
  if (&dst != __XZR)
    dst = (INT64) val;
}

/* Move a 64-bit vector lane zero-extended to Xt register, if not WZR */
static inline void __A64_umov_Xt(INT width, INT size, INT64& dst, INT64& src, INT lane)
{
  extern INT64* __XZR;

  FDK_ASSERT(width == 64);
  FDK_ASSERT(size == 128);
  FDK_ASSERT(lane == 0 || lane == 1);

  UINT64 val;
  UINT64* Src = (UINT64*)src;
  val = Src[lane];
  if (&dst != __XZR)
    dst = (INT64)val;
}

static inline void __A64_sxtw_Xt(INT64& dst, INT64& src)
{
  extern INT64* __XZR;
  INT val = (INT) src;

  if (&dst != __XZR)
    dst = (INT64)val;

}

/* SMOV Wd, Vn.<T>[index]  T = B with lane = 0..15 or T = H with lane = 0..7 */
static inline void __A64_smov_Wt(INT width, INT size, INT64 &dst, INT64 &src, INT lane)
{
  FDK_ASSERT(width == 8 || width == 16);
  FDK_ASSERT(size == 64 || size == 128);
  FDK_ASSERT (lane >= 0 && (lane < (size / width)));

  switch (width)
  {
    case 8:
    {
      SCHAR *Src = (SCHAR *) src;
      SCHAR val = Src[lane];
      INT *Dst = (INT *) &dst;
      Dst[0] = (INT) val;
      break;
    }
    case 16:
    {
      SHORT *Src = (SHORT *) src;
      SHORT val = Src[lane];
      INT *Dst = (INT *) &dst;
      Dst[0] = (INT) val;
      break;
    }
  }
}

/*
 * SMOV Xd, Vn.<T>[index]  T = S with lane = 0..3 or
 *                         T = H with lane = 0..7 or
 *                         T = B with lane = 0..15
 */
static inline void __A64_smov_Xt(INT width, INT size, INT64 &dst, INT64 &src, INT lane)
{
  FDK_ASSERT(width == 8 || width == 16 || width == 32);
  FDK_ASSERT(size == 64 || size == 128);
  FDK_ASSERT (lane >= 0 && (lane < (size / width)));

  switch (width)
  {
    case 8:
    {
      SCHAR *Src = (SCHAR *) src;
      SCHAR val = Src[lane];
      INT64 *Dst = (INT64 *) &dst;
      Dst[0] = (INT64) val;
      break;
    }
    case 16:
    {
      SHORT *Src = (SHORT *) src;
      SHORT val = Src[lane];
      INT64 *Dst = (INT64 *) &dst;
      Dst[0] = (INT64) val;
      break;
    }
    case 32:
    {
      INT *Src = (INT *) src;
      INT val = Src[lane];
      INT64 *Dst = (INT64 *) &dst;
      Dst[0] = (INT64) val;
      break;
    }
  }
}

#endif
#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.4 vector arithmetic */
/* ARMv8 GCC */
#define __A64_fmul(dst, src1, src2)                 "FMUL  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_fmul(size, width, dst, src1, src2)       __A64_fmul(dst, src1, src2)
#else
#define A64_fmul(size, width, dst, src1, src2)       __A64_fmul((INT) size, (INT) width, dst, src1, src2);

static void __A64_fmul(INT size, INT width, A64_SP dst, A64_SP src1, A64_SP src2)
{
    INT i, num = size/width;
    switch (width)
    {
    case 32:
        for (i=0; i < num; i++)
            dst[i] = src1[i] * src2[i];
        break;
    case 64:
        A64_DP Dst  = (A64_DP) dst;
        A64_DP Src1 = (A64_DP) src1;
        A64_DP Src2 = (A64_DP) src2;
        for (i=0; i < num; i++)
            Dst[i] = Src1[i] * Src2[i];
        break;
    }
}
#endif

#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.4 vector arithmetic */
/* ARMv8 GCC */
#define __A64_fadd(dst, src1, src2)                 "FADD  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_fadd(size, width, dst, src1, src2)       __A64_fadd(dst, src1, src2)
#else
#define A64_fadd(size, width, dst, src1, src2)       __A64_fadd((INT) size, (INT) width, dst, src1, src2);

static void __A64_fadd(INT size, INT width, A64_SP dst, A64_SP src1, A64_SP src2)
{
    INT i, num = size/width;
    switch (width)
    {
    case 32:
        for (i=0; i < num; i++)
            dst[i] = src1[i] + src2[i];
        break;
    case 64:
        A64_DP Dst  = (A64_DP) dst;
        A64_DP Src1 = (A64_DP) src1;
        A64_DP Src2 = (A64_DP) src2;
        for (i=0; i < num; i++)
            Dst[i] = Src1[i] + Src2[i];
        break;
    }
}
#endif

#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.4 vector arithmetic */
/* ARMv8 GCC */
#define __A64_fmls(dst, src1, src2)                 "FMLS  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_fmls(size, width, dst, src1, src2)       __A64_fmls(dst, src1, src2)
#else
#define A64_fmls(size, width, dst, src1, src2)       __A64_fmls((INT) size, (INT) width, dst, src1, src2);

static void __A64_fmls(INT size, INT width, A64_SP dst, A64_SP src1, A64_SP src2)
{
    INT i, num = size/width;
    switch (width)
    {
    case 32:
        for (i=0; i < num; i++)
            dst[i] -= src1[i] * src2[i];
        break;
    case 64:
        A64_DP Dst  = (A64_DP) dst;
        A64_DP Src1 = (A64_DP) src1;
        A64_DP Src2 = (A64_DP) src2;
        for (i=0; i < num; i++)
            Dst[i] -= Src1[i] * Src2[i];
        break;
    }
}
#endif

#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.4 vector arithmetic */
/* ARMv8 GCC */
#define __A64_fmla(dst, src1, src2)                 "FMLA  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_fmla(size, width, dst, src1, src2)       __A64_fmls(dst, src1, src2)
#else
#define A64_fmla(size, width, dst, src1, src2)       __A64_fmla((INT) size, (INT) width, dst, src1, src2);

static void __A64_fmla(INT size, INT width, A64_SP dst, A64_SP src1, A64_SP src2)
{
    INT i, num = size/width;
    switch (width)
    {
    case 32:
        for (i=0; i < num; i++)
            dst[i] += src1[i] * src2[i];
        break;
    case 64:
        A64_DP Dst  = (A64_DP) dst;
        A64_DP Src1 = (A64_DP) src1;
        A64_DP Src2 = (A64_DP) src2;
        for (i=0; i < num; i++)
            Dst[i] += Src1[i] * Src2[i];
        break;
    }
}
#endif
#ifdef __ARM_AARCH64_NEON__
/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter  5.7.4 vector arithmetic */
/* ARMv8 GCC */
#define __A64_fsub(dst, src1, src2)                 "FSUB  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_fsub(size, width, dst, src1, src2)       __A64_fsub(dst, src1, src2)
#else
#define A64_fsub(size, width, dst, src1, src2)       __A64_fsub((INT) size, (INT) width, dst, src1, src2);

static void __A64_fsub(INT size, INT width, A64_SP dst, A64_SP src1, A64_SP src2)
{
    INT i, num = size/width;
    switch (width)
    {
    case 32:
        for (i=0; i < num; i++)
            dst[i] = src1[i] - src2[i];
        break;
    case 64:
        A64_DP Dst  = (A64_DP) dst;
        A64_DP Src1 = (A64_DP) src1;
        A64_DP Src2 = (A64_DP) src2;
        for (i=0; i < num; i++)
            Dst[i] = Src1[i] - Src2[i];
        break;
    }
}
#endif

/*#################################################################################*/
/*
   Description: Load a single 16/32-bit lane of a D register from memory (src)
   Parameter size: 16 or 32
   Parameter dst:  Dx[0,1] 64-bit NEON register
   Parameter src:  r core register used as a pointer to 16/32-Bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_AARCH64_NEON__
/* ARMv8 GCC */
#else
  /* Visual Studio / Linux gcc */
#endif



#ifdef __ARM_AARCH64_NEON__
#define FDK_mvpush(first_src, last_src)                            "VPUSH { " #first_src "-" #last_src " } \n\t"
#else
#define FDK_mvpush(first_src, last_src)                            ;
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_mvpop(first_src, last_src)                             "VPOP { " #first_src "-" #last_src " } \n\t"
#else
#define FDK_mvpop(first_src, last_src)                             ;
#endif







#ifdef __ARM_AARCH64_NEON__
#define FDK_vneg_q( size, dst, src)  "VNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vneg_d( size, dst, src)  "VNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqneg_q(size, dst, src)  "VQNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqneg_d(size, dst, src)  "VQNEG.S" #size " " #dst ", " #src " \n\t"
#else
#define FDK_vneg_q(size, dst, src)  { if (size == 32) FDK_vneg_s32_q(dst, src);  else if (size == 16) FDK_vneg_s16_q(dst, src); }
#define FDK_vneg_d(size, dst, src)  { if (size == 32) FDK_vneg_s32_d(dst, src);  else if (size == 16) FDK_vneg_s16_d(dst, src); }
#define FDK_vqneg_q(size, dst, src) { if (size == 32) FDK_vqneg_s32_q(dst, src); else if (size == 16) FDK_vqneg_s16_q(dst, src); }
#define FDK_vqneg_d(size, dst, src) { if (size == 32) FDK_vqneg_s32_d(dst, src); else if (size == 16) FDK_vqneg_s16_d(dst, src); }

static void inline FDK_vneg_s32_q (A64_V dst, A64_V src)
{
  A64_S* Dst = (A64_S*) dst;
  A64_S* Src = (A64_S*) src;
  for(int i = 0; i < 4; i++)
  {
    FDK_check_s32_overflow((INT64) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s32_d (A64_X dst, A64_X src)
{
  A64_S *Dst = (A64_S*) dst;
  A64_S *Src = (A64_S*) src;
  for(int i = 0; i < 2; i++)
  {
    FDK_check_s32_overflow((INT64) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s16_q (A64_V dst, A64_V src)
{
  A64_H *Dst = (A64_H*) dst;
  A64_H *Src = (A64_H*) src;
  for(int i = 0; i < 4; i++)
  {
    FDK_check_s16_overflow((A64_S) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s16_d (A64_X dst, A64_X src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 2; i++)
  {
    FDK_check_s16_overflow((A64_S) -Src[i]);
    Dst[i] = -Src[i];
  }
}

static void inline FDK_vqneg_s32_q (A64_V dst, A64_V src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQNeg32(Src[i]);
}
static void inline FDK_vqneg_s32_d (A64_X dst, A64_X src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fQNeg32(Src[i]);
}
static void inline FDK_vqneg_s16_q (A64_V dst, A64_V src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQNeg16(Src[i]);
}
static void inline FDK_vqneg_s16_d (A64_X dst, A64_X src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fQNeg16(Src[i]);
}
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_vabs_q( size, dst, src)  "VABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vabs_d( size, dst, src)  "VABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqabs_q(size, dst, src)  "VQABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqabs_d(size, dst, src)  "VQABS.S" #size " " #dst ", " #src " \n\t"
#else
#define FDK_vabs_q(size, dst, src) { if (size == 32)  FDK_vabs_s32_q(dst, src);   else if (size == 16) FDK_vabs_s16_q(dst, src); }
#define FDK_vabs_d(size, dst, src) { if (size == 32)  FDK_vabs_s32_d(dst, src);   else if (size == 16) FDK_vabs_s16_d(dst, src); }
#define FDK_vqabs_q(size, dst, src){ if (size == 32)  FDK_vqabs_s32_q(dst, src);  else if (size == 16) FDK_vqabs_s16_q(dst, src); }
#define FDK_vqabs_d(size, dst, src){ if (size == 32)  FDK_vqabs_s32_d(dst, src);  else if (size == 16) FDK_vqabs_s16_d(dst, src); }

static void inline FDK_vabs_s32_q(A64_V dst, A64_V src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s32_d(A64_X dst, A64_X src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s16_q(A64_V dst, A64_V src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 8; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s16_d(A64_X dst, A64_X src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vqabs_s32_q(A64_V dst, A64_V src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S*) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQAbs32(Src[i]);
}
static void inline FDK_vqabs_s32_d(A64_X dst, A64_X src)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fQAbs32(Src[i]);
}
static void inline FDK_vqabs_s16_q(A64_V dst, A64_V src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 8; i++)
    Dst[i] = fQAbs16(Src[i]);
}
static void inline FDK_vqabs_s16_d(A64_X dst, A64_X src)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQAbs16(Src[i]);
}
#endif


#ifndef __ARM_AARCH64_NEON__
static INT64 FDK_sat_shl_s64(INT64 val, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 64);
  if (val && immediate)
  {
     INT64 valPos = (val < (INT64) 0) ? ~val : val;
     FIXP_DBL valH = (FIXP_DBL) (valPos >> 32);
     FIXP_DBL valL = (FIXP_DBL) (valPos);


     INT lbits = valH ? CountLeadingBits(valH) : 32;
     if (lbits == 32)
     {   // check also lower part
       if (valL >= 0)
       {
         lbits += valL ? CountLeadingBits(valL) : 31;
       }
       else
       {
         // upper part shows 0x00000000, lower part starts with binary '1', shift is limited to 31 then
         lbits--;
       }
     }
     if (immediate > lbits)
     {
       if (val > 0)  val = MAXVAL_INT64;
       else          val = MINVAL_INT64;
     }
     else
     {
       val <<= immediate;
     }
  }
  return val;
}
static A64_S FDK_sat_shl_s32(A64_S val, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = (A64_S) 0x7FFFFFFF;
       else          val = (A64_S) 0x80000000;
     }
     else
     {
       val <<= immediate;
     }
  }
  return val;
}
static A64_H FDK_sat_shl_s16(A64_H val, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 16);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = A64_H(0x7FFF);
       else          val = A64_H(-0x8000);
     }
     else
     {
       val <<= immediate;
     }
  }
  return val;
}
static SCHAR FDK_sat_shl_s8(SCHAR val, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate <  8);
  if (val && immediate)
  {
    INT lbits = CountLeadingBits((LONG)val) - 24;
    if (immediate > lbits)
    {
      if (val > 0)  val = SCHAR(0x7F);
      else          val = SCHAR(-0x80);
    }
    else
    {
      val <<= immediate;
    }
  }
  return val;
}

static INT64 FDK_sat_sshl_s64(INT64 val, INT immediate)
{
  FDK_ASSERT(immediate > -64);
  FDK_ASSERT(immediate < 64);
  if (val && immediate)
  {
     INT64 valPos = (val < (INT64) 0) ? ~val : val;
     FIXP_DBL valH = (FIXP_DBL) (valPos >> 32);
     FIXP_DBL valL = (FIXP_DBL) (valPos);


     INT lbits = valH ? CountLeadingBits(valH) : 32;
     if (lbits == 32)
     {   // check also lower part
       if (valL >= 0)
       {
         lbits += valL ? CountLeadingBits(valL) : 31;
       }
       else
       {
         // upper part shows 0x00000000, lower part starts with binary '1', shift is limited to 31 then
         lbits--;
       }
     }
     if (immediate > lbits)
     {
       if (val > 0)  val = MAXVAL_INT64;
       else          val = MINVAL_INT64;
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}
static A64_S FDK_sat_sshl_s32(A64_S val, INT immediate)
{
  FDK_ASSERT(immediate > -32);
  FDK_ASSERT(immediate < 32);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = (A64_S) 0x7FFFFFFF;
       else          val = (A64_S) 0x80000000;
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}
static A64_H FDK_sat_sshl_s16(A64_H val, INT immediate)
{
  FDK_ASSERT(immediate > -16);
  FDK_ASSERT(immediate < 16);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = A64_H(0x7FFF);
       else          val = A64_H(-0x8000);
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}
static SCHAR FDK_sat_sshl_s8(SCHAR val, INT immediate)
{
  FDK_ASSERT(immediate > -8);
  FDK_ASSERT(immediate <  8);
  if (val && immediate)
  {
    INT lbits = CountLeadingBits((LONG)val) - 24;
    if (immediate > lbits)
    {
      if (val > 0)  val = SCHAR(0x7F);
      else          val = SCHAR(-0x80);
    }
    else
    {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
    }
  }
  return val;
}

static A64_S FDK_sat_sshl_rnd_s32(A64_S val, INT immediate)
{
  FDK_ASSERT(immediate > -32);
  FDK_ASSERT(immediate < 32);
  if (val && immediate)
  {
    INT lbits = CountLeadingBits(val);
    if (immediate > lbits)
    {
      if (val > 0)  val = (A64_S)0x7FFFFFFF;
      else          val = (A64_S)0x80000000;
    }
    else
    {
      if (immediate >= 0)
        val <<= immediate;
      else
      {
        /* round result, when shifting right */
        val >>= (-immediate - 1);
        val = (val + 1) >> 1;
      }
    }
  }
  return val;
}

static A64_S FDK_sat_add_s32(A64_S val1, A64_S val2)
{
  A64_S retval = val1 + val2;
  if ((val1 ^ val2) >= 0)
  {
    // both values have the same sign, check, if sign has changed
    if ((retval ^ val1) < 0)
    {
      // sign has changed
      if (val1 > 0)
        retval = MAXVAL_DBL;
      else
        retval = MINVAL_DBL;
    }
  }
  return retval;
}

static A64_S FDK_sat_sub_s32(A64_S val1, A64_S val2)
{
  A64_S retval = val1 - val2;
  if ((val1 ^ val2) < 0)
  {
    // values have different sign, check, if sign has changed
    if ((retval ^ val1) < 0)
    {
      // sign has changed
      if (val1 > 0)
        retval = MAXVAL_DBL;
      else
        retval = MINVAL_DBL;
    }
  }
  return retval;
}
static INT64 FDK_sat_add_s64(INT64 val1, INT64 val2)
{
  INT64 retval = val1 + val2;
  if ((val1 ^ val2) >= 0)
  {
    // both values have the same sign, check, if sign has changed compared to val1
    // example: 0x3000 + 0x5000 = 0x8000 -> 0x7FFF
    // example: 0xD000 + 0xAFFF = 0x7FFF -> 0x8000
    if ((retval ^ val1) < 0)
    {
      // sign has changed
      if (val1 > 0) retval = MAXVAL_INT64;
      else          retval = MINVAL_INT64;
    }
  }
  return retval;
}
static INT64 FDK_sat_sub_s64(INT64 val1, INT64 val2)
{
  INT64 retval = val1 - val2;
  if ((val1 ^ val2) < 0)
  {
    // values have different sign, check, if sign has changed compared to val1
    // example: 0x4000 - 0xC000 = 0x8000 -> 0x7FFF
    // example: 0xC000 - 0x4001 = 0x7FFF -> 0x8000
    if ((retval ^ val1) < 0)
    {
      // sign has changed
      if (val1 > 0) retval = MAXVAL_INT64;
      else          retval = MINVAL_INT64;
    }
  }
  return retval;
}
#endif

/* reference: ARM Cortex-A Series Version 1.0 Programmer's Guide for ARMv8-A */
/* chapter 5.7.3  Data Movement                                              */
#ifdef __ARM_AARCH64_NEON__
#define __A64_dup_Wt(dst,src)                  "DUP " #dst ", " #src "\n\t"
#define __A64_dup_lane(dst, src, lane)         "DUP " #dst ", " #src "[" #lane "]\n\t"
#define A64_dup_Wt(width,size,dst,src)         __A64_dup_Wt(dst,src)
#define A64_dup_lane(width,size,dst,src,lane)  __A64_dup_lane(dst, src, lane)
#else
/* DUP Vd.<T>, Wn   where <T> in 8B, 16B, 4H, 8H, 2S, 4S */
#define A64_dup_Wt(width,size,dst,src)          __A64_dup_Wt(  (INT64) width, (INT64) size, (INT64 &) dst, (INT *)  &src);
#define A64_dup_lane(width,size,dst,src,lane)   __A64_dup_lane((INT64) width, (INT64) size, (INT64 &) dst, (INT64 &)src, (INT) lane);

static void __A64_dup_Wt(INT64 width, INT64 size, INT64 &dst, INT *src)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    FDK_ASSERT((size == 128) || (width == 64) || (width == 8));
    INT i, num = (INT) size / (INT) width;
    INT value = *src;
extern INT* __WZR;
    if (src == (INT *) __WZR)
    {
      /* Reading XZR results in zero */
      value = (INT) 0;
    }
    switch (width)
    {
      case 32:
      {
        INT *Dst = (INT *) dst;
        for (i = 0; i < num; i++)
        {
            Dst[i] = (INT) value;
        }
      }
      break;
      case 16:
      {
        SHORT *Dst = (SHORT *) dst;
        for (i = 0; i < num; i++)
        {
            Dst[i] = (SHORT) value;
        }
      }
      break;
      case 8:
      {
        SCHAR *Dst = (SCHAR *) dst;
        for (i = 0; i < num; i++)
        {
            Dst[i] = (SCHAR) value;
        }
      }
      break;
    }
}


static void __A64_dup_lane(INT64 width, INT64 size, INT64 &dst, INT64 &src, INT lane)
{
    FDK_ASSERT((width == 32) || (width == 16) || (width == 8));
    FDK_ASSERT((size == 128) || (width == 64) || (width == 8));
    INT i, num = (INT) size / (INT) width;
    FDK_ASSERT((lane >= 0) && (lane <= (num - 1)));
    switch (width)
    {
      case 64:
      {
        INT64 *Src = (INT64 *) src;
        INT64 *Dst = (INT64 *) dst;
        INT64 val = Src[lane];
        for (i = 0; i < num; i++)
        {
            Dst[i] = (INT64) val;
        }
      }
      break;
      case 32:
      {
        INT *Src = (INT *) src;
        INT *Dst = (INT *) dst;
        INT val = Src[lane];
        for (i = 0; i < num; i++)
        {
            Dst[i] = (INT) val;
        }
      }
      break;
      case 16:
      {
        SHORT *Src = (SHORT *) src;
        SHORT *Dst = (SHORT *) dst;
        SHORT val = Src[lane];
        for (i = 0; i < num; i++)
        {
            Dst[i] = (SHORT) val;
        }
      }
      break;
      case 8:
      {
        SCHAR *Src = (SCHAR *) src;
        SCHAR *Dst = (SCHAR *) dst;
        SCHAR val = Src[lane];
        for (i = 0; i < num; i++)
        {
            Dst[i] = (SCHAR) val;
        }
      }
      break;
    }
}


#endif

#ifdef __ARM_AARCH64_NEON__
//#define FDK_vqshl_s8_imm( size, dst, src, immediate)   "VQSHL.s8  " #dst ", " #src ", # " #immediate " \n\t"
  #define FDK_vqshl_s16_imm(size, dst, src, immediate)   "VQSHL.s16 " #dst ", " #src ", # " #immediate " \n\t"
  #define FDK_vqshl_s32_imm(size, dst, src, immediate)   "VQSHL.s32 " #dst ", " #src ", # " #immediate " \n\t"
//#define FDK_vqshl_s64_imm(size, dst, src, immediate)   "VQSHL.s64 " #dst ", " #src ", # " #immediate " \n\t"
//#define FDK_vqshl_s8( size, dst, src1, src2)           "VQSHL.s8  " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vqshl_s16(size, dst, src1, src2)           "VQSHL.s16 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vqshl_s32(size, dst, src1, src2)           "VQSHL.s32 " #dst ", " #src1 ", " #src2 " \n\t"
//#define FDK_vqshl_s64(size, dst, src1, src2)           "VQSHL.s64 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vqshl_s32(size, dst, src1, src2)           { if (size == 128) { FDK_vqshl_q_s32((A64_V) dst, (A64_V) src1, (A64_V) src2); }   else if (size == 64) { FDK_vqshl_d_s32((A64_X) dst, (A64_X) src1, (A64_X) src2); } }
#define FDK_vqshl_s32_imm(size, dst, src, immediate)   { if (size == 128) { FDK_vqshl_q_s32_imm((A64_V) dst, (A64_V) src, immediate); }    else if (size == 64) { FDK_vqshl_d_s32_imm((A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vqshl_s16(size, dst, src1, src2)           { if (size == 128) { FDK_vqshl_q_s16((A64_V) dst, (A64_V) src1, (A64_V) src2); }   else if (size == 64) { FDK_vqshl_d_s16((A64_X) dst, (A64_X) src1, (A64_X) src2); } }
#define FDK_vqshl_s16_imm(size, dst, src, immediate)   { if (size == 128) { FDK_vqshl_q_s16_imm((A64_V) dst, (A64_V) src, immediate); }    else if (size == 64) { FDK_vqshl_d_s16_imm((A64_X) dst, (A64_X) src, immediate); } }

static void inline FDK_vqshl_q_s32(A64_V dst, A64_V src1, A64_V src2)
{
  A64_S *Dst  = (A64_S *) dst;
  A64_S *Src1 = (A64_S *) src1;
  A64_S *Src2 = (A64_S *) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_d_s32(A64_X dst, A64_X src1, A64_X src2)
{
  A64_S *Dst  = (A64_S *) dst;
  A64_S *Src1 = (A64_S *) src1;
  A64_S *Src2 = (A64_S *) src2;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_q_s32_imm (A64_V dst, A64_V src, INT immediate)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src[i],immediate);
  }
}
static void inline FDK_vqshl_d_s32_imm (A64_X dst, A64_X src, INT immediate)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src[i],immediate);
  }
}

static void inline FDK_vqshl_q_s16(A64_V dst, A64_V src1, A64_V src2)
{
  A64_H *Dst  = (A64_H *) dst;
  A64_H *Src1 = (A64_H *) src1;
  A64_H *Src2 = (A64_H *) src2;
  for (int i = 0; i < 8; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_d_s16(A64_X dst, A64_X src1, A64_X src2)
{
  A64_H *Dst  = (A64_H *) dst;
  A64_H *Src1 = (A64_H *) src1;
  A64_H *Src2 = (A64_H *) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_q_s16_imm (A64_V dst, A64_V src, INT immediate)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src[i],immediate);
  }
}
static void inline FDK_vqshl_d_s16_imm (A64_X dst, A64_X src, INT immediate)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src[i],immediate);
  }
}

#endif

// reference: NEON and VFP programming, chapter 4.5.5: VLSI and VRSI
#ifdef __ARM_AARCH64_NEON__
#define FDK_vsri_8( size, dst, src, immediate)    "VSRI.8  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_16(size, dst, src, immediate)    "VSRI.16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_32(size, dst, src, immediate)    "VSRI.32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_64(size, dst, src, immediate)    "VSRI.64 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_8( size, dst, src, immediate)    "VSLI.8  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_16(size, dst, src, immediate)    "VSLI.16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_32(size, dst, src, immediate)    "VSLI.32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_64(size, dst, src, immediate)    "VSLI.64 " #dst ", " #src ", # " #immediate " \n\t"
#else
#define FDK_vsri_8( size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(8, (A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsri_d(8, (A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsri_16(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(16,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsri_d(16,(A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsri_32(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(32,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsri_d(32,(A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsri_64(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(64,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsri_d(64,(A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsli_8( size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(8, (A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsli_d(8, (A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsli_16(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(16,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsli_d(16,(A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsli_32(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(32,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsli_d(32,(A64_X) dst, (A64_X) src, immediate); } }
#define FDK_vsli_64(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(64,(A64_V) dst, (A64_V) src, immediate); }  else if (size == 64) { FDK_vsli_d(64,(A64_X) dst, (A64_X) src, immediate); } }

static inline void FDK_vsri_q(INT size, A64_V dst, A64_V src, int immediate)
{
  FDK_ASSERT(immediate >= 1);
  FDK_ASSERT(immediate <= size);
  switch (size)
  {
    case 8:
    {
      A64_B Dst = (A64_B) dst;
      A64_B Src = (A64_B) src;
      UCHAR mask = (UCHAR) (0x7F >> (immediate-1));
      for (int i = 0; i < 16; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 16:
    {
      A64_H *Dst = (A64_H *) dst;
      A64_H *Src = (A64_H *) src;
      A64_H mask = (A64_H) (0x7FFF >> (immediate-1));
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 32:
    {
      A64_S *Dst = (A64_S *) dst;
      A64_S *Src = (A64_S *) src;
      A64_S mask = (A64_S) (0x7FFFFFFF >> (immediate-1));
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 64:
    {
      A64_X Dst = (A64_X) dst;
      A64_X Src = (A64_X) src;
      INT64 mask = (INT64) (0x7FFFFFFFFFFFFFFF >> (immediate-1));
      for (int i = 0; i < 2; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
    }
    break;
  }
}

static inline void FDK_vsri_d(INT size, A64_X dst, A64_X src, int immediate)
{
  FDK_ASSERT(immediate >= 1);
  FDK_ASSERT(immediate <= size);
  switch (size)
  {
    case 8:
    {
      A64_B Dst = (A64_B) dst;
      A64_B Src = (A64_B) src;
      UCHAR mask = (UCHAR) (0x7F >> (immediate-1));
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 16:
    {
      A64_H *Dst = (A64_H *) dst;
      A64_H *Src = (A64_H *) src;
      A64_H mask = (A64_H) (0x7FFF >> (immediate-1));
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 32:
    {
      A64_S *Dst = (A64_S *) dst;
      A64_S *Src = (A64_S *) src;
      A64_S mask = (A64_S) (0x7FFFFFFF >> (immediate-1));
      for (int i = 0; i < 2; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 64:
    {
      A64_X Dst = (A64_X) dst;
      A64_X Src = (A64_X) src;
      INT64 mask = (INT64) (0x7FFFFFFFFFFFFFFF >> (immediate-1));
      for (int i = 0; i < 1; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
    }
    break;
  }
}
#endif

// reference: NEON and VFP programming, chapter 4.4.8: VSWP
#ifdef __ARM_AARCH64_NEON__
#define FDK_vswp(size, v0, v1)    "VSWP " #v0 ", " #v1 " \n\t"
#else
#define FDK_vswp(size, v0, v1)   { if (size == 128) { FDK_vswp_q(v0, v1); } else if (size == 64) { FDK_vswp_d(v0,v1); } }

static void inline FDK_vswp_q (A64_V q0, A64_V q1)
{
  A64_S tmp;
  tmp = q1[0]; q1[0] = q0[0]; q0[0] = tmp;
  tmp = q1[1]; q1[1] = q0[1]; q0[1] = tmp;
  tmp = q1[2]; q1[2] = q0[2]; q0[2] = tmp;
  tmp = q1[3]; q1[3] = q0[3]; q0[3] = tmp;
}
static void inline FDK_vswp_d (A64_S *d0, A64_S *d1)
{
  A64_S tmp;
  tmp = d1[0]; d1[0] = d0[0]; d0[0] = tmp;
  tmp = d1[1]; d1[1] = d0[1]; d0[1] = tmp;
}
#endif

// reference: NEON and VFP programming, chapter 4.3.1: VAND, VBIC, VEOR, VORN, VORR (register)
#ifdef __ARM_AARCH64_NEON__
#define FDK_vand(size, dst, src1, src2)   "VAND " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vbic(size, dst, src1, src2)   "VBIC " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_veor(size, dst, src1, src2)   "VEOR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vorr(size, dst, src1, src2)   "VORR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vorn(size, dst, src1, src2)   "VORN " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vand(size, dst,  src1, src2)   __FDK_vand((INT)size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vbic(size, dst,  src1, src2)   __FDK_vbic((INT)size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_veor(size, dst,  src1, src2)   __FDK_veor((INT)size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vorr(size, dst,  src1, src2)   __FDK_vorr((INT)size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vorn(size, dst,  src1, src2)   __FDK_vorn((INT)size, (A64_S) dst, (A64_S) src1, (A64_S) src2);

static inline void __FDK_vand(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] & src2[i];
  }
}
static inline void __FDK_vbic(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] & (~src2[i]);
  }
}
static inline void __FDK_veor(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] ^ src2[i];
  }
}
static inline void __FDK_vorr(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] | src2[i];
  }
}
static inline void __FDK_vorn(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] | (~src2[i]);
  }
}
#endif //__ARM_AARCH64_NEON__


#ifdef __ARM_AARCH64_NEON__
#define FDK_vmax_s32(size, dst, src1, src2)     "VMAX.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmax_s16(size, dst, src1, src2)     "VMAX.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmin_s32(size, dst, src1, src2)     "VMIN.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmin_s16(size, dst, src1, src2)     "VMIN.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmax_s32(size, dst, src1, src2)    "VPMAX.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmax_s16(size, dst, src1, src2)    "VPMAX.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmin_s32(size, dst, src1, src2)    "VPMIN.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmin_s16(size, dst, src1, src2)    "VPMIN.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vmax_s32(size, dst,  src1, src2)   __FDK_vmax_s32(size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vmax_s16(size, dst,  src1, src2)   __FDK_vmax_s16(size, (A64_H) dst, (A64_H) src1, (A64_H) src2);
#define FDK_vmin_s32(size, dst,  src1, src2)   __FDK_vmin_s32(size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vmin_s16(size, dst,  src1, src2)   __FDK_vmin_s16(size, (A64_H) dst, (A64_H) src1, (A64_H) src2);
#define FDK_vpmax_s32(size, dst,  src1, src2)  __FDK_vpmax_s32(size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vpmax_s16(size, dst,  src1, src2)  __FDK_vpmax_s16(size, (A64_H) dst, (A64_H) src1, (A64_H) src2);
#define FDK_vpmin_s32(size, dst,  src1, src2)  __FDK_vpmin_s32(size, (A64_S) dst, (A64_S) src1, (A64_S) src2);
#define FDK_vpmin_s16(size, dst,  src1, src2)  __FDK_vpmin_s16(size, (A64_H) dst, (A64_H) src1, (A64_H) src2);

static inline void __FDK_vmax_s32(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = fMax(src1[i], src2[i]);
  }
}
static inline void __FDK_vmax_s16(INT size, A64_H *dst, A64_H *src1, A64_H *src2)
{
  for (int i = 0; i < (size>>4); i++)
  {
    dst[i] = fMax(src1[i], src2[i]);
  }
}
static inline void __FDK_vmin_s32(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = fMin(src1[i], src2[i]);
  }
}
static inline void __FDK_vmin_s16(INT size, A64_H *dst, A64_H *src1, A64_H *src2)
{
  for (int i = 0; i < (size>>4); i++)
  {
  dst[i] = fMin(src1[i], src2[i]);
  }
}

static inline void __FDK_vpmax_s32(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  A64_S tmp[4];
  int i;
  for (i = 0; i < (size>>6); i++)
  {
    tmp[i] = fMax(src1[2*i], src1[2*i+i]);
  }
  for (; i < (size>>5); i++)
  {
    tmp[i] = fMax(src2[2*i], src2[2*i+i]);
  }
  for (i = 0; i < (size>>5); i++)
  {
    dst[i] = tmp[i];
  }
}

static inline void __FDK_vpmin_s32(INT size, A64_S *dst, A64_S *src1, A64_S *src2)
{
  A64_S tmp[4];
  int i;
  for (i = 0; i < (size>>6); i++)
  {
    tmp[i] = fMin(src1[2*i], src1[2*i+i]);
  }
  for (; i < (size>>5); i++)
  {
    tmp[i] = fMin(src2[2*i], src2[2*i+i]);
  }
  for (i = 0; i < (size>>5); i++)
  {
    dst[i] = tmp[i];
  }
}
static inline void __FDK_vpmax_s16(INT size, A64_H *dst, A64_H *src1, A64_H *src2)
{
  A64_H tmp[8];
  int i;
  for (i = 0; i < (size>>5); i++)
  {
    tmp[i] = fMax(src1[2*i], src1[2*i+i]);
  }
  for (; i < (size>>4); i++)
  {
    tmp[i] = fMax(src2[2*i], src2[2*i+i]);
  }
  for (i = 0; i < (size>>4); i++)
  {
    dst[i] = tmp[i];
  }
}

static inline void __FDK_vpmin_s16(INT size, A64_H *dst, A64_H *src1, A64_H *src2)
{
  A64_H tmp[8];
  int i;
  for (i = 0; i < (size>>5); i++)
  {
    tmp[i] = fMin(src1[2*i], src1[2*i+i]);
  }
  for (; i < (size>>4); i++)
  {
    tmp[i] = fMin(src2[2*i], src2[2*i+i]);
  }
  for (i = 0; i < (size>>4); i++)
  {
    dst[i] = tmp[i];
  }
}
#endif //__ARM_AARCH64_NEON__


#ifdef __ARM_AARCH64_NEON__
#define __A64_smax(dst, src1, src2)               "SMAX " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_smin(dst, src1, src2)               "SMIN " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_smax(width, size, dst, src1, src2)    __A64_smax(dst, src1, src2)
#define A64_smin(width, size, dst, src1, src2)    __A64_smin(dst, src1, src2)
#else
#define A64_smax(width, size, dst, src1, src2)    __A64_smax((INT) width, (INT) size, (INT64 &)dst, (INT64 &)src1, (INT64 &)src2);
#define A64_smin(width, size, dst, src1, src2)    __A64_smin((INT) width, (INT) size, (INT64 &)dst, (INT64 &)src1, (INT64 &)src2);

static inline void __A64_smax(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  INT i, num = size / width;

  switch (width)
  {
    case 64:
    {
      INT64* Src1 = (INT64*)src1;
      INT64* Src2 = (INT64*)src2;
      INT64* Dst = (INT64*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = FDKmax(Src1[i], Src2[i]);
        }
    break;
    case 32:
    {
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      INT* Dst = (INT*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMax(Src1[i], Src2[i]);
    }
    break;
    case 16:
    {
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      SHORT* Dst = (SHORT*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMax(Src1[i], Src2[i]);
    }
    break;
    case 8:
    {
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      SCHAR* Dst = (SCHAR*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMax(Src1[i], Src2[i]);
    }
    break;
  }
}

static inline void __A64_smin(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  INT i, num = size / width;

  switch (width)
  {
    case 64:
    {
      INT64* Src1 = (INT64*)src1;
      INT64* Src2 = (INT64*)src2;
      INT64* Dst = (INT64*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = FDKmin(Src1[i], Src2[i]);
        }
    break;
    case 32:
    {
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      INT* Dst = (INT*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMin(Src1[i], Src2[i]);
    }
    break;
    case 16:
    {
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      SHORT* Dst = (SHORT*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMin(Src1[i], Src2[i]);
    }
    break;
    case 8:
    {
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      SCHAR* Dst = (SCHAR*)dst;
      for (i = 0; i < num; i++)
        Dst[i] = fMin(Src1[i], Src2[i]);
    }
    break;
  }
}
#endif /* __ARM_AARCH64_NEON__ */


#ifdef __ARM_AARCH64_NEON__
#define FDK_vsub_s32_q(dst, src1, src2)   "VSUB.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_s32_d(dst, src1, src2)   "VSUB.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vsub_s32_q(dst, src1, src2)   __FDK_vsub_s32_q(dst, src1, src2);
#define FDK_vsub_s32_d(dst, src1, src2)   __FDK_vsub_s32_d(dst, src1, src2);
static inline void __FDK_vsub_s32_q(A64_V dst, A64_V src1, A64_V src2)
{
  for (int i = 0; i < 4; i++)
  {
    FDK_check_s32_overflow((INT64) src1[i] - (INT64) src2[i]);
    dst[i] = src1[i] - src2[i];
  }
}
static inline void __FDK_vsub_s32_d(A64_S *dst, A64_S *src1, A64_S *src2)
{
  for (int i = 0; i < 2; i++)
  {
    FDK_check_s32_overflow((INT64) src1[i] - (INT64) src2[i]);
    dst[i] = src1[i] - src2[i];
  }
}
#endif //__ARM_AARCH64_NEON__

#ifdef __ARM_AARCH64_NEON__
#define FDK_vsub_s64_q(dst, src1, src2)   "VSUB.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_s64_d(dst, src1, src2)   "VSUB.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vsub_s64_q(dst, src1, src2)   __FDK_vsub_s64_q(dst, src1, src2);
#define FDK_vsub_s64_d(dst, src1, src2)   __FDK_vsub_s64_d(dst, src1, src2);

static inline void __FDK_vsub_s64_q(A64_V dst, A64_V src1, A64_V src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] - Src2[0];
  Dst[1] = Src1[1] - Src2[1];
}
static inline void __FDK_vsub_s64_d(A64_X dst, A64_X src1, A64_X src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] - Src2[0];
}
#endif //__ARM_AARCH64_NEON__


#ifdef __ARM_AARCH64_NEON__
#define __A64_sqadd(dst, src1, src2)             " SQADD " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_sqadd(width, size, dst, src1, src2)  __A64_sqadd(dst, src1, src2)
#define __A64_sqsub(dst, src1, src2)             " SQSUB " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_sqsub(width, size, dst, src1, src2)  __A64_sqsub(dst, src1, src2)
#else
#define A64_sqadd(width, size, dst, src1, src2)   __A64_sqadd(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_sqsub(width, size, dst, src1, src2)   __A64_sqsub(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);

static inline void __A64_sqadd(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((width == 8) || (width == 16) || (width == 32));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:
    {
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT *Dst  = (INT *) dst;
      for (i = 0; i < num; i++) 
      {
        INT64 value = (INT64) Src1[i] + (INT64) Src2[i];
        
        if (value > (INT64) 0x7FFFFFFF) {
          Dst[i] = (INT)0x7FFFFFFF;
        }
        else if (-value > (INT64) 0x80000000) {
          Dst[i] = (INT)0x80000000;
        }
        else {
          Dst[i] = Src1[i] + Src2[i];
        }
      }
    }
    break;
    case 16:
    {
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT *Dst  = (SHORT *) dst;
      for (i = 0; i < num; i++) 
      {
        INT value = (INT) Src1[i] + (INT) Src2[i];
        
        if (value > (INT) 0x7FFF) {
          Dst[i] = (SHORT)0x7FFF;
        }
        else if (-value > (INT) 0x8000) {
          Dst[i] = (SHORT)0x8000;
        }
        else {
          Dst[i] = Src1[i] + Src2[i];
        }
      }
    }
    break;
    case 8:
    {
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR *Dst  = (SCHAR *) dst;
      for (i = 0; i < num; i++) 
      {
        INT value = (INT) Src1[i] + (INT) Src2[i];
        
        if (value > (INT) 0x7F) {
          Dst[i] = (SCHAR)0x7F;
        }
        else if (-value > (INT) 0x80) {
          Dst[i] = (SCHAR)0x80;
        }
        else {
          Dst[i] = Src1[i] + Src2[i];
        }
      }
    }
    break;
  }
}
static inline void __A64_sqsub(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((width == 8) || (width == 16) || (width == 32));
  INT num = size / width;
  INT i;
  switch (width)
  {
  case 32:
  {
    INT* Src1 = (INT*)src1;
    INT* Src2 = (INT*)src2;
    INT* Dst = (INT*)dst;
    for (i = 0; i < num; i++)
    {
      INT64 value = (INT64)Src1[i] - (INT64)Src2[i];

      if (value > (INT64) 0x7FFFFFFF) {
        Dst[i] = (INT)0x7FFFFFFF;
      }
      else if (-value > (INT64)0x80000000) {
        Dst[i] = (INT)0x80000000;
      }
      else {
        Dst[i] = Src1[i] - Src2[i];
}
      }
    }
  break;
  case 16:
  {
    SHORT* Src1 = (SHORT*)src1;
    SHORT* Src2 = (SHORT*)src2;
    SHORT* Dst = (SHORT*)dst;
    for (i = 0; i < num; i++)
    {
      INT value = (INT)Src1[i] - (INT)Src2[i];

      if (value > (INT) 0x7FFF) {
        Dst[i] = (SHORT)0x7FFF;
      }
      else if (-value > (INT)0x8000) {
        Dst[i] = (SHORT)0x8000;
      }
      else {
        Dst[i] = Src1[i] - Src2[i];
      }
    }
  }
  break;
  case 8:
  {
    SCHAR* Src1 = (SCHAR*)src1;
    SCHAR* Src2 = (SCHAR*)src2;
    SCHAR* Dst = (SCHAR*)dst;
    for (i = 0; i < num; i++)
    {
      INT value = (INT)Src1[i] - (INT)Src2[i];

      if (value > (INT) 0x7F) {
        Dst[i] = (SCHAR)0x7F;
      }
      else if (-value > (INT)0x80) {
        Dst[i] = (SCHAR)0x80;
      }
      else {
        Dst[i] = Src1[i] - Src2[i];
      }
    }
  }
  break;
  }
}
#endif /* __ARM_AARCH64_NEON__ */

#ifdef __ARM_AARCH64_NEON__
#define __A64_shadd(dst, src1, src2)   "SHADD " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_shsub(dst, src1, src2)   "SHSUB " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_add(dst, src1, src2)     "ADD   " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_sub(dst, src1, src2)     "SUB   " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_and(dst, src1, src2)     "AND   " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_eor(dst, src1, src2)     "EOR   " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_addv(dst,src)            "ADDV  " #dst ", " #src "\n\t"
#define __A64_sminv(dst,src)           "SMINV " #dst ", " #src "\n\t"

#define   A64_shadd(width, size, dst, src1, src2)  __A64_shadd(dst, src1, src2)
#define   A64_shsub(width, size, dst, src1, src2)  __A64_shsub(dst, src1, src2)
#define   A64_add(  width, size, dst, src1, src2)  __A64_add(  dst, src1, src2)
#define   A64_sub(  width, size, dst, src1, src2)  __A64_sub(  dst, src1, src2)
/* __A64_sub(dst, src1, src2) */
#define   A64_and(  width, size, dst, src1, src2)  __A64_and(  dst, src1, src2)
#define   A64_eor(  width, size, dst, src1, src2)  __A64_eor(  dst, src1, src2)
#define   A64_addv( width, size, dst, src)         __A64_addv( dst, src)
#define   A64_sminv(width, size, dst, src)         __A64_sminv(dst, src)

#else
#define A64_shadd(width, size, dst, src1, src2)   __A64_shadd(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_shsub(width, size, dst, src1, src2)   __A64_shsub(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_add(  width, size, dst, src1, src2)   __A64_add(  width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_sub(  width, size, dst, src1, src2)   __A64_sub(  width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_and(  width, size, dst, src1, src2)   __A64_and(  width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_eor(  width, size, dst, src1, src2)   __A64_eor(  width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_addv( width, size, dst, src)          __A64_addv( width, size, (SCHAR *) dst, (INT64 &) src);
#define A64_sminv(width, size, dst, src)          __A64_sminv(width, size, (SCHAR *) dst, (INT64 &) src);

static inline void __A64_shadd(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:  /* Vt.2S or Vt.4S */
    {
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT *Dst  = (INT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (INT) (((INT64) Src1[i] + (INT64) Src2[i])>>1);
    }
      break;
    case 16:  /* Vt.4H or Vt.8H */
    {
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT *Dst  = (SHORT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SHORT) (((INT) Src1[i] + (INT) Src2[i])>>1);
    }
      break;
    case 8:   /* Vt.8B or Vt.16B */
    {
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR *Dst  = (SCHAR *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SCHAR) (((SHORT) Src1[i] + (SHORT) Src2[i])>>1);
    }
      break;
  }
}

static inline void __A64_shsub(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:  /* Vt.2S or Vt.4S */
    {
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT *Dst  = (INT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (INT) (((INT64) Src1[i] - (INT64) Src2[i])>>1);
    }
      break;
    case 16:  /* Vt.4H or Vt.8H */
    {
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT *Dst  = (SHORT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SHORT) (((INT) Src1[i] - (INT) Src2[i])>>1);
    }
      break;
    case 8:   /* Vt.8B or Vt.16B */
    {
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR *Dst  = (SCHAR *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SCHAR) (((SHORT) Src1[i] - (SHORT) Src2[i])>>1);
    }
      break;
  }
}

static inline void __A64_add(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:  /* Vt.2S or Vt.4S */
    {
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT *Dst  = (INT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (INT) (((INT64) Src1[i] + (INT64) Src2[i])>>0);
    }
      break;
    case 16:  /* Vt.4H or Vt.8H */
    {
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT *Dst  = (SHORT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SHORT) (((INT) Src1[i] + (INT) Src2[i])>>0);
    }
      break;
    case 8:   /* Vt.8B or Vt.16B */
    {
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR *Dst  = (SCHAR *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SCHAR) (((SHORT) Src1[i] + (SHORT) Src2[i])>>0);
    }
      break;
  }
}

static inline void __A64_sub(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:  /* Vt.2S or Vt.4S */
    {
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT *Dst  = (INT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (INT) (((INT64) Src1[i] - (INT64) Src2[i])>>0);
    }
      break;
    case 16:  /* Vt.4H or Vt.8H */
    {
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT *Dst  = (SHORT *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SHORT) (((INT) Src1[i] - (INT) Src2[i])>>0);
    }
      break;
    case 8:   /* Vt.8B or Vt.16B */
    {
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR *Dst  = (SCHAR *) dst;
      for (i = 0; i < num; i++)
          Dst[i] = (SCHAR) (((SHORT) Src1[i] - (SHORT) Src2[i])>>0);
    }
      break;
  }
}


static inline void __A64_and(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT(width == 8);
  INT num = size / width;
  INT i;

  UCHAR *Src1 = (UCHAR *) src1;
  UCHAR *Src2 = (UCHAR *) src2;
  UCHAR *Dst  = (UCHAR *) dst;
  for (i = 0; i < num; i++)
      Dst[i] = Src1[i] & Src2[i];
}

static inline void __A64_eor(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT(width == 8);
  INT num = size / width;
  INT i;

  UCHAR *Src1 = (UCHAR *) src1;
  UCHAR *Src2 = (UCHAR *) src2;
  UCHAR *Dst  = (UCHAR *) dst;
  for (i = 0; i < num; i++)
      Dst[i] = Src1[i] ^ Src2[i];
}

static inline void __A64_addv(INT width, INT size, SCHAR *dst, INT64 &src)
{
  FDK_ASSERT((size == 128) || (size == 64));
  INT num = size / width;
  INT i;
  switch (width)
  {
    case 32:  /* Vt.2S or Vt.4S */
    {
      INT *Src = (INT *) src;
      INT *Dst = (INT *) dst;
      INT val  = 0;
      for (i = 0; i < num; i++)
          val += Src[i];
      Dst[0] = val;
    }
      break;
    case 16:  /* Vt.4H or Vt.8H */
    {
      SHORT *Src = (SHORT *) src;
      SHORT *Dst = (SHORT *) dst;
      SHORT val = 0;
      for (i = 0; i < num; i++)
          val += Src[i];
      Dst[0] = val;
    }
      break;
    case 8:   /* Vt.8B or Vt.16B */
    {
      SCHAR *Src = (SCHAR *) src;
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR val = 0;
      for (i = 0; i < num; i++)
          val += Src[i];
      Dst[0] = val;
    }
      break;
  }
}

static inline void __A64_sminv(INT width, INT size, SCHAR* dst, INT64& src)
{
    FDK_ASSERT((size == 128) || (size == 64));
    INT num = size / width;
    INT i;
    switch (width)
    {
        case 32:  /* Vt.2S or Vt.4S */
        {
            INT* Src = (INT*)src;
            INT* Dst = (INT*)dst;
            INT val = MINVAL_DBL;
            for (i = 0; i < num; i++)
                val = fMin(val, Src[i]);
            Dst[0] = val;
        }
        break;
        case 16:  /* Vt.4H or Vt.8H */
        {
            SHORT* Src = (SHORT*)src;
            SHORT* Dst = (SHORT*)dst;
            SHORT val = (FIXP_SGL) 0x8000;
            for (i = 0; i < num; i++)
                val = fMin(val, Src[i]);
            Dst[0] = val;
        }
        break;
        case 8:   /* Vt.8B or Vt.16B */
        {
            SCHAR* Src = (SCHAR*)src;
            SCHAR* Dst = (SCHAR*)dst;
            SCHAR val = (SCHAR) 0x80;
            for (i = 0; i < num; i++)
                val = fMin(val, Src[i]);
            Dst[0] = val;
        }
        break;
    }
}


#endif //__ARM_AARCH64_NEON__


#ifdef __ARM_AARCH64_NEON__
#define FDK_vpadd_s32(dst, src1, src2)   "VPADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpadd_s16(dst, src1, src2)   "VPADD.s16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpadd_s8( dst, src1, src2)   "VPADD.s8  " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vpadd_s32(dst, src1, src2)   __FDK_vpadd(32, dst, src1, src2);
#define FDK_vpadd_s16(dst, src1, src2)   __FDK_vpadd(16, dst, src1, src2);
#define FDK_vpadd_s8( dst, src1, src2)   __FDK_vpadd(8,  dst, src1, src2);

static inline void __FDK_vpadd(INT size, A64_X dst, A64_X src1, A64_X src2)
{
  switch (size)
  {
    case 8:
    {
      A64_B Dst  = (A64_B) dst;
      A64_B Src1 = (A64_B) src1;
      A64_B Src2 = (A64_B) src2;
      char tmp[8];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src1[2] + Src1[3];
      tmp[2] = Src1[4] + Src1[5];
      tmp[3] = Src1[6] + Src1[7];
      tmp[4] = Src2[0] + Src2[1];
      tmp[5] = Src2[2] + Src2[3];
      tmp[6] = Src2[4] + Src2[5];
      tmp[7] = Src2[6] + Src2[7];
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = tmp[i];
      }
      break;
    }
    case 16:
    {
      A64_H *Dst  = (A64_H *) dst;
      A64_H *Src1 = (A64_H *) src1;
      A64_H *Src2 = (A64_H *) src2;
      A64_H tmp[4];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src1[2] + Src1[3];
      tmp[2] = Src2[0] + Src2[1];
      tmp[3] = Src2[2] + Src2[3];
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = tmp[i];
      }
      break;
    }
    case 32:
    {
      A64_S *Dst  = (A64_S *) dst;
      A64_S *Src1 = (A64_S *) src1;
      A64_S *Src2 = (A64_S *) src2;
      A64_S tmp[2];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src2[0] + Src2[1];
      Dst[0] = tmp[0];
      Dst[1] = tmp[1];
      break;
    }
  }
}
#endif //__ARM_AARCH64_NEON__


// Vector shift right by immediate
#ifdef __ARM_AARCH64_NEON__
#define FDK_vshr_s32_q_imm(dst, src, immediate)     "VSHR.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshr_s32_d_imm(dst, src, immediate)     "VSHR.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshr_s16_q_imm(dst, src, immediate)     "VSHR.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshr_s16_d_imm(dst, src, immediate)     "VSHR.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vqshrn_s64_imm(dst, src, immediate)     "VQSHRN.s64 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vqshrn_s32_imm(dst, src, immediate)     "VQSHRN.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vqshrn_s16_imm(dst, src, immediate)     "VQSHRN.s16 " #dst ", " #src ", # " #immediate " \n\t"
#else
#define FDK_vshr_s32_q_imm(dst, src, immediate)     __FDK_vshr_s32_q_imm((A64_V)dst,(A64_V)src, immediate);
#define FDK_vshr_s32_d_imm(dst, src, immediate)     __FDK_vshr_s32_d_imm((A64_X)dst,(A64_X)src, immediate);
#define FDK_vshr_s16_q_imm(dst, src, immediate)     __FDK_vshr_s16_q_imm((A64_V)dst,(A64_V)src, immediate);
#define FDK_vshr_s16_d_imm(dst, src, immediate)     __FDK_vshr_s16_d_imm((A64_X)dst,(A64_X)src, immediate);
#define FDK_vqshrn_s64_imm(dst, src, immediate)     __FDK_vqshrn_imm(64,(A64_X)dst,(A64_V)src, immediate);
#define FDK_vqshrn_s32_imm(dst, src, immediate)     __FDK_vqshrn_imm(32,(A64_X)dst,(A64_V)src, immediate);
#define FDK_vqshrn_s16_imm(dst, src, immediate)     __FDK_vqshrn_imm(16,(A64_X)dst,(A64_V)src, immediate);

static inline void __FDK_vshr_s32_d_imm(A64_X dst, A64_X src, INT immediate)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 2; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s32_q_imm(A64_V dst, A64_V src, INT immediate)
{
  A64_S *Dst = (A64_S *) dst;
  A64_S *Src = (A64_S *) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 4; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s16_d_imm(A64_X dst, A64_X src, INT immediate)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 16);
  for (int i = 0; i < 4; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s16_q_imm(A64_V dst, A64_V src, INT immediate)
{
  A64_H *Dst = (A64_H *) dst;
  A64_H *Src = (A64_H *) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 16);
  for (int i = 0; i < 8; i++)
    Dst[i] = Src[i]>>immediate;
}

static A64_S FDK_saturate_shr_narrow_s64(INT64 val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT64 valPos = (val < (INT64) 0) ? ~val : val;
    A64_S valH = (A64_S) (valPos >> 32);
    A64_S valL = (A64_S) (valPos);
    INT lbits = valH ? CountLeadingBits(valH) : 32;
    if (lbits == 32)
    {   // check also lower part
      if (valL >= 0)
      {
        lbits += valL ? CountLeadingBits(valL) : 31;
      }
      else
      {
        // upper part shows 0x00000000, lower part starts with binary '1', shift is limited to 31 then
        lbits--;
      }
    }
    if (lbits < 32)
    {
      if (val > 0)  val = (INT64) MAXVAL_DBL;
      else          val = (INT64) MINVAL_DBL;
    }
  }
  return (A64_S) val;
}
static A64_H FDK_saturate_shr_narrow_s32(A64_S val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT lbits = CountLeadingBits(val);
    if (lbits < 16)
    {
      if (val > 0)  val = (A64_S) MAXVAL_SGL;
      else          val = (A64_S) MINVAL_SGL;
    }
  }
  return (A64_H) val;
}
static SCHAR FDK_saturate_shr_narrow_s16(A64_H val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT lbits = CountLeadingBits(val);
    if (lbits < 8)
    {
      if (val > 0)  val = (A64_H)  127;
      else          val = (A64_H) -128;
    }
  }
  return (SCHAR) val;
}

static inline void __FDK_vqshrn_imm(INT size, A64_X dst, A64_V src, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < size);
  switch (size)
  {
    case 64:
    {
      INT64 *Src = (INT64 *) src;
      A64_S *Dst = (A64_S *) dst;
      A64_S tmp[2];
      for (int i = 0; i < 2; i++)
        tmp[i] = FDK_saturate_shr_narrow_s64(Src[i], immediate);
      for (int i = 0; i < 2; i++)
        Dst[i] = tmp[i];
      break;
    }
    case 32:
    {
      A64_S *Src = (A64_S *) src;
      A64_H *Dst = (A64_H *) dst;
      A64_H tmp[4];
      for (int i = 0; i < 4; i++)
        tmp[i] = FDK_saturate_shr_narrow_s32(Src[i], immediate);
      for (int i = 0; i < 4; i++)
        Dst[i] = tmp[i];
      break;
    }
    case 16:
    {
      A64_H *Src = (A64_H *) src;
      A64_B Dst  = (A64_B)   dst;
      SCHAR tmp[8];
      for (int i = 0; i < 8; i++)
        tmp[i] = FDK_saturate_shr_narrow_s16(Src[i], immediate);
      for (int i = 0; i < 8; i++)
        Dst[i] = tmp[i];
      break;
    }
  }
}
#endif /*  __ARM_AARCH64_NEON__ */


// Vector shift right by immediate and accumulate
#ifdef __ARM_AARCH64_NEON__
#define FDK_vsra_s32_q_imm(dst, src, immediate)   "VSRA.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsra_s32_d_imm(dst, src, immediate)   "VSRA.s32 " #dst ", " #src ", # " #immediate " \n\t"
#else
#define FDK_vsra_s32_q_imm(dst, src, immediate)   __FDK_vsra_s32_q_imm(dst, src, immediate);
#define FDK_vsra_s32_d_imm(dst, src, immediate)   __FDK_vsra_s32_d_imm(dst, src, immediate);

static inline void __FDK_vsra_s32_d_imm(A64_S *dst, A64_S *src, int immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  dst[0] += src[0]>>immediate;
  dst[1] += src[1]>>immediate;
}
static inline void __FDK_vsra_s32_q_imm(A64_V dst, A64_V src, int immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  dst[0] += src[0]>>immediate;
  dst[1] += src[1]>>immediate;
  dst[2] += src[2]>>immediate;
  dst[3] += src[3]>>immediate;
}
#endif /*  __ARM_AARCH64_NEON__ */

// Vector shift right by immediate with narrowing
#ifdef __ARM_AARCH64_NEON__
#define FDK_vshrn_imm(size, Dd, Qm, immediate)    "VSHRN.I" #size " " #Dd ", " #Qm ", # " #immediate " \n\t"
#else
#define FDK_vshrn_imm(size, Dd, Qm, immediate)   __FDK_vshrn_imm(size, Dd, Qm, immediate);

static inline void __FDK_vshrn_imm(INT size, A64_X Dd, A64_V Qm, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < size);

  A64_S tmp[4];
  A64_S *qm = (A64_S *) Qm;
  tmp[0] = qm[0];
  tmp[1] = qm[1];
  switch(size)
  {
    case 16:
    {
      A64_H *Tmp = (A64_H *) tmp;
      A64_B dd  = (A64_B) Dd;
      for (int i = 0; i < 8; i++)
        dd[i] = (SCHAR) (Tmp[i]>>immediate);
      break;
    }
    case 32:
    {
      A64_S *Tmp = (A64_S *) tmp;
      A64_H *dd  = (A64_H *) Dd;
      for (int i = 0; i < 4; i++)
        dd[i] = (A64_H) (Tmp[i]>>immediate);
      break;
    }
    case 64:
    {
      INT64 *Tmp = (INT64 *) tmp;
      A64_S *dd  = (A64_S *) Dd;
      for (int i = 0; i < 2; i++)
        dd[i] = (A64_S) (Tmp[i]>>immediate);
      break;
    }
  }
}
#endif /*  __ARM_AARCH64_NEON__ */

// Vector shift left by immediate
#ifdef __ARM_AARCH64_NEON__
#define FDK_vshl_s16_q_imm(dst, src, immediate)   "VSHL.S16  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshl_s16_d_imm(dst, src, immediate)   "VSHL.S16  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshl_s32_q_imm(dst, src, immediate)   "VSHL.S32  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshl_s32_d_imm(dst, src, immediate)   "VSHL.S32  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshll_s16_imm(dst, src, immediate)    "VSHLL.S16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshll_s32_imm(dst, src, immediate)    "VSHLL.S32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshl_s16_q(dst, src1, src2)   "VSHL.S16  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s16_d(dst, src1, src2)   "VSHL.S16  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s32_q(dst, src1, src2)   "VSHL.S32  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s32_d(dst, src1, src2)   "VSHL.S32  " #dst ", " #src1 ", " #src2 " \n\t"

#define __A64_sshll(dst, src, immediate)        "SSHLL " #dst ", " #src ", # " #immediate "\n\t"
#define A64_sshll(width, dst, src, immediate)   __A64_sshll(dst, src, immediate)
#else
// Reference: ARMv8 Instruction Set Overview:
//    5.7.14 Vector Shift (immediate)      Vd.<Td>, Vn.<Ts>, #shift  Signed integer shift left long (vector). Where <Td>/<Ts> is 8H/8B, 4S/4H, or 2D/2S and shift in [0..elsize(<Ts>)-1]
#define A64_sshll(width, dst, src, immediate)   __A64_sshll( (INT) width, (INT64 &) dst, (INT64 &) src, (INT) immediate);



#define FDK_vshl_s16_q_imm(dst, src, immediate)   __FDK_vshl_s16_q_imm(dst, src, immediate);
#define FDK_vshl_s16_d_imm(dst, src, immediate)   __FDK_vshl_s16_d_imm(dst, src, immediate);
#define FDK_vshl_s32_q_imm(dst, src, immediate)   __FDK_vshl_s32_q_imm(dst, src, immediate);
#define FDK_vshl_s32_d_imm(dst, src, immediate)   __FDK_vshl_s32_d_imm(dst, src, immediate);

#define FDK_vshl_s16_q(dst, src1, src2)   __FDK_vshl_s16_q(dst, src1, src2);
#define FDK_vshl_s16_d(dst, src1, src2)   __FDK_vshl_s16_d(dst, src1, src2);
#define FDK_vshl_s32_q(dst, src1, src2)   __FDK_vshl_s32_q(dst, src1, src2);
#define FDK_vshl_s32_d(dst, src1, src2)   __FDK_vshl_s32_d(dst, src1, src2);

static inline void __FDK_vshl_s32_d_imm(A64_S *dst, A64_S *src, INT immediate)
{
  for (int i = 0; i < 2; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s32_q_imm(A64_S *dst, A64_S *src, INT immediate)
{
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s16_d_imm(A64_H *dst, A64_H *src, INT immediate)
{
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s16_q_imm(A64_V dst, A64_V src, INT immediate)
{
  for (int i = 0; i < 8; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}

static inline void __A64_sshll(INT width, INT64 &dst, INT64 &src, INT immediate)
{
  FDK_ASSERT((width == 8) || (width == 16) || (width == 32));  /* width of source vector elements to widen */
  FDK_ASSERT(immediate >= 0);

  INT i, num = 64 / width;
  INT64 tmp = ((INT64 *) src)[0];
  switch (width)
  {
    case 8:
    {
        FDK_ASSERT(immediate <= 8);
        SHORT *Dst = (SHORT *) dst;
        SCHAR *Src = (SCHAR *) &tmp;
        for (i = 0; i < num; i++)
            Dst[i] = (SHORT) Src[i] << immediate;
    }
      break;
    case 16:
    {
        FDK_ASSERT(immediate <= 16);
        INT *Dst = (INT *) dst;
        SHORT *Src = (SHORT *) &tmp;
        for (i = 0; i < num; i++)
            Dst[i] = (INT) Src[i] << immediate;
    }
      break;
    case 32:
    {
        FDK_ASSERT(immediate <= 32);
        INT64 *Dst = (INT64 *) dst;
        INT *Src = (INT *) &tmp;
        for (i = 0; i < num; i++)
            Dst[i] = (INT64) Src[i] << immediate;
    }
      break;
  }
}

static inline void __FDK_vshl_s32_d(A64_X dst, A64_X src1, A64_X src2 )
{
  A64_S *Dst  = (A64_S *) dst;
  A64_S *Src1 = (A64_S *) src1;
  A64_S *Src2 = (A64_S *) src2;
  for (int i = 0; i < 2; i++)
  {
    FDK_ASSERT (Src2[i] >= 0);
    FDK_ASSERT (Src2[i] < 32);
    Dst[i] = Src1[i]<<Src2[i];
  }
}
static inline void __FDK_vshl_s32_q(A64_V dst, A64_V src1, A64_V src2)
{
  A64_S *Dst  = (A64_S *) dst;
  A64_S *Src1 = (A64_S *) src1;
  A64_S *Src2 = (A64_S *) src2;
  for (int i = 0; i < 4; i++)
  {
    FDK_ASSERT (Src2[i] >= 0);
    FDK_ASSERT (Src2[i] < 32);
    Dst[i] = Src1[i]<<Src2[i];
  }
}
static inline void __FDK_vshl_s16_d(A64_X dst, A64_X src1, A64_X src2)
{
  A64_H *Dst  = (A64_H *) dst;
  A64_H *Src1 = (A64_H *) src1;
  A64_H *Src2 = (A64_H *) src2;
  for (int i = 0; i < 4; i++)
  {
    FDK_ASSERT (Src2[i] >= 0);
    FDK_ASSERT (Src2[i] < 16);
    Dst[i] = Src1[i]<<Src2[i];
  }
}
static inline void __FDK_vshl_s16_q(A64_V dst, A64_V src1, A64_V src2)
{
  A64_H *Dst  = (A64_H *) dst;
  A64_H *Src1 = (A64_H *) src1;
  A64_H *Src2 = (A64_H *) src2;
  for (int i = 0; i < 8; i++)
  {
    FDK_ASSERT (Src2[i] >= 0);
    FDK_ASSERT (Src2[i] < 16);
    Dst[i] = Src1[i]<<Src2[i];
  }
}

#endif /*  __ARM_AARCH64_NEON__ */

// Vector reverse elements
#ifdef __ARM_AARCH64_NEON__
#define __A64_rev64(dst, src)               "REV64 " #dst ", " #src "\n\t"
#define __A64_rev32(dst, src)               "REV32 " #dst ", " #src "\n\t"
#define __A64_rev16(dst, src)               "REV16 " #dst ", " #src "\n\t"
#define A64_rev64(width, size, dst, src)   __A64_rev64(dst, src)
#define A64_rev32(width, size, dst, src)   __A64_rev32(dst, src)
#define A64_rev16(width, size, dst, src)   __A64_rev16(dst, src)
#else
#define A64_rev64(width, size, dst, src)   __A64_rev64((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src);
#define A64_rev32(width, size, dst, src)   __A64_rev32((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src);
#define A64_rev16(width, size, dst, src)   __A64_rev16((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src);

static inline void __A64_rev64(INT width, INT size, INT64 &dst, INT64 &src)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32));
  FDK_ASSERT((size == 64) || (size == 128));
  INT i, num = size / 64;
  switch (size)
  {
    case 8:
    {
      SCHAR tmp;
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR *Src = (SCHAR *) src;
      for (i = 0; i < num; i++, Dst += 8, Src += 8)
      {
        tmp = Src[0];  Dst[0] = Src[7];  Dst[7] = tmp;
        tmp = Src[1];  Dst[1] = Src[6];  Dst[6] = tmp;
        tmp = Src[2];  Dst[2] = Src[5];  Dst[5] = tmp;
        tmp = Src[3];  Dst[3] = Src[4];  Dst[4] = tmp;
      }
    }
    break;
    case 16:
    {
      SHORT tmp;
      SHORT *Dst = (SHORT *) dst;
      SHORT *Src = (SHORT *) src;
      for (i = 0; i < num; i++, Dst += 4, Src += 4)
      {
        tmp = Src[0];  Dst[0] = Src[3];  Dst[3] = tmp;
        tmp = Src[1];  Dst[1] = Src[2];  Dst[2] = tmp;
      }
    }
    break;
    case 32:
    {
      INT tmp;
      INT *Dst = (INT *) dst;
      INT *Src = (INT *) src;
      for (i = 0; i < num; i++, Dst += 2, Src += 2)
      {
        tmp = Src[0];  Dst[0] = Src[1];  Dst[1] = tmp;
      }
    }
    break;
  }
}

static inline void __A64_rev32(INT width, INT size, INT64 &dst, INT64 &src)
{
  FDK_ASSERT ((width == 8) || (width == 16));
  FDK_ASSERT((size == 64) || (size == 128));
  INT i, num = size / 32;

  switch(width)
  {
    case 8:
    {
      SCHAR tmp;
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR *Src = (SCHAR *) src;
      for (i = 0; i < num; i++, Dst += 4, Src += 4)
      {
        tmp = Src[0];  Dst[0] = Src[3];  Dst[3] = tmp;
        tmp = Src[1];  Dst[1] = Src[2];  Dst[2] = tmp;
      }
    }
    break;
    case 16:
    {
      SHORT tmp;
      SHORT *Dst = (SHORT *) dst;
      SHORT *Src = (SHORT *) src;
      for (i = 0; i < num; i++, Dst += 2, Src += 2)
      {
        tmp = Src[0];  Dst[0] = Src[1];  Dst[1] = tmp;
      }
    }
    break;
  }
}

static inline void __A64_rev16(INT width, INT size, INT64 &dst, INT64 &src)
{
  FDK_ASSERT ((width == 8));
  FDK_ASSERT((size == 64) || (size == 128));
  INT i, num = size / 16;

  SCHAR tmp;
  SCHAR *Dst = (SCHAR *) dst;
  SCHAR *Src = (SCHAR *) src;
  for (i = 0; i < num; i++, Dst += 2, Src += 2)
  {
    tmp = Src[0];  Dst[0] = Src[1];  Dst[1] = tmp;
  }
}


#endif /*  __ARM_AARCH64_NEON__ */

#ifndef __ARM_AARCH64_NEON__
static inline FIXP_DBL ARMv8_sqdmulh(FIXP_DBL src1, FIXP_DBL src2)
{
   /* Perform 32x32 MPY with doubling, high part result */
   INT64 result = ((INT64) src1 * (INT64) src2) >> 31;
   /* Apply saturation */
   if (result == (INT64) 0x0000000080000000)
       return (MAXVAL_DBL);
   else
      return (FIXP_DBL) result;
}

static inline FIXP_SGL ARMv8_sqdmulh(FIXP_SGL src1, FIXP_SGL src2)
{
   /* Perform 16x16 MPY with doubling, high part result */
   INT result = ((INT) src1 * (INT) src2) >> 15;
   /* Apply saturation */
   if (result == (INT) 0x00008000)
       return (MAXVAL_SGL);
   else
      return (FIXP_SGL) result;
}
#endif
// Reference: ARMv8 Instruction Set Overview:
//    5.7.4  Vector Arithmetic             Vd.<T>, Vn.<T>, Vm.<T>, where <T> is 4H, 8H, 2S, 4S
//    5.7.5  Scalar Arithmetic             Vd.<T>, Vn.<T>, Vm.<T>, where <T> is H, S
//    5.7.10 Vector-by-element Arithmetic  Vd.<Td>, Vn.<Td>, Vm.<Ts>[index], where <Td>/<Ts> is 4H/H, 8H/H, 2S/S, 4S/S
//    5.7.11 Scalar-by-element Arithmetic  <V>d, <V>n, Vm.<Ts>[index], where <V>/<Ts> is H/H, S/S
#ifdef __ARM_AARCH64_NEON__
#define __A64_sqdmulh(       dst, src1, src2)                        "SQDMULH " #dst ", " #src1 ", " #src2 "\n\t"
#define __A64_sqdmulh_scalar(dst, src1, src2, index)                 "SQDMULH " #dst ", " #src1 ", " #src2 "[" #index "]\n\t"
#define A64_sqdmulh(         width, size, dst, src1, src2)           __A64_sqdmulh(dst, src1, src2)
#define A64_sqdmulh_scalar(  width, size, dst, src1, src2, index)    __A64_sqdmulh_scalar(dst, src1, src2, index)
#define __A64_mul(dst, src1, src2)                                   "MUL " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_mul(width, size, dst, src1, src2)                        __A64_mul(dst, src1, src2)
#define __A64_mla(dst, src1, src2)                                   "MLA " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_mla(width, size, dst, src1, src2)                        __A64_mla(dst, src1, src2)
#define __A64_mls(dst, src1, src2)                                   "MLS " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_mls(width, size, dst, src1, src2)                        __A64_mls(dst, src1, src2)
#define __A64_mla_lane(             dst, src1, src2, index)          "MLA " #dst ", " #src1 ", " #src2 "[" #index "]\n\t"
#define   A64_mla_lane(width, size, dst, src1, src2, index)          __A64_mla_lane(dst, src1, src2, index)
#define __A64_mls_lane(             dst, src1, src2, index)          "MLS " #dst ", " #src1 ", " #src2 "[" #index "]\n\t"
#define   A64_mls_lane(width, size, dst, src1, src2, index)          __A64_mls_lane(dst, src1, src2, index)
#define __A64_smull(dst, src1, src2)                                 "SMULL " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_smull(width, size, dst, src1, src2)                        __A64_smull(dst, src1, src2)
#define __A64_smull2(dst, src1, src2)                                "SMULL2 " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_smull2(width, size, dst, src1, src2)                        __A64_smull2(dst, src1, src2)
#define __A64_smlal(dst, src1, src2)                                 "SMLAL " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_smlal(width, size, dst, src1, src2)                        __A64_smlal(dst, src1, src2)
#define __A64_smlal2(dst, src1, src2)                                "SMLAL2 " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_smlal2(width, size, dst, src1, src2)                        __A64_smlal2(dst, src1, src2)

#else
#define A64_sqdmulh(       width, size, dst, src1, src2)              __A64_sqdmulh(       (INT) width, (INT) size, (INT64 &)dst, (INT64 &)src1, (INT64 &)src2);
#define A64_sqdmulh_scalar(width, size, dst, src1, src2, index)       __A64_sqdmulh_scalar((INT) width, (INT) size, (INT64 &)dst, (INT64 &)src1, (INT64 &)src2, (INT) index);
#define A64_mul(width, size, dst, src1, src2)                         __A64_mul(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_mla(width, size, dst, src1, src2)                         __A64_mla(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_mls(width, size, dst, src1, src2)                         __A64_mls(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_mla_lane(width, size, dst, src1, src2, index)             __A64_mla_lane(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2, (INT) index);
#define A64_mls_lane(width, size, dst, src1, src2, index)             __A64_mls_lane(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2, (INT) index);
#define A64_smull(width, size, dst, src1, src2)                       __A64_smull(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_smull2(width, size, dst, src1, src2)                      __A64_smull2(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_smlal(width, size, dst, src1, src2)                       __A64_smlal(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_smlal2(width, size, dst, src1, src2)                      __A64_smlal2(width, size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);

static inline void __A64_sqdmulh(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64) || (size == width));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      FIXP_DBL *Dst  = (FIXP_DBL *) dst;
      FIXP_DBL *Src1 = (FIXP_DBL *) src1;
      FIXP_DBL *Src2 = (FIXP_DBL *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = ARMv8_sqdmulh(Src1[i], Src2[i]);
      }
    }
      break;
    case 16:
    {
      FIXP_SGL *Dst  = (FIXP_SGL *) dst;
      FIXP_SGL *Src1 = (FIXP_SGL *) src1;
      FIXP_SGL *Src2 = (FIXP_SGL *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = ARMv8_sqdmulh(Src1[i], Src2[i]);
      }
    }
      break;
  }
}

static inline void __A64_sqdmulh_scalar(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2, INT index)
{
  FDK_ASSERT((width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64) || (size == width));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      FIXP_DBL *Dst  = (FIXP_DBL *) dst;
      FIXP_DBL *Src1 = (FIXP_DBL *) src1;
      FIXP_DBL *Src2 = (FIXP_DBL *) src2;
      FIXP_DBL Scalar = Src2[index];
      for (i = 0; i < num; i++)
      {
        Dst[i] = ARMv8_sqdmulh(Src1[i], Scalar);
      }
    }
      break;
    case 16:
    {
      FIXP_SGL *Dst  = (FIXP_SGL *) dst;
      FIXP_SGL *Src1 = (FIXP_SGL *) src1;
      FIXP_SGL *Src2 = (FIXP_SGL *) src2;
      FIXP_SGL Scalar = Src2[index];
      for (i = 0; i < num; i++)
      {
        Dst[i] = ARMv8_sqdmulh(Src1[i], Scalar);
      }
    }
      break;
  }
}

static inline void __A64_mul(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 32) || (width == 16) );
  FDK_ASSERT((size == 128) || (size  == 64) || (size == width));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] * Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] * Src2[i];
      }
    }
    break;
  }
}


static inline void __A64_mla(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += Src1[i] * Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += Src1[i] * Src2[i];
      }
    }
    break;
  }
}

static inline void __A64_mls(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] -= Src1[i] * Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] -= Src1[i] * Src2[i];
      }
    }
    break;
  }
}

static void __A64_mla_lane(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2, INT lane)
{
  FDK_ASSERT((width == 8) ||(width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64));
  INT i, num = size / width;
  FDK_ASSERT((lane >= 0) && (lane < num));

  switch (width)
  {
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] += (INT)(Src1[i] * val);
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] += (SHORT)(Src1[i] * val);
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) dst;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] += (SCHAR)(Src1[i] * val);
      }
    }
    break;
  }
}

static void __A64_mls_lane(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2, INT lane)
{
  FDK_ASSERT((width == 8) ||(width == 32) || (width == 16));
  FDK_ASSERT((size == 128) || (size  == 64));
  INT i, num = size / width;
  FDK_ASSERT((lane >= 0) && (lane < num));

  switch (width)
  {
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      INT val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] -= (INT)(Src1[i] * val);
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      SHORT val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] -= (SHORT)(Src1[i] * val);
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) dst;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      SCHAR val   = Src2[lane];
      for (i = 0; i < num; i++)
      {
        Dst[i] -= (SCHAR)(Src1[i] * val);
      }
    }
    break;
  }
}

static void __A64_smull(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{            /*  8H/8B, 4S/4H or 2D/2S */
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));  /* width of MPY sources */
  FDK_ASSERT((size == 128));                                   /* size of all MPY output, each of 2*width */
  INT i, num = size / (2 * width);
  switch (width)
  {
    case 32:
    {
      INT64* Dst = (INT64*)dst;
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = (INT64)Src1[i] * Src2[i];
      }
    }
    break;
    case 16:
    {
      INT* Dst = (INT*)dst;
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = (INT)Src1[i] * Src2[i];
      }
    }
    break;
    case 8:
    {
      SHORT* Dst = (SHORT*)dst;
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = (SHORT)Src1[i] * Src2[i];
      }
    }
    break;
  }
}

static void __A64_smull2(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));  /* width of MPY sources */
  FDK_ASSERT((size == 128));                                   /* size of all MPY output, each of 2*width */
  INT i, num = size / (2 * width);
  switch (width)
  {
  case 32:
  {
    INT64* Dst = (INT64*)dst;
    INT* Src1 = (INT*)src1;
    INT* Src2 = (INT*)src2;
    for (i = 0; i < num; i++)
    {
      Dst[i] = (INT64)Src1[num+i] * Src2[num+i];
    }
  }
  break;
  case 16:
  {
    INT* Dst = (INT*)dst;
    SHORT* Src1 = (SHORT*)src1;
    SHORT* Src2 = (SHORT*)src2;
    for (i = 0; i < num; i++)
    {
      Dst[i] = (INT)Src1[num+i] * Src2[num+i];
    }
  }
  break;
  case 8:
  {
    SHORT* Dst = (SHORT*)dst;
    SCHAR* Src1 = (SCHAR*)src1;
    SCHAR* Src2 = (SCHAR*)src2;
    for (i = 0; i < num; i++)
    {
      Dst[i] = (SHORT)Src1[num+i] * Src2[num+i];
    }
  }
  break;
  }

}

static void __A64_smlal(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{            /*  8H/8B, 4S/4H or 2D/2S */
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));  /* width of MPY sources */
  FDK_ASSERT((size == 128));                                   /* size of all MPY output, each of 2*width */
  INT i, num = size / (2*width);
  switch (width)
  {
    case 32:
    {
      INT64* Dst = (INT64*)dst;
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (INT64) Src1[i] * Src2[i];
      }
    }
    break;
    case 16:
    {
      INT* Dst = (INT*)dst;
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (INT) Src1[i] * Src2[i];
      }
    }
    break;
    case 8:
    {
      SHORT* Dst = (SHORT*)dst;
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (SHORT) Src1[i] * Src2[i];
      }
    }
    break;
  }
}

static void __A64_smlal2(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT((width == 32) || (width == 16) || (width == 8));  /* width of MPY sources */
  FDK_ASSERT((size == 128));                                   /* size of all MPY output, each of 2*width */
  INT i, num = size / (2 * width);
  switch (width)
  {
    case 32:
    {
      INT64* Dst = (INT64*)dst;
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (INT64)Src1[num+i] * Src2[num+i];
      }
      break;
    }
    case 16:
    {
      INT* Dst = (INT*)dst;
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (INT)Src1[num+i] * Src2[num+i];
      }
      break;
    }
    case 8:
    {
      SHORT* Dst = (SHORT*)dst;
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] += (SHORT)Src1[num+i] * Src2[num+i];
      }
      break;
    }
  }
}


#endif /*  __ARM_AARCH64_NEON__ */

// Interleave elements
#ifdef __ARM_AARCH64_NEON__
#define __A64_trn1(dst, src1, src2)                 "TRN1 " #dst ", " #src1 ", " #src2 "\n\t"
#define __A64_trn2(dst, src1, src2)                 "TRN2 " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_trn1(width, size, dst, src1, src2)      __A64_trn1(dst, src1, src2)
#define A64_trn2(width, size, dst, src1, src2)      __A64_trn2(dst, src1, src2)
#define __A64_zip1(dst, src1, src2)                 "ZIP1 " #dst ", " #src1 ", " #src2 "\n\t"
#define __A64_zip2(dst, src1, src2)                 "ZIP2 " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_zip1(width, size, dst, src1, src2)      __A64_zip1(dst, src1, src2)
#define A64_zip2(width, size, dst, src1, src2)      __A64_zip2(dst, src1, src2)
#define __A64_uzp1(dst, src1, src2)                 "UZP1 " #dst ", " #src1 ", " #src2 "\n\t"
#define __A64_uzp2(dst, src1, src2)                 "UZP2 " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_uzp1(width, size, dst, src1, src2)      __A64_uzp1(dst, src1, src2)
#define A64_uzp2(width, size, dst, src1, src2)      __A64_uzp2(dst, src1, src2)
#define __A64_ext(dst, src1, src2, idx)                "EXT " #dst ", " #src1 ", " #src2 ", " #idx "\n\t"
#define A64_ext(width, size, dst, src1, src2, idx)    __A64_ext(dst, src1, src2, idx)

#else
/* Description as for ARMv7: ZIP Vd, Vm -> zip1 results in Vd, zip2 results in Vm */
#define A64_trn1(width, size, dst, src1, src2)  __A64_trn1((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_trn2(width, size, dst, src1, src2)  __A64_trn2((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_zip1(width, size, dst, src1, src2)  __A64_zip1((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_zip2(width, size, dst, src1, src2)  __A64_zip2((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_uzp1(width, size, dst, src1, src2)  __A64_uzp1((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_uzp2(width, size, dst, src1, src2)  __A64_uzp2((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_ext(width, size, dst, src1, src2, idx) __A64_ext((INT) width, (INT) size, (INT64 *) dst, (INT64 &) src1, (INT64 &) src2, (INT) idx);

static void __A64_ext(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2, INT idx)
{
  FDK_ASSERT (width == 8);
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / width;
  FDK_ASSERT ((idx >=  0) && (idx < num));
  INT64 tmp[2];

  switch(width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
  }
  dst[0] = tmp[0];
  if (size == 128)
    dst[1] = tmp[1];
}

static inline void __A64_trn1(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+0];
          Dst[2*i+1] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+0];
          Dst[2*i+1] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+0];
          Dst[2*i+1] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
    case 64:
    {
      INT64 *Dst  = (INT64 *)&tmp;
      INT64 *Src1 = (INT64 *)src1;
      INT64 *Src2 = (INT64 *)src2;
      for (i = 0; i < num; i++)
      {
        Dst[2 * i + 0] = Src1[2 * i + 0];
        Dst[2 * i + 1] = Src2[2 * i + 0];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}


static inline void __A64_trn2(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+1];
          Dst[2*i+1] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+1];
          Dst[2*i+1] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[2*i+1];
          Dst[2*i+1] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}

static inline void __A64_zip1(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[i];
          Dst[2*i+1] = Src2[i];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}


static inline void __A64_zip2(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[num+i];
          Dst[2*i+1] = Src2[num+i];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[num+i];
          Dst[2*i+1] = Src2[num+i];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[2*i+0] = Src1[num+i];
          Dst[2*i+1] = Src2[num+i];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}

static inline void __A64_uzp1(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+0];
          Dst[i+num] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+0];
          Dst[i+num] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+0];
          Dst[i+num] = Src2[2*i+0];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}

static inline void __A64_uzp2(INT width, INT size, INT64 *dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT ((width == 8) || (width == 16) || (width == 32) || (width == 64) );
  FDK_ASSERT ((size == 64) || (size == 128) );
  INT i, num = size / (2*width);
  INT64 tmp[2];

  switch (width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+1];
          Dst[i+num] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 8)
        dst[1] = tmp[1];
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) src1;
      SHORT *Src2 = (SHORT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+1];
          Dst[i+num] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 4)
        dst[1] = tmp[1];
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
          Dst[i+0]   = Src1[2*i+1];
          Dst[i+num] = Src2[2*i+1];
      }
      dst[0] = tmp[0];
      if (num == 2)
        dst[1] = tmp[1];
    }
    break;
  }
}




#endif /*  __ARM_AARCH64_NEON__ */


/*----------------------------------------------------------------------------*/
/* General ARM core instructions                                              */
/*----------------------------------------------------------------------------*/
#ifdef __ARM_AARCH64_NEON__
#define FDK_push(reg)                     "PUSH { " #reg " } \n\t"
#define FDK_pop(reg)                      "POP  { " #reg " } \n\t"
#define FDK_push_lr(reg)                  "PUSH { " #reg ", lr } \n\t"
#define FDK_pop_pc(reg)                   "POP  { " #reg ", pc } \n\t"
#define FDK_mpush(first_reg, last_reg)    "PUSH { " #first_reg "-" #last_reg " } \n\t"
#define FDK_mpop(first_reg, last_reg)     "POP  { " #first_reg "-" #last_reg " } \n\t"
#define FDK_mpush_lr(first_reg, last_reg) "PUSH { " #first_reg "-" #last_reg ", lr } \n\t"
#define FDK_mpop_pc(first_reg, last_reg)  "POP  { " #first_reg "-" #last_reg ", pc } \n\t"
#else
#define FDK_push(reg)
#define FDK_pop(reg)
#define FDK_push_lr(reg)
#define FDK_pop_pc(reg)
#define FDK_mpush(first_reg, last_reg)
#define FDK_mpop(first_reg, last_reg)
#define FDK_mpush_lr(first_reg, last_reg)
#define FDK_mpop_pc(first_reg, last_reg)
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_ldr(dst, src, offset, name)      "LDR " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_ldr_ia(dst, src, offset)         "LDR " #dst ", [" #src "], #" #offset " \n\t"
#else
// example: LDR r2, [sp, #0x20]  <- r2 = call parameter #3 (A64_S *TimeOut)
#define FDK_ldr(dst, src, offset, name)       dst = name;
// example: LDR r2, [r0], #0x4   <- r2 = *ptr; ptr+=2; (A64_H)
// example: LDR r2, [r0], #0x4   <- r2 = *ptr++;       (A64_S)
#define FDK_ldr_ia(dst, src, offset)        { dst = *src; src += offset/(INT)sizeof(*src); }
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_ldrh(dst, src, offset, name)     "LDRH " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_ldrh_ia(dst, src, offset)        "LDRH " #dst ", [" #src "], #" #offset " \n\t"
#define FDK_ldrh_pu_add(dst, src, reg)       "LDRH " #dst ", [" #src "], +" #reg " \n\t"
#else

#define FDK_ldrh(dst, src, offset, name)            { dst = src[offset/(INT)sizeof(*src)]; }
#define FDK_ldrh_ia(dst, src, offset)               { dst = *src; src += offset/(INT)sizeof(*src); }
#define FDK_ldrh_pu_add(dst, src, reg)              { dst = *src; src += reg/(INT)sizeof(*src); }
#endif


#ifdef __ARM_AARCH64_NEON__
#define FDK_ldrd(dst1, dst2, src, offset, name1, name2)       "LDRD " #dst1 ", " #dst2 ", [" #src ", #" #offset "] \n\t"
#else
#define FDK_ldrd(dst1, dst2, src, offset, name1, name2)       dst1 = name1; dst2 = name2;
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_str(src, dst, offset, name)       "STR " #src ", [" #dst ", #" #offset "] \n\t"
#else
#define FDK_str(src, dst, offset, name)       name = src;
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_strd_pi(src1, src2, dst, offset)      "STRD " #src1 ", " #src2 ", [" #dst "], #" #offset " \n\t"
#else
#define FDK_strd_pi(src1, src2, dst, offset)      dst[0] = INT(src1); dst[1] = INT(src2); dst += offset/INT(sizeof(*dst));
#endif


#ifdef __ARM_AARCH64_NEON__
#define FDK_mov_reg(dst, src)    "MOV " #dst ", " #src "\n\t"
#else
#define FDK_mov_reg(dst, src)    dst = src;
#endif

#ifdef __ARM_AARCH64_NEON__
#define __FDK_mov_Xt_imm(dst, imm)    "MOV " #dst ", # " #imm "\n\t"
#define __FDK_mov_Wt_imm(dst, imm)    "MOV " #dst ", # " #imm "\n\t"
// GNUC does not allow const expressions instead of constants, we need one layer inbetween
#define FDK_mov_Xt_imm(dst,imm)       __FDK_mov_Xt_imm(dst,imm)
#define FDK_mov_Wt_imm(dst,imm)       __FDK_mov_Wt_imm(dst,imm)
#else
#define FDK_mov_Xt_imm(dst, imm)    dst = imm;
#define FDK_mov_Wt_imm(dst, imm)    dst = imm;
#endif

#ifdef __ARM_AARCH64_NEON__
// Reference: ARMv8 Instruction Set Overview: 5.3.7 Shift (immediate)

#define A64_asr_Xt_imm(Xd, Xn, uimm)   "ASR " #Xd ", " #Xn ", # " #uimm " \n\t"
#define A64_lsl_Xt_imm(Xd, Xn, uimm)   "LSL " #Xd ", " #Xn ", # " #uimm " \n\t"
#define A64_lsr_Xt_imm(Xd, Xn, uimm)   "LSR " #Xd ", " #Xn ", # " #uimm " \n\t"
#define A64_ror_Xt_imm(Xd, Xn, uimm)   "ROR " #Xd ", " #Xn ", # " #uimm " \n\t"
#define A64_asr_Wt_imm(Wd, Wn, uimm)   "ASR " #Wd ", " #Wn ", # " #uimm " \n\t"
#define A64_lsl_Wt_imm(Wd, Wn, uimm)   "LSL " #Wd ", " #Wn ", # " #uimm " \n\t"
#define A64_lsr_Wt_imm(Wd, Wn, uimm)   "LSR " #Wd ", " #Wn ", # " #uimm " \n\t"
#define A64_ror_Wt_imm(Wd, Wn, uimm)   "ROR " #Wd ", " #Wn ", # " #uimm " \n\t"

#define A64_asr_Xt(Xd, Xn, Xs)         "ASR " #Xd ", " #Xn ", " #Xs " \n\t"
#define A64_lsl_Xt(Xd, Xn, Xs)         "LSL " #Xd ", " #Xn ", " #Xs " \n\t"
#define A64_lsr_Xt(Xd, Xn, Xs)         "LSR " #Xd ", " #Xn ", " #Xs " \n\t"
#define A64_ror_Xt(Xd, Xn, Xs)         "ROR " #Xd ", " #Xn ", " #Xs " \n\t"
#define A64_asr_Wt(Wd, Wn, Ws)         "ASR " #Wd ", " #Wn ", " #Ws " \n\t"
#define A64_lsl_Wt(Wd, Wn, Ws)         "LSL " #Wd ", " #Wn ", " #Ws " \n\t"
#define A64_lsr_Wt(Wd, Wn, Ws)         "LSR " #Wd ", " #Wn ", " #Ws " \n\t"
#define A64_ror_Wt(Wd, Wn, Ws)         "ROR " #Wd ", " #Wn ", " #Ws " \n\t"
#else
#define A64_asr_Xt_imm(Xd, Xn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 63)); Xd = (INT64) Xn >> uimm; }
#define A64_lsl_Xt_imm(Xd, Xn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 63)); Xd = (INT64) Xn << uimm; }
#define A64_lsr_Xt_imm(Xd, Xn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 63)); Xd = (UINT64) Xn >> uimm; }
#define A64_ror_Xt_imm(Xd, Xn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 63)); Xd = ((UINT64) Xn >> uimm) | ((UINT64) Xn << ((64 - uimm) & 0x3F)); }
#define A64_asr_Wt_imm(Wd, Wn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 31)); Wd = (INT) Wn >> uimm; }
#define A64_lsl_Wt_imm(Wd, Wn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 31)); Wd = (INT) Wn << uimm; }
#define A64_lsr_Wt_imm(Wd, Wn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 31)); Wd = (UINT) Wn >> uimm; }
#define A64_ror_Wt_imm(Wd, Wn, uimm)    {  FDK_ASSERT((uimm >= 0) && (uimm <= 31)); Wd = ((UINT) Xn >> uimm) | ((UINT) Xn << ((32 - uimm) & 0x1F)); }
#define A64_asr_Xt(Xd, Xn, Xs)          {  Xd = (INT64) Xn >> (Xs & 0x3F); }
#define A64_lsl_Xt(Xd, Xn, Xs)          {  Xd = (INT64) Xn << (Xs & 0x3F); }
#define A64_lsr_Xt(Xd, Xn, Xs)          {  Xd = (UINT64) Xn >> (Xs & 0x3F); }
#define A64_ror_Xt(Xd, Xn, Xs)          {  Xd = ((UINT64) Xn >> (Xs & 0x3F)) | ((UINT64) Xn << ((64 - Xs) & 0x3F)); }
#define A64_asr_Wt(Wd, Wn, Ws)          {  Wd = (INT) Wn >> (Ws & 0x1F); }
#define A64_lsl_Wt(Wd, Wn, Ws)          {  Wd = (INT) Wn << (Ws & 0x1F); }
#define A64_lsr_Wt(Wd, Wn, Ws)          {  Wd = (UINT) Wn >> (Ws & 0x1F); }
#define A64_ror_Wt(Wd, Wn, Ws)          {  Wd = ((UINT) Xn >> (Xs & 0x1F)) | ((UINT) Xn << ((32 - Xs) & 0x1F)); }
#endif


#ifdef __ARM_AARCH64_NEON__
// Reference: ARMv8 Instruction Set Overview: 5.4.1 Arithmetic (shift register))
#define __A64_adds_Xt_asr_imm(Xd, Xn, Xm, uimm)    "ADDS " #Xd ", " #Xn ", " #Xm ", ASR # " #uimm " \n\t"
#define __A64_adds_Xt_lsl_imm(Xd, Xn, Xm, uimm)    "ADDS " #Xd ", " #Xn ", " #Xm ", LSL # " #uimm " \n\t"
#define __A64_adds_Xt_lsr_imm(Xd, Xn, Xm, uimm)    "ADDS " #Xd ", " #Xn ", " #Xm ", LSR # " #uimm " \n\t"
#define __A64_adds_Wt_asr_imm(Wd, Wn, Wm, uimm)    "ADDS " #Wd ", " #Wn ", " #Wm ", ASR # " #uimm " \n\t"
#define __A64_adds_Wt_lsl_imm(Wd, Wn, Wm, uimm)    "ADDS " #Wd ", " #Wn ", " #Wm ", LSL # " #uimm " \n\t"
#define __A64_adds_Wt_lsr_imm(Wd, Wn, Wm, uimm)    "ADDS " #Wd ", " #Wn ", " #Wm ", LSR # " #uimm " \n\t"
#define __A64_add_Xt_asr_imm(Xd, Xn, Xm, uimm)     "ADD  " #Xd ", " #Xn ", " #Xm ", ASR # " #uimm " \n\t"
#define __A64_add_Xt_lsl_imm(Xd, Xn, Xm, uimm)     "ADD  " #Xd ", " #Xn ", " #Xm ", LSL # " #uimm " \n\t"
#define __A64_add_Xt_lsr_imm(Xd, Xn, Xm, uimm)     "ADD  " #Xd ", " #Xn ", " #Xm ", LSR # " #uimm " \n\t"
#define __A64_sub_Xt_asr_imm(Xd, Xn, Xm, uimm)     "SUB  " #Xd ", " #Xn ", " #Xm ", ASR # " #uimm " \n\t"
#define __A64_sub_Xt_lsl_imm(Xd, Xn, Xm, uimm)     "SUB  " #Xd ", " #Xn ", " #Xm ", LSL # " #uimm " \n\t"
#define __A64_sub_Xt_lsr_imm(Xd, Xn, Xm, uimm)     "SUB  " #Xd ", " #Xn ", " #Xm ", LSR # " #uimm " \n\t"
#define __A64_add_Xt_sxtw(   Xd, Xn, Wm)           "ADD  " #Xd ", " #Xn ", " #Wm ", SXTW\n\t"

#define A64_adds_Xt_asr_imm(Xd, Xn, Xm, uimm)     __A64_adds_Xt_asr_imm(Xd, Xn, Xm, uimm)
#define A64_adds_Xt_lsl_imm(Xd, Xn, Xm, uimm)     __A64_adds_Xt_lsl_imm(Xd, Xn, Xm, uimm)
#define A64_adds_Xt_lsr_imm(Xd, Xn, Xm, uimm)     __A64_adds_Xt_lsr_imm(Xd, Xn, Xm, uimm)
#define A64_adds_Wt_asr_imm(Wd, Wn, Wm, uimm)     __A64_adds_Wt_asr_imm(Wd, Wn, Wm, uimm)
#define A64_adds_Wt_lsl_imm(Wd, Wn, Wm, uimm)     __A64_adds_Wt_lsl_imm(Wd, Wn, Wm, uimm)
#define A64_adds_Wt_lsr_imm(Wd, Wn, Wm, uimm)     __A64_adds_Wt_lsr_imm(Wd, Wn, Wm, uimm)
#define A64_add_Xt_asr_imm(Xd, Xn, Xm, uimm)      __A64_add_Xt_asr_imm(Xd, Xn, Xm, uimm)
#define A64_add_Xt_lsl_imm(Xd, Xn, Xm, uimm)      __A64_add_Xt_lsl_imm(Xd, Xn, Xm, uimm)
#define A64_add_Xtp_lsl_imm(Xd, Xn, Xm, uimm, sc) __A64_add_Xt_lsl_imm(Xd, Xn, Xm, uimm)
#define A64_add_Xt_lsr_imm(Xd, Xn, Xm, uimm)      __A64_add_Xt_lsr_imm(Xd, Xn, Xm, uimm)
#define A64_sub_Xt_asr_imm(Xd, Xn, Xm, uimm)      __A64_sub_Xt_asr_imm(Xd, Xn, Xm, uimm)
#define A64_sub_Xt_lsl_imm(Xd, Xn, Xm, uimm)      __A64_sub_Xt_lsl_imm(Xd, Xn, Xm, uimm)
#define A64_sub_Xtp_lsl_imm(Xd, Xn, Xm, uimm, sc) __A64_sub_Xt_lsl_imm(Xd, Xn, Xm, uimm)
#define A64_sub_Xt_lsr_imm(Xd, Xn, Xm, uimm)      __A64_sub_Xt_lsr_imm(Xd, Xn, Xm, uimm)
#define A64_add_Xt_sxtw(Xd, Xn, Wm, sc)           __A64_add_Xt_sxtw(Xd, Xn, Wm)


#else
/* Example: Add shifted register to any number/counter */
#define A64_adds_Xt_asr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  Xd = ARMv8_adds_Xt((INT64)Xn, (INT64)Xm >> uimm);  }
#define A64_adds_Xt_lsr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  Xd = ARMv8_adds_Xt((INT64)Xn,(UINT64)Xm >> uimm);  }
#define A64_adds_Xt_lsl_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  Xd = ARMv8_adds_Xt((INT64)Xn, (INT64)Xm << uimm);  }
#define A64_adds_Wt_lsl_imm(Wd, Wn, Wm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 31));  Wd = ARMv8_adds_Wt((INT)Wn, (INT)Wm << uimm);  }
/* Example: Add shifted offset to pointer: ptr = buffer + offset;  uimm-scale must be gretaer/equal zero */
#define A64_add_Xt_asr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = (INT64) Xn + ((INT64) Xm >> (uimm));  }
#define A64_add_Xt_lsl_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = (INT64)Xn + (INT64)((INT64)Xm << (uimm));  }
#define A64_add_Xtp_lsl_imm(Xd, Xn, Xm, uimm, sc) { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = Xn + (INT64)(((INT64)Xm << (uimm))>>sc);  }
#define A64_add_Xt_lsr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = ((INT64)Xn + (INT64)((UINT64) Xm >> (uimm)));  }
/* Example: Subtract shifted offset from pointer: ptr = buffer - offset;  uimm-scale must be gretaer/equal zero */
#define A64_sub_Xt_asr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = (INT64)Xn - ((INT64)Xm >> (uimm));  }
#define A64_sub_Xt_lsl_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = ((INT64) Xn - (INT64)((INT64)Xm << (uimm)));  }
#define A64_sub_Xtp_lsl_imm(Xd, Xn, Xm, uimm, sc) { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = Xn - (INT64)(((INT64)Xm << (uimm)) >> sc);  }
#define A64_sub_Xt_lsr_imm(Xd, Xn, Xm, uimm)  { FDK_ASSERT((uimm >= 0) && (uimm <= 63));  \
                                                       FDK_ASSERT((uimm) >= 0);            \
                                                       Xd = (INT64)Xn - (INT64)((UINT64) Xm >> (uimm));  }
#define A64_add_Xt_sxtw(Xd, Xn, Wm, sc)       {        Xd = Xn + (Wm >> sc);   }
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_pld(Rn, offset, name)                 "PLD [" #Rn ", #" #offset "] \n\t"
#define FDK_pld_label(name)                       "PLD # " #name " \n\t"
#define FDK_pld_add_op_lsl(Rn, Rm, scale, name)   "PLD [" #Rn ", +" #Rm ", LSL # " #scale "] \n\t"
#define FDK_pld_add_op_asr(Rn, Rm, scale, name)   "PLD [" #Rn ", +" #Rm ", ASR # " #scale "] \n\t"
#define FDK_pld_sub_op_lsl(Rn, Rm, scale, name)   "PLD [" #Rn ", -" #Rm ", LSL # " #scale "] \n\t"
#define FDK_pld_sub_op_asr(Rn, Rm, scale, name)   "PLD [" #Rn ", -" #Rm ", ASR # " #scale "] \n\t"
#define FDK_pli_label(name)                       "PLI # " #name " \n\t"
#else
                                                        /* L2 cache data preload, core executes a nop */
#define FDK_pld(Rn, offset, name)
#define FDK_pld_label(name)
#define FDK_pld_add_op_lsl(Rn, Rm, scale, name)
#define FDK_pld_add_op_asr(Rn, Rm, scale, name)
#define FDK_pld_sub_op_lsl(Rn, Rm, scale, name)
#define FDK_pld_sub_op_asr(Rn, Rm, scale, name)
#define FDK_pli_label(name)
#endif



#ifdef __ARM_AARCH64_NEON__
#define FDK_mov_cond_imm(cond, dst, imm)     "MOV" #cond " " #dst ", # " #imm " \n\t"
#else
#define FDK_mov_cond_imm(cond, dst, imm)     {   if (__FDK_coreflags_ ## cond) dst = imm;  }
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_mvn_cond_imm(cond, dst, imm)     "MVN" #cond " " #dst ", # " #imm " \n\t"
#else
#define FDK_mvn_cond_imm(cond, dst, imm)     {   if (__FDK_coreflags_ ## cond) dst = -imm;  }
#endif


#ifdef __ARM_AARCH64_NEON__
#define FDK_lsr_imm(dst, src, imm)     "LSR " #dst ", " #src ", # " #imm " \n\t"
#else
#define FDK_lsr_imm(dst, src, imm)      dst = (ULONG) src >> imm;
#endif

#ifdef __ARM_AARCH64_NEON__
#define __A64_shl_imm(dst, src, imm)                 "SHL " #dst ", " #src ", " #imm " \n\t"
#define A64_shl_imm(width, size, dst, src, imm)      __A64_shl_imm(dst, src, imm)
#define __A64_sqshl(dst, src1, src2)                 "SQSHL " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_sqshl(width, size, dst, src1, src2)      __A64_sqshl(dst, src1, src2)
#define __A64_sqrshl(dst, src1, src2)                "SQRSHL " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_sqrshl(width, size, dst, src1, src2)     __A64_sqrshl(dst, src1, src2)
#define __A64_sqshl_imm(dst, src, imm)               "SQSHL " #dst ", " #src ", " #imm " \n\t"
#define A64_sqshl_imm(width, size, dst, src, imm)    __A64_sqshl_imm(dst, src, imm)
#define __A64_sshr_imm(dst, src, imm)                 "SSHR " #dst ", " #src ", " #imm " \n\t"
#define A64_sshr_imm(width, size, dst, src, imm)      __A64_sshr_imm(dst, src, imm)
#define __A64_sshl(dst, src1, src2)                  "SSHL " #dst ", " #src1 ", " #src2 " \n\t"
#define A64_sshl(width, size, dst, src1, src2)        __A64_sshl(dst, src1, src2)
#define A64_lsrv_Wt(dst, src1, src2)                  "LSRV " #dst ", " #src1 ", " #src2 " \n\t"
#define __A64_ssra_imm(dst, src, imm)                 "SSRA " #dst ", " #src ", " #imm " \n\t"
#define A64_ssra_imm(width, size, dst, src, imm)      __A64_ssra_imm(dst, src, imm)
#else
#define A64_shl_imm(width, size, dst, src, imm)      __A64_shl_imm((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT64) imm);
#define A64_sqshl(width, size, dst, src1, src2)      __A64_sqshl((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_sqrshl(width, size, dst, src1, src2)     __A64_sqrshl((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src1, (INT64 &) src2);
#define A64_sqshl_imm(width, size, dst, src, imm)    __A64_sqshl_imm((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT) imm);

#define A64_sshr_imm(width, size, dst, src, imm)     __A64_sshr_imm((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT64) imm);
#define A64_sshl(width, size, dst, src1, src2)       __A64_sshl((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src1,(INT64 &) src2);
#define A64_lsrv_Wt(dst, src1, src2)                 { dst = (UINT) src1 >> ((UINT) src2 & 0x1F); }
#define A64_ssra_imm(width, size, dst, src, imm)     __A64_ssra_imm((INT) width, (INT) size, (INT64 &) dst, (INT64 &) src, (INT64) imm);

static inline void __A64_shl_imm(INT width, INT size, INT64 &dst, INT64 &src, INT64 imm)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((imm >= 0) && (imm < width));
  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64 *Dst = (INT64 *) dst;
      INT64 *Src = (INT64 *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] << imm;
      }
    }
    break;
    case 32:
    {
      INT *Dst = (INT *) dst;
      INT *Src = (INT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] << imm;
      }
    }
    break;
    case 16:
    {
      SHORT *Dst = (SHORT *) dst;
      SHORT *Src = (SHORT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] << imm;
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR *Src = (SCHAR *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] << imm;
      }
    }
    break;
  }
}

static inline void __A64_sqshl(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64* Dst = (INT64*)dst;
      INT64* Src1 = (INT64*)src1;
      INT64* Src2 = (INT64*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_sshl_s64(Src1[i], (INT)Src2[i]);
      }
    }
    break;
    case 32:
    {
      INT* Dst = (INT*)dst;
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_sshl_s32(Src1[i], Src2[i]);
      }
    }
    break;
    case 16:
    {
      SHORT* Dst = (SHORT*)dst;
      SHORT* Src1 = (SHORT*)src1;
      SHORT* Src2 = (SHORT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_sshl_s16(Src1[i], Src2[i]);
      }
    }
    break;
    case 8:
    {
      SCHAR* Dst = (SCHAR*)dst;
      SCHAR* Src1 = (SCHAR*)src1;
      SCHAR* Src2 = (SCHAR*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_sshl_s8(Src1[i], Src2[i]);
      }
    }
    break;
  }
}

static inline void __A64_sqrshl(INT width, INT size, INT64& dst, INT64& src1, INT64& src2)
{
  FDK_ASSERT(width == 32);
  FDK_ASSERT((size == 128) || (size == 64));
  INT i, num = size / width;
  switch (width)
  {
    case 32:
    {
      INT* Dst = (INT*)dst;
      INT* Src1 = (INT*)src1;
      INT* Src2 = (INT*)src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_sshl_rnd_s32(Src1[i], Src2[i]);
      }
    }
    break;
  }
}

static inline void __A64_sqshl_imm(INT width, INT size, INT64 &dst, INT64 &src, INT imm)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((imm >= 0) && (imm < width));
  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64 *Dst = (INT64 *) dst;
      INT64 *Src = (INT64 *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_shl_s64(Src[i], imm);
      }
    }
    break;
    case 32:
    {
      INT *Dst = (INT *) dst;
      INT *Src = (INT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_shl_s32(Src[i], imm);
      }
    }
    break;
    case 16:
    {
      SHORT *Dst = (SHORT *) dst;
      SHORT *Src = (SHORT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_shl_s16(Src[i], imm);
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR *Src = (SCHAR *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = FDK_sat_shl_s8(Src[i], imm);
      }
    }
    break;
  }
}

static inline void __A64_sshr_imm(INT width, INT size, INT64 &dst, INT64 &src, INT64 imm)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((imm >= 0) && (imm < width));
  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64 *Dst = (INT64 *) dst;
      INT64 *Src = (INT64 *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] >> imm;
      }
    }
    break;
    case 32:
    {
      INT *Dst = (INT *) dst;
      INT *Src = (INT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] >> imm;
      }
    }
    break;
    case 16:
    {
      SHORT *Dst = (SHORT *) dst;
      SHORT *Src = (SHORT *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] >> imm;
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst = (SCHAR *) dst;
      SCHAR *Src = (SCHAR *) src;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src[i] >> imm;
      }
    }
    break;
  }
}

static inline void __A64_ssra_imm(INT width, INT size, INT64& dst, INT64& src, INT64 imm)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT((imm >= 0) && (imm < width));
  INT i, num = size / width;
  switch (width)
  {
  case 64:
  {
    INT64* Dst = (INT64*)dst;
    INT64* Src = (INT64*)src;
    for (i = 0; i < num; i++)
    {
      Dst[i] += Src[i] >> imm;
    }
  }
  break;
  case 32:
  {
    INT* Dst = (INT*)dst;
    INT* Src = (INT*)src;
    for (i = 0; i < num; i++)
    {
      Dst[i] += Src[i] >> imm;
    }
  }
  break;
  case 16:
  {
    SHORT* Dst = (SHORT*)dst;
    SHORT* Src = (SHORT*)src;
    for (i = 0; i < num; i++)
    {
      Dst[i] += Src[i] >> imm;
    }
  }
  break;
  case 8:
  {
    SCHAR* Dst = (SCHAR*)dst;
    SCHAR* Src = (SCHAR*)src;
    for (i = 0; i < num; i++)
    {
      Dst[i] += Src[i] >> imm;
    }
  }
  break;
  }
}


static inline void __A64_sshl(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT(!((width == 64) && (size == 64)));

  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64 *Dst  = (INT64 *) dst;
      INT64 *Src1 = (INT64 *) src1;
      INT64 *Src2 = (INT64 *) src2;
      for (i = 0; i < num; i++)
      {
        if (Src2[i] >= 0)
          Dst[i] = Src1[i] << Src2[i];
        else
          Dst[i] = Src1[i] >> (-Src2[i]);

      }
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
        if (Src2[i] >= 0)
          Dst[i] = Src1[i] << Src2[i];
        else
          Dst[i] = Src1[i] >> (-Src2[i]);
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src2;
      SHORT *Src2 = (SHORT *) src1;
      for (i = 0; i < num; i++)
      {
        if (Src2[i] >= 0)
          Dst[i] = Src1[i] << Src2[i];
        else
          Dst[i] = Src1[i] >> (-Src2[i]);
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) dst;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
        if (Src2[i] >= 0)
          Dst[i] = Src1[i] << Src2[i];
        else
          Dst[i] = Src1[i] >> (-Src2[i]);
      }
    }
    break;
  }
}

static inline void __A64_sshr(INT width, INT size, INT64 &dst, INT64 &src1, INT64 &src2)
{
  FDK_ASSERT((width == 64) || (width == 32) || (width == 16) || (width == 8));
  FDK_ASSERT((size == 128) || (size == 64));
  FDK_ASSERT(!((width == 64) && (size == 64)));

  INT i, num = size / width;
  switch (width)
  {
    case 64:
    {
      INT64 *Dst  = (INT64 *) dst;
      INT64 *Src1 = (INT64 *) src1;
      INT64 *Src2 = (INT64 *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] >> Src2[i];
      }
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) dst;
      INT *Src1 = (INT *) src1;
      INT *Src2 = (INT *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] >> Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) dst;
      SHORT *Src1 = (SHORT *) src2;
      SHORT *Src2 = (SHORT *) src1;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] >> Src2[i];
      }
    }
    break;
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) dst;
      SCHAR *Src1 = (SCHAR *) src1;
      SCHAR *Src2 = (SCHAR *) src2;
      for (i = 0; i < num; i++)
      {
        Dst[i] = Src1[i] >> Src2[i];
      }
    }
    break;
  }
}

#endif

#ifdef __ARM_AARCH64_NEON__
#define __A64_ands_Xt_imm(dst, src, imm)           "ANDS " #dst ", " #src ",  # " #imm "  \n\t"
#define __A64_ands_Wt_imm(dst, src, imm)           "ANDS " #dst ", " #src ",  # " #imm "  \n\t"
#define A64_ands_Xt_imm(dst, src, imm)             __A64_ands_Xt_imm(dst, src, imm)
#define A64_ands_Wt_imm(dst, src, imm)             __A64_ands_Wt_imm(dst, src, imm)

#define __A64_and_Xt_imm(dst, src, imm)            "AND " #dst ", " #src ",  # " #imm "  \n\t"
#define __A64_and_Wt_imm(dst, src, imm)            "AND " #dst ", " #src ",  # " #imm "  \n\t"
#define A64_and_Xt_imm(dst, src, imm)              __A64_and_Xt_imm(dst, src, imm)
#define A64_and_Wt_imm(dst, src, imm)              __A64_and_Wt_imm(dst, src, imm)

#define __A64_and_Xt_lsl_imm(Xd, Xn, Xm, imm)      "AND  " #Xd ", " #Xn ", " #Xm ", LSL # " #imm " \n\t"
#define __A64_and_Xt_lsr_imm(Xd, Xn, Xm, imm)      "AND  " #Xd ", " #Xn ", " #Xm ", LSR # " #imm " \n\t"
#define A64_and_Xt_lsl_imm(Xd, Xn, Xm, imm)        __A64_and_Xt_lsl_imm(Xd, Xn, Xm, imm)
#define A64_and_Xt_lsr_imm(Xd, Xn, Xm, imm)        __A64_and_Xt_lsr_imm(Xd, Xn, Xm, imm)

#else
#define A64_and_Xt_imm(Xd, Xm, imm)          __A64_and_Xt_imm((INT64 &) Xd, (INT64 &) Xm, (INT64) imm);
#define A64_and_Wt_imm(Wd, Wm, imm)          __A64_and_Wt_imm((INT &) Wd, (INT &) Wm, (INT64) imm);
#define A64_ands_Xt_imm(Xd, Xm, imm)         __A64_ands_Xt_imm((INT64 &) Xd, (INT64 &) Xm, (INT64) imm);
#define A64_ands_Wt_imm(Wd, Wm, imm)         __A64_ands_Wt_imm((INT &) Wd, (INT &) Wm, (INT64) imm);
#define A64_and_Xt_lsl_imm(Xd, Xn, Xm, imm)  { FDK_ASSERT((imm >= 0) && (imm <= 63));  \
                                                Xd = (INT64) Xn & (INT64)((INT64) Xm << (imm));  }
#define A64_and_Xt_lsr_imm(Xd, Xn, Xm, imm)  { FDK_ASSERT((imm >= 0) && (imm <= 63));  \
                                                Xd = (INT64) Xn & (INT64)((INT64) Xm >> (imm));  }

static inline void __A64_and_Xt_imm(INT64 &Xd, INT64 &Xm, INT64 imm)
{
    INT64 result = Xm & imm;
extern INT64 * __XZR;
    if (&Xm == __XZR)
    {
      result = 0;
    }
    if (&Xd != __XZR)  // do not write to XZR
    {
      Xd = result;
    }
}
static inline void __A64_and_Wt_imm(INT &Wd, INT &Wm, INT64 imm)
{
    FDK_check_s32_overflow(imm);
    INT result = Wm & imm;
extern INT* __WZR;
    if (&Wm == __WZR)
    {
      result = 0;
    }
    if (&Wd != __WZR)  // do not write to XZR
    {
      Wd = result;
    }
}

static inline void __A64_ands_Xt_imm(INT64 &Xd, INT64 &Xm, INT64 imm)
{
extern INT64 * __XZR;
    INT64 result = Xm & imm;
    if (&Xm == __XZR)
    {
      result = 0;
    }
    if (&Xd != __XZR)  // do not write to XZR
    {
      Xd = result;
    }
    __FDK_coreflags_PL = (result>=0) ? 1 : 0;
    __FDK_coreflags_LE = (result<=0) ? 1 : 0;
    __FDK_coreflags_GT = (result> 0) ? 1 : 0;
    __FDK_coreflags_MI = (result< 0) ? 1 : 0;
    __FDK_coreflags_NE = result ? 1 : 0;
    __FDK_coreflags_EQ = result ? 0 : 1;
}

static inline void __A64_ands_Wt_imm(INT &Wd, INT &Wm, INT64 imm)
{
    FDK_check_s32_overflow(imm);
extern INT * __WZR;
    INT result = Wm & imm;
    if (&Wm == __WZR)
    {
      result = 0;
    }
    if (&Wd != __WZR)  // do not write to WZR
    {
      Wd = result;
    }
    __FDK_coreflags_PL = (result>=0) ? 1 : 0;
    __FDK_coreflags_LE = (result<=0) ? 1 : 0;
    __FDK_coreflags_GT = (result> 0) ? 1 : 0;
    __FDK_coreflags_MI = (result< 0) ? 1 : 0;
    __FDK_coreflags_NE = result ? 1 : 0;
    __FDK_coreflags_EQ = result ? 0 : 1;
}
#endif

#ifdef __ARM_AARCH64_NEON__
#define __A64_clz(dst, src)             "CLZ " #dst ", " #src " \n\t"
#define A64_clz(width, size, dst, src)  __A64_clz(dst, src)

#define A64_clz_Xt(Xd, Xm)              "CLZ " #Xd ", " #Xm " \n\t"
#define A64_clz_Wt(Wd, Wm)              "CLZ " #Wd ", " #Wm " \n\t"
#define A64_rbit_Xt(Xd, Xm)             "RBIT " #Xd ", " #Xm " \n\t"
#define A64_rbit_Wt(Wd, Wm)             "RBIT " #Wd ", " #Wm " \n\t"
#else
#define A64_clz(width, size, dst, src)   {FDK_ASSERT((size == 128) || (size == 64)); \
                                               if (size == 128)  { if (width == 32) { __FDK_vclz_32_q((A64_V)dst, (A64_V)src); } else if (width == 16)  { __FDK_vclz_16_q((A64_V)dst, (A64_V)src); } else if (width == 8) { __FDK_vclz_8_q((A64_V)dst, (A64_V)src); } }\
                                          else if (size == 64)   { if (width == 32) { __FDK_vclz_32_d((A64_X)dst, (A64_X)src); } else if (width == 16)  { __FDK_vclz_16_d((A64_X)dst, (A64_X)src); } else if (width == 8) { __FDK_vclz_8_d((A64_X)dst, (A64_X)src); } }\
                                         }
#define A64_clz_Xt(Xd, Xm)               Xd = fNormz64(Xm);
#define A64_clz_Wt(Wd, Wm)               Wd = fNormz(Wm);
#define A64_rbit_Xt(Xd, Xm)              Xd = __A64_rbit_Xt((UINT64) Xm);
#define A64_rbit_Wt(Wd, Wm)              Wd = __A64_rbit_Wt((UINT) Wm);

static void inline __FDK_vclz_32_q(A64_V dst, A64_V src)
{
  A64_S *Src = (A64_S *) src;
  A64_S *Dst = (A64_S *) dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = (A64_S) CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_32_d(A64_X dst, A64_X src)
{
  A64_S *Src = (A64_S *) src;
  A64_S *Dst = (A64_S *) dst;

  for (int i = 0; i < 2; i++)
    Dst[i] = (A64_S) CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_16_q(A64_V dst, A64_V src)
{
  A64_H *Src = (A64_H *) src;
  A64_H *Dst = (A64_H *) dst;

  for (int i = 0; i < 8; i++)
      Dst[i] = (A64_H) CntLeadingZeros(((INT)Src[i] << 16) | 0x00008000);
}
static void inline __FDK_vclz_16_d(A64_X dst, A64_X src)
{
  A64_H *Src = (A64_H *) src;
  A64_H *Dst = (A64_H *) dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = (A64_H) CntLeadingZeros(((INT) Src[i] << 16) | 0x00008000);
}
static void inline __FDK_vclz_8_q(A64_V dst, A64_V src)
{
  A64_B Src = (A64_B) src;
  A64_B Dst = (A64_B) dst;

  for (int i = 0; i < 16; i++)
      Dst[i] = CntLeadingZeros((((INT)(Src[i])) << 24) | 0x00800000);
}
static void inline __FDK_vclz_8_d(A64_X dst, A64_X src)
{
  A64_B Src = (A64_B) src;
  A64_B Dst = (A64_B) dst;

  for (int i = 0; i < 8; i++)
    Dst[i] = CntLeadingZeros((((INT)(Src[i])) << 24) | 0x00800000);
}

static inline UINT64 __A64_rbit_Xt(UINT64 Xm)
{
    UINT64 result = 0;
    for(int i = 0; i < 64; i++)
    {
        if (Xm & ((UINT64) 1 << i))
           result |=  (UINT64) 1 << (63 - i);
    }
    return result;
}

static inline UINT __A64_rbit_Wt(UINT Wm)
{
    UINT result = 0;
    for(int i = 0; i < 32; i++)
    {
        if (Wm & ((UINT) 1 << i))
           result |=  (UINT) 1 << (31 - i);
    }
    return result;
}

#endif


// Example:
//  addition of integer: r0 = r1 + (r2<<3)              => FDK_add_op_lsl(r0, r1, r2, 3, 0)
//                                                      => ADD r0, r1, r3, LSL #3
//  indexing a pointer:  r0 = (A64_S *) r1 + (r2<<3) => FDK_add_op_lsl(r0, r1, r2, 5, 2)
//                                                      => ARM: ADD r0, r1, r3, LSL #5
//                                                      => C++: A64_S *r0 = &r1[8*r2];
#ifdef __ARM_AARCH64_NEON__
  #define FDK_add_op_lsl(dst, src1, src2, imm, scale)    "ADD " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
  #define FDK_add_op_asr(dst, src1, src2, imm, scale)    "ADD " #dst ", " #src1 ", " #src2 ", ASR # " #imm "\n\t"
  #define FDK_sub_op_lsl(dst, src1, src2, imm, scale)    "SUB " #dst ", " #src1 ", " #src2 ", LSL # " #imm "\n\t"
  #define FDK_sub_op_asr(dst, src1, src2, imm, scale)    "SUB " #dst ", " #src1 ", " #src2 ", ASR # " #imm "\n\t"
#else
#define FDK_add_op_lsl(dst, src1, src2, imm, scale)      dst = src1 + (src2<<(imm-scale));
#define FDK_add_op_asr(dst, src1, src2, imm, scale)      dst = src1 + (src2>>(imm-scale));
#define FDK_sub_op_lsl(dst, src1, src2, imm, scale)      dst = src1 - (src2<<(imm-scale));
#define FDK_sub_op_asr(dst, src1, src2, imm, scale)      dst = src1 - (src2>>(imm-scale));
#endif

#ifdef __ARM_AARCH64_NEON__
  #define FDK_sub(dst, src1, src2)    "SUB " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_rsb_imm(dst, src, imm)    "RSB " #dst ", " #src ", # " #imm "\n\t"
#else
#define FDK_sub(dst, src1, src2)      dst = src1 - src2;
#define FDK_rsb_imm(dst, src, imm)    dst = imm - src;
#endif

#ifdef __ARM_AARCH64_NEON__
#define A64_subs_imm(dst, src, immediate)       "SUBS " #dst ", " #src ", # " #immediate "\n\t"
#define A64_subs(dst, src1, src2)               "SUBS " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_sub_Xt_imm(dst,src,immediate)       "SUB  " #dst ", " #src ", # " #immediate "\n\t"
#define A64_sub_Wt_imm(dst,src,immediate)       "SUB  " #dst ", " #src ", # " #immediate "\n\t"
#define A64_sub_Xt(dst, src1, src2)             "SUB  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_sub_Wt(dst, src1, src2)             "SUB  " #dst ", " #src1 ", " #src2 "\n\t"
#define A64_add_Xt_imm(dst,src,immediate)       "ADD  " #dst ", " #src ", # " #immediate "\n\t"
#define A64_add_Wt_imm(dst,src,immediate)       "ADD  " #dst ", " #src ", # " #immediate "\n\t"
#else
#define A64_subs_imm(dst, src, immediate)  { dst = src - immediate; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }
#define A64_subs(dst, src1, src2)          { dst = src1 - src2; \
                                             __FDK_coreflags_PL = (dst>=0) ? 1 : 0; \
                                             __FDK_coreflags_LE = (dst<=0) ? 1 : 0; \
                                             __FDK_coreflags_GT = (dst> 0) ? 1 : 0; \
                                             __FDK_coreflags_MI = (dst< 0) ? 1 : 0; \
                                             __FDK_coreflags_NE = dst ? 1 : 0; \
                                             __FDK_coreflags_EQ = dst ? 0 : 1; }
#define A64_sub_Xt_imm(dst, src, immediate)   { (INT64 &)dst = (INT64 &) src - (INT64)(immediate); }
#define A64_sub_Wt_imm(dst, src, immediate)   { dst = (INT)src - (INT)(immediate); }
#define A64_sub_Xt(dst, src1, src2)           { dst = (INT64) src1 - (INT64) src2; }
#define A64_sub_Wt(dst, src1, src2)           { extern INT * __WZR;  \
                                                if ((INT64 *) (&src1) == (INT64 *) __WZR) \
                                                 dst = (INT64) ((INT) 0 - (INT) src2) & ((INT64) 0x00000000FFFFFFFF); \
                                                else \
                                                 dst = (INT64) ((INT) src1 - (INT) src2) & ((INT64) 0x00000000FFFFFFFF); \
                                              }
#define A64_add_Xt_imm(dst, src, immediate)   { dst = src + (INT64)(immediate)/sizeof(src[0]); }
#define A64_add_Wt_imm(dst, src, immediate)   { dst = (INT)src + (INT)(immediate); }
#endif

#ifdef __ARM_AARCH64_NEON__
#define FDK_pkhbt_lsl_imm(Rd, Rn, Rm, imm)  "PKHBT " #Rd ", " #Rn ", " #Rm ", LSL #" #imm " \n\t"
#else
#define FDK_pkhbt_lsl_imm(Rd, Rn, Rm, imm) FDK_ASSERT(imm >= 0 && imm < 32); Rd = INT(Rn & 0x0000FFFF) | (INT((Rm << imm) & INT(0xFFFF0000)));
#endif


#ifdef __ARM_AARCH64_NEON__
#define __A64_cmp_Xt_imm(src, immediate)     "CMP " #src ", # " #immediate "\n\t"
#define __A64_cmp_Wt_imm(src, immediate)     "CMP " #src ", # " #immediate "\n\t"
#define __A64_cmp_Xt(src1, src2)             "CMP " #src1 ", " #src2 "\n\t"
#define __A64_cmp_Wt(src1, src2)             "CMP " #src1 ", " #src2 "\n\t"
#define __A64_cmp_Xt_X_ASR(src1, src2, src3) "CMP " #src1 ", " #src2 " ASR " #src3 "\n\t"
#define A64_cmp_Xt_imm(src, immediate)     __A64_cmp_Xt_imm(src, immediate)
#define A64_cmp_Wt_imm(src, immediate)     __A64_cmp_Wt_imm(src, immediate)
#define A64_cmp_Xt(src1, src2)             __A64_cmp_Xt(src1, src2)
#define A64_cmp_Xt_X_ASR(src1, src2, src3) __A64_cmp_Xt_X_ASR(src1, src2, src3)
#define A64_cmp_Wt(src1, src2)             __A64_cmp_Wt(src1, src2)
#else
#define A64_cmp_Xt_imm(src, immediate)  {__FDK_coreflags_EQ = (INT64(src) == (INT64)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT64(src) != (INT64)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT64(src) >= (INT64)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT64(src) <  (INT64)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT64(src) >  (INT64)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT64(src) <= (INT64)(immediate)) ? 1 : 0;  }
#define A64_cmp_Wt_imm(src, immediate)  {__FDK_coreflags_EQ = (INT(src) == (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src) != (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src) >= (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src) <  (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src) >  (INT)(immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src) <= (INT)(immediate)) ? 1 : 0;  }
#define A64_cmp_Xt(src1, src2)          {__FDK_coreflags_EQ = (INT64(src1) == (INT64)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT64(src1) != (INT64)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT64(src1) >= (INT64)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT64(src1) <  (INT64)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT64(src1) >  (INT64)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT64(src1) <= (INT64)(src2)) ? 1 : 0;  }

#define A64_cmp_Wt(src1, src2)          {__FDK_coreflags_EQ = (INT(src1) == (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src1) != (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src1) >= (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src1) <  (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src1) >  (INT)(src2)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src1) <= (INT)(src2)) ? 1 : 0;  }

#define A64_cmp_Xt_X_ASR(src1, src2, src3)  {__FDK_coreflags_EQ = (INT64(src1) == ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0; \
                                             __FDK_coreflags_NE = (INT64(src1) != ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0; \
                                             __FDK_coreflags_GE = (INT64(src1) >= ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0; \
                                             __FDK_coreflags_LT = (INT64(src1) <  ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0; \
                                             __FDK_coreflags_GT = (INT64(src1) >  ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0; \
                                             __FDK_coreflags_LE = (INT64(src1) <= ((INT64)(src2) >> (INT64)(src3))) ? 1 : 0;  }
#endif
#ifdef __ARM_AARCH64_NEON__
#define FDK_cmn_imm(src, immediate)  "CMN " #src ", # " #immediate "\n\t"
#else
#define FDK_cmn_imm(src, immediate)  {   __FDK_coreflags_EQ = (INT(src) == (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_NE = (INT(src) != (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GE = (INT(src) >= (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LT = (INT(src) <  (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_GT = (INT(src) >  (INT)(-immediate)) ? 1 : 0; \
                                         __FDK_coreflags_LE = (INT(src) <= (INT)(-immediate)) ? 1 : 0;  }
#endif


#ifdef __ARM_AARCH64_NEON__
#define A64_return()         "RET \n\t"
#else
#define A64_return()         return;
#endif

#ifdef __ARM_AARCH64_NEON__
#define A64_label(name)       "" #name ": \n\t"
#else
#define A64_label(name)       name:
#endif

// For parameter 'cond' please refer to FDK_aarch64_neon_regs.h
#ifdef __ARM_AARCH64_NEON__
#define __A64_branch_AL(cond,name)   "B          " #name " \n\t"
#define __A64_branch_EQ(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_NE(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_CS(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_CC(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_MI(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_PL(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_GE(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_LT(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_GT(cond,name)   "B" #cond " " #name " \n\t"
#define __A64_branch_LE(cond,name)   "B" #cond " " #name " \n\t"
#define A64_branch(cond,name)        __A64_branch_##cond(cond,name)
#else
#define A64_branch(cond,name)        if (__FDK_coreflags_ ## cond) goto name;
#endif

#ifdef __ARM_AARCH64_NEON__
#define A64_tbz_Wt(rt, imm6, imm14)  " TBZ " #rt ", " #imm6 ", " #imm14 " \n\t"  
#define A64_tbz_Xt(rt, imm6, imm14)  " TBZ " #rt ", " #imm6 ", " #imm14 " \n\t"  
#define A64_tbnz_Wt(rt, imm6, imm14) " TBNZ " #rt ", " #imm6 ", " #imm14 " \n\t"  
#define A64_tbnz_Xt(rt, imm6, imm14) " TBNZ " #rt ", " #imm6 ", " #imm14 " \n\t"  
#else
#define A64_tbz_Wt(rt, imm6, imm14)  FDK_ASSERT( ( (INT64)(imm6) >= (INT64)0 ) && ( (INT64)(imm6) <= (INT64)31 ) ); \
                                     if ( !((INT)(rt) & ((INT)1<<(INT)(imm6))) ) goto imm14;

#define A64_tbz_Xt(rt, imm6, imm14)  FDK_ASSERT( ( (INT64)(imm6) >= (INT64)0 ) && ( (INT64)(imm6) <= (INT64)63 ) ); \
                                     if ( !((INT64)(rt) & ((INT64)1<<(INT)(imm6))) ) goto imm14;

#define A64_tbnz_Wt(rt, imm6, imm14) FDK_ASSERT( ( (INT64)(imm6) >= (INT64)0 ) && ( (INT64)(imm6) <= (INT64)31 ) ); \
                                     if ( ((INT)(rt) & ((INT)1<<(INT)(imm6))) ) goto imm14;

#define A64_tbnz_Xt(rt, imm6, imm14) FDK_ASSERT( ( (INT64)(imm6) >= (INT64)0 ) && ( (INT64)(imm6) <= (INT64)63 ) ); \
                                     if ( ((INT64)(rt) & ((INT64)1<<(INT)(imm6))) ) goto imm14;
#endif

/*----------------------------------------------------------------------------*/
/* Pseudo instructions to allow embedded assembly                             */
/*----------------------------------------------------------------------------*/


#if defined (__ARM_AARCH64_NEON__) && defined (__GNUC__)
#define FDK_ASM_ROUTINE         __attribute__((noinline))
#define FDK_INLINE_ASM_ROUTINE  __attribute__((always_inline)) static
#else
#define FDK_ASM_ROUTINE
#define FDK_INLINE_ASM_ROUTINE  FDK_INLINE
#endif


#if defined (__ARM_AARCH64_NEON__) && defined (__GNUC__)
#define A64_ASM_START()   __asm__ (
#define A64_ASM_START_RETURN()  INT result;  __asm__ (
#define A64_ASM_END()        ::: );
#define A64_ASM_END_RETURN() "mov %0, r0;\n" : "=r"(result) :: ); return result;
#else
#define A64_ASM_START()
#define A64_ASM_START_RETURN()
#define A64_ASM_END()
#define A64_ASM_END_RETURN()
#endif


#if defined (__ARM_AARCH64_NEON__) && defined (__GNUC__)
#ifdef __cplusplus
#define A64_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             extern "C" { proc_type proc_name proc_args;   }  \
                             asm ( "\n\t" \
                             ".section .text\n\t" \
                             "" #proc_name ": \n\t"
#else
#define A64_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                             proc_type proc_name proc_args;    \
                             __asm__ ( "\n\t" \
                             ".section .text\n\t" \
                             "" #proc_name ": \n\t"
#endif                             
#define A64_ASM_ROUTINE_END()               A64_return()  );
#define A64_ASM_ROUTINE_RETURN(proc_type)   A64_return()  \
                                           );
#else
#define A64_ASM_ROUTINE_START(proc_type, proc_name, proc_args)   \
                              proc_type  proc_name  proc_args {
#define A64_ASM_ROUTINE_END()              }
#define A64_ASM_ROUTINE_RETURN(proc_type)  return (proc_type) X0; }
#endif

#endif  /* FDK_AARCH64_NEON_FUNCS_H */
