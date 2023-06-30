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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Max Staudt

   Description: TCX/AAC Frequency Domain Prediction

*******************************************************************************/

#include "channelinfo.h"

#define HLM 128

static const USHORT fdp_exp[182] =
    {/* 64 * (0:181) .^ (4/3) */
     0,     64,    161,   277,   406,   547,   698,   857,   1024,  1198,  1379,  1566,  1758,
     1956,  2159,  2368,  2580,  2798,  3019,  3245,  3474,  3708,  3945,  4186,  4431,  4678,
     4930,  5184,  5442,  5702,  5966,  6232,  6502,  6774,  7049,  7327,  7608,  7891,  8176,
     8464,  8755,  9048,  9344,  9641,  9941,  10244, 10548, 10855, 11164, 11476, 11789, 12104,
     12422, 12741, 13063, 13386, 13712, 14039, 14369, 14700, 15033, 15368, 15705, 16044, 16384,
     16726, 17070, 17416, 17763, 18113, 18463, 18816, 19170, 19526, 19883, 20242, 20603, 20965,
     21329, 21694, 22061, 22430, 22800, 23171, 23544, 23919, 24295, 24672, 25051, 25431, 25813,
     26196, 26581, 26966, 27354, 27742, 28132, 28524, 28917, 29311, 29706, 30103, 30501, 30900,
     31301, 31703, 32106, 32511, 32916, 33323, 33732, 34141, 34552, 34964, 35377, 35791, 36207,
     36624, 37042, 37461, 37881, 38303, 38725, 39149, 39574, 40000, 40427, 40856, 41285, 41716,
     42147, 42580, 43014, 43449, 43885, 44323, 44761, 45200, 45641, 46082, 46525, 46968, 47413,
     47859, 48306, 48753, 49202, 49652, 50103, 50555, 51008, 51462, 51916, 52372, 52829, 53287,
     53746, 54206, 54667, 55128, 55591, 56055, 56520, 56985, 57452, 57920, 58388, 58858, 59328,
     59799, 60271, 60745, 61219, 61694, 62170, 62647, 63124, 63603, 64083, 64563, 65044, 65527};

static const USHORT fdp_scf[63] = {/* 2 .^ ((0:62 + 1) / 4) */
                                   1,     1,     2,     2,     2,     3,     3,     4,
                                   5,     6,     7,     8,     10,    11,    13,    16,
                                   19,    23,    27,    32,    38,    45,    54,    64,
                                   76,    91,    108,   128,   152,   181,   215,   256,
                                   304,   362,   431,   512,   609,   724,   861,   1024,
                                   1218,  1448,  1722,  2048,  2435,  2896,  3444,  4096,
                                   4871,  5793,  6889,  8192,  9742,  11585, 13777, 16384,
                                   19484, 23170, 27554, 32768, 38968, 46341, 55109}; /*65536*/

static const USHORT fdp_s1[129] =
    {/*  49152 * fdp_scl .* fdp_sin */
     0,     726,   1451,  2176,  2901,  3625,  4348,  5071,  5792,  6512,  7230,  7947,  8662,
     9376,  10087, 10796, 11503, 12207, 12908, 13607, 14303, 14995, 15685, 16371, 17054, 17732,
     18408, 19079, 19746, 20409, 21068, 21722, 22372, 23017, 23658, 24293, 24924, 25549, 26169,
     26784, 27394, 27998, 28596, 29189, 29776, 30357, 30932, 31501, 32064, 32621, 33171, 33715,
     34253, 34784, 35309, 35827, 36339, 36844, 37282, 37698, 38105, 38503, 38892, 39272, 39643,
     40005, 40359, 40703, 41039, 41366, 41684, 41994, 42296, 42589, 42874, 43151, 43419, 43680,
     43933, 44179, 44416, 44646, 44869, 45085, 45293, 45494, 45689, 45876, 46057, 46232, 46400,
     46562, 46717, 46867, 47011, 47149, 47281, 47408, 47529, 47645, 47756, 47862, 47963, 48059,
     48150, 48237, 48319, 48397, 48471, 48540, 48605, 48666, 48724, 48777, 48826, 48872, 48914,
     48953, 48988, 49019, 49047, 49072, 49093, 49111, 49126, 49137, 49146, 49150, 49152};

