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

#ifndef SCRAMBLE_H
#define SCRAMBLE_H

#include "common_fix.h"

#if defined(__arm__)
#include "arm/scramble_arm.h"

#endif

/*****************************************************************************

    functionname: scramble
    description:  bitreversal of input data
    returns:
    input:
    output:

*****************************************************************************/
#if !defined(FUNCTION_scramble)

inline void scramble(FIXP_DBL* x, INT length) {
  C_ALLOC_ALIGNED_CHECK(x);
  DWORD_ALIGNED(x)

#ifndef SCRAMBLE
#define SCRAMBLE(x, a, b) \
  a0 = x[2 * (a) + 0];    \
  a1 = x[2 * (a) + 1];    \
  b0 = x[2 * (b) + 0];    \
  b1 = x[2 * (b) + 1];    \
  x[2 * (b) + 0] = a0;    \
  x[2 * (b) + 1] = a1;    \
  x[2 * (a) + 0] = b0;    \
  x[2 * (a) + 1] = b1;
#endif

  FIXP_DBL a0, a1, b0, b1;

  switch (length) {
    case 64: {
      /* Note: This version needs on ARM926 only 33% of the cycles compared to the generic loop
       * below */
      /* This table indicates, which entries of the vector should be exchanged. */
      static const UCHAR br_offsets64[28][2] = {
          {1, 32},  {2, 16},  {3, 48},  {4, 8},   {5, 40},  {6, 24},  {7, 56},
          {9, 36},  {10, 20}, {11, 52}, {13, 44}, {14, 28}, {15, 60}, {17, 34},
          {19, 50}, {21, 42}, {22, 26}, {23, 58}, {25, 38}, {27, 54}, {29, 46},
          {31, 62}, {35, 49}, {37, 41}, {39, 57}, {43, 53}, {47, 61}, {55, 59}};
      int i;

      for (i = 28 - 1; i >= 0; i--) {
        int src = br_offsets64[i][0];
        int dst = br_offsets64[i][1];
        SCRAMBLE(x, src, dst)
      }
    } break;

    case 128: {
      /* In order to bit-reverse an 6-bit value, the lookup   */
      /* table must be used twice:                            */
      /* rev(n) = bitreverse[n&0xF]<<2 + bitreverse[n>>4]>>2  */

      static const UCHAR bitreverse[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                           0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};

      /* These bits indicate, if the entry x[n] has to be exchanged or not.  */
      /* The last words are skipped and replaced by hard-coded exchanges at  */
      /* the procedure end. The table was generated with a C-code.           */
      static const ULONG bits[3] = {0xEEEEFEFE, 0xAAAAEAEA, 0x8888A8A8, /* 0x00008080 */};

      int i, j, rev_n;
      ULONG bits16;

      int n = 0;
      for (i = 0; i < 2; i++) /* bits[0,1] */
      {
        bits16 = bits[i];
        for (j = 16; j--; n++) {
          rev_n = (bitreverse[n & 0xF] << 2) + (bitreverse[n >> 4] >> 2);
          SCRAMBLE(x, 2 * n + 1, rev_n + 64)
          if (bits16 & 1) {
            SCRAMBLE(x, 2 * n, rev_n)
          }
          bits16 >>= 2;
        }
      }

      bits16 = bits[i];
      for (j = 8; j--; n++) /* bits[2] */
      {
        if (bits16 & 2) {
          rev_n = (bitreverse[n & 0xF] << 2) + (bitreverse[n >> 4] >> 2);
          SCRAMBLE(x, 2 * n + 1, rev_n + 64)
        }
        n++;
        if (bits16 & 8) {
          rev_n = (bitreverse[n & 0xF] << 2) + (bitreverse[n >> 4] >> 2);
          SCRAMBLE(x, 2 * n + 1, rev_n + 64)
        }
        bits16 >>= 4;
      }

      SCRAMBLE(x, 103, 115)
      SCRAMBLE(x, 111, 123)
    } break;

    case 256: {
#ifdef FUNCTION_scramble_256
      scramble_256(x);
#else
      /* In order to bit-reverse a 7-bit value, the lookup    */
      /* table must be used twice:                            */
      /* rev_n = (bitreverse[n&0xF]<<3)+(bitreverse[n>>4]>>1) */
      static const UCHAR bitreverse[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                           0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};

      /* These bits indicate, if the entry x[n] has to be exchanged or not.  */
      /* The last words are skipped and replaced by hard-coded exchange at   */
      /* the procedure end. The table was generated with a C-code.           */
      /* Note: The table is the same as for case 512, but packed in LONG     */
      static const ULONG bits[7] = {
          0xFEFEFFFE, 0xEEEEFEEE, 0xEAEAEEEA, 0xAAAAEAAA,
          0xA8A8AAA8, 0x8888A888, 0x80808880, /*0x00008000*/
      };

      int i, j, rev_n;
      ULONG bits16;

      int n = 0;
      /* First loop does not check bit #2, it is always set */
      for (i = 0; i < 4; i++) {
        bits16 = bits[i];
        int bitreverse_lsb = (int)(bitreverse[i] >> 1);
        for (j = 0; j < 16; j++, n++) {
          rev_n = bitreverse_lsb + (bitreverse[j] << 3);
          SCRAMBLE(x, 2 * n + 1, rev_n + 128)
          if (bits16 & 1) {
            SCRAMBLE(x, 2 * n, rev_n)
          }
          bits16 >>= 2;
        }
      }
      /* Second loop does not check bit #1, it is always cleared */
      for (; i < 7; i++) {
        bits16 = bits[i];
        int bitreverse_lsb = (int)(bitreverse[i] >> 1) + 128;
        for (j = 0; j < 16; j++, n++) {
          if (bits16 & 2) {
            rev_n = bitreverse_lsb + (bitreverse[j] << 3);
            SCRAMBLE(x, 2 * n + 1, rev_n)
          }
          bits16 >>= 2;
        }
      }

      SCRAMBLE(x, 239, 247)
#endif /* #ifdef FUNCTION_scramble_256 */
    } break;

    case 512: {
      /* Note: This version needs on ARM926 only 48% of the cycles compared to the generic loop
       * below */

      /* A 4-bit bit-reverse is done via lookup-table.      */
      /* Usage: rev(n) = bitreverse[n];  with n = [0..15]   */
      /* In order to bit-reverse an 8-bit value, the lookup */
      /* table must be used for each nibble:                */
      /* rev(n) = bitreverse[n&0xF]<<4 + bitreverse[n>>4];  */
      static const UCHAR bitreverse[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                           0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};

      /* These bits indicate, if the entry x[n] has to be exchanged or not.  */
      /* Each word must be used twice, the table is thus reduced to the half.*/
      /* The last words are skipped and replaced by hard-coded exchanges at  */
      /* the procedure end. The table was generated with a C-code.           */
      static const USHORT bits[14] = {
          0xFFFE, 0xFEFE, 0xFEEE, 0xEEEE, 0xEEEA, 0xEAEA, 0xEAAA,
          0xAAAA, 0xAAA8, 0xA8A8, 0xA888, 0x8888, 0x8880, 0x8080 /*0x8000, 0x0000 */
      };

      int i, j, rev_n;
      ULONG bits16;

      int n = 0;
      /* First loop does not check bit #2, it is always set */
      for (i = 0; i < 8; i++) {
        bits16 = (ULONG)bits[i];
        bits16 = bits16 + (bits16 << 16); /* duplicate bits16 into high and low part of 32-bit */
        int bitreverse_lsb = (int)(bitreverse[i] >> 0);
        for (j = 0; j < 16; j++, n++) {
          rev_n = bitreverse_lsb + (bitreverse[j] << 4);
          if (bits16 & 1) {
            SCRAMBLE(x, 2 * n, rev_n)
          }
          SCRAMBLE(x, 2 * n + 1, rev_n + 256)
          bits16 >>= 2;
        }
      }
      /* Second loop does not check bit #1, it is always cleared */
      for (; i < 14; i++) {
        bits16 = (ULONG)bits[i];
        bits16 = bits16 + (bits16 << 16); /* duplicate bits16 into high and low part of 32-bit */
        int bitreverse_lsb = (int)(bitreverse[i] >> 0) + 256;
        for (j = 0; j < 16; j++, n++) {
          if (bits16 & 2) {
            rev_n = bitreverse_lsb + (bitreverse[j] << 4);
            SCRAMBLE(x, 2 * n + 1, rev_n)
          }
          bits16 >>= 2;
        }
      }
      SCRAMBLE(x, 463, 487)
      SCRAMBLE(x, 479, 503)
    } break;
    default:
      INT m, k, j;
      for (m = 1, j = 0; m < length - 1; m++) {
        {
          for (k = length >> 1; (!((j ^= k) & k)); k >>= 1)
            ;
        }
        if (j > m) {
          SCRAMBLE(x, m, j)
        }
      }
      break;
  }
}

#endif /* !defined(FUNCTION_scramble) */

#endif /* SCRAMBLE_H */
