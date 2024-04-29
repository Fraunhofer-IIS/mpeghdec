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

   Author(s):

   Description:

*******************************************************************************/

#ifndef FDK_FORMATCONVERTER_INTERN_H
#define FDK_FORMATCONVERTER_INTERN_H

#include "FDK_cicp2geometry.h"
#include "FDK_formatConverterLib.h"
#include "FDK_stftfilterbank_api.h"
#include "FDK_tools_rom.h"

#define FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNEL_GROUPS (28)
#define FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS (28)
#define FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS (24)

/**
 * Converter parameters
 */
#define IN_OUT_N (100)
#define MAXBANDS (58)
#define NCHANOUT_MAX (FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS)
#define NCHANIN_MAX (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS)
#define FDK_FORMAT_CONVERTER_MAX_LFE (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS)

/* Downmix Module data types and definitions */

/* Data type for downmix matrix coefficients */

#define FIXP_DMX_H FIXP_SGL
#define FL2FXCONST_DMX_H(val) FL2FXCONST_SGL(val)
#define FL2FX_DMX_H(val) FL2FX_SGL(val)
#define FX_DBL2FX_DMX_H(x) FX_DBL2FX_SGL(x)
#define FX_DMX_H2FX_DBL(x) FX_SGL2FX_DBL(x)
#define MAXVAL_DMX_H MAXVAL_SGL
#define MINVAL_DMX_H MINVAL_SGL

/* Data type for equalizer coefficients */

#define FIXP_0_dot_5 0x40000000
#define FIXP_0_dot_25 0x20000000
#define FIXP_EQ_H FIXP_DBL
#define FL2FXCONST_EQ_H(val) FL2FXCONST_DBL(val)
#define FX_DBL2FX_EQ_H(x) (x)
#define MAXVAL_EQ_H MAXVAL_DBL
#define MINVAL_EQ_H MINVAL_DBL

#define EQ_H_EXP 1 /* default EQs exponent */
#define FIXP_EQ_H_FORMAT_1_dot_0 FIXP_EQ_H(FIXP_0_dot_5)
#define EQ_BITSTREAM_H_EXP 2 /* bitstream parsed EQs exponent */
#define FIXP_EQ_BITSTREAM_H_FORMAT_1_dot_0 FIXP_EQ_H(FIXP_0_dot_25)

/* Data type for downmix input/output data */
#define DMXH_PCM FIXP_DBL
#define DMXH_PCMF FIXP_DBL
#define DMXH_HEADROOM 8 /* PCM output headroom, must have the same values as PCM_OUT_HEADROOM */
#define FX_DBL2FX_DMXH(x) (x)

#define FX_PI FL2FXCONST_DBL(M_PI / 4)
#define FIX_ANGLE(x) (x / 360)

/** Handle to Format Converter parameter struct.
    Holds Format Converter parameter data. */
typedef struct T_FORMAT_CONVERTER_PARAMS* HANDLE_FORMAT_CONVERTER_PARAMS;

/** Handle to Format Converter state struct.
    Holds Format Converter state data. */
typedef struct T_FORMAT_CONVERTER_STATE* HANDLE_FORMAT_CONVERTER_STATE;

/* downmix processing rules */