static const SHORT fdp_s2[129] =
    {/* -18432 * fdp_scl .* fdp_scl */
     -26681, -26680, -26678, -26675, -26670, -26665, -26658, -26650, -26641, -26630, -26618, -26605,
     -26591, -26576, -26559, -26541, -26522, -26502, -26481, -26459, -26436, -26411, -26386, -26359,
     -26331, -26303, -26273, -26242, -26211, -26178, -26145, -26110, -26075, -26039, -26002, -25964,
     -25926, -25887, -25847, -25806, -25765, -25723, -25680, -25637, -25593, -25549, -25504, -25458,
     -25413, -25366, -25320, -25273, -25225, -25178, -25130, -25082, -25033, -24985, -24856, -24710,
     -24564, -24417, -24272, -24126, -23981, -23836, -23691, -23547, -23404, -23262, -23121, -22980,
     -22841, -22702, -22565, -22429, -22295, -22162, -22030, -21900, -21771, -21644, -21519, -21396,
     -21274, -21155, -21037, -20921, -20808, -20697, -20587, -20481, -20376, -20274, -20174, -20077,
     -19982, -19889, -19799, -19712, -19627, -19546, -19466, -19390, -19316, -19245, -19177, -19112,
     -19049, -18990, -18933, -18879, -18829, -18781, -18736, -18695, -18656, -18620, -18588, -18558,
     -18532, -18508, -18488, -18471, -18457, -18446, -18438, -18434, -18432};

#if defined(__arm__)
#include "arm/fdp_arm.cpp"
#endif

