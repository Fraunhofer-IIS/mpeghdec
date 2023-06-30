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

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(CLZ_ARM_H)
#define CLZ_ARM_H

#if defined(__arm__)

#define FUNCTION_fixnormz_D
#if defined(__GNUC__)
/* ARM gcc*/

#if defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_8__) || \
    defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#if defined FUNCTION_fixnormz_D
#define FUNCTION_fixnorm_D
#define FUNCTION_fixnormz_S
#define FUNCTION_fixnorm_S
#endif

#ifdef FUNCTION_fixnormz_D
inline INT fixnormz_D(LONG value) {
  INT result;
#if defined(__ARM_ARCH_8__)
  asm("clz %w0, %w1 " : "=r"(result) : "r"(value));
#else
  asm("clz %0, %1 " : "=r"(result) : "r"(value));
#endif
  return result;
}
#endif /* #ifdef FUNCTION_fixnormz_D */

#ifdef FUNCTION_fixnorm_D
inline INT fixnorm_D(LONG value) {
  INT result;
#if defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if (!value) return 0;
  if (value < 0) value = ~value;
  return fixnormz_D(value) - 1;
#else
  asm("subs  %0, %1, #0 \n\t"
      "mvnlt %0, %0     \n\t"
      "clzne %0, %0     \n\t"
      "subne %0, %0, #1 \n\t"
      : "=r"(result)
      : "r"(value)
      : "cc");
#endif
  return result;
}
#endif /* #ifdef FUNCTION_fixnorm_D */

#ifdef FUNCTION_fixnormz_S
inline INT fixnormz_S(SHORT value) {
  INT result;
#if defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  result = (LONG)(value << 16);
  if (result == 0)
    result = 16;
  else
    result = fixnormz_D(result);
#else
  asm("subs  %0, %1, #0    \n\t"
      "clzge %0, %0        \n\t"
      "movlt %0, #0        \n\t"
      "subge %0, %0, #16   \n\t"
      : "=r"(result)
      : "r"(value)
      : "cc");
#endif
  return result;
}
#endif /* #ifdef FUNCTION_fixnormz_S */

#ifdef FUNCTION_fixnorm_S
inline INT fixnorm_S(SHORT value) {
  INT result;
#if defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  LONG lvalue = (LONG)(value << 16);
  if (!lvalue) return 0;
  if (lvalue < 0) lvalue = ~lvalue;
  return fixnormz_D(lvalue) - 1;
#else
  asm("lsls  %0, %1, #16 \n\t"
      "mvnlt %0, %0      \n\t"
      "clzne %0, %0      \n\t"
      "subne %0, %0, #1  \n\t"
      : "=r"(result)
      : "r"(value)
      : "cc");
#endif
  return result;
}
#endif /* #ifdef FUNCTION_fixnorm_S */

#endif

#endif /* arm toolchain */

#endif /* __arm__ */

#endif /* !defined(CLZ_ARM_H) */