typedef enum {
  /* external rules (indices 0..10) */
  RULE_NOPROC = 0, /* no processing channel copy with gain */
  RULE_EQ1 = 1,    /* eq for front up-median downmix */
  RULE_EQ2 = 2,    /* eq for surround up-median downmix */
  RULE_EQ3 = 3,    /* eq for top-up downmix */
  RULE_EQ4 = 4,    /* eq for top-median downmix */
  RULE_EQ5 = 5,    /* eq for horizontal channel that is displaced to height */
  /* New equalizers */
  RULE_EQVF = 7,   /* eq for CH_U_L030, CH_U_R030, CH_U_L045, CH_U_R045 */
  RULE_EQVB = 8,   /* eq for CH_U_L135, CH_U_R135 */
  RULE_EQVFC = 9,  /* eq for CH_U_000 */
  RULE_EQVBC = 10, /* eq for CH_U_180 */
  RULE_EQVOG = 11, /* eq for CH_T_000 */
  RULE_EQVS = 12,  /* eq for CH_U_L090, CH_U_R110 */
  RULE_EQBTM = 13, /* eq for CH_L_000, CH_L_L045, CH_L_R045 */
  RULE_EQVBA = 14, /* eq for CH_U_L110, CH_U_R110 */

  RULE_06_03 = 15, /* eq for CH_M_L060 and CH_M_R060 to CH_M_L030 and CH_M_R030 */
  RULE_09_03 = 16, /* eq for CH_M_L090 and CH_M_R090 to CH_M_L030 and CH_M_R030 */
  RULE_06_11 = 17, /* eq for CH_M_L060 and CH_M_R060 to CH_M_L110 and CH_M_R110 */
  RULE_09_11 = 18, /* eq for CH_M_L090 and CH_M_R090 to CH_M_L110 and CH_M_R110 */
  RULE_13_11 = 19, /* eq for CH_M_L135 and CH_M_R135 to CH_M_L110 and CH_M_R110 */
  RULE_18_11 = 20, /* eq for CH_M_180          to CH_M_L110 and CH_M_R110 */
  RULE_REQ = 21,   /* first eq for random setups */
  N_EQ = 40, /* number of eqs, the value of N_EQ is 40 in case of randomization, otherwise 20 */
  /* rules only used within converter only */
  RULE_PANNING = 100,  /* manual amplitude panning (specifying alpha0, alpha) */
  RULE_TOP2ALLU = 101, /* top channel to all upper channels */
  RULE_TOP2ALLM = 102, /* top channel to all horizontal channels */
  RULE_AUTOPAN = 103,  /* automatic amplitude panning (alpha0 defined by destimation */
  RULE_VIRTUAL = 104
  /* channels, alpha defined by source channels) */
} converter_dmxrulesid_t;

typedef struct converter_pr {
  FIXP_EQ_H eq[N_EQ][MAXBANDS]; /* equalizers [lin gains] */

} converter_pr_t;

typedef struct converter_pr_tmp {
  SCHAR in_out_src[IN_OUT_N];       /* in-out conversion source */
  SCHAR in_out_dst[IN_OUT_N];       /* in-out conversion destination */
  FIXP_DMX_H in_out_gain[IN_OUT_N]; /* in-out conversion gain [lin] */
  SCHAR in_out_proc[IN_OUT_N];      /* in-out conversion processing */

  FIXP_DMX_H in_out_gainL[IN_OUT_N]; /* in-out conversion gain, G_vL [lin] */

  /* variables for the timbral elevation rendering */
  SCHAR in_out_src2[IN_OUT_N];       /* in-out conversion source */
  SCHAR in_out_dst2[IN_OUT_N];       /* in-out conversion destination */
  FIXP_DMX_H in_out_gain2[IN_OUT_N]; /* in-out conversion gain [lin] */
  SCHAR in_out_proc2[IN_OUT_N];      /* in-out conversion processing */

} converter_pr_tmp_t;

/* INDEX : FOR THE CONVENIENCE FOR ONLY HEIGHT CHANNELS */
typedef enum { TFC = 0, TFL, TFR, TFLA, TFRA, TSL, TSR, TBLA, TBRA, TBL, TBR, TBC, VOG } H;

/* INDEX : FOR THE 5.1 CHANNEL SYSTEM */
typedef enum { FL = 0, FR, FC, SW, SL, SR } C;

/*
IMPORTANT/WARNING:
    -the converter implementation assumes that all horizontal channels are:
        CH_M_000 <= i <= CH_M_180
    -the converter implementation assumes that all height channels are:
        CH_U_000 <= i <= CH_U_180
    Should this be changed, then converter.c needs to be modified!
*/

