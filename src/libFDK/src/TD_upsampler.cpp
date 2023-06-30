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

   Author(s):   M. Huettenberger

   Description: Fixed-point implementation of a factor 1, 1.5, 2 and 3
                upsampling LP-interpolator utilizing biquad sections.

*******************************************************************************/

#include "TD_upsampler.h"

/* BiQuad Coefficient */
#define TD_BQC(x) (FX_DBL2FXCONST_SGL(x))

/* helper indices: position of coefficient in sos-array */
#define B1 0
#define B2 1
#define A1 2
#define A2 3

struct filter {
  FIXP_SGL coeff[7][4];
  FIXP_SGL sos_gain[7];
  FIXP_DBL scaleFacMantissa;
  char delay[2];
  char scaleFacExp;
};

/* biquad coefficient tables */
RAM_ALIGN
LNK_SECTION_CONSTDATA
static struct filter sos_2 = {
    /* Filter for upsampling factor: 2

    Filter parameters:
    - Order:                14
    - Stopband Attenuation: 85 dB
    - Cutoff Frequency:     0.56 rad
    - Filter Type:          Chebyshev Type 2 for passband without ripple
    - Gain:                 2

    Parameter generated with matlab/octave:
    - gain values calculated in this manner that overflow is avoided after each
      biquad-section if input does not exceed unity

    Comments:
    - Biquad coefficients for chebychev interpolator (b1, b2, a1, a2)
    */

    {
        /*       b1               b2               a1              a2          */
        {
            TD_BQC(0x2bbc6c1e),
            TD_BQC(0x40000000),
            TD_BQC(0x02554192),
            TD_BQC(0x1d6c8624),
        },
        {
            TD_BQC(0x7dd285e5),
            TD_BQC(0x40000000),
            TD_BQC(0x181c3ece),
            TD_BQC(0x02bbc357),
        },
        {
            TD_BQC(0x5674b149),
            TD_BQC(0x40000000),
            TD_BQC(0x0fc80d9a),
            TD_BQC(0x0b873685),
        },
        {
            TD_BQC(0x18c34def),
            TD_BQC(0x40000000),
            TD_BQC(0xf85eb0fa),
            TD_BQC(0x377fcdc8),
        },
        {
            TD_BQC(0x3ebbaec9),
            TD_BQC(0x40000000),
            TD_BQC(0x09230688),
            TD_BQC(0x13878202),
        },
        {
            TD_BQC(0x6e374b62),
            TD_BQC(0x40000000),
            TD_BQC(0x151ee9a7),
            TD_BQC(0x05c5fb45),
        },
        {
            TD_BQC(0x1f089256),
            TD_BQC(0x40000000),
            TD_BQC(0xfc6ceb47),
            TD_BQC(0x2937690a),
        },
    },

    /* soso_exp is 1 due to division by 2 in the table for upsampling 2 above */
    /* static const int sos_exp = 1; */

    /* gain per biquad-section * scaling */
    {
        TD_BQC(0x475edaa8),
        TD_BQC(0x2dcfc3fa),
        TD_BQC(0x367fbfeb),
        TD_BQC(0x5dbc2fff),
        TD_BQC(0x3e30111c),
        TD_BQC(0x30d7042e),
        TD_BQC(0x51cec5ee),
    },

    /* unscaling mantissa*/
    0x7fdffffe,

    /* filter delay */
    {
        (char)4, (char)0, /* second value is a dummy */
    },

    /* unscaling exponent*/
    (char)1,
};

RAM_ALIGN
LNK_SECTION_CONSTDATA
static struct filter sos_3 = {

    /* Filter for upsampling factor: 1.5; 3

    Filter parameters:
    - Order:                14
    - Stopband Attenuation: 85 dB
    - Cutoff Frequency:     0.39 rad
    - Filter Type:          Chebyshev Type 2 for passband without ripple
    - Gain:                 3

    Parameter generated with matlab/octave:
    - gain values calculated in this manner that overflow is avoided after each
    biquad-section if input does not exceed unity

    Comments:
    - Biquad coefficients for chebychev interpolator (b1, b2, a1, a2)
    */

