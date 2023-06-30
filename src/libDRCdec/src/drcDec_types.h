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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#ifndef DRCDEC_TYPES_H
#define DRCDEC_TYPES_H

#include "common_fix.h"

/* Data structures corresponding to static and dynamic DRC/Loudness payload
   as defined in section 7 of MPEG-D DRC standard, ISO/IEC 23003-4 */

/**************/
/* uniDrcGain */
/**************/

typedef struct {
  FIXP_SGL gainDb; /* e = 7 */
  SHORT time;
} GAIN_NODE;

/* uniDrcGainExtension() (Table 56) */
typedef struct {
  UCHAR uniDrcGainExtType[8];
  ULONG extBitSize[8 - 1];
} UNI_DRC_GAIN_EXTENSION;

/* uniDrcGain() (Table 55) */
typedef struct {
  UCHAR nNodes[48]; /* unsaturated value, i.e. as provided in bitstream */
  GAIN_NODE gainNode[48][32];

  UCHAR uniDrcGainExtPresent;
  UNI_DRC_GAIN_EXTENSION uniDrcGainExtension;

  /* derived data */
  UCHAR nDecodedSequences[4];
  UCHAR status;
} UNI_DRC_GAIN, *HANDLE_UNI_DRC_GAIN;

/****************/
/* uniDrcConfig */
/****************/

typedef enum {
  EB_NIGHT = 0x0001,
  EB_NOISY = 0x0002,
  EB_LIMITED = 0x0004,
  EB_LOWLEVEL = 0x0008,
  EB_DIALOG = 0x0010,
  EB_GENERAL_COMPR = 0x0020,
  EB_EXPAND = 0x0040,
  EB_ARTISTIC = 0x0080,
  EB_CLIPPING = 0x0100,
  EB_FADE = 0x0200,
  EB_DUCK_OTHER = 0x0400,
  EB_DUCK_SELF = 0x0800
} EFFECT_BIT;

typedef enum {
  GCP_REGULAR = 0,
  GCP_FADING = 1,
  GCP_CLIPPING_DUCKING = 2,
  GCP_CONSTANT = 3
} GAIN_CODING_PROFILE;

typedef enum { GIT_SPLINE = 0, GIT_LINEAR = 1 } GAIN_INTERPOLATION_TYPE;

typedef enum { CS_LEFT = 0, CS_RIGHT = 1 } CHARACTERISTIC_SIDE;

typedef enum { CF_SIGMOID = 0, CF_NODES = 1 } CHARACTERISTIC_FORMAT;

typedef enum {
  GF_QMF32 = 0x1,
  GF_QMFHYBRID39 = 0x2,
  GF_QMF64 = 0x3,
  GF_QMFHYBRID71 = 0x4,
  GF_QMF128 = 0x5,
  GF_QMFHYBRID135 = 0x6,
  GF_UNIFORM = 0x7
} EQ_SUBBAND_GAIN_FORMAT;

typedef struct {
  UCHAR duckingScalingPresent;
  FIXP_SGL duckingScaling; /* e = 2 */
} DUCKING_MODIFICATION;

typedef struct {
  UCHAR targetCharacteristicLeftPresent;
  UCHAR targetCharacteristicLeftIndex;
  UCHAR targetCharacteristicRightPresent;
  UCHAR targetCharacteristicRightIndex;
  UCHAR gainScalingPresent;
  FIXP_SGL attenuationScaling;   /* e = 2 */
  FIXP_SGL amplificationScaling; /* e = 2 */
  UCHAR gainOffsetPresent;
  FIXP_SGL gainOffset; /* e = 4 */
} GAIN_MODIFICATION;

typedef union {
  UCHAR crossoverFreqIndex;
  USHORT startSubBandIndex;
} BAND_BORDER;

typedef struct {
  UCHAR left;
  UCHAR right;
} CUSTOM_INDEX;

typedef struct {
  UCHAR present;
  UCHAR isCICP;
  union {
    UCHAR cicpIndex;
    CUSTOM_INDEX custom;
  };
} DRC_CHARACTERISTIC;

