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

/******************** MPEG-H 3DA channel rendering library *********************

   Author(s):   Achim Kuntz, Alfonso Pino, Thomas Blender

   Description: format converter library

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for
 their own purpose, assign or donate the code to a third party and to inhibit third
 parties from using the code for products that do not conform to MPEG-related ITU
 Recommendations and/or ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/

#ifndef FDK_FORMATCONVERTER_INIT_H
#define FDK_FORMATCONVERTER_INIT_H

#include "FDK_formatConverter_intern.h"
#include "common_fix.h"

/**

 * Error codes
 * ************************************************************************** */
typedef enum {
  FORMAT_CONVERTER_STATUS_OK = 0,             /* success */
  FORMAT_CONVERTER_STATUS_FAILED = -1,        /* generic error */
  FORMAT_CONVERTER_STATUS_MISSING_RULE = -2,  /* missing downmix rule */
  FORMAT_CONVERTER_STATUS_INFORMAT = -3,      /* invalid input format index */
  FORMAT_CONVERTER_STATUS_OUTFORMAT = -4,     /* invalid output format index */
  FORMAT_CONVERTER_STATUS_SFREQ = -5,         /* invalid sampling frequency */
  FORMAT_CONVERTER_STATUS_BLOCKLENGTH = -6,   /* invalid block length */
  FORMAT_CONVERTER_STATUS_TRIM = -7,          /* out-of-range trim values */
  FORMAT_CONVERTER_STATUS_RANDOMIZATION = -8, /* randomization out-of-range */
  FORMAT_CONVERTER_STATUS_BANDS = -9          /* bands out-of-range */
} converter_status_t;

/* public data types / structures
 * ************************************************************************** */

/* API functions
 * ************************************************************************** */

/**
 * Set input format or output format that is not in list of known formats.
 * Each function call either sets input _or_ output format.
 * Allocates memory.
 */
void converter_set_inout_format(
    IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt,
    const INT inout,        /* in:  0: set input format, 1: set output format */
    const INT num_channels, /* in:  number of channels in setup to define */
    const AFC_FORMAT_CONVERTER_CHANNEL_ID* channel_vector /* in:  vector containing channel enums */
);

/**
 * Initialize audio processing.
 *
 * No memory is allocated, nothing to release.
 */
converter_status_t converter_init(
    IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt,
    converter_pr_t* params,                       /* out: initialized parameters */
    converter_pr_tmp_t* params_tmp,               /* out: initialized temporary parameters */
    const FDK_converter_formatid_t input_format,  /* in:  input format id */
    const FDK_converter_formatid_t output_format, /* in:  output format id */
    const INT* randomization, /* in:  randomization angles [azi,ele,azi,ele, ... in degrees] */
    const INT sfreq_Hz,       /* in:  sampling frequency [Hz] */
    const INT blocklength,    /* in:  processing block length [samples] */
    const UINT nbands,        /* in:  number of filterbank processing bands */
    const FIXP_DBL*
        bands_nrm, /* in:  filterbank processing bands center frequencies [normalized frequency] */
    INT* p_bufffer /* in:  temporary buffer */
);

FIXP_DBL peak_filter(const FIXP_DBL f,          /* peak frequency [Hz] */
                     INT f_e, const FIXP_DBL q, /* peak Q factor */
                     INT q_e, const FIXP_DBL g, /* peak gain */
                     INT g_e, const FIXP_DBL G, /* gain */
                     INT G_e, const FIXP_DBL b, /* band center frequency [Hz] */
                     INT b_e, INT* result_e);

void setActiveDownmixRange_StftErb(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, INT fs);
void normalizePG(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt);

#endif /* FDK_FORMATCONVERTER_INIT_H */
