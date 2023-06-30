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

   Author(s):   Manuel Jander

   Description:

*******************************************************************************/

/** \file   FDK_audio.h
 *  \brief  Global audio struct and constant definitions.
 */

#ifndef FDK_AUDIO_H
#define FDK_AUDIO_H

#include "machine_type.h"
#include "genericStds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transport type identifiers.
 */
typedef enum {
  TT_UNKNOWN = -1, /**< Unknown format.            */
  TT_MP4_RAW = 0, /**< "as is" access units (packet based since there is obviously no sync layer) */
  TT_MP4_ADIF = 1, /**< ADIF bitstream format.     */
  TT_MP4_ADTS = 2, /**< ADTS bitstream format.     */

  TT_MP4_LATM_MCP1 = 6, /**< Audio Mux Elements with muxConfigPresent = 1 */
  TT_MP4_LATM_MCP0 =
      7, /**< Audio Mux Elements with muxConfigPresent = 0, out of band StreamMuxConfig */

  TT_MP4_LOAS = 10, /**< Audio Sync Stream.         */

  TT_DRM = 12, /**< Digital Radio Mondial (DRM30/DRM+) bitstream format. */

  TT_MHAS = 20,            /**< MPEG-H 3D Audio Stream. */
  TT_MHAS_PACKETIZED = 24, /**< MPEG-H 3D Audio Packets. */

  TT_MHA_RAW = 60 /**< MPEGH 3D Audio */

} TRANSPORT_TYPE;

#define TT_IS_PACKET(x)                                                   \
  (((x) == TT_MP4_RAW) || ((x) == TT_DRM) || ((x) == TT_MP4_LATM_MCP0) || \
   ((x) == TT_MP4_LATM_MCP1) || ((x) == TT_MHA_RAW) || ((x) == TT_MHAS_PACKETIZED))

#define TT_CFG_IS_INBAND(x)                                                     \
  (((x) == TT_MP4_ADTS) || ((x) == TT_MP4_ADIF) || ((x) == TT_MP4_LATM_MCP1) || \
   ((x) == TT_MHAS) || ((x) == TT_MHAS_PACKETIZED))

#define TT_NEEDS_BS_COPY(x) (((x) == TT_MHA_RAW) || ((x) == TT_MHAS) || ((x) == TT_MHAS_PACKETIZED))

#define TT_IS_MPEGH(x) (((x) == TT_MHAS) || ((x) == TT_MHAS_PACKETIZED) || ((x) == TT_MHA_RAW))

/**
 * Audio Object Type definitions.
 */
