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

   Author(s):   Josef Hoepfl, A. Tritthart

   Description: Fix point FFT

*******************************************************************************/

#include "fft_rad2.h"
#include "FDK_tools_rom.h"

#define W_PiFOURTH STC(0x5a82799a)
//#define W_PiFOURTH ((FIXP_DBL)(0x5a82799a))
#ifndef SUMDIFF_PIFOURTH
#define SUMDIFF_PIFOURTH(diff, sum, a, b) \
  {                                       \
    FIXP_DBL wa, wb;                      \
    wa = fMultDiv2(a, W_PiFOURTH);        \
    wb = fMultDiv2(b, W_PiFOURTH);        \
    diff = wb - wa;                       \
    sum = wb + wa;                        \
  }
#define SUMDIFF_PIFOURTH16(diff, sum, a, b)       \
  {                                               \
    FIXP_SGL wa, wb;                              \
    wa = FX_DBL2FX_SGL(fMultDiv2(a, W_PiFOURTH)); \
    wb = FX_DBL2FX_SGL(fMultDiv2(b, W_PiFOURTH)); \
    diff = wb - wa;                               \
    sum = wb + wa;                                \
  }
#endif /* #ifndef SUMDIFF_PIFOURTH */

#define SCALEFACTOR2048 10
#define SCALEFACTOR1024 9
#define SCALEFACTOR512 8
#define SCALEFACTOR256 7
#define SCALEFACTOR128 6
#define SCALEFACTOR64 5
#define SCALEFACTOR32 4
#define SCALEFACTOR16 3
#define SCALEFACTOR8 2
#define SCALEFACTOR4 1
#define SCALEFACTOR2 1

#define SCALEFACTOR3 1
#define SCALEFACTOR5 1
#define SCALEFACTOR6 4
#define SCALEFACTOR7 2
#define SCALEFACTOR9 2
#define SCALEFACTOR10 5
#define SCALEFACTOR12 3
#define SCALEFACTOR15 3
#define SCALEFACTOR18 (SCALEFACTOR2 + SCALEFACTOR9 + 2)
#define SCALEFACTOR20 (SCALEFACTOR4 + SCALEFACTOR5 + 2)
#define SCALEFACTOR21 (SCALEFACTOR3 + SCALEFACTOR7 + 2)
#define SCALEFACTOR24 (SCALEFACTOR2 + SCALEFACTOR12 + 2)
#define SCALEFACTOR30 (SCALEFACTOR2 + SCALEFACTOR15 + 2)
#define SCALEFACTOR40 (SCALEFACTOR5 + SCALEFACTOR8 + 2)
#define SCALEFACTOR48 (SCALEFACTOR4 + SCALEFACTOR12 + 2)
#define SCALEFACTOR60 (SCALEFACTOR4 + SCALEFACTOR15 + 2)
#define SCALEFACTOR80 (SCALEFACTOR5 + SCALEFACTOR16 + 2)
#define SCALEFACTOR96 (SCALEFACTOR3 + SCALEFACTOR32 + 2)
#define SCALEFACTOR120 (SCALEFACTOR8 + SCALEFACTOR15 + 2)
#define SCALEFACTOR160 (SCALEFACTOR10 + SCALEFACTOR16 + 2)
#define SCALEFACTOR168 (SCALEFACTOR21 + SCALEFACTOR8 + 2)
#define SCALEFACTOR192 (SCALEFACTOR12 + SCALEFACTOR16 + 2)
#define SCALEFACTOR240 (SCALEFACTOR16 + SCALEFACTOR15 + 2)
#define SCALEFACTOR320 (SCALEFACTOR10 + SCALEFACTOR32 + 2)
#define SCALEFACTOR336 (SCALEFACTOR21 + SCALEFACTOR16 + 2)
#define SCALEFACTOR384 (SCALEFACTOR12 + SCALEFACTOR32 + 2)
#define SCALEFACTOR480 (SCALEFACTOR32 + SCALEFACTOR15 + 2)
#define SCALEFACTOR768 (SCALEFACTOR12 + SCALEFACTOR64 + 2)
#define SCALEFACTOR960 (SCALEFACTOR15 + SCALEFACTOR64 + 2)

#include "fft.h"

#define C31 (STC(0x91261468)) /* FL2FXCONST_DBL(-0.86602540) = -sqrt(3)/2  */

/*
 Select shift placement.
 Some processors like ARM may shift "for free" in combination with an addition or substraction,
 but others don't so either combining shift with +/- or reduce the total amount or shift operations
 is optimal
 */
#if !defined(__arm__)
#define SHIFT_A >> 1
#define SHIFT_B
#else
#define SHIFT_A
#define SHIFT_B >> 1
#endif

/*!
 *
 *  \brief  complex FFT of length 12,18,24,30,48,60,96, 192, 240, 384, 480
 *  \param  pInput contains the input signal prescaled right by 2
 *          pInput contains the output signal scaled by SCALEFACTOR<#length>
 *          The output signal does not have any fixed headroom
 *  \return void
 *
 */

void fft(int length, FIXP_DBL* pInput, INT* pScalefactor) {
  /* Ensure, that the io-ptr is always (at least 8-byte) aligned */
  C_ALLOC_ALIGNED_CHECK(pInput);

  {
    switch (length) {
      case 64:
        dit_fft(pInput, 6, SineTable512, 512);
        *pScalefactor += SCALEFACTOR64;
        break;
      case 128:
        dit_fft(pInput, 7, SineTable512, 512);
        *pScalefactor += SCALEFACTOR128;
        break;
      case 256:
        dit_fft(pInput, 8, SineTable512, 512);
        *pScalefactor += SCALEFACTOR256;
        break;
      case 512:
        dit_fft(pInput, 9, SineTable512, 512);
        *pScalefactor += SCALEFACTOR512;
        break;
      default:
        FDK_ASSERT(0); /* FFT length not supported! */
        break;
    }
  }
}

void ifft(int length, FIXP_DBL* pInput, INT* scalefactor) {
  switch (length) {
    default:
      FDK_ASSERT(0); /* IFFT length not supported! */
      break;
  }
}
