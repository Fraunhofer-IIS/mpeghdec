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

/******************** MPEG-H 3DA object rendering library **********************

   Author(s):   Thomas Blender, Arthur Tritthart

   Description: Vector Base Amplitude Panning (VBAP)

*******************************************************************************/

#include "vbap_core.h"
#include "cartesianMath.h"

static void normalizePower(HANDLE_GVBAPRENDERER hgVBAPRenderer);

/*
 * @brief use Cramer rule to calculate the inverse of the Matrix M
 *        Store the results in transposed order in the gVBAPRenderer Handle.
 * @param hgVBAPRenderer     gVBAPRenderer Handle
 */
int generateInverseMatrices(HANDLE_GVBAPRENDERER hgVBAPRenderer) {
  int i, j;
  int greatestExponent;
  int detM_inv_exponent;

  for (i = 0; i < hgVBAPRenderer->speakerSetup.speakerTripletSize; i++) {
    SPEAKERTRIPLET* speakerTriplet;
    speakerTriplet = &hgVBAPRenderer->speakerSetup.speakerTriplet[i];

    PointCartesian* matrix = speakerTriplet->matrix;
    FIXP_DBL detM, detS, detM_inv, tmp;

    /* First calculate det(M) */
    /* additive part */
    tmp = fMultDiv2(matrix[0].x, matrix[1].y); /* tmp  has an exponent of 1 */
    detM = fMultDiv2(tmp, matrix[2].z);        /* detM has an exponent of 2 */
    tmp = fMultDiv2(matrix[0].y, matrix[1].z);
    detM = fMultAddDiv2(
        detM, tmp, matrix[2].x); /* detM could be 2 at most -> exponent of 2 prevents overflow */
    tmp = fMultDiv2(matrix[0].z, matrix[1].x);
    detM = fMultAddDiv2(
        detM, tmp, matrix[2].y); /* detM could be 3 at most -> exponent of 2 prevents overflow */

    /* subtractive part, add first, subtract later from detM */
    tmp = fMultDiv2(matrix[0].z, matrix[1].y);
    detS = fMultDiv2(tmp, matrix[2].x);
    tmp = fMultDiv2(matrix[0].y, matrix[1].x);
    detS = fMultAddDiv2(detS, tmp, matrix[2].z);
    tmp = fMultDiv2(matrix[0].x, matrix[1].z);
    detS = fMultAddDiv2(detS, tmp, matrix[2].y);
    detM =
        (detM >> 1) - (detS >> 1); /* detM could be 6 at most -> exponent of 3 prevents overflow */

    /* If loudspeaker distance is at least 3 degrees elevation and 1.5 degrees azimuth then det(M)
     * is always greater than 0.001369995621215 */
    /* Use this limitation to prevent to great numbers in inverted matrix */
    FDK_ASSERT(fAbs(detM) > ((FIXP_DBL)FL2FXCONST_DBL(0.001369995621215)) >> 3);

    /* Calculate inverse of detM */
    detM_inv_exponent = 3; /* 3 because detM has an exponent of 3 */
    detM_inv = invFixp(detM, &detM_inv_exponent);

    /* Calculate Det(A_ij) */
    speakerTriplet->inverseMatrix[0].x = fMult(
        fMultSubDiv2(fMultDiv2(matrix[1].y, matrix[2].z), matrix[1].z, matrix[2].y), detM_inv);
    speakerTriplet->inverseMatrix[1].x = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].z, matrix[2].y), matrix[0].y, matrix[2].z), detM_inv);
    speakerTriplet->inverseMatrix[2].x = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].y, matrix[1].z), matrix[0].z, matrix[1].y), detM_inv);
    speakerTriplet->inverseMatrix[0].y = fMult(
        fMultSubDiv2(fMultDiv2(matrix[1].z, matrix[2].x), matrix[1].x, matrix[2].z), detM_inv);
    speakerTriplet->inverseMatrix[1].y = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].x, matrix[2].z), matrix[0].z, matrix[2].x), detM_inv);
    speakerTriplet->inverseMatrix[2].y = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].z, matrix[1].x), matrix[0].x, matrix[1].z), detM_inv);
    speakerTriplet->inverseMatrix[0].z = fMult(
        fMultSubDiv2(fMultDiv2(matrix[1].x, matrix[2].y), matrix[1].y, matrix[2].x), detM_inv);
    speakerTriplet->inverseMatrix[1].z = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].y, matrix[2].x), matrix[0].x, matrix[2].y), detM_inv);
    speakerTriplet->inverseMatrix[2].z = fMult(
        fMultSubDiv2(fMultDiv2(matrix[0].x, matrix[1].y), matrix[0].y, matrix[1].x), detM_inv);

    detM_inv_exponent += 1; /* Add 1 because all sub determinants has exponents of 1 */
    /* Matrix exponent mustn't be negative */
    if (detM_inv_exponent < 0) {
      /* normalize exponent to 0 */
      for (j = 0; j < 3; j++) {
        speakerTriplet->inverseMatrix[j].x =
            (speakerTriplet->inverseMatrix[j].x) >> (-detM_inv_exponent);
        speakerTriplet->inverseMatrix[j].y =
            (speakerTriplet->inverseMatrix[j].y) >> (-detM_inv_exponent);
        speakerTriplet->inverseMatrix[j].z =
            (speakerTriplet->inverseMatrix[j].z) >> (-detM_inv_exponent);
      }
    }
    speakerTriplet->exponentInverseMatrix =
        detM_inv_exponent; /* Subtract 2 because detM has an exponent of 3 and all sub determinants
                              has exponents of 1*/
  }

  /* search for the biggest exponent over all matrices */
  greatestExponent = hgVBAPRenderer->speakerSetup.speakerTriplet[0].exponentInverseMatrix;
  for (i = 1; i < hgVBAPRenderer->speakerSetup.speakerTripletSize; i++) {
    if (hgVBAPRenderer->speakerSetup.speakerTriplet[i].exponentInverseMatrix > greatestExponent)
      greatestExponent = hgVBAPRenderer->speakerSetup.speakerTriplet[i].exponentInverseMatrix;
  }
  hgVBAPRenderer->speakerSetup.greatestInverseMatrixExponent = greatestExponent;

  return 0;
}