typedef enum {
  AOT_NONE = -1,
  AOT_NULL_OBJECT = 0,
  AOT_AAC_MAIN = 1, /**< Main profile                              */
  AOT_AAC_LC = 2,   /**< Low Complexity object                     */
  AOT_AAC_SSR = 3,
  AOT_AAC_LTP = 4,
  AOT_SBR = 5,
  AOT_AAC_SCAL = 6,
  AOT_TWIN_VQ = 7,
  AOT_CELP = 8,
  AOT_HVXC = 9,
  AOT_RSVD_10 = 10,          /**< (reserved)                                */
  AOT_RSVD_11 = 11,          /**< (reserved)                                */
  AOT_TTSI = 12,             /**< TTSI Object                               */
  AOT_MAIN_SYNTH = 13,       /**< Main Synthetic object                     */
  AOT_WAV_TAB_SYNTH = 14,    /**< Wavetable Synthesis object                */
  AOT_GEN_MIDI = 15,         /**< General MIDI object                       */
  AOT_ALG_SYNTH_AUD_FX = 16, /**< Algorithmic Synthesis and Audio FX object */
  AOT_ER_AAC_LC = 17,        /**< Error Resilient(ER) AAC Low Complexity    */
  AOT_RSVD_18 = 18,          /**< (reserved)                                */
  AOT_ER_AAC_LTP = 19,       /**< Error Resilient(ER) AAC LTP object        */
  AOT_ER_AAC_SCAL = 20,      /**< Error Resilient(ER) AAC Scalable object   */
  AOT_ER_TWIN_VQ = 21,       /**< Error Resilient(ER) TwinVQ object         */
  AOT_ER_BSAC = 22,          /**< Error Resilient(ER) BSAC object           */
  AOT_ER_AAC_LD = 23,        /**< Error Resilient(ER) AAC LowDelay object   */
  AOT_ER_CELP = 24,          /**< Error Resilient(ER) CELP object           */
  AOT_ER_HVXC = 25,          /**< Error Resilient(ER) HVXC object           */
  AOT_ER_HILN = 26,          /**< Error Resilient(ER) HILN object           */
  AOT_ER_PARA = 27,          /**< Error Resilient(ER) Parametric object     */
  AOT_RSVD_28 = 28,          /**< might become SSC                          */
  AOT_PS = 29,               /**< PS, Parametric Stereo (includes SBR)      */
  AOT_MPEGS = 30,            /**< MPEG Surround                             */

  AOT_ESCAPE = 31, /**< Signal AOT uses more than 5 bits          */

  AOT_MP3ONMP4_L1 = 32, /**< MPEG-Layer1 in mp4                        */
  AOT_MP3ONMP4_L2 = 33, /**< MPEG-Layer2 in mp4                        */
  AOT_MP3ONMP4_L3 = 34, /**< MPEG-Layer3 in mp4                        */
  AOT_RSVD_35 = 35,     /**< might become DST                          */
  AOT_RSVD_36 = 36,     /**< might become ALS                          */
  AOT_AAC_SLS = 37,     /**< AAC + SLS                                 */
  AOT_SLS = 38,         /**< SLS                                       */
  AOT_ER_AAC_ELD = 39,  /**< AAC Enhanced Low Delay                    */

  AOT_USAC = 42,     /**< USAC                                      */
  AOT_SAOC = 43,     /**< SAOC                                      */
  AOT_LD_MPEGS = 44, /**< Low Delay MPEG Surround                   */

  /* Pseudo AOTs */
  AOT_MP2_AAC_LC = 129, /**< Virtual AOT MP2 Low Complexity profile                 */
  AOT_MP2_SBR = 132,    /**< Virtual AOT MP2 Low Complexity Profile with SBR        */

  AOT_DRM_AAC = 143,      /**< Virtual AOT for DRM (ER-AAC-SCAL without SBR)          */
  AOT_DRM_SBR = 144,      /**< Virtual AOT for DRM (ER-AAC-SCAL with SBR)             */
  AOT_DRM_MPEG_PS = 145,  /**< Virtual AOT for DRM (ER-AAC-SCAL with SBR and MPEG-PS) */
  AOT_DRM_SURROUND = 146, /**< Virtual AOT for DRM Surround (ER-AAC-SCAL (+SBR) +MPS) */
  AOT_DRM_USAC = 147,     /**< Virtual AOT for DRM with USAC                          */

  AOT_MPEGH3DA = 150, /**< Virtual AOT for MPEG-H 3D audio                        */

  AOT_MPEGD_RESIDUALS = 256 /**< Virtual AOT for MPEG-D residuals                       */

} AUDIO_OBJECT_TYPE;

#define CAN_DO_PS(aot)                                                                   \
  ((aot) == AOT_AAC_LC || (aot) == AOT_SBR || (aot) == AOT_PS || (aot) == AOT_ER_BSAC || \
   (aot) == AOT_DRM_AAC)

#define IS_USAC(aot) ((aot) == AOT_USAC || (aot) == AOT_RSVD50 || (aot) == AOT_MPEGH3DA)

#define IS_LOWDELAY(aot) ((aot) == AOT_ER_AAC_LD || (aot) == AOT_ER_AAC_ELD)

/** Represents the order of the channels. */
typedef enum {
  CH_ORDER_MPEG = 0, /* ISO/IEC 14496-3 */
  CH_ORDER_WAV = 1,  /* RIFF WAV fmt */
  CH_ORDER_CICP = 2  /* ISO/IEC 23001-8 */

} CHANNEL_ORDER;

