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

   Description: Math functions for Vector Base Amplitude Panning (VBAP)

*******************************************************************************/

#include "cartesianMath.h"

PointCartesian sphericalToCartesian(PointSpherical sph) {
  PointCartesian cart;

  FIXP_DBL azi;
  FIXP_DBL ele;

  FIXP_DBL out[4];

  /* Scale sph from -pi ... pi to a power of 2 */
  /* New exponent will be 2 (-4 ... 4) */
  azi = fMult(sph.azi, (FIXP_DBL)1686629713); /* FL2FXCONST_DBL(M_PI/4.0) */
  ele = fMult(sph.ele, (FIXP_DBL)1686629713); /* FL2FXCONST_DBL(M_PI/4.0) */

  inline_fixp_cos_sin(azi, ele, 2, &out[0]); /* out = cos(azi), sin(azi), cos(ele), sin(ele) */

  cart.x = fMult(out[0], out[2]);
  cart.y = fMult(out[1], out[2]);
  cart.z = out[3];

  if (sph.rad < ((FIXP_DBL)1 << (DFRACT_BITS - 1 - 4))) {
    FIXP_DBL rad = sph.rad << 4; /* apply radius exponent 4 */

    cart.x = fMult(cart.x, rad);
    cart.y = fMult(cart.y, rad);
    cart.z = fMult(cart.z, rad);
  }

  return cart;
}

PointCartesian crossProduct(PointCartesian* a, PointCartesian* b) {
  PointCartesian result;

  /* a and b don't need to be scaled if length of input vectors is < 1, because the length
   * of result vector is equal to the area of the parallelogram spanned by the vector a and b.
   * It must be less or equal to 1 => every entry of the result vector must be less or equal to +-1.
   * To keep this function general we scaled a and b with an exponent of 1 (divide by 2).
   * Because in worst case we have to handle a result vector of length 3,
   * where every entry will be less than +-2 */

  /* Multiplicate and prescale for addition */
  result.x = fMultDiv2(a->y, b->z) - fMultDiv2(a->z, b->y);
  result.y = fMultDiv2(a->z, b->x) - fMultDiv2(a->x, b->z);
  result.z = fMultDiv2(a->x, b->y) - fMultDiv2(a->y, b->x);

  return result;
}

FIXP_DBL dotProduct(PointCartesian* a, PointCartesian* b) {
  FIXP_DBL result;

  /* Prescale with exponent of 2 (divide by 4), because in worst case the value of result could be 3
   */
  FIXP_DBL px, py, pz;
  px = fMultDiv2(a->x, b->x);
  py = fMultDiv2(a->y, b->y);
  pz = fMultDiv2(a->z, b->z);
  result = (px >> 1) + (py >> 1) + (pz >> 1);

  return result;
}

/************************************************
 *     Operator overloading                     *
 ************************************************/
PointCartesian operator-(const PointCartesian& lhs, const PointCartesian& rhs) {
  PointCartesian result;
  result.x = lhs.x - rhs.x;
  result.y = lhs.y - rhs.y;
  result.z = lhs.z - rhs.z;

  return result;
}

PointCartesian operator+(const PointCartesian& lhs, const PointCartesian& rhs) {
  PointCartesian result;
  result.x = lhs.x + rhs.x;
  result.y = lhs.y + rhs.y;
  result.z = lhs.z + rhs.z;

  return result;
}

PointCartesian operator*(const FIXP_DBL& lhs, const PointCartesian& rhs) {
  PointCartesian result;
  result.x = fMult(lhs, rhs.x);
  result.y = fMult(lhs, rhs.y);
  result.z = fMult(lhs, rhs.z);

  return result;
}

PointCartesian operator>>=(PointCartesian& lhs, const FIXP_DBL& rhs) {
  lhs.x >>= rhs;
  lhs.y >>= rhs;
  lhs.z >>= rhs;

  return lhs;
}

PointCartesian operator<<=(PointCartesian& lhs, const FIXP_DBL& rhs) {
  lhs.x <<= rhs;
  lhs.y <<= rhs;
  lhs.z <<= rhs;

  return lhs;
}

PointCartesian operator-(PointCartesian& lhs) {
  PointCartesian result;
  result.x = -lhs.x;
  result.y = -lhs.y;
  result.z = -lhs.z;

  return result;
}