void calculateVbap(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source, FIXP_DBL* final_gains,
                   int noDownmix) {
  int row, col;
  int i;
  FIXP_DBL gain_norm = 0, gain_norm_inv = 0;
  INT gain_norm_inv_exp = 0;
  int numSpeaker;

  numSpeaker = hgVBAPRenderer->numChannels - hgVBAPRenderer->numLFE;

  /* first calculate gains for all speakers including ghosts. Store the result to gain cache */
  calculateVbapGain(hgVBAPRenderer, source);

  /* Check whether a downmix matrix for ghost speaker exists or not */
  if ((hgVBAPRenderer->downmixMatrix != NULL) && !noDownmix) {
    FDKmemclear(final_gains, numSpeaker * sizeof(FIXP_DBL));

    /* Downmix ghost speakers */
    for (row = 0; row < numSpeaker; row++) {
      FIXP_DBL tmp_gains = (FIXP_DBL)0;
      int col_runs =
          hgVBAPRenderer->numChannels - hgVBAPRenderer->numLFE + hgVBAPRenderer->numGhosts;
      for (col = 0; col < col_runs; col++) {
        tmp_gains = fMultAddDiv2(tmp_gains, hgVBAPRenderer->downmixMatrix[row][col],
                                 hgVBAPRenderer->gainCache[col]);
      }
      final_gains[row] = tmp_gains;
    }
  } else {
    numSpeaker = hgVBAPRenderer->numChannels - hgVBAPRenderer->numLFE + hgVBAPRenderer->numGhosts;
    FDKmemclear(final_gains, numSpeaker * sizeof(FIXP_DBL));

    for (row = 0; row < numSpeaker; row++) {
      final_gains[row] = hgVBAPRenderer->gainCache[row];
    }
  }

  /* gain correction is necessary because adding power normalized gains through downmix */
  for (i = 0; i < numSpeaker; i++) {
    gain_norm =
        fPow2Add(gain_norm, final_gains[i] >> 2); /* this will limit the number of speakers to 64 */
  }

  gain_norm_inv = invSqrtNorm2(gain_norm, &gain_norm_inv_exp);
  gain_norm_inv = fMult(gain_norm_inv, source.gain); /* Incorporate gainFactor */

  /* gain_norm should always be greater than each final_gains[i]
   * but if there is only one gain value in final_gains vector different from 0
   * it could be happen that gain_norm is a little smaller than final_gain[i] because of arithmetic
   * inaccuracy
   */
  for (i = 0; i < numSpeaker; i++) {
    FIXP_DBL tmp = fMult(final_gains[i] >> 2, gain_norm_inv);
    final_gains[i] = scaleValueSaturate(tmp, gain_norm_inv_exp);
  }
}

