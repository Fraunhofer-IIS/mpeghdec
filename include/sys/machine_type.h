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

/************************* System integration library **************************

   Author(s):

   Description:

*******************************************************************************/

/** \file   machine_type.h
 *  \brief  Type defines for various processors and compiler tools.
 */

#if !defined(MACHINE_TYPE_H)
#define MACHINE_TYPE_H

#include <stddef.h> /* Needed to define size_t */

/* Library calling convention spec. __cdecl and friends might be added here as required. */
#define LINKSPEC_H
#define LINKSPEC_CPP

/* for doxygen the following docu parts must be separated */
/** \var  SCHAR
 *        Data type representing at least 1 byte signed integer on all supported platforms.
 */
/** \var  UCHAR
 *        Data type representing at least 1 byte unsigned integer on all supported platforms.
 */
/** \var  INT
 *        Data type representing at least 4 byte signed integer on all supported platforms.
 */
/** \var  UINT
 *        Data type representing at least 4 byte unsigned integer on all supported platforms.
 */
/** \var  LONG
 *        Data type representing 4 byte signed integer on all supported platforms.
 */
/** \var  ULONG
 *        Data type representing 4 byte unsigned integer on all supported platforms.
 */
/** \var  SHORT
 *        Data type representing 2 byte signed integer on all supported platforms.
 */
/** \var  USHORT
 *        Data type representing 2 byte unsigned integer on all supported platforms.
 */
/** \var  INT64
 *        Data type representing 8 byte signed integer on all supported platforms.
 */
/** \var  UINT64
 *        Data type representing 8 byte unsigned integer on all supported platforms.
 */
/** \var  INT128
 *        Data type representing 16 byte signed integer on all supported platforms.
 */
/** \var  UINT128
 *        Data type representing 16 byte unsigned integer on all supported platforms.
 */
/** \def  SHORT_BITS
 *        Number of bits the data type short represents. sizeof() is not suited to get this info,
 *        because a byte is not always defined as 8 bits.
 */
/** \def  CHAR_BITS
 *        Number of bits the data type char represents. sizeof() is not suited to get this info,
 *        because a byte is not always defined as 8 bits.
 */
/** \var  INT_PCM
 *        Data type representing the width of input and output PCM samples.
 */

typedef signed int INT;
typedef unsigned int UINT;
#ifdef __LP64__
/* force FDK long-datatypes to 4 byte  */
/* Use defines to avoid type alias problems on 64 bit machines. */
#define LONG INT
#define ULONG UINT
#else  /* __LP64__ */
typedef signed long LONG;
typedef unsigned long ULONG;
#endif /* __LP64__ */
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef signed char SCHAR;
typedef unsigned char UCHAR;

#define SHORT_BITS 16
#define CHAR_BITS 8

/* Define 64 bit base integer type. */
#ifdef _MSC_VER
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
#else
typedef long long INT64;
typedef unsigned long long UINT64;
#endif

/* Define 128 bit base integer type. */
#if (defined(__GNUC__) || defined(__gnu_linux__)) && (defined(__x86__) || defined(__x86_64__)) && \
    defined(__LP64__)
typedef __int128_t INT128;
typedef __uint128_t UINT128;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#if ((defined(__i686__) || defined(__i586__) || defined(__i386__) || defined(__x86_64__)) || \
     (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64)))) &&                        \
    !defined(FDK_ASSERT_ENABLE)
#if !defined(NDEBUG)
#define FDK_ASSERT_ENABLE
#endif
#endif

#if defined(FDK_ASSERT_ENABLE)
#include <assert.h>
#define FDK_ASSERT(x) assert(x)
#else
#define FDK_ASSERT(ignore)
#endif

typedef LONG INT_PCM;
#define MAXVAL_PCM MAXVAL_DBL
#define MINVAL_PCM MINVAL_DBL
#define WAV_BITS 24
#define SAMPLE_BITS 32
#define SAMPLE_MAX ((INT_PCM)(((ULONG)1 << (SAMPLE_BITS - 1)) - 1))
#define SAMPLE_MIN (~SAMPLE_MAX)

/*!
* \def    RAM_ALIGN
*  Used to align memory as prefix before memory declaration. For example:
   \code
   RAM_ALIGN
   int myArray[16];
   \endcode

   Note, that not all platforms support this mechanism. For example with TI compilers
   a preprocessor pragma is used, but to do something like

   \code
   #define RAM_ALIGN #pragma DATA_ALIGN(x)
   \endcode

   would require the preprocessor to process this line twice to fully resolve it. Hence,
   a fully platform-independant way to use alignment is not supported.

* \def    ALIGNMENT_DEFAULT
*         Default alignment in bytes.
*/