typedef struct {
  UCHAR gainCodingProfile;
  UCHAR gainInterpolationType;
  UCHAR fullFrame;
  UCHAR timeAlignment;
  UCHAR timeDeltaMinPresent;
  USHORT timeDeltaMin;
  UCHAR bandCount;
  UCHAR drcBandType;
  UCHAR gainSequenceIndex[4];
  DRC_CHARACTERISTIC drcCharacteristic[4];
  BAND_BORDER bandBorder[4];
} GAIN_SET;

/* drcCoefficientsUniDrc() (Table 67) */
typedef struct {
  UCHAR drcLocation;
  UCHAR drcFrameSizePresent;
  USHORT drcFrameSize;
  UCHAR gainSequenceCount;          /* unsaturated value, i.e. as provided in bitstream */
  UCHAR gainSetCount;               /* saturated to 48 */
  UCHAR streamGainSequenceCount[4]; /* MPEG-H: gainSequence count for each substream */
  UCHAR streamGainSetCount[4];      /* MPEG-H: gainSet count for each substream */
  GAIN_SET gainSet[48];
  /* derived data */
  UCHAR gainSetIndexForGainSequence[48];
} DRC_COEFFICIENTS_UNI_DRC;

/* drcInstructionsUniDrc() (Table 72) */
typedef struct {
  UCHAR drcInstructionsType;
  union {
    UCHAR mae_groupID;
    UCHAR mae_groupPresetID;
  };
  SCHAR drcSetId;
  UCHAR drcSetComplexityLevel;
  UCHAR drcLocation;
  UCHAR drcApplyToDownmix;
  UCHAR downmixIdCount;
  UCHAR downmixId[8];
  USHORT drcSetEffect;
  UCHAR limiterPeakTargetPresent;
  FIXP_SGL limiterPeakTarget; /* e = 5 */
  UCHAR drcSetTargetLoudnessPresent;
  SCHAR drcSetTargetLoudnessValueUpper;
  SCHAR drcSetTargetLoudnessValueLower;
  UCHAR dependsOnDrcSetPresent;
  union {
    SCHAR dependsOnDrcSet;
    UCHAR noIndependentUse;
  };
  UCHAR requiresEq;
  shouldBeUnion {
    GAIN_MODIFICATION gainModificationForChannelGroup[28][4];
    DUCKING_MODIFICATION duckingModificationForChannel[2 * 28];
  };
  SCHAR gainSetIndex[2 * 28];

  /* derived data */
  UCHAR drcChannelCount;
  UCHAR streamNDrcChannelGroups[4]; /* MPEG-H: nDrcChannelGroups for each substream */
  UCHAR nDrcChannelGroups;
  SCHAR gainSetIndexForChannelGroup[28];
} DRC_INSTRUCTIONS_UNI_DRC;

/* channelLayout() (Table 62) */
typedef struct {
  UCHAR baseChannelCount;
  UCHAR layoutSignalingPresent;
} CHANNEL_LAYOUT;

/* downmixInstructions() (Table 63) */
typedef struct {
  UCHAR downmixId;
  UCHAR targetChannelCount;
  UCHAR targetLayout;
  UCHAR downmixCoefficientsPresent;
} DOWNMIX_INSTRUCTIONS;

typedef struct {
  UCHAR uniDrcConfigExtType[8];
  ULONG extBitSize[8 - 1];
} UNI_DRC_CONFIG_EXTENSION;

/* uniDrcConfig() (Table 57) */
typedef struct {
  UCHAR downmixInstructionsCountV0;
  UCHAR downmixInstructionsCountV1;
  UCHAR downmixInstructionsCount; /* saturated to 32 */
  UCHAR drcCoefficientsUniDrcCountV0[4];
  UCHAR drcCoefficientsUniDrcCountV1;
  UCHAR drcCoefficientsUniDrcCount; /* saturated to 2 */
  UCHAR drcInstructionsUniDrcCountV0[4];
  UCHAR drcInstructionsUniDrcCountV1;
  UCHAR drcInstructionsUniDrcCount; /* saturated to (32 + 1) */
  UCHAR streamChannelCount[4];      /* MPEG-H: base channel count for each substream */
  CHANNEL_LAYOUT channelLayout;
  DOWNMIX_INSTRUCTIONS downmixInstructions[32];
  DRC_COEFFICIENTS_UNI_DRC drcCoefficientsUniDrc[2];
  DRC_INSTRUCTIONS_UNI_DRC drcInstructionsUniDrc[(32 + 1)];
  UCHAR loudnessInfoSetPresent;
  UCHAR uniDrcConfigExtPresent;
  UNI_DRC_CONFIG_EXTENSION uniDrcConfigExt;

  /* derived data */
  UCHAR drcInstructionsCountInclVirtual;
  UCHAR diff;
  UINT* p_scratch; /* pointer to temporary memory buffer */
} UNI_DRC_CONFIG, *HANDLE_UNI_DRC_CONFIG;