void calculateVbapGain(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source) {
  /* clear gain cache in handler */
  FDKmemclear(hgVBAPRenderer->gainCache,
              (hgVBAPRenderer->numChannels + hgVBAPRenderer->numGhosts - hgVBAPRenderer->numLFE) *
                  sizeof(FIXP_DBL));

  if (hgVBAPRenderer->renderMode == GVBAP_LEGACY) {
    /* calculate spread vectors (0 - 90 degrees) */
    calcSpreadVectors(hgVBAPRenderer, source);

    /* calculate gains for all spread vectors */
    calculateOneSourcePosition(hgVBAPRenderer, NULL);

    /* calculate second part spreading (90 - 180 degrees), include gain normalization */
    /* resulting gains has an exponent of 2 */
    calcSpreadGains(hgVBAPRenderer, source.spreadAngle);
  } else {
    calculateOneSourcePosition(hgVBAPRenderer, &source.cart);

    normalizePower(hgVBAPRenderer);
  }
}

void calcSpreadVectors(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE source) {
  PointSpherical direction;
  PointCartesian u, v, p0;
  PointCartesian zero;

  FIXP_DBL sin_spread;
  FIXP_DBL cos_spread;

  int exp = 0;
  FIXP_DBL inv_sin_spread;
  INT result_exponent_max;
  PointCartesian summand;

  zero.x = (FIXP_DBL)0;
  zero.y = (FIXP_DBL)0;
  zero.z = (FIXP_DBL)0;

  /* Limit spread Angle to 0.001 <= alpha <= 90 degrees */
  /* "source.spreadAngle" is in percent. We calculate: round((0.001 deg/180 deg)*2^31) = 11931*/
  FIXP_DBL spreadAngle = fMax(source.spreadAngle, (FIXP_DBL)11931);
  /* "spreadAngle" is in percent. We calculate: round((90deg/180deg)*2^31) = 1073741824*/
  spreadAngle = fMin(spreadAngle, (FIXP_DBL)1073741824);

  p0 = source.cart;
  /* Rotate p0 vector by 90 degrees in elevation direction */
  if (source.sph.ele < (FIXP_DBL)0)
    direction.ele = source.sph.ele + (FIXP_DBL)1073741824; /* Add 90 degrees */
  else
    direction.ele = source.sph.ele - (FIXP_DBL)1073741824; /* Subtract 90 degrees */
  direction.azi = source.sph.azi;
  direction.rad = FL2FXCONST_DBL(1.0);

  v = sphericalToCartesian(direction);
  u = crossProduct(&v, &source.cart);

  /* u is already scaled with an exponent of 1. This scale should not be undone
   * because baseVectors 7, 10, 13, 16 could overflow */

  /*Store p0 in full precision with a relative exponent (to p0) for subsequent calculations */
  PointCartesian p1 = p0;
  INT p1_relative_exp = 1;
  if (hgVBAPRenderer->hasUniformSpread) {
    /* also scale v and p0 to the half */
    v >>= 1;
    p0 >>= 1;
  } else {
    /* With spreadAngle = spreadWidth determine the ratio
       alpha_r = az_el_spread_ratio * 2 ^ az_el_spread_ratio_e = spreadHeight / spreadAngle. The
       base vector v is multiplied with alpha_r. Furthermore, scale u and p0 as needed: The exponent
       of u, v and p0 can be set to any equal value so that it cancels out in the division later.*/

    /* Limit spreadHeight to 0.001 <= alpha <= 90 degrees */
    /* "spreadHeight" is in percent. We calculate: round((0.001 deg/180 deg)*2^31) = 11931*/
    FIXP_DBL spreadHeight = fMax(source.spreadHeight, (FIXP_DBL)11931);
    /* "spreadHeight" is in percent. We calculate: round((90deg/180deg)*2^31) = 1073741824*/
    spreadHeight = fMin(spreadHeight, (FIXP_DBL)1073741824);

    if ((spreadHeight != spreadAngle) && (hgVBAPRenderer->speakerSetup.hasHeightSpeakers == true)) {
      INT az_el_spread_ratio_e;
      FIXP_DBL az_el_spread_ratio = fDivNorm(spreadHeight, spreadAngle, &az_el_spread_ratio_e);
      // we assert the divison returns an exponent in the range -(DFRACT_BITS - 1) <=
      // az_el_spread_ratio_e <= (DFRACT_BITS - 1)
      v = az_el_spread_ratio * v;
      if (az_el_spread_ratio_e > 0) {
        u >>= (az_el_spread_ratio_e - 1);
        p0 >>= az_el_spread_ratio_e;
        p1_relative_exp = az_el_spread_ratio_e;
      } else {
        v >>= fMin(DFRACT_BITS - 1, 1 - az_el_spread_ratio_e);
        p0 >>= 1;
      }
    } else {
      v >>= 1;
      p0 >>= 1;
    }
  }

  if (hgVBAPRenderer->speakerSetup.hasHeightSpeakers == false) {
    v.x = (FIXP_DBL)0;
    v.y = (FIXP_DBL)0;
    v.z = (FIXP_DBL)0;
  }

  hgVBAPRenderer->baseVectors[0] = p0;
  hgVBAPRenderer->baseVectors[1] = u;
  hgVBAPRenderer->baseVectors[4] = zero - u;

  hgVBAPRenderer->baseVectors[2] =
      (FIXP_DBL)1610612736 * u + (FIXP_DBL)536870912 * p0; /* 0.75f * u + 0.25f * p0 */
  hgVBAPRenderer->baseVectors[5] =
      FIXP_DBL(-1610612736) * u + (FIXP_DBL)536870912 * p0; /* -0.75f * u + 0.25f * p0 */

  hgVBAPRenderer->baseVectors[3] =
      (FIXP_DBL)805306368 * u + (FIXP_DBL)1342177280 * p0; /* 0.375f * u + 0.625f * p0 */
  hgVBAPRenderer->baseVectors[6] =
      (FIXP_DBL)-805306368 * u + (FIXP_DBL)1342177280 * p0; /* -0.375f * u + 0.625f * p0 */

  hgVBAPRenderer->baseVectors[7] =
      (FIXP_DBL)1073741824 * u + (FIXP_DBL)1859720839 * v +
      (FIXP_DBL)715112054 * p0; /*  0.5f * u + 0.866f * v + 0.333f * p0 */
  hgVBAPRenderer->baseVectors[10] =
      (FIXP_DBL)-1073741824 * u + (FIXP_DBL)1859720839 * v +
      (FIXP_DBL)715112054 * p0; /* -0.5f * u + 0.866f * v + 0.333f * p0 */
  hgVBAPRenderer->baseVectors[13] =
      (FIXP_DBL)-1073741824 * u - (FIXP_DBL)1859720839 * v +
      (FIXP_DBL)715112054 * p0; /* -0.5f * u - 0.866f * v + 0.333f * p0 */
  hgVBAPRenderer->baseVectors[16] =
      (FIXP_DBL)1073741824 * u - (FIXP_DBL)1859720839 * v +
      (FIXP_DBL)715112054 * p0; /*  0.5f * u - 0.866f * v + 0.333f * p0 */

  hgVBAPRenderer->baseVectors[8] = (FIXP_DBL)1073741824 * hgVBAPRenderer->baseVectors[7] +
                                   (FIXP_DBL)1073741824 * p0; /*  0.5f * p7  + 0.5f * p0 */
  hgVBAPRenderer->baseVectors[11] = (FIXP_DBL)1073741824 * hgVBAPRenderer->baseVectors[10] +
                                    (FIXP_DBL)1073741824 * p0; /*  0.5f * p10 + 0.5f * p0 */
  hgVBAPRenderer->baseVectors[14] = (FIXP_DBL)1073741824 * hgVBAPRenderer->baseVectors[13] +
                                    (FIXP_DBL)1073741824 * p0; /*  0.5f * p13 + 0.5f * p0 */
  hgVBAPRenderer->baseVectors[17] = (FIXP_DBL)1073741824 * hgVBAPRenderer->baseVectors[16] +
                                    (FIXP_DBL)1073741824 * p0; /*  0.5f * p16 + 0.5f * p0 */

  hgVBAPRenderer->baseVectors[9] = (FIXP_DBL)536870912 * hgVBAPRenderer->baseVectors[7] +
                                   (FIXP_DBL)1610612736 * p0; /*  0.25f * p7  + 0.75f * p0 */
  hgVBAPRenderer->baseVectors[12] = (FIXP_DBL)536870912 * hgVBAPRenderer->baseVectors[10] +
                                    (FIXP_DBL)1610612736 * p0; /*  0.25f * p10 + 0.75f * p0 */
  hgVBAPRenderer->baseVectors[15] = (FIXP_DBL)536870912 * hgVBAPRenderer->baseVectors[13] +
                                    (FIXP_DBL)1610612736 * p0; /*  0.25f * p13 + 0.75f * p0 */
  hgVBAPRenderer->baseVectors[18] = (FIXP_DBL)536870912 * hgVBAPRenderer->baseVectors[16] +
                                    (FIXP_DBL)1610612736 * p0; /*  0.25f * p16 + 0.75f * p0 */

  /* Calculate tan(spreadangle) use cos sin for calculation */
  /* Scale spreadAngle from -pi ... pi to a power of 2 */
  /* New exponent will be 2 */
  spreadAngle = fMult(spreadAngle, (FIXP_DBL)1686629713); /* Multiply by pi/4 */

  fixp_cos_sin(spreadAngle, 2, &cos_spread, &sin_spread);
  inv_sin_spread = invFixp(sin_spread, &exp);

  summand = fMultDiv2(cos_spread, inv_sin_spread) * p1;
  result_exponent_max = exp + 1 - p1_relative_exp;

  if (result_exponent_max >= 0) {
    result_exponent_max = fMin(result_exponent_max, 31);
  } else {
    result_exponent_max = fMin(-result_exponent_max, 31);

    summand.x >>= result_exponent_max;
    summand.y >>= result_exponent_max;
    summand.z >>= result_exponent_max;

    result_exponent_max = 0;
  }

  for (int i = 0; i < 19; i++) {
    hgVBAPRenderer->baseVectors[i].x =
        ((hgVBAPRenderer->baseVectors[i].x) >> (result_exponent_max)) + summand.x;
    hgVBAPRenderer->baseVectors[i].y =
        ((hgVBAPRenderer->baseVectors[i].y) >> (result_exponent_max)) + summand.y;
    hgVBAPRenderer->baseVectors[i].z =
        ((hgVBAPRenderer->baseVectors[i].z) >> (result_exponent_max)) + summand.z;
  }
}