#if (defined(_M_ARM) || defined(__arm__) || defined(__aarch64__)) &&                   \
    (defined(__TARGET_FEATURE_NEON) || defined(__ARM_NEON) || defined(__ARM_NEON__) || \
     defined(__ARM_AARCH64_NEON__))
#define ALIGNMENT_DEFAULT 16
#else
#define ALIGNMENT_DEFAULT 16
#endif

/* RAM_ALIGN keyword causes memory alignment of global variables. */
#if defined(_MSC_VER)
#define RAM_ALIGN __declspec(align(ALIGNMENT_DEFAULT))
#elif defined(__GNUC__)
#define RAM_ALIGN __attribute__((aligned(ALIGNMENT_DEFAULT)))
#else
#define RAM_ALIGN
#endif

/*!
 * \def  RESTRICT
 *       The restrict keyword is supported by some platforms and RESTRICT maps to
 *       either the corresponding keyword on each platform or to void if the
 *       compiler does not provide such feature. It tells the compiler that a pointer
 *       points to memory that does not overlap with other memories pointed to by other
 *       pointers. If this keyword is used and the assumption of no overlap is not true
 *       the resulting code might crash.
 *
 * \def  WORD_ALIGNED(x)
 *       Tells the compiler that pointer x is 32 bit aligned. It does not cause the address
 *       itself to be aligned, but serves as a hint to the optimizer. The alignment of the
 *       pointer must be guarranteed, if not the code might crash.
 *
 * \def  DWORD_ALIGNED(x)
 *       Tells the compiler that pointer x is 64 bit aligned. It does not cause the address
 *       itself to be aligned, but serves as a hint to the optimizer. The alignment of the
 *       pointer must be guarranteed, if not the code might crash.
 *
 * \def  FDK_PRAGMA_MUST_ITERATE(var,min,max,multiple)
 *       Tells the compiler that the loop next to this line runs
 *       minimum 'min' times, maximum 'max' times, the number of loop
 *       iterations is always a multiple of 'multiple'.
 *       'min' defaults to 0, if not set
 *       'max' defaults to 0x7FFF.FFF, if not set
 *       'multiple' defaults to 1, if not set
 *       The parameter 'var' is used for debug purpose and checked to be in range [min,max], if
 * DEBUG=1 FDK_PRAGMA_MUST_ITERATE(nBands, 8, 64, 4) for (int i = 0; i < nBands; i++) { ... } Note:
 * Use this macro without an ending semicolon
 *
 * \def  FDK_PRAGMA_CODE_SECTION(section)
 *       Tells the compiler that the function code, next to this line should
 *       be linked into the section, given in quotes. Example
 *       FDK_PRAGMA_CODE_SECTION(".text:text_qmf")
 *       Note: Use this macro without an ending semicolon
 *
 * \def  FDK_PRAGMA_DATA_SECTION(section)
 *       Tells the compiler that the data block, next to this line should
 *       be linked into the section, given in quotes. Example
 *       FDK_PRAGMA_DATA_SECTION(".L1ram")
 *       Note: Use this macro without an ending semicolon
 */
#if defined(__arm__)
#define RESTRICT __restrict
#define WORD_ALIGNED(name)                                                      \
  const FIXP_SGL* __attribute__((aligned(4))) __##name = (const FIXP_SGL*)name; \
  (void)__##name;
#define DWORD_ALIGNED(name)                                                     \
  const FIXP_SGL* __attribute__((aligned(8))) __##name = (const FIXP_SGL*)name; \
  (void)__##name;
#define FDK_PRAGMA_MUST_ITERATE(var, min, max, multiple)
#define FDK_PRAGMA_CODE_SECTION(section)
#define FDK_PRAGMA_DATA_SECTION(section)
#else
#define RESTRICT
/* Non-debug macros */
#define WORD_ALIGNED(x) C_ALLOC_ALIGNED_CHECK2((const void*)(x), 4);
#define DWORD_ALIGNED(x) C_ALLOC_ALIGNED_CHECK2((const void*)(x), 8);
#define FDK_PRAGMA_MUST_ITERATE(var, min, max, multiple)
#define FDK_PRAGMA_CODE_SECTION(section)
#define FDK_PRAGMA_DATA_SECTION(section)
#endif

