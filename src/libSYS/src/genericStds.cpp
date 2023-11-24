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

   Description: - Generic memory, stdio, string, etc. function wrappers or
                builtins.
                - OS dependant function wrappers.

*******************************************************************************/

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define __GENERICSTDS_CPP__

#include "genericStds.h"

/* library info */
#define SYS_LIB_VL0 3
#define SYS_LIB_VL1 0
#define SYS_LIB_VL2 0
#define SYS_LIB_TITLE "System Integration Library"
#ifdef __ANDROID__
#define SYS_LIB_BUILD_DATE ""
#define SYS_LIB_BUILD_TIME ""
#else
#define SYS_LIB_BUILD_DATE __DATE__
#define SYS_LIB_BUILD_TIME __TIME__
#endif

#include <ctype.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/***************************************************************
 * memory allocation monitoring variables
 ***************************************************************/

/* Include OS/System specific implementations. */

INT FDKisalpha(INT c) {
  return c < 0 ? 0 : c > 255 ? 0 : isalpha(c);
}

INT FDKtolower(INT c) {
  return c < 0 ? 0 : c > 255 ? 0 : tolower(c);
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void FDKprintf(const char* szFmt, ...) {
  va_list ap;
  va_start(ap, szFmt);
  vprintf(szFmt, ap);
  va_end(ap);
}

void FDKprintfErr(const char* szFmt, ...) {
  va_list ap;
  va_start(ap, szFmt);
  vfprintf(stderr, szFmt, ap);
  va_end(ap);
}

INT FDKgetchar(void) {
  return (INT)getchar();
}

INT FDKfprintf(FDKFILE* stream, const char* format, ...) {
  INT chars = 0;
  va_list ap;
  va_start(ap, format);
  chars += vfprintf((FILE*)stream, format, ap);
  va_end(ap);
  return chars;
}

/************************************************************************************************/

/************************************************************************************************/

char* FDKstrchr(char* s, INT c) {
  return strchr(s, c);
}
const char* FDKstrstr(const char* haystack, const char* needle) {
  return strstr(haystack, needle);
}
char* FDKstrcpy(char* dest, const char* src) {
  return strcpy(dest, src);
}
char* FDKstrncpy(char* dest, const char* src, UINT n) {
  return strncpy(dest, src, n);
}

/*************************************************************************
 * DYNAMIC MEMORY management (heap)
 *************************************************************************/

void* FDKcalloc(const UINT n, const UINT size) {
  void* ptr;

  ptr = calloc(n, size);

  return ptr;
}

void* FDKmalloc(const UINT size) {
  void* ptr;

  ptr = malloc(size);

  return ptr;
}

void FDKfree(void* ptr) {
  free((INT*)ptr);
}

void* FDKaalloc(const UINT size, const UINT alignment) {
  void *addr, *result = NULL;
  addr = FDKcalloc(1, size + alignment + (UINT)sizeof(void*)); /* Malloc and clear memory. */

  if (addr != NULL) {
    result = ALIGN_PTR((unsigned char*)addr + sizeof(void*)); /* Get aligned memory base address. */
    *(((void**)result) - 1) = addr;                           /* Save malloc'ed memory pointer.   */
    C_ALLOC_ALIGNED_REGISTER(result, size);
  }

  return result; /* Return aligned address.          */
}

void FDKafree(void* ptr) {
  void* addr;
  addr = *(((void**)ptr) - 1); /* Get pointer to malloc'ed memory. */

  C_ALLOC_ALIGNED_UNREGISTER(ptr);

  FDKfree(addr); /* Free malloc'ed memory area.      */
}

/*--------------------------------------------------------------------------*
 * DATA MEMORY L1/L2 (fallback)
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * FDKcalloc_L
 *--------------------------------------------------------------------------*/
void* FDKcalloc_L(const UINT dim, const UINT size, MEMORY_SECTION s) {
  return FDKcalloc(dim, size);
}

void FDKfree_L(void* p) {
  FDKfree(p);
}

void* FDKaalloc_L(const UINT size, const UINT alignment, MEMORY_SECTION s) {
  void *addr, *result = NULL;
  addr = FDKcalloc_L(1, size + alignment + (UINT)sizeof(void*), s); /* Malloc and clear memory. */

  if (addr != NULL) {
    result = ALIGN_PTR((unsigned char*)addr + sizeof(void*)); /* Get aligned memory base address. */
    *(((void**)result) - 1) = addr;                           /* Save malloc'ed memory pointer.   */
    C_ALLOC_ALIGNED_REGISTER(result, size);
  }

  return result; /* Return aligned address.          */
}

void FDKafree_L(void* ptr) {
  void* addr;

  addr = *(((void**)ptr) - 1); /* Get pointer to malloc'ed memory. */

  C_ALLOC_ALIGNED_UNREGISTER(ptr);

  FDKfree_L(addr); /* Free malloc'ed memory area.      */
}

/*---------------------------------------------------------------------------------------
 * FUNCTION:    FDKmemcpy
 * DESCRIPTION: - copies memory from "src" to "dst" with length "size" bytes
 *              - compiled with FDK_DEBUG will give you warnings
 *---------------------------------------------------------------------------------------*/
void FDKmemcpy(void* dst, const void* src, const UINT size) {
  /* -- check for overlapping memory areas -- */
  FDK_ASSERT(((const unsigned char*)dst - (const unsigned char*)src) >= (ptrdiff_t)size ||
             ((const unsigned char*)src - (const unsigned char*)dst) >= (ptrdiff_t)size);

  /* do the copy */
  memcpy(dst, src, size);
}

void FDKmemmove(void* dst, const void* src, const UINT size) {
  memmove(dst, src, size);
}

void FDKmemset(void* memPtr, const INT value, const UINT size) {
  memset(memPtr, value, size);
}

void FDKmemclear(void* memPtr, const UINT size) {
  FDKmemset(memPtr, 0, size);
}

UINT FDKstrlen(const char* s) {
  return (UINT)strlen(s);
}

/* Compare function wrappers */
INT FDKmemcmp(const void* s1, const void* s2, const UINT size) {
  return memcmp(s1, s2, size);
}
INT FDKstrcmp(const char* s1, const char* s2) {
  return strcmp(s1, s2);
}
INT FDKstrncmp(const char* s1, const char* s2, const UINT size) {
  return strncmp(s1, s2, size);
}

int IS_LITTLE_ENDIAN(void) {
  int __dummy = 1;
  return (*((UCHAR*)(&(__dummy))));
}

UINT TO_LITTLE_ENDIAN(UINT val) {
  return IS_LITTLE_ENDIAN() ? val
                            : (((val & 0xff) << 24) | ((val & 0xff00) << 8) |
                               ((val & 0xff0000) >> 8) | ((val & 0xff000000) >> 24));
}

/* ==================== FILE I/O ====================== */

FDKFILE* FDKfopen(const char* filename, const char* mode) {
  return fopen(filename, mode);
}
INT FDKfclose(FDKFILE* fp) {
  return fclose((FILE*)fp);
}
INT FDKfseek(FDKFILE* fp, LONG OFFSET, int WHENCE) {
  return fseek((FILE*)fp, OFFSET, WHENCE);
}
INT FDKftell(FDKFILE* fp) {
  return (INT)ftell((FILE*)fp);
}
#if !defined(FUNCTION_FDKfseek64)
INT FDKfseek64(FDKFILE* fp, INT64 OFFSET, int WHENCE) {
  INT err;
  err = fseek((FILE*)fp, OFFSET, WHENCE);
  if (err) {
    FDKprintfErr("Error: FDKfseek64() = %x\n", err);
  }
  return err;
}
#endif
#if !defined(FUNCTION_FDKftell64)
INT64 FDKftell64(FDKFILE* fp) {
  INT64 pos;
  pos = ftell((FILE*)fp);
  if (pos == -1L) {
    FDKprintfErr("Error: FDKftell64() returns invalid file position indicator.\n");
  }
  return pos;
}
#endif
INT FDKfflush(FDKFILE* fp) {
  return fflush((FILE*)fp);
}
const INT FDKSEEK_SET = SEEK_SET;
const INT FDKSEEK_CUR = SEEK_CUR;
const INT FDKSEEK_END = SEEK_END;

UINT FDKfwrite(const void* ptrf, INT size, UINT nmemb, FDKFILE* fp) {
  return (UINT)fwrite(ptrf, size, nmemb, (FILE*)fp);
}
UINT FDKfread(void* dst, INT size, UINT nmemb, FDKFILE* fp) {
  return (UINT)fread(dst, size, nmemb, (FILE*)fp);
}
char* FDKfgets(void* dst, INT size, FDKFILE* fp) {
  return fgets((char*)dst, size, (FILE*)fp);
}
void FDKrewind(FDKFILE* fp) {
  FDKfseek((FILE*)fp, 0, FDKSEEK_SET);
}

UINT FDKfwrite_EL(const void* ptrf, INT size, UINT nmemb, FDKFILE* fp) {
  if (IS_LITTLE_ENDIAN()) {
    FDKfwrite(ptrf, size, nmemb, fp);
  } else {
    UINT n;
    INT s;

    const UCHAR* ptr = (const UCHAR*)ptrf;

    for (n = 0; n < nmemb; n++) {
      for (s = size - 1; s >= 0; s--) {
        FDKfwrite(ptr + s, 1, 1, fp);
      }
      ptr = ptr + size;
    }
  }
  return nmemb;
}

UINT FDKfread_EL(void* dst, INT size, UINT nmemb, FDKFILE* fp) {
  UINT n, s0, s1, err;
  UCHAR tmp, *ptr;
  UCHAR tmp24[3];

  /* Enforce alignment of 24 bit data. */
  if (size == 3) {
    ptr = (UCHAR*)dst;
    for (n = 0; n < nmemb; n++) {
      if ((err = FDKfread(tmp24, 1, 3, fp)) != 3) {
        return err;
      }
      *ptr++ = tmp24[0];
      *ptr++ = tmp24[1];
      *ptr++ = tmp24[2];
      /* Sign extension */
      if (tmp24[2] & 0x80) {
        *ptr++ = 0xff;
      } else {
        *ptr++ = 0;
      }
    }
    err = nmemb;
    size = sizeof(LONG);
  } else {
    if ((err = FDKfread(dst, size, nmemb, fp)) != nmemb) {
      return err;
    }
  }
  if (!IS_LITTLE_ENDIAN() && size > 1) {
    ptr = (UCHAR*)dst;
    for (n = 0; n < nmemb; n++) {
      for (s0 = 0, s1 = size - 1; s0 < s1; s0++, s1--) {
        tmp = ptr[s0];
        ptr[s0] = ptr[s1];
        ptr[s1] = tmp;
      }
      ptr += size;
    }
  }
  return err;
}

INT FDKfeof(FDKFILE* fp) {
  return feof((FILE*)fp);
}