void calculateOneSourcePosition(HANDLE_GVBAPRENDERER hgVBAPRenderer, PointCartesian* baseVector) {
  INT inv_mat_max_exp = hgVBAPRenderer->speakerSetup.greatestInverseMatrixExponent;
#if defined(FUNCTION_calculateOneSourcePosition_func1)
  SPEAKERTRIPLET* speakerTriplet = hgVBAPRenderer->speakerSetup.speakerTriplet;
#endif

  /* iterate over all spread (base) vectors */
  for (int i = 0; i < 19; i++) {
    int winner_set = 0;
    if (hgVBAPRenderer->renderMode == GVBAP_LEGACY) baseVector = &hgVBAPRenderer->baseVectors[i];

#if defined(FUNCTION_calculateOneSourcePosition_func1)
    PointCartesian gain;
    winner_set =
        calculateOneSourcePosition_func1(hgVBAPRenderer->speakerSetup.speakerTripletSize, &gain,
                                         baseVector, &speakerTriplet[0].inverseMatrix[0]);

#else
    PointCartesian gain = {};
    FIXP_DBL greatest_smallest_gain = (FIXP_DBL)MINVAL_DBL; /* set to lowest possible value */

    /* iterate over all triplets */
    for (int j = 0; j < hgVBAPRenderer->speakerSetup.speakerTripletSize; j++) {
      PointCartesian gain_tmp;
      FIXP_DBL smallest_gain_of_set;

      PointCartesian* inverseMatrix = hgVBAPRenderer->speakerSetup.speakerTriplet[j].inverseMatrix;

#ifdef FUNCTION_dotProductXYZ
      smallest_gain_of_set = dotProductXYZ(baseVector, &inverseMatrix[0], &inverseMatrix[1],
                                           &inverseMatrix[2], &gain_tmp);
#else
      PointCartesian* bx = &inverseMatrix[0];
      PointCartesian* by = &inverseMatrix[1];
      PointCartesian* bz = &inverseMatrix[2];
      PointCartesian* a = baseVector;
      gain_tmp.x =
          (fMult(a->x, bx->x) >> 2) + (fMult(a->y, bx->y) >> 2) + (fMult(a->z, bx->z) >> 2);
      gain_tmp.y =
          (fMult(a->x, by->x) >> 2) + (fMult(a->y, by->y) >> 2) + (fMult(a->z, by->z) >> 2);
      gain_tmp.z =
          (fMult(a->x, bz->x) >> 2) + (fMult(a->y, bz->y) >> 2) + (fMult(a->z, bz->z) >> 2);
      /* search the smallest gain value of the three gains */
      smallest_gain_of_set = fMin(fMin(gain_tmp.x, gain_tmp.y), gain_tmp.z);
#endif /* FUNCTION_dotProductXYZ */

      smallest_gain_of_set =
          smallest_gain_of_set >>
          (inv_mat_max_exp - hgVBAPRenderer->speakerSetup.speakerTriplet[j].exponentInverseMatrix);

      /* find the best possible set */
      if (smallest_gain_of_set > greatest_smallest_gain) {
        winner_set = j;
        gain.x = gain_tmp.x;
        gain.y = gain_tmp.y;
        gain.z = gain_tmp.z;
        greatest_smallest_gain = smallest_gain_of_set;
      }
    } /* j */

#endif
    /* If there are negative gains then set them to 0 */
    gain.x = fMax(gain.x, (FIXP_DBL)0);
    gain.y = fMax(gain.y, (FIXP_DBL)0);
    gain.z = fMax(gain.z, (FIXP_DBL)0);
    normalize(&gain);

    /* Add gains to cache */
    /* In worst case one speaker gets the whole energy. That means the value could be 19 at most,
     * because of the 19 base vectors */
    /* To prevent overflow scale wit an exponent of 5 (divide by 32) */
    FDK_ASSERT(hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[0] <
               hgVBAPRenderer->gainCacheLength); /* Don't use more memory than allocated */
    FDK_ASSERT(hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[1] <
               hgVBAPRenderer->gainCacheLength);
    FDK_ASSERT(hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[2] <
               hgVBAPRenderer->gainCacheLength);
    hgVBAPRenderer
        ->gainCache[hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[0]] +=
        (gain.x >> 5);
    hgVBAPRenderer
        ->gainCache[hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[1]] +=
        (gain.y >> 5);
    hgVBAPRenderer
        ->gainCache[hgVBAPRenderer->speakerSetup.speakerTriplet[winner_set].triangle[2]] +=
        (gain.z >> 5);

    if (hgVBAPRenderer->renderMode == GVBAP_ENHANCED) break;
  } /* i */
}