typedef enum {
  /* horizontal channels */
  CH_M_000 = 0,
  CH_M_L022 = 1,
  CH_M_R022 = 2,
  CH_M_L030 = 3,
  CH_M_R030 = 4,
  CH_M_L045 = 5,
  CH_M_R045 = 6,
  CH_M_L060 = 7,
  CH_M_R060 = 8,
  CH_M_L090 = 9,
  CH_M_R090 = 10,
  CH_M_L110 = 11,
  CH_M_R110 = 12,
  CH_M_L135 = 13,
  CH_M_R135 = 14,
  CH_M_L150 = 15,
  CH_M_R150 = 16,
  CH_M_180 = 17,
  /* height channels */
  CH_U_000 = 18,
  CH_U_L045 = 19,
  CH_U_R045 = 20,
  CH_U_L030 = 21,
  CH_U_R030 = 22,
  CH_U_L090 = 23,
  CH_U_R090 = 24,
  CH_U_L110 = 25,
  CH_U_R110 = 26,
  CH_U_L135 = 27,
  CH_U_R135 = 28,
  CH_U_180 = 29,
  /* top channel */
  CH_T_000 = 30,
  /* low channels */
  CH_L_000 = 31,
  CH_L_L045 = 32,
  CH_L_R045 = 33,
  /* low frequency effects */
  CH_LFE1 = 34,
  CH_LFE2 = 35,
  CH_LFE3 = 36,
  /* SCR */
  CH_M_LSCR = 37,
  CH_M_RSCR = 38,
  /* empty channel */
  CH_EMPTY = -1
} AFC_FORMAT_CONVERTER_CHANNEL_ID;

typedef enum {
  FDK_FORMAT_CONVERTER_INPUT_FORMAT_INVALID = 0,
  FDK_FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS,
  FDK_FORMAT_CONVERTER_INPUT_FORMAT_GENERIC
} FDK_FORMAT_CONVERTER_INPUT_FORMAT;

typedef enum {
  FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID = 0,
  FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS,
  FDK_FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC
} FDK_FORMAT_CONVERTER_OUTPUT_FORMAT;

typedef struct T_FORMAT_CONVERTER_PARAMS {
  /** Sampling rate. */
  INT samplingRate;

  /** Needs to be set, if the IIS_FORMATCONVERTER_MODE ==
   * IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN */
  FIXP_DBL phaseAlignStrength;
  FIXP_DBL adaptiveEQStrength;

  /** format converter input format of internal init structure */
  FDK_converter_formatid_t formatConverterInputFormat_internal;
  /** format converter output format of internal init structure */
  FDK_converter_formatid_t formatConverterOutputFormat_internal;

  /** handle to internal init structure*/
  converter_pr_t* formatConverterParams_internal;

  /** flag signalling generic input,output formats */
  INT genericIOFmt;

  /** dmx matrix */
  FIXP_DMX_H** dmxMtx;
  FIXP_DMX_H* dmxMtx_sorted;
  /* equalizer index vector [in][out]*/
  INT** eqIndexVec;
  INT* eqIndexVec_sorted;

  /** dmx matrix L */
  FIXP_DMX_H** dmxMtxL;
  FIXP_DMX_H* dmxMtxL_sorted;
  /** dmx matrix 2 */
  FIXP_DMX_H** dmxMtx2;
  FIXP_DMX_H* dmxMtx2_sorted;
  /* equalizer index vector 2 [in][out]*/
  INT** eqIndexVec2;
  INT* eqIndexVec2_sorted;

  /* Immersive mode control */
  INT immersiveMode;
  INT Mode3Drendering;
  INT Mode2Drendering;
  INT rendering3DTypeFlag_internal;

  INT chOut_count[NCHANOUT_MAX];
  INT chOut_exp[NCHANOUT_MAX];
  INT dmx_iterations;

  FIXP_DMX_H* dmxMatrixL_FDK;
  FIXP_DMX_H* dmxMatrixH_FDK;
  INT* eqIndex_FDK;

  /** flag signalling the DMX matrix coefficients have been set */
  INT dmxMtxIsSet;

  /** formatConverterDelay */
  INT formatConverterDelay;

  /** center frequencies */
  const FIXP_DBL* centerFrequenciesNormalized;

} FORMAT_CONVERTER_PARAMS;