/** Channel Mode ( 1-7 equals MPEG channel configurations, others are arbitrary). */
typedef enum {
  MODE_INVALID = -1,
  MODE_UNKNOWN = 0,
  MODE_1 = 1,         /**< C */
  MODE_2 = 2,         /**< L+R */
  MODE_1_2 = 3,       /**< C, L+R */
  MODE_1_2_1 = 4,     /**< C, L+R, Rear */
  MODE_1_2_2 = 5,     /**< C, L+R, LS+RS */
  MODE_1_2_2_1 = 6,   /**< C, L+R, LS+RS, LFE */
  MODE_1_2_2_2_1 = 7, /**< C, LC+RC, L+R, LS+RS, LFE */

  MODE_6_1 = 11,           /**< C, L+R, LS+RS, Crear, LFE */
  MODE_7_1_BACK = 12,      /**< C, L+R, LS+RS, Lrear+Rrear, LFE */
  MODE_22_2 = 13,          /**< */
  MODE_7_1_TOP_FRONT = 14, /**< C, L+R, LS+RS, LFE, Ltop+Rtop */

  MODE_1_1 = 16,                     /**< 2 SCEs (dual channel) */
  MODE_1_1_1_1 = 17,                 /**< 4 SCEs */
  MODE_1_1_1_1_1_1 = 18,             /**< 6 SCEs */
  MODE_1_1_1_1_1_1_1_1 = 19,         /**< 8 SCEs */
  MODE_1_1_1_1_1_1_1_1_1_1_1_1 = 20, /**< 12 SCEs */

  MODE_2_2 = 21,         /**< 2 CPEs */
  MODE_2_2_2 = 22,       /**< 3 CPEs */
  MODE_2_2_2_2 = 23,     /**< 4 CPEs */
  MODE_2_2_2_2_2_2 = 24, /**< 6 CPEs */

  MODE_2_1 = 30,               /**< CPE,SCE (ARIB standard B32) */
  MODE_7_1_REAR_SURROUND = 33, /**< C, L+R, LS+RS, Lrear+Rrear, LFE */
  MODE_7_1_FRONT_CENTER = 34,  /**< C, LC+RC, L+R, LS+RS, LFE */

  MODE_212 = 128 /**< 212 configuration, used in ELDv2 */

} CHANNEL_MODE;

/**
 * Speaker description tags.
 * Do not change the enumeration values unless it keeps the following segmentation:
 * - Bit 0-3: Horizontal postion (0: none, 1: front, 2: side, 3: back, 4: lfe)
 * - Bit 4-7: Vertical position (0: normal, 1: top, 2: bottom)
 */
typedef enum {
  ACT_NONE = 0x00,
  ACT_FRONT = 0x01, /*!< Front speaker position (at normal height) */
  ACT_SIDE = 0x02,  /*!< Side speaker position (at normal height) */
  ACT_BACK = 0x03,  /*!< Back speaker position (at normal height) */
  ACT_LFE = 0x04,   /*!< Low frequency effect speaker postion (front) */

  ACT_TOP = 0x10,       /*!< Top speaker area (for combination with speaker positions) */
  ACT_FRONT_TOP = 0x11, /*!< Top front speaker = (ACT_FRONT|ACT_TOP) */
  ACT_SIDE_TOP = 0x12,  /*!< Top side speaker  = (ACT_SIDE |ACT_TOP) */
  ACT_BACK_TOP = 0x13,  /*!< Top back speaker  = (ACT_BACK |ACT_TOP) */

  ACT_BOTTOM = 0x20,       /*!< Bottom speaker area (for combination with speaker positions) */
  ACT_FRONT_BOTTOM = 0x21, /*!< Bottom front speaker = (ACT_FRONT|ACT_BOTTOM) */
  ACT_SIDE_BOTTOM = 0x22,  /*!< Bottom side speaker  = (ACT_SIDE |ACT_BOTTOM) */
  ACT_BACK_BOTTOM = 0x23   /*!< Bottom back speaker  = (ACT_BACK |ACT_BOTTOM) */

} AUDIO_CHANNEL_TYPE;

typedef enum {
  SIG_UNKNOWN = -1,
  SIG_IMPLICIT = 0,
  SIG_EXPLICIT_BW_COMPATIBLE = 1,
  SIG_EXPLICIT_HIERARCHICAL = 2

} SBR_PS_SIGNALING;