/*-----------------------------------------------------------------------------------
 * ALIGN_SIZE
 *-----------------------------------------------------------------------------------*/
/*!
 * \brief  This macro aligns a given value depending on ::ALIGNMENT_DEFAULT.
 *
 * For example if #ALIGNMENT_DEFAULT equals 8, then:
 * - ALIGN_SIZE(3) returns 8
 * - ALIGN_SIZE(8) returns 8
 * - ALIGN_SIZE(9) returns 16
 */
#define ALIGN_SIZE(a) \
  ((a) +              \
   (((INT)ALIGNMENT_DEFAULT - ((size_t)(a) & (ALIGNMENT_DEFAULT - 1))) & (ALIGNMENT_DEFAULT - 1)))

/*!
 * \brief  This macro aligns a given address depending on ::ALIGNMENT_DEFAULT.
 */
#define ALIGN_PTR(a)                                                              \
  ((void*)((unsigned char*)(a) +                                                  \
           ((((INT)ALIGNMENT_DEFAULT - ((size_t)(a) & (ALIGNMENT_DEFAULT - 1))) & \
             (ALIGNMENT_DEFAULT - 1)))))

/* Alignment macro for libSYS heap implementation */
#define ALIGNMENT_EXTRES (ALIGNMENT_DEFAULT)
#define ALGN_SIZE_EXTRES(a) \
  ((a) + (((INT)ALIGNMENT_EXTRES - ((INT)(a) & (ALIGNMENT_EXTRES - 1))) & (ALIGNMENT_EXTRES - 1)))

/*!
 * \def  FDK_FORCEINLINE
 *       Sometimes compiler do not do what they are told to do, and in case of inlining some
 *       additional command might be necessary depending on the platform.
 *
 * \def  FDK_INLINE
 *       Defines how the compiler is told to inline stuff.
 */
#ifndef FDK_FORCEINLINE
#if defined(__GNUC__)
#define FDK_FORCEINLINE inline __attribute((always_inline))
#else
#define FDK_FORCEINLINE inline
#endif
#endif

#define FDK_INLINE static inline

/*!
 * \def  LNK_SECTION_DATA_L1
 *       The LNK_SECTION_* defines allow memory to be drawn from specific memory
 *       sections. Used as prefix before variable declaration.
 *
 * \def  LNK_SECTION_DATA_L2
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_L1_DATA_A
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_L1_DATA_B
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CONSTDATA_L1
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CONSTDATA
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CODE_L1
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CODE_L2
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_INITCODE
 *       See ::LNK_SECTION_DATA_L1
 */
/**************************************************
 * Code Section macros
 **************************************************/
#define LNK_SECTION_CODE_L1
#define LNK_SECTION_CODE_L2
#define LNK_SECTION_INITCODE

/* Memory section macros. */

/* default fall back */
#define LNK_SECTION_DATA_L1
#define LNK_SECTION_DATA_L2
#define LNK_SECTION_CONSTDATA
#define LNK_SECTION_CONSTDATA_L1

#define LNK_SECTION_L1_DATA_A
#define LNK_SECTION_L1_DATA_B

/**************************************************
 * Macros regarding static code analysis
 **************************************************/
#if defined(__cplusplus) && defined(__has_cpp_attribute) && !(_MSC_VER >= 1921)
#if __cplusplus >= 201703L && __has_cpp_attribute(fallthrough)
#define FDK_FALLTHROUGH [[fallthrough]]
#elif __cplusplus >= 201103L && __has_cpp_attribute(gnu::fallthrough)
#define FDK_FALLTHROUGH [[gnu::fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#define FDK_FALLTHROUGH [[clang::fallthrough]]
#endif
#endif

#ifndef FDK_FALLTHROUGH
#if defined(__GNUC__) && (__GNUC__ >= 7)
#define FDK_FALLTHROUGH __attribute__((fallthrough))
#else
#define FDK_FALLTHROUGH
#endif
#endif

#ifdef _MSC_VER
/*
 * Sometimes certain features are excluded from compilation and therefore the warning 4065 may
 * occur: "switch statement contains 'default' but no 'case' labels" We consider this warning
 * irrelevant and disable it.
 */
#pragma warning(disable : 4065)
#endif

#endif /* MACHINE_TYPE_H */