/**********************************************************************************************************************************/

typedef struct T_FORMAT_CONVERTER_STATE {
  /** handle for params and states of active dmx in stft domain */
  void* handleActiveDmxStft;

} FORMAT_CONVERTER_STATE;

typedef struct {
  UINT fcNumFreqBands;
  const FIXP_DBL* fcCenterFrequencies;

  CICP2GEOMETRY_CHANNEL_GEOMETRY inputChannelGeo[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];

  UINT numInputChannels[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNEL_GROUPS];
  UINT numInputChannelGroups;

  UINT amountOfAddedDmxMatricesAndEqualizers;
  /* EqualizerConfig variables. Parsed and decoded bitstream EQs. */
  FIXP_EQ_H* eqGains[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];

  UINT numTotalInputChannels;

  UINT frameSize;
  INT STFT_headroom_prescaling;
  UINT numTimeSlots;
  UINT samplingRate;

  UINT immersiveDownmixFlag;
  UINT rendering3DTypeFlag;

  /* Candidates */
  IIS_FORMATCONVERTER_MODE mode;
  FDK_FORMAT_CONVERTER_INPUT_FORMAT fcInputFormat;
  FDK_FORMAT_CONVERTER_OUTPUT_FORMAT fcOutputFormat;
  INT aes;
  UINT numOutputChannels;
  CICP2GEOMETRY_CHANNEL_GEOMETRY outChannelGeo[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];
  INT cicpLayoutIndex;
  UINT outChannelVbapFlag;
  UINT inChannelVbapFlag;

  HANDLE_FORMAT_CONVERTER_PARAMS fcParams;
  HANDLE_FORMAT_CONVERTER_STATE fcState;

  /***************Buffers and handles for STFT processing *************/
  FIXP_DBL**
      inputBufferStft; /*real, imag interleaved, N/2+1 coefficient stored in imaginary part of first
                          coefficient-> stft of length N results in N interleaved coefficients*/
  FIXP_DBL** prevInputBufferStft;
  FIXP_DBL** outputBufferStft;
  HANDLE_STFT_FILTERBANK stftFilterbankAnalysis[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNEL_GROUPS];
  HANDLE_STFT_FILTERBANK stftFilterbankSynthesis[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];

  STFT_FILTERBANK_CONFIG stftFilterbankConfigAnalysis;
  STFT_FILTERBANK_CONFIG stftFilterbankConfigSynthesis;
  UINT stftFrameSize;
  UINT stftLength;
  UINT stftNumErbBands;
  AFC_FORMAT_CONVERTER_CHANNEL_ID format_in_listOfChannels[FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS];
  int format_in_listOfChannels_nchan;
  AFC_FORMAT_CONVERTER_CHANNEL_ID
  format_out_listOfChannels[FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS];
  int format_out_listOfChannels_nchan;

  FIXP_DBL GVH[13][6];
  INT GVH_e[13][6];
  FIXP_DBL GVL[13][6];
  INT GVL_e[13][6];

  UINT is4GVH_StftErb[58];
  INT topIn[13];
  UINT midOut[6];

  UINT erb_is4GVH_L;
  UINT erb_is4GVH_H;

} IIS_FORMATCONVERTER_INTERNAL, *IIS_FORMATCONVERTER_INTERNAL_HANDLE;

#define MAX_CHANNELS 33
#define HOPSIZE 32

/* For memory allocation of temporary vectors that are used in the processing */
#define NUMTEMPVECTORS 4

typedef struct {
  FIXP_DBL* re;
  FIXP_DBL* im;
} compexVector;

#define AFC_H_EPSILON ((FIXP_DBL)1) /* Used as denumerator minimum instead of division by zero */

#endif /* _FORMATCONVERTER_INTERN_H__ */