/**
 * Audio Codec flags.
 */
#define AC_ER_VCB11 \
  0x000001 /*!< aacSectionDataResilienceFlag     flag (from ASC): 1 means use virtual codebooks */
#define AC_ER_RVLC                                                                              \
  0x000002 /*!< aacSpectralDataResilienceFlag     flag (from ASC): 1 means use huffman codeword \
              reordering */
#define AC_ER_HCR \
  0x000004 /*!< aacSectionDataResilienceFlag     flag (from ASC): 1 means use virtual codebooks */
#define AC_SCALABLE 0x000008       /*!< AAC Scalable*/
#define AC_ELD 0x000010            /*!< AAC-ELD */
#define AC_LD 0x000020             /*!< AAC-LD */
#define AC_ER 0x000040             /*!< ER syntax */
#define AC_BSAC 0x000080           /*!< BSAC */
#define AC_USAC 0x000100           /*!< USAC */
#define AC_MPEGH3DA 0x000200       /*!< MPEG-H 3D audio */
#define AC_HDAAC 0x000400          /*!< HD-AAC */
#define AC_RSVD50 0x004000         /*!< Rsvd50 */
#define AC_SBR_PRESENT 0x008000    /*!< SBR present flag (from ASC)             */
#define AC_SBRCRC 0x010000         /*!< SBR CRC present flag. Only relevant for AAC-ELD for now. */
#define AC_PS_PRESENT 0x020000     /*!< PS present flag (from ASC or implicit)  */
#define AC_MPS_PRESENT 0x040000    /*!< MPS present flag (from ASC or implicit) */
#define AC_DRM 0x080000            /*!< DRM bit stream syntax */
#define AC_INDEP 0x100000          /*!< Independency flag */
#define AC_MPEGD_RES 0x200000      /*!< MPEG-D residual individual channel data. */
#define AC_SAOC_PRESENT 0x400000   /*!< SAOC Present Flag */
#define AC_DAB 0x800000            /*!< DAB bit stream syntax */
#define AC_ELD_DOWNSCALE 0x1000000 /*!< ELD Downscaled playout */
#define AC_LD_MPS 0x2000000        /*!< Low Delay MPS. */
#define AC_DRC_PRESENT 0x4000000   /*!< Dynamic Range Control (DRC) data found. */
#define AC_USAC_SCFGI3 0x8000000   /*!< USAC flag: If stereoConfigIndex is 3 the flag is set. */
/**
 * Audio Codec flags (reconfiguration).
 */
#define AC_CM_DET_CFG_CHANGE \
  0x000001 /*!< Config mode signalizes the callback to work in config change detection mode */
#define AC_CM_ALLOC_MEM \
  0x000002 /*!< Config mode signalizes the callback to work in memory allocation mode */

/**
 * Audio Codec flags (element specific).
 */
#define AC_EL_GA_CCE 0x000001      /*!< GA AAC coupling channel element (CCE) */
#define AC_EL_USAC_NOISE 0x000002  /*!< USAC noise filling is active */
#define AC_EL_USAC_ITES 0x000004   /*!< USAC SBR inter-TES tool is active */
#define AC_EL_USAC_PVC 0x000008    /*!< USAC SBR predictive vector coding tool is active */
#define AC_EL_USAC_MPS212 0x000010 /*!< USAC MPS212 tool is active */
#define AC_EL_USAC_LFE 0x000020    /*!< USAC element is LFE */
#define AC_EL_USAC_CP_POSSIBLE \
  0x000040 /*!< USAC may use Complex Stereo Prediction in this channel element */
#define AC_EL_ENHANCED_NOISE 0x000080   /*!< Enhanced noise filling*/
#define AC_EL_IGF_AFTER_TNS 0x000100    /*!< IGF after TNS */
#define AC_EL_IGF_INDEP_TILING 0x000200 /*!< IGF independent tiling */
#define AC_EL_IGF_USE_ENF 0x000400      /*!< IGF use enhanced noise filling */
#define AC_EL_FULLBANDLPD 0x000800      /*!< enable fullband LPD tools */
#define AC_EL_LPDSTEREOIDX 0x001000     /*!< LPD-stereo-tool stereo index */
#define AC_EL_LFE 0x002000              /*!< The element is of type LFE. */