    {
        /*       b1               b2               a1              a2          */
        {
            TD_BQC(0xe86d6d4d),
            TD_BQC(0x40000000),
            TD_BQC(0xd21375f3),
            TD_BQC(0x208e45cc),
        },
        {
            TD_BQC(0x79a9e644),
            TD_BQC(0x40000000),
            TD_BQC(0xf573bac7),
            TD_BQC(0x00eccf50),
        },
        {
            TD_BQC(0x22bd9020),
            TD_BQC(0x40000000),
            TD_BQC(0xe79ee0ab),
            TD_BQC(0x0c7e1922),
        },
        {
            TD_BQC(0xd55b990a),
            TD_BQC(0x40000000),
            TD_BQC(0xbe9c74e1),
            TD_BQC(0x38c34b31),
        },
        {
            TD_BQC(0xff385c57),
            TD_BQC(0x40000000),
            TD_BQC(0xdcfbd87a),
            TD_BQC(0x15f47d04),
        },
        {
            TD_BQC(0x51b0ee6d),
            TD_BQC(0x40000000),
            TD_BQC(0xf06660a5),
            TD_BQC(0x05140aa2),
        },
        {
            TD_BQC(0xdb4f063f),
            TD_BQC(0x40000000),
            TD_BQC(0xc7d703f5),
            TD_BQC(0x2c029708),
        },
    },

    /* soso_exp is 1 due to division by 2 in the table for upsampling 1.5 or 3 above */
    /* static const int sos_exp = 1; */

    /* gain per biquad-section * scaling */
    {
        TD_BQC(0x3dd029f4),
        TD_BQC(0x1bc7f9d1),
        TD_BQC(0x28dd9d9f),
        TD_BQC(0x52d47055),
        TD_BQC(0x33260c66),
        TD_BQC(0x209865fc),
        TD_BQC(0x489cd7d7),
    },

    /* unscaling mantissa*/
    0x61998d08,

    /* filter delay */
    {
        (char)3,
        (char)7,
    },

    /* unscaling exponent*/
    (char)2,
};

#if defined(__arm__)
#include "arm/TD_upsampler_arm.cpp"
#endif

/**
 * \brief  LP-filter implemented with second order sections
 *
 * \param   sampleIn     i  : next sample of sequence to be filtered
 * \param   sosIdx       i  : select biquad coefficient table
 * \param  *states       i/o: filter states
 * \return  LP-filtered value
 */

#ifndef FUNCTION_TD_applyFilter
static FIXP_DBL TD_applyFilter(FIXP_DBL sampleIn, filter* sosData, FIXP_DBL* states) {
  int s;
  FIXP_DBL input, output, state0, state1, state2;

  input = sampleIn;

  for (s = 0; s < 7; s++) {
    state0 = states[s * 2 + 0];
    state1 = states[s * 2 + 1];
    /* scale input */
    input = fMult(input, sosData->sos_gain[s]);

    /* filter in DF2 transformed */
    output = input + (state0 << 1);
    state1 = state1 + fMult(input, sosData->coeff[s][B1]) - fMult(output, sosData->coeff[s][A1]);
    state2 = fMult(input, sosData->coeff[s][B2]) - fMult(output, sosData->coeff[s][A2]);

    /* save states of this section */
    states[s * 2 + 0] = state1;
    states[s * 2 + 1] = state2;

    input = output;
  }
  /* unscale output */
  return scaleValueSaturate(fMult(output, sosData->scaleFacMantissa), (INT)sosData->scaleFacExp);
}
#endif /* FUNCTION_applyFilter */