/*******************/
/* loudnessInfoSet */
/*******************/

typedef enum {
  MD_UNKNOWN_OTHER = 0,
  MD_PROGRAM_LOUDNESS = 1,
  MD_ANCHOR_LOUDNESS = 2,
  MD_MAX_OF_LOUDNESS_RANGE = 3,
  MD_MOMENTARY_LOUDNESS_MAX = 4,
  MD_SHORT_TERM_LOUDNESS_MAX = 5,
  MD_LOUDNESS_RANGE = 6,
  MD_MIXING_LEVEL = 7,
  MD_ROOM_TYPE = 8,
  MD_SHORT_TERM_LOUDNESS = 9
} METHOD_DEFINITION;

typedef enum {
  MS_UNKNOWN_OTHER = 0,
  MS_EBU_R_128 = 1,
  MS_BS_1770_4 = 2,
  MS_BS_1770_4_PRE_PROCESSING = 3,
  MS_USER = 4,
  MS_EXPERT_PANEL = 5,
  MS_BS_1771_1 = 6,
  MS_RESERVED_A = 7,
  MS_RESERVED_B = 8,
  MS_RESERVED_C = 9,
  MS_RESERVED_D = 10,
  MS_RESERVED_E = 11
} MEASUREMENT_SYSTEM;

typedef enum { R_UKNOWN = 0, R_UNVERIFIED = 1, R_CEILING = 2, R_ACCURATE = 3 } RELIABILITY;

typedef struct {
  UCHAR methodDefinition;
  FIXP_DBL methodValue; /* e = 7 for all methodDefinitions */
  UCHAR measurementSystem;
  UCHAR reliability;
} LOUDNESS_MEASUREMENT;

/* loudnessInfo() (Table 59) */
typedef struct {
  UCHAR loudnessInfoType;
  union {
    UCHAR mae_groupID;
    UCHAR mae_groupPresetID;
  };
  SCHAR drcSetId;
  UCHAR downmixId;
  UCHAR samplePeakLevelPresent;
  FIXP_DBL samplePeakLevel; /* e = 7 */
  UCHAR truePeakLevelPresent;
  FIXP_DBL truePeakLevel; /* e = 7 */
  UCHAR truePeakLevelMeasurementSystem;
  UCHAR truePeakLevelReliability;
  UCHAR measurementCount; /* saturated to 16 */
  LOUDNESS_MEASUREMENT loudnessMeasurement[16];
} LOUDNESS_INFO;

/* loudnessInfoSetExtension() (Table 61) */
typedef struct {
  UCHAR loudnessInfoSetExtType[8];
  ULONG extBitSize[8 - 1];
} LOUDNESS_INFO_SET_EXTENSION;

/* loudnessInfoSet() (Table 58) */
typedef struct {
  UCHAR loudnessInfoAlbumCountV0[4];
  UCHAR loudnessInfoAlbumCountV1;
  UCHAR loudnessInfoAlbumCount; /* saturated to 32 */
  UCHAR loudnessInfoCountV0[4];
  UCHAR loudnessInfoCountV1;
  UCHAR loudnessInfoCount; /* saturated to 32 */
  LOUDNESS_INFO loudnessInfoAlbum[32];
  LOUDNESS_INFO loudnessInfo[32];
  UCHAR loudnessInfoSetExtPresent;
  LOUDNESS_INFO_SET_EXTENSION loudnessInfoSetExt;
  /* derived data */
  UCHAR diff;
} LOUDNESS_INFO_SET, *HANDLE_LOUDNESS_INFO_SET;

#endif