/* CODER_CONFIG::flags */
#define CC_MPEG_ID 0x00100000
#define CC_IS_BASELAYER 0x00200000
#define CC_PROTECTION 0x00400000
#define CC_SBR 0x00800000
#define CC_SBRCRC 0x00010000
#define CC_SAC 0x00020000
#define CC_RVLC 0x01000000
#define CC_VCB11 0x02000000
#define CC_HCR 0x04000000
#define CC_PSEUDO_SURROUND 0x08000000
#define CC_USAC_NOISE 0x10000000
#define CC_USAC_TW 0x20000000
#define CC_USAC_HBE 0x40000000

/** Generic audio coder configuration structure. */
typedef struct {
  AUDIO_OBJECT_TYPE aot;     /**< Audio Object Type (AOT).           */
  AUDIO_OBJECT_TYPE extAOT;  /**< Extension Audio Object Type (SBR). */
  CHANNEL_MODE channelMode;  /**< Channel mode.                      */
  UCHAR channelConfigZero;   /**< Use channel config zero + pce although a standard channel config
                                could be signaled. */
  INT samplingRate;          /**< Sampling rate.                     */
  INT extSamplingRate;       /**< Extended samplerate (SBR).         */
  INT downscaleSamplingRate; /**< Downscale sampling rate (ELD downscaled mode)  */
  INT bitRate;               /**< Average bitrate.                   */
  int samplesPerFrame;       /**< Number of PCM samples per codec frame and audio channel. */
  int noChannels;            /**< Number of audio channels.          */
  int bitsFrame;
  int nSubFrames;        /**< Amount of encoder subframes. 1 means no subframing.        */
  int BSACnumOfSubFrame; /**< The number of the sub-frames which are grouped and transmitted in a
                            super-frame (BSAC). */
  int BSAClayerLength;   /**< The average length of the large-step layers in bytes (BSAC).   */
  UINT flags;            /**< flags */
  UCHAR matrixMixdownA;  /**< Matrix mixdown index to put into PCE. Default value 0 means no mixdown
                            coefficient,  valid values are 1-4 which correspond to matrix_mixdown_idx
                            0-3. */
  UCHAR headerPeriod; /**< Frame period for sending in band configuration buffers in the transport
                         layer. */

  UCHAR stereoConfigIndex;       /**< USAC MPS stereo mode */
  UCHAR sbrMode;                 /**< USAC SBR mode */
  SBR_PS_SIGNALING sbrSignaling; /**< 0: implicit signaling, 1: backwards compatible explicit
                                    signaling, 2: hierarcical explicit signaling */

  UCHAR rawConfig[512];
  /**< raw codec specific config as bit stream */ /* size: TP_USAC_MAX_CONFIG_LEN */
  int rawConfigBits;                              /**< Size of rawConfig in bits */

  UCHAR sbrPresent;
  UCHAR psPresent;
} CODER_CONFIG;

#define USAC_ID_BIT 16 /** USAC element IDs start at USAC_ID_BIT */

/** MP4 Element IDs. */
typedef enum {
  /* mp4 element IDs */
  ID_NONE = -1, /**< Invalid Element helper ID.             */
  ID_SCE = 0,   /**< Single Channel Element.                */
  ID_CPE = 1,   /**< Channel Pair Element.                  */
  ID_CCE = 2,   /**< Coupling Channel Element.              */
  ID_LFE = 3,   /**< LFE Channel Element.                   */
  ID_DSE = 4,   /**< Currently one Data Stream Element for ancillary data is supported. */
  ID_PCE = 5,   /**< Program Config Element.                */
  ID_FIL = 6,   /**< Fill Element.                          */
  ID_END = 7,   /**< Arnie (End Element = Terminator).      */
  ID_EXT = 8,   /**< Extension Payload (ER only).           */
  ID_SCAL = 9,  /**< AAC scalable element (ER only).        */
  /* USAC element IDs */
  ID_USAC_SCE = 0 + USAC_ID_BIT, /**< Single Channel Element.                */
  ID_USAC_CPE = 1 + USAC_ID_BIT, /**< Channel Pair Element.                  */
  ID_USAC_LFE = 2 + USAC_ID_BIT, /**< LFE Channel Element.                   */
  ID_USAC_EXT = 3 + USAC_ID_BIT, /**< Extension Element.                     */
  ID_USAC_END = 4 + USAC_ID_BIT, /**< Arnie (End Element = Terminator).      */
  ID_LAST
} MP4_ELEMENT_ID;

