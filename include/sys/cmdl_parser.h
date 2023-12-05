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

   Description: command line parser

*******************************************************************************/

/** \file   cmdl_parser.h
 *  \brief  Command line parser.
 *
 *  The command line parser can extract certain data fields out of a character
 *  string and assign values to variables. It has 2 main functions. One to parse
 *  a command line in the form of standard C runtime "argc" and "argv" parameters,
 *  and the other to assemble these parameters reading text lines from a file in
 *  case the C runtime does not provide them.
 */

#ifndef CMDL_PARSER_H
#define CMDL_PARSER_H

#include "machine_type.h"

#define CMDL_MAX_STRLEN 511
#define CMDL_MAX_ARGC 64

/* \cond */
/* Type definition for text */
#define TEXTCHAR char
#define _tcslen(a) strlen(a)
#define _tcscpy strcpy
#define _tcscmp strcmp
#define _tcsncmp strncmp
#define _tscanf scanf
#define _TEXT(x) x
#define _tfopen fopen
#define _ftprintf fprintf
#define _tcsncpy strncpy
#define _tstof(x) (float)strtod(x, NULL)
#define _tstol atol
#define _tstoi atoi
#define _tcstol strtol
#define _istdigit isdigit
/* \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Scans argc, argv and a scanf style format string for parameters and stores the
 *  values in the variable number of pointers passed to the function.

 For example:
   \code
   #define ARG_PARAM  "(-a %d) (-v %1)"
   #define ARG_VALUE &config->aot, &verbose
   int nFoundArgs = IIS_ScanCmdl(argc, argv, ARG_PARAM, ARG_VALUE);
   \endcode
   wheras the wild-cards (\%d, \%1, ..) define the data type of the argument:
   - \%1 boolean (e. g. -x)
   - \%d integer (e. g. -x 23)
   - \%f float (e. g. -x 3.4)
   - \%y double (e. g. -x 31415926535897932384626433832795028841971693993751)
   - \%s string (e. g. -x "file.dat")
   - \%u unsigned character (e. g. -x 3)
   - \%c signed character (e. g. -x -3)
    More examples on how to use it are located in every (encoder/decoder) example framework.

 * \param argc      Number of arguments.
 * \param argv      Complete character string of the command line arguments.
 * \param pReqArgs  A list of parameters and a corresponding list of memory addresses to
 *                  assign each parameter to.
 *
 * \return  Number of found arguments.
 */
INT IIS_ScanCmdl(INT argc, TEXTCHAR* argv[], const TEXTCHAR* pReqArgs, ...);

typedef int (*iis_process_commandline_function)(int, TEXTCHAR**);

/**
 *  Reads a text file, assembles argc and argv parameters for each text line
 *  and calls the given function for each set of argc, argv parameters.
 *
 * \param param_filename  Name of text file that should be parsed.
 * \param pFunction       Pointer to function that should be called for every text line found.
 *
 * \return  0 on success, 1 on failure.
 */
INT IIS_ProcessCmdlList(const TEXTCHAR* param_filename, iis_process_commandline_function pFunction);
#ifdef __cplusplus
}
#endif

#endif /* CMDL_PARSER_H */