void FDP_DecodeBins(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                    CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo, const UINT flags,
                    const FIXP_DBL* quantSpecCurr, const short* sfbOffsets,
                    const int predictionBandwidth, const int isTcx, const FIXP_DBL i_gain_m,
                    const INT i_gain_e, const INT g_div64, const INT lg) {
  short* quantSpecPrev2 = pAacDecoderStaticChannelInfo->quantSpecPrev2;
  short* quantSpecPrev1 = pAacDecoderStaticChannelInfo->quantSpecPrev1;
  const short* quantScfCurr = pAacDecoderChannelInfo->pDynData->aScaleFactor;
  SPECTRAL_PTR outputSpecCurr = pAacDecoderChannelInfo->pSpectralCoefficient;
  const int fdp_spacing_index = pAacDecoderChannelInfo->fdp_spacing_index;
  const int bEightShortSequence = pAacDecoderChannelInfo->icsInfo.WindowSequence == 2;
  const int maxLines = predictionBandwidth;
  int sfbNumActive = pAacDecoderChannelInfo->icsInfo.MaxSfBands;

  const int fdp_spacing_value = 894 / 3 - fdp_spacing_index;
  int i = 0, s1, s2, sfb, harmonicSpacing = 0;
  int harmIndex = -1 * HLM, compIndex = 2 * HLM; /* harmonic/compare index */
  FIXP_DBL fdp_int[160];

  /* if indepFlag==1 -> reset FDP history */
  if (flags & AC_INDEP) {
    FDKmemclear(quantSpecPrev1, 160 * sizeof(quantSpecPrev1[0]));
    FDKmemclear(quantSpecPrev2, 160 * sizeof(quantSpecPrev2[0]));
  }

  if (!isTcx && (bEightShortSequence || (sfbNumActive <= 0))) { /* no FD predictor */
    sfbNumActive = 0;
  } else if (isTcx ||
             (!isTcx && (maxLines < sfbOffsets[sfbNumActive]))) { /* ISO 23003-3, Table 109 */
    if (!isTcx) {
      sfbNumActive = 0;
      while (sfbOffsets[sfbNumActive] < maxLines) sfbNumActive++;
      if (sfbOffsets[sfbNumActive] > maxLines) sfbNumActive--; /* sanity */
    }
    if (pAacDecoderChannelInfo->fdp_data_present && !(flags & AC_INDEP) &&
        (fdp_spacing_index >= 0) && (fdp_spacing_index < 256)) {
      harmonicSpacing = (894 * 512 + fdp_spacing_value) / (2 * fdp_spacing_value);
    }
  }

  s1 = 0;
  s2 = 0; /* reset coefficients, adapt both for each harmonic */

  /* start decoding: for each quantized bin compute predictor estimate */
  {
    if (harmonicSpacing) { /* FDP active and allowed, compute estimate */
      for (i = sfbOffsets[0]; i < sfbOffsets[sfbNumActive]; i++) {
        if (fAbs(i * 2 * HLM - harmIndex) >= 3 * HLM) { /* bin not harmonic */
          fdp_int[i] = (FIXP_DBL)0;
        } else { /* bin is part of the currently active harmonic line */
          const int reg32 = s1 * (int)quantSpecPrev1[i] + s2 * (int)quantSpecPrev2[i];
          fdp_int[i] = (FIXP_DBL)(((unsigned int)fAbs(reg32) + 16384) >> 15);
          if (reg32 < 0) {
            fdp_int[i] = -fdp_int[i];
          }
        }
        if (i * 2 * HLM == compIndex) { /* update indices and LPC coeffs */
          harmIndex += harmonicSpacing;
          if ((compIndex = harmIndex & (2 * HLM - 1)) > HLM) {
            compIndex = 2 * HLM - compIndex; /* exploit trigon. symmetry */
          }
          s1 = fdp_s1[compIndex];
          s2 = fdp_s2[compIndex];
          compIndex = harmIndex >> 8; /* integer unscaled harm. index */
          if ((compIndex & 1) == 0) {
            s1 *= -1; /* negate first LPC coeff for even harm. indices */
          }
          compIndex = 2 * HLM + ((harmIndex + HLM) >> 8) * 2 * HLM; /* index */
        }
      }
    }
    for (sfb = 0; sfb < sfbNumActive; sfb++) {
      int scf16 = quantScfCurr[sfb] - 21; /* shifted sf[sfb] */

      if (scf16 < 0) {
        scf16 = 0;
      } else {
        scf16 = scf16 > 62 ? 65536 : (int)fdp_scf[scf16]; /* scf look-up */
      }

#if defined(FUNCTION_FDP_DecodeBins_func1)
      i = FDP_DecodeBins_func1((const INT*)quantSpecCurr, quantSpecPrev1, quantSpecPrev2,
                               sfbOffsets, sfb, scf16, harmonicSpacing, fdp_int, fdp_exp);
#else
      /* start update: compute integer sum of each line and its estimate */
      for (i = sfbOffsets[sfb]; i < sfbOffsets[sfb + 1]; i++) {
        INT x_int = fdp_exp[fMin(fAbs((INT)quantSpecCurr[i]), 181)]; /* look-up */

        x_int = (int)((512 + (unsigned int)x_int * (unsigned int)scf16) >> 10);
        if ((INT)quantSpecCurr[i] < 0) {
          x_int *= -1;
        }
        if (harmonicSpacing) {
          x_int += (INT)fdp_int[i]; /* add previously computed FDP estimate */
        }
        quantSpecPrev2[i] = quantSpecPrev1[i];
        quantSpecPrev1[i] = (short)fMin(fMax(x_int, -31775), 31775);
      }
#endif
    }
    /* finalize update: reset states of currently uncoded spectral lines */
    for (; i < maxLines; i++) {
      quantSpecPrev2[i] = quantSpecPrev1[i] = 0;
    }

    if (harmonicSpacing) { /* FDP active and allowed, compute estimate */

      SPECTRAL_PTR p2output = outputSpecCurr;
      FIXP_DBL* p2fdp = fdp_int;

      for (sfb = 0; sfb < sfbNumActive; sfb++) {
        int width = sfbOffsets[sfb + 1] - sfbOffsets[sfb];

        /* We calculate the number of missing (non-available) headroom which would happen in fdp_int
         * after being scaled */
        int fdp_scale = getScalefactor(p2fdp, width);
        int fdp_shift = 40 - pAacDecoderChannelInfo->pDynData->aSfbScale[sfb];
        fdp_scale = fdp_scale - fdp_shift;

        /* Find the headroom of the signal */
        int output_scale = getScalefactor(p2output, width);

        /* Find the minimum of the headroom of the signal and the missing fdp headroom */
        int diffMin = fMin(fdp_scale, output_scale);

        /* When binary point adjustment is necessary. We make sure that fdp data does not overflow
        after shifting and that there is at least one bit headroom to accomodate the increase due to
        addition */
        if (diffMin < 1) {
          int shift = diffMin - 1;

          /* Correct scale factor */
          pAacDecoderChannelInfo->pDynData->aSfbScale[sfb] -= shift;

          fdp_shift = 40 - pAacDecoderChannelInfo->pDynData->aSfbScale[sfb];

          shift = fMax(-31, shift);

          for (i = sfbOffsets[sfb]; i < sfbOffsets[sfb + 1]; i++) {
            FIXP_DBL val1, val2;
            val1 = scaleValue(*p2output, shift);
            val2 = scaleValue(*p2fdp++, fdp_shift);
            *p2output++ = val1 + val2;
          }
        }
        /* When binary point adjustment is not necessary */
        else {
          for (i = sfbOffsets[sfb]; i < sfbOffsets[sfb + 1]; i++) {
            int val = scaleValue(*p2fdp++, fdp_shift);
            (*p2output++) += val;
          }
        }

      } /*  for (sfb = 0; sfb < sfbNumActive; sfb++) */

    } /* if (harmonicSpacing) */
  }   /* if (isTcx) */
}