/* usacConfigExtType q.v. ISO/IEC DIS 23008-3 Table 52  and  ISO/IEC FDIS 23003-3:2011(E) Table 74*/
typedef enum {
  /* USAC and MPEG-H 3DA */
  ID_CONFIG_EXT_FILL = 0,
  /* MPEG-H 3DA */
  ID_CONFIG_EXT_DOWNMIX = 1,
  ID_CONFIG_EXT_LOUDNESS_INFO = 2,
  ID_CONFIG_EXT_AUDIOSCENE_INFO = 3,
  ID_CONFIG_EXT_HOA_MATRIX = 4,
  ID_CONFIG_EXT_SIG_GROUP_INFO = 6,
  ID_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET = 7
  /* 8-127 => reserved for ISO use */
  /* > 128 => reserved for use outside of ISO scope */
} CONFIG_EXT_ID;

#define IS_CHANNEL_ELEMENT(elementId)                                         \
  ((elementId) == ID_SCE || (elementId) == ID_CPE || (elementId) == ID_LFE || \
   (elementId) == ID_USAC_SCE || (elementId) == ID_USAC_CPE || (elementId) == ID_USAC_LFE)

#define IS_MP4_CHANNEL_ELEMENT(elementId) \
  ((elementId) == ID_SCE || (elementId) == ID_CPE || (elementId) == ID_LFE)

#define EXT_ID_BITS 4 /**< Size in bits of extension payload type tags. */

/** Extension payload types. */
typedef enum {
  EXT_FIL = 0x00,
  EXT_FILL_DATA = 0x01,
  EXT_DATA_ELEMENT = 0x02,
  EXT_DATA_LENGTH = 0x03,
  EXT_UNI_DRC = 0x04,
  EXT_LDSAC_DATA = 0x09,
  EXT_SAOC_DATA = 0x0a,
  EXT_DYNAMIC_RANGE = 0x0b,
  EXT_SAC_DATA = 0x0c,
  EXT_SBR_DATA = 0x0d,
  EXT_SBR_DATA_CRC = 0x0e
} EXT_PAYLOAD_TYPE;

#define IS_USAC_CHANNEL_ELEMENT(elementId) \
  ((elementId) == ID_USAC_SCE || (elementId) == ID_USAC_CPE || (elementId) == ID_USAC_LFE)

/** MPEG-D USAC & MPEG-H 3D audio Extension Element Types. */
typedef enum {
  /* usac */
  ID_EXT_ELE_FILL = 0x00,
  ID_EXT_ELE_MPEGS = 0x01,
  ID_EXT_ELE_SAOC = 0x02,
  ID_EXT_ELE_AUDIOPREROLL = 0x03,
  ID_EXT_ELE_UNI_DRC = 0x04,
  /* mpegh3da */
  ID_EXT_ELE_OBJ_METADATA = 0x05,
  ID_EXT_ELE_SAOC_3D = 0x06,
  ID_EXT_ELE_HOA = 0x07,
  ID_EXT_ELE_FMT_CNVRTR = 0x08,
  ID_EXT_ELE_MCT = 0x09,
  ID_EXT_ELE_ENHANCED_OBJ_METADATA = 0x0d,
  ID_EXT_ELE_PROD_METADATA = 0x0e,
  /* reserved for use outside of ISO scope */
  ID_EXT_ELE_VR_METADATA = 0x81,
  ID_EXT_ELE_UNKNOWN = 0xFF
} USAC_EXT_ELEMENT_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* FDK_AUDIO_H */