short TD_upsampler_init(TD_FAC_UPSAMPLE facUpsample, FIXP_DBL* states) {
  int delay = 0;
  FDKmemclear(states, TD_STATES_MEM_SIZE * sizeof(FIXP_DBL));

  switch (facUpsample) {
    case TD_FAC_UPSAMPLE_1_1:
      delay = 0;
      break;
    case TD_FAC_UPSAMPLE_3_2:
      delay = sos_3.delay[0];
      break;
    case TD_FAC_UPSAMPLE_2_1:
      delay = sos_2.delay[0];
      break;
    case TD_FAC_UPSAMPLE_3_1:
      delay = sos_3.delay[1];
  }
  return delay;
}

short TD_upsampler(TD_FAC_UPSAMPLE facUpsample, const FIXP_DBL* sigIn,
                   short lenIn, /* must be multiple of 2 in case upsampling 3/2 and 3 */
                   FIXP_DBL* sigOut, FIXP_DBL* states) {
  int lenOut = 0;
  const FIXP_DBL* sigIn_ptr;

  if (lenIn <= 0) {
    return 0;
  }

  sigIn_ptr = sigIn;

  switch (facUpsample) {
    case TD_FAC_UPSAMPLE_1_1: /* copy values from input to output as they are */
      if (sigIn_ptr != sigOut) FDKmemmove(sigOut, sigIn, lenIn * sizeof(FIXP_DBL));
      lenOut = lenIn;
      break;

    case TD_FAC_UPSAMPLE_3_2: /* upsampling with 3 whereupon only each second value is taken  */
    {
      FDK_ASSERT(!(lenIn & 1));
#if defined(FUNCTION_TD_upsampler_3_2)
      lenOut = TD_upsampler_3_2(sigIn, sigOut, states, lenIn);
#elif defined(FUNCTION_TD_upsampler_3_1)
      lenOut = TD_upsampler_3_1(sigIn, sigOut, states, lenIn, facUpsample);
#else
      FIXP_DBL temp[6];
      for (int i = 0; i < lenIn / 2; i++) {
        for (int k = 0; k < 2; k++) {
          FIXP_DBL input = *sigIn_ptr++;
          temp[3 * k] = TD_applyFilter(input, &sos_3, states);
          temp[3 * k + 1] = TD_applyFilter(FIXP_DBL(0), &sos_3, states);
          temp[3 * k + 2] = TD_applyFilter(FIXP_DBL(0), &sos_3, states);
        }
        *sigOut++ = temp[0];
        *sigOut++ = temp[2];
        *sigOut++ = temp[4];
      }
      lenOut = (3 * lenIn) / 2;
#endif
      break;
    }

    case TD_FAC_UPSAMPLE_2_1: /* upsampling with 2 */
#ifdef FUNCTION_TD_upsampler_2_1
      lenOut = TD_upsampler_2_1(sigIn, sigOut, states, lenIn);
#else
      for (int i = 0; i < lenIn; i++) {
        FIXP_DBL input = *sigIn_ptr++ >> 1;
        *sigOut++ = TD_applyFilter(input, &sos_2, states) << 1;
        *sigOut++ = TD_applyFilter(FIXP_DBL(0), &sos_2, states) << 1;
      }
#endif
      lenOut = 2 * lenIn;
      break;

    case TD_FAC_UPSAMPLE_3_1: /* upsampling with 3 */
      FDK_ASSERT(!(lenIn & 1));
#ifdef FUNCTION_TD_upsampler_3_1
      lenOut = TD_upsampler_3_1(sigIn, sigOut, states, lenIn, facUpsample);
#else
      for (int i = 0; i < lenIn; i++) {
        FIXP_DBL input = *sigIn_ptr++;
        *sigOut++ = TD_applyFilter(input, &sos_3, states);
        *sigOut++ = TD_applyFilter(FIXP_DBL(0), &sos_3, states);
        *sigOut++ = TD_applyFilter(FIXP_DBL(0), &sos_3, states);
      }
      lenOut = 3 * lenIn;
#endif
      break;
  }
  return lenOut;
}
