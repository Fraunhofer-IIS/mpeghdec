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

   Author(s):   Oliver Moser

   Description: ROM tables used by FDK tools

*******************************************************************************/

#ifndef FDK_TOOLS_ROM_H
#define FDK_TOOLS_ROM_H

#include "common_fix.h"
#include "FDK_audio.h"

/* TI FFT twiddles */

/* MPEGD MDCT2QMF Module */

/* sinetables */

/* None radix2 rotation vectors */
#ifdef USE_PACKED_ROTVECTOR_240
extern RAM_ALIGN const LONG RotVector240[210];
#else
#endif /* USE_PACKED_ROTVECTOR_240 */

/* Regular sine tables */
extern RAM_ALIGN const FIXP_STP SineTable1024[];
extern RAM_ALIGN const FIXP_STP SineTable512[];

/* AAC-LC windows */
extern RAM_ALIGN const FIXP_WTP SineWindow1024[];
extern RAM_ALIGN const FIXP_WTP KBDWindow1024[];
extern RAM_ALIGN const FIXP_WTP SineWindow128[];
extern RAM_ALIGN const FIXP_WTP KBDWindow128[];

/* AAC-LD windows */
extern RAM_ALIGN const FIXP_WTP SineWindow512[];

/* USAC TCX Window */
extern RAM_ALIGN const FIXP_WTP SineWindow256[256];

/* USAC 8/3 windows */

/* DCT and others */

/**
 * \brief Helper table for window slope mapping. You should prefer the usage of the
 * function FDKgetWindowSlope(), this table is only made public for some optimized
 * access inside dct.cpp.
 */
extern const FIXP_WTP* const windowSlopes[2][4][10];

/**
 * \brief Window slope access helper. Obtain a window of given length and shape.
 * \param length Length of the window slope.
 * \param shape Shape index of the window slope. 0: sine window, 1: Kaiser-Bessel. Any other
 *              value is applied a mask of 1 to, mapping it to either 0 or 1.
 * \param Pointer to window slope or NULL if the requested window slope is not available.
 */
const FIXP_WTP* FDKgetWindowSlope(int length, int shape);

extern const FIXP_WTP sin_twiddle_L64[];

/*
 * Raw Data Block list items.
 */
typedef enum {
  element_instance_tag,
  common_window, /* -> decision for link_sequence */
  global_gain,
  ics_info, /* ics_reserved_bit, window_sequence, window_shape, max_sfb, scale_factor_grouping,
               predictor_data_present, ltp_data_present, ltp_data */
  max_sfb,
  ms,                         /* ms_mask_present, ms_used */
  /*predictor_data_present,*/ /* part of ics_info */
  ltp_data_present,
  ltp_data,
  section_data,
  scale_factor_data,
  pulse, /* pulse_data_present, pulse_data  */
  tns_data_present,
  tns_data,
  gain_control_data_present,
  gain_control_data,
  esc1_hcr,
  esc2_rvlc,
  spectral_data,
  enhancedNoiseFilling,

  scale_factor_data_usac,
  core_mode, /* -> decision for link_sequence */
  common_tw,
  lpd_channel_stream,
  tw_data,
  noise,
  ac_spectral_data,
  fac_data,
  tns_active, /* introduced in MPEG-D usac CD */
  tns_data_present_usac,
  common_max_sfb,

  coupled_elements,   /* only for CCE parsing */
  gain_element_lists, /* only for CCE parsing */

  /* Non data list items */
  adtscrc_start_reg1,
  adtscrc_start_reg2,
  adtscrc_end_reg1,
  adtscrc_end_reg2,
  drmcrc_start_reg,
  drmcrc_end_reg,
  next_channel,
  next_channel_loop,
  link_sequence,
  end_of_sequence
} rbd_id_t;

struct element_list {
  const rbd_id_t* id;
  const struct element_list* next[2];
};

typedef struct element_list element_list_t;
/**
 * \brief get elementary stream pieces list for given parameters.
 * \param aot audio object type
 * \param epConfig the epConfig value from the current Audio Specific Config
 * \param nChannels amount of channels contained in the current element.
 * \param layer the layer of the current element.
 * \param elFlags element specific flags.
 * \return element_list_t parser guidance structure.
 */
const element_list_t* getBitstreamElementList(AUDIO_OBJECT_TYPE aot, SCHAR epConfig,
                                              UCHAR nChannels, UCHAR layer, UINT elFlags);

typedef enum {
  /* n.a. */
  FDK_FORMAT_1_0 = 1,     /* mono */
  FDK_FORMAT_2_0 = 2,     /* stereo */
  FDK_FORMAT_3_0_FC = 3,  /* 3/0.0 */
  FDK_FORMAT_3_1_0 = 4,   /* 3/1.0 */
  FDK_FORMAT_5_0 = 5,     /* 3/2.0 */
  FDK_FORMAT_5_1 = 6,     /* 5.1 */
  FDK_FORMAT_7_1_ALT = 7, /* 5/2.1 ALT */
  /* 8 n.a.*/
  FDK_FORMAT_3_0_RC = 9, /* 2/1.0 */
  FDK_FORMAT_2_2_0 = 10, /* 2/2.0 */
  FDK_FORMAT_6_1 = 11,   /* 3/3.1 */
  FDK_FORMAT_7_1 = 12,   /* 3/4.1 */
  FDK_FORMAT_22_2 = 13,  /* 22.2 */
  FDK_FORMAT_5_2_1 = 14, /* 5/2.1*/
  FDK_FORMAT_5_5_2 = 15, /* 5/5.2 */
  FDK_FORMAT_9_1 = 16,   /* 5/4.1 */
  FDK_FORMAT_6_5_1 = 17, /* 6/5.1 */
  FDK_FORMAT_6_7_1 = 18, /* 6/7.1 */
  FDK_FORMAT_5_6_1 = 19, /* 5/6.1 */
  FDK_FORMAT_7_6_1 = 20, /* 7/6.1 */
  FDK_FORMAT_7_2_1 = 21, /* 7/2.1 */
  FDK_FORMAT_IN_LISTOFCHANNELS = 22,
  FDK_FORMAT_OUT_LISTOFCHANNELS = 23,
  /* 21 formats + In & Out list of channels */
  FDK_NFORMATS = 24,
  FDK_FORMAT_FAIL = -1
} FDK_converter_formatid_t;

#endif
