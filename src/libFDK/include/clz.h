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

   Author(s):   Marc Gayer

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(CLZ_H)
#define CLZ_H

#include "FDK_archdef.h"
#include "machine_type.h"

#if defined(__arm__)
#include "arm/clz_arm.h"

#elif defined(__x86__)
#include "x86/clz_x86.h"

#endif /* all cores */

/*************************************************************************
 *************************************************************************
    Software fallbacks for missing functions.
**************************************************************************
**************************************************************************/

#if !defined(FUNCTION_fixnormz_S)
#ifdef FUNCTION_fixnormz_D
inline INT fixnormz_S(SHORT a) {
  if (a < 0) {
    return 0;
  }
  return fixnormz_D((INT)(a)) - 16;
}
#else
inline INT fixnormz_S(SHORT a) {
  int leadingBits = 0;
  a = ~a;
  while (a & 0x8000) {
    leadingBits++;
    a <<= 1;
  }

  return (leadingBits);
}
#endif
#endif

#if !defined(FUNCTION_fixnormz_D)
inline INT fixnormz_D(LONG a) {
  INT leadingBits = 0;
  a = ~a;
  while (a & 0x80000000) {
    leadingBits++;
    a <<= 1;
  }

  return (leadingBits);
}
#endif

/*****************************************************************************

    functionname: fixnorm_D
    description:  Count leading ones or zeros of operand val for dfract/LONG INT values.
                  Return this value minus 1. Return 0 if operand==0.
*****************************************************************************/
#if !defined(FUNCTION_fixnorm_S)
#ifdef FUNCTION_fixnorm_D
inline INT fixnorm_S(FIXP_SGL val) {
  if (val == (FIXP_SGL)0) {
    return 0;
  }
  return fixnorm_D((INT)(val)) - 16;
}
#else
inline INT fixnorm_S(FIXP_SGL val) {
  INT leadingBits = 0;
  if (val != (FIXP_SGL)0) {
    if (val < (FIXP_SGL)0) {
      val = ~val;
    }
    leadingBits = fixnormz_S(val) - 1;
  }
  return (leadingBits);
}
#endif
#endif

#if !defined(FUNCTION_fixnorm_D)
inline INT fixnorm_D(FIXP_DBL val) {
  INT leadingBits = 0;
  if (val != (FIXP_DBL)0) {
    if (val < (FIXP_DBL)0) {
      val = ~val;
    }
    leadingBits = fixnormz_D(val) - 1;
  }
  return (leadingBits);
}
#endif

#endif /* CLZ_H */