void calcSpreadGains(HANDLE_GVBAPRENDERER hgVBAPRenderer, FIXP_DBL spreadAngle) {
  int i;
  FIXP_DBL w, w_1;
  FIXP_DBL gain_norm = 0, gain_norm_inv = 0;
  INT gain_norm_inv_exp = 0;
  FIXP_DBL unitGain;
  FIXP_DBL tmp;

  if (spreadAngle > (FIXP_DBL)0x40000000) /* if greater than 90 degrees */
  {
    w = spreadAngle - (FIXP_DBL)0x40000000; /* w = spreadAngle - 90 degrees */
    w = w << 1;
    w_1 = (FIXP_DBL)MAXVAL_DBL - w;

    /* Calculate one element of a virtual unit gain vector (all elements are equal) */
    unitGain = (FIXP_DBL)(MAXVAL_DBL) >> 8; /* The value of 8 means, that are not more than 256
                                               speakers including ghosts are allowed */
    unitGain = FIXP_DBL((LONG)unitGain * (LONG)(hgVBAPRenderer->gainCacheLength));
    gain_norm_inv = invSqrtNorm2(unitGain, &gain_norm_inv_exp);
    unitGain = fMult(w >> 4, gain_norm_inv) << gain_norm_inv_exp; /* results exponent is 4-4 = 0 */
  } else {
    w = (FIXP_DBL)0;
    w_1 = (FIXP_DBL)MAXVAL_DBL;
    unitGain = (FIXP_DBL)0;
  }

  /* Adding all gains in gaincache array never results in a higher value than 19/32 see code line
   * 418 */
  for (i = 0; i < hgVBAPRenderer->gainCacheLength; i++) {
    gain_norm = fPow2Add(gain_norm, hgVBAPRenderer->gainCache[i]);
  }

  /* Normalize gains */
  gain_norm_inv = invSqrtNorm2(gain_norm, &gain_norm_inv_exp);

  /* gain_norm should always be greater than each final_gains[i]
   * but if there is only one gain value in final_gains vector different from 0
   * it could be happen that gain_norm is a little smaller than final_gain[i] because of arithmetic
   * inaccuracy
   */
  FIXP_DBL gain_norm_inv_w_1 = gain_norm_inv;
  if (w != (FIXP_DBL)0) {
    gain_norm_inv_w_1 = fMult(gain_norm_inv, w_1); /* include factor for g_MDAP' */
  }
  for (i = 0; i < hgVBAPRenderer->gainCacheLength; i++) {
    tmp = fMult(hgVBAPRenderer->gainCache[i], gain_norm_inv_w_1);
    tmp = scaleValueSaturate(tmp, gain_norm_inv_exp - 1);
    hgVBAPRenderer->gainCache[i] = tmp + (unitGain >> 1);
  }

  /* If spread angle was over 90 degrees then power normalize again */
  if (w != (FIXP_DBL)0) {
    gain_norm = fPow2(unitGain) >> 8;
    INT width_shift = (INT)(fNormz((FIXP_SGL)hgVBAPRenderer->gainCacheLength)) - 1;
    FIXP_SGL temp = (SHORT)(hgVBAPRenderer->gainCacheLength) << width_shift;
    gain_norm <<= (15 - width_shift);
    gain_norm = fMult(gain_norm, temp);
    gain_norm = gain_norm + (w_1 >> 8);
    gain_norm_inv = invSqrtNorm2(gain_norm, &gain_norm_inv_exp);
    for (i = 0; i < hgVBAPRenderer->gainCacheLength; i++) {
      tmp = (hgVBAPRenderer->gainCache[i]) >> 4;
      hgVBAPRenderer->gainCache[i] = fMult(tmp, gain_norm_inv) << gain_norm_inv_exp;
    }
  }
}

static void normalizePower(HANDLE_GVBAPRENDERER hgVBAPRenderer) {
  int i;
  FIXP_DBL gain_norm = 0, gain_norm_inv = 0;
  INT gain_norm_inv_exp = 0;
  FIXP_DBL tmp;

  /* Adding all gains in gaincache array never results in a higher value than 19/32 see code line
   * 418 */
  for (i = 0; i < hgVBAPRenderer->gainCacheLength; i++) {
    gain_norm = fPow2Add(gain_norm, hgVBAPRenderer->gainCache[i]);
  }

  /* Normalize gains */
  gain_norm_inv = invSqrtNorm2(gain_norm, &gain_norm_inv_exp);

  /* gain_norm should always be greater than each final_gains[i]
   * but if there is only one gain value in final_gains vector different from 0
   * it could be happen that gain_norm is a little smaller than final_gain[i] because of arithmetic
   * inaccuracy
   */
  for (i = 0; i < hgVBAPRenderer->gainCacheLength; i++) {
    tmp = fMult(hgVBAPRenderer->gainCache[i], gain_norm_inv);
    hgVBAPRenderer->gainCache[i] = scaleValueSaturate(tmp, gain_norm_inv_exp - 1);
  }
}
