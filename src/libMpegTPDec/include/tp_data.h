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

/******************* MPEG transport format decoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport data tables

*******************************************************************************/

#ifndef TP_DATA_H
#define TP_DATA_H

#include "machine_type.h"
#include "FDK_audio.h"
#include "FDK_bitstream.h"

/*
 * Configuration
 */

#define TPDEC_MAX_PROGRAMS 1
#define TPDEC_MAX_LAYERS 1
#define TPDEC_MAX_TRACKS (TPDEC_MAX_PROGRAMS * TPDEC_MAX_LAYERS)

#define TP_MPEGH3DA_MAX_LEVEL (5)

#define TP_USAC_MAX_SPEAKERS (28)

#define TP_MAX_CHANNELS_PER_SIGNAL_GROUP (28)

/*  X OAM side-info + X extOAM + X/2 MCT + N preroll + N fill element + N DRC + N VR-EXT */
#define TP_USAC_MAX_EXT_ELEMENTS ((2 * (2 * 28) + ((2 * 28)) / 2 + 4 * 1))

#define TP_USAC_MAX_ELEMENTS ((2 * 28) + TP_USAC_MAX_EXT_ELEMENTS)

#define TP_MPEGH3DA_MAX_CONFIG_LEN 4096
#define TP_USAC_MAX_CONFIG_LEN \
  512 /* next power of two of maximum of escapedValue(hBs, 4, 4, 8) in AudioPreRoll() (285) */

#define TP_MPEGH_MAX_PROD_METADATA_CONFIG_LEN 5

#define TP_MPEGH_MAX_SIGNAL_GROUPS (2 * 28)

typedef enum {
  MHA_PACTYP_NONE = -1,
  MHA_PACTYP_FILLDATA = 0,
  MHA_PACTYP_MPEGH3DACFG = 1,
  MHA_PACTYP_MPEGH3DAFRAME = 2,
  MHA_PACTYP_AUDIOSCENEINFO = 3,
  /* reserved for ISO use 4-5 */
  MHA_PACTYP_SYNC = 6,
  MHA_PACTYP_SYNCGAP = 7,
  MHA_PACTYP_MARKER = 8,
  MHA_PACTYP_CRC16 = 9,
  MHA_PACTYP_CRC32 = 10,
  MHA_PACTYP_DESCRIPTOR = 11,
  MHA_PACTYP_USERINTERACTION = 12,
  MHA_PACTYP_LOUDNESS_DRC = 13,
  MHA_PACTYP_BUFFERINFO = 14,
  MHA_PACTYP_GLOBAL_CRC16 = 15,    /* MPEG112 input */
  MHA_PACTYP_GLOBAL_CRC32 = 16,    /* MPEG112 input */
  MHA_PACTYP_AUDIOTRUNCATION = 17, /* MPEG112 input */
  MHA_PACTYPE_EARCON = 19,         /* ISO/IEC 23008-3:2015/FDAM 5:201x Amd 1*/
  MHA_PACTYPE_PCMCONFIG = 20,      /* ISO/IEC 23008-3:2015/FDAM 5:201x Amd 1*/
  MHA_PACTYPE_PCMDATA = 21,        /* ISO/IEC 23008-3:2015/FDAM 5:201x Amd 1*/
  MHA_PACTYP_LOUDNESS = 22,        /* 23008-3:2019/Amd 1 (Amendment 1 to 2nd edition of MPEG-H) */
  MHA_PACTYP_FRAMELENGTH = 129,
  MHA_PACTYP_UNKNOWN = 518 /* max(escapedValue(3,8,8)) + 1 */
} mha_pactyp_t;

#define MHA_IS_MAIN_STREAM(x) (0 < (x) && (x) <= 16)

#define TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES (1) /* Number of frames for config change in ATSC */
#define TPDEC_USAC_NUM_CONFIG_CHANGE_FRAMES (1)  /* Number of frames for config change in USAC */

enum {
  TPDEC_FLUSH_OFF = 0,
  TPDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON = 1,
  TPDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON = 2,
  TPDEC_USAC_DASH_IPF_FLUSH_ON = 3
};

enum {
  TPDEC_BUILD_UP_OFF = 0,
  TPDEC_MPEGH_BUILD_UP_ON = 1,
  TPDEC_MPEGH_BUILD_UP_ON_IN_BAND = 2,
  TPDEC_USAC_BUILD_UP_ON = 3,
  TPDEC_MPEGH_BUILD_UP_IDLE = 4,
  TPDEC_MPEGH_BUILD_UP_IDLE_IN_BAND = 5
};

/**
 * ProgramConfig struct.
 */
/* ISO/IEC 14496-3 4.4.1.1 Table 4.2 Program config element */
#define PC_FSB_CHANNELS_MAX 16 /* Front/Side/Back channels */
#define PC_LFE_CHANNELS_MAX 4
#define PC_ASSOCDATA_MAX 8
#define PC_CCEL_MAX 16 /* CC elements */
#define PC_COMMENTLENGTH 256
#define PC_NUM_HEIGHT_LAYER 3

typedef struct {
  /* Helper variables for administration: */
  UCHAR isValid; /*!< States whether PCE has been read successfully. (0: invalid, 1: valid, 2: valid
                    but implicit for trial) */
  UCHAR NumChannels; /*!< Amount of audio channels summing all channel elements including LFEs */
  UCHAR NumEffectiveChannels; /*!< Amount of audio channels summing only SCEs and CPEs */
  UCHAR elCounter;

} CProgramConfig;

typedef enum {
  ASCEXT_UNKOWN = -1,
  ASCEXT_SBR = 0x2b7,
  ASCEXT_PS = 0x548,
  ASCEXT_MPS = 0x76a,
  ASCEXT_SAOC = 0x7cb,
  ASCEXT_LDMPS = 0x7cc

} TP_ASC_EXTENSION_ID;

struct OAMCONFIG {
  USHORT OAMframeLength;
  UCHAR lowDelayMetadataCoding;
  UCHAR hasDynamicObjectPriority;
  UCHAR hasUniformSpread;
  UCHAR numObjectSignals;
};

typedef struct {
  USAC_EXT_ELEMENT_TYPE usacExtElementType;
  USHORT usacExtElementDefaultLength;
  UCHAR usacExtElementPayloadFrag;
  UCHAR usacExtElementHasAudioPreRoll;
  union {
    struct OAMCONFIG oam;
    struct {
      ULONG hasObjectDistance; /* msb aligned bit field: one bit for each object group */
      UCHAR hasReferenceDistance;
      UCHAR bsReferenceDistance;
    } prodMetadata;

    struct {
      ULONG mctChanMask; /* msb aligned bit field: one bit for each channel */
    } mct;
  } extConfig;
} CSUsacExtElementConfig;

typedef struct {
  MP4_ELEMENT_ID usacElementType;
  UCHAR fullbandLpd;
  UCHAR lpdStereoIndex;
  UCHAR m_noiseFilling;
  /* enhanced noise filling / intelligent gap filling */
  UCHAR enhancedNoiseFilling;
  UCHAR igfUseEnf;
  UCHAR igfUseWhitening;
  UCHAR igfAfterTnsSynth;
  UCHAR igfIndependentTiling;
  UCHAR igfStartIndex;
  UCHAR igfUseHighRes;
  UCHAR igfStopIndex;
  UCHAR igfNTiles;
  UCHAR shiftIndex1;
  UCHAR shiftChannel1;
  CSUsacExtElementConfig extElement;
} CSUsacElementConfig;

typedef struct {
  UCHAR Lfe; /* Flag indicating if the speaker is LFE. */
  SHORT Az;  /* Azimuth angle in degrees quantized in steps of 1 degree. Positive is counter clock
                wise. */
  SHORT El;  /* Elevation angle in degrees quantized in steps of 1 degree. Positive is up. */
} mpegh_speaker_t;

typedef struct {
  /* general configuration data */
  UCHAR type;  /* Type 0: Channel signals / 1:  Object signals / 2: SAOC signals / 3: HOA signals /
                  Other: USAC */
  UCHAR count; /* Number of signals related to this signal group */
  UCHAR Layout;
  mpegh_speaker_t* speakers;
  UCHAR firstSigIdx;             /* index of first signal in group */
  UCHAR bUseCustomDownmixMatrix; /* flag indicating that a downmix matrix delivered by the bit
                                    stream should be used. */
} CSSignalGroup;

typedef struct {
  UCHAR m_frameLengthFlag;
  UCHAR m_coreSbrFrameLengthIndex;
  UCHAR m_sbrRatioIndex;
  UCHAR m_nUsacChannels; /* number of audio channels signaled in UsacDecoderConfig() /
                            mpegh3daDecoderConfig() via numElements and usacElementType */
  UCHAR m_channelConfigurationIndex;
  UINT m_usacNumElements;
  CSUsacElementConfig element[TP_USAC_MAX_ELEMENTS];

  /* general configuration data */
  CSSignalGroup m_signalGroupType[TP_MPEGH_MAX_SIGNAL_GROUPS]; /**< Signal group type.  */
  mpegh_speaker_t m_signalGroupTypeSpeakers[(2 * 28)];
  UCHAR referenceLayout;
  mpegh_speaker_t speakers[TP_USAC_MAX_SPEAKERS];
  UCHAR bsNumSignalGroups;
  UCHAR mpegh3daProfileLevelIndication;
  UCHAR numAudioObjects;
  UCHAR numHOATransportChannels;
  UCHAR chNumSignalGroups;  /* count signal groups of type channels */
  UCHAR objNumSignalGroups; /* count singal groups of type object */
  UCHAR
  objSignalGroupsIndices[TP_MPEGH_MAX_SIGNAL_GROUPS]; /* store indice of current signal group */
  UCHAR sigIdx2GrpIdx[TP_MPEGH_MAX_SIGNAL_GROUPS];    /* signal index to group index table */
  /* downmixConfig() parameters */
  UCHAR passiveDownmixFlag;
  UCHAR phaseAlignStrength;
  UCHAR immersiveDownmixFlag;
  UCHAR downmixConfigType;
  UCHAR downmixIdCount;
  UCHAR subStreamIndex;
  INT targetLayout;
  UCHAR uiManagerActive;
  UCHAR numAudioChannels;
  UCHAR m_usacConfigExtensionPresent;
  UCHAR elementLengthPresent;
} CSUsacConfig;

/**
 * Audio configuration struct, suitable for encoder and decoder configuration.
 */
typedef struct {
  /* XYZ Specific Data */
  union {
    CSUsacConfig m_usacConfig; /**< USAC specific configuration                   */
  } m_sc;

  /* Common ASC parameters */

  AUDIO_OBJECT_TYPE m_aot;  /**< Audio Object Type.                              */
  UINT m_samplingFrequency; /**< Samplerate.                                     */
  UINT m_samplesPerFrame;   /**< Amount of samples per frame.                    */
  UINT m_directMapping;     /**< Document this please !!                         */

  AUDIO_OBJECT_TYPE m_extensionAudioObjectType; /**< Audio object type                      */
  UINT m_extensionSamplingFrequency;            /**< Samplerate                             */

  SCHAR m_channelConfiguration; /**< Channel configuration index                     */

  SCHAR m_epConfig;  /**< Error protection index                           */
  SCHAR m_vcb11Flag; /**< aacSectionDataResilienceFlag                     */
  SCHAR m_rvlcFlag;  /**< aacScalefactorDataResilienceFlag                 */
  SCHAR m_hcrFlag;   /**< aacSpectralDataResilienceFlag                    */

  SCHAR m_sbrPresentFlag; /**< Flag indicating the presence of SBR data in the bitstream         */
  SCHAR m_psPresentFlag;  /**< Flag indicating the presence of parametric stereo data in the
                             bitstream */
  UCHAR m_samplingFrequencyIndex;          /**< Samplerate index                                 */
  UCHAR m_extensionSamplingFrequencyIndex; /**< Samplerate index                        */
  SCHAR m_extensionChannelConfiguration;   /**< Channel configuration index             */

  UCHAR configMode; /**< The flag indicates if the callback shall work in memory allocation mode or
                       in config change detection mode */
  UCHAR AacConfigChanged; /**< The flag will be set if at least one aac config parameter has changed
                             that requires a memory reconfiguration, otherwise it will be cleared */
  UCHAR SbrConfigChanged; /**< The flag will be set if at least one sbr config parameter has changed
                             that requires a memory reconfiguration, otherwise it will be cleared */
  UCHAR SacConfigChanged; /**< The flag will be set if at least one sac config parameter has changed
                             that requires a memory reconfiguration, otherwise it will be cleared */

} CSAudioSpecificConfig;

typedef struct {
  UCHAR Mpegh3daConfig[TP_MPEGH3DA_MAX_CONFIG_LEN]; /**< MHAS config */
  UINT Mpegh3daConfigLen;                           /**< MHAS config length */
  SCHAR flushCnt;                                   /**< Flush frame counter */
  UCHAR flushStatus;                                /**< Flag indicates flush mode: on|off */
  SCHAR buildUpCnt;                                 /**< Build up frame counter */
  UCHAR buildUpStatus;                              /**< Flag indicates build up mode: on|off */
  UCHAR cfgChanged;     /**< Flag indicates that the config changed and the decoder needs to be
                           initialized again via callback.     Make sure that memory is freed before
                           initialization. */
  UCHAR contentChanged; /**< Flag indicates that the content changed i.e. a right truncation occured
                           before */
  UCHAR forceCfgChange; /**< Flag indicates if config change has to be forced even if new config is
                           the same */
  UCHAR truncationPresent; /**< Flag indicates if truncation message was received */
  UCHAR forceCrossfade;
} CCtrlCFGChange;

#define ASI_MAX_LIMIT 28
#define ASI_MAX_GROUP_PRESETS 16
#define ASI_MAX_DESCRIPTION_LANGUAGES 8

#define ASI_MAX_GROUPS ASI_MAX_LIMIT
#define ASI_MAX_SWITCH_GROUPS (ASI_MAX_LIMIT / 2)
#define ASI_MAX_TARGET_LOUDNESS_CONDITIONS (ASI_MAX_LIMIT / 2)
#define ASI_MAX_GROUP_MEMBERS ASI_MAX_LIMIT
#define ASI_MAX_SWITCH_GROUP_MEMBERS ASI_MAX_LIMIT
#define ASI_MAX_GROUP_PRESET_CONDITIONS 16
#define ASI_MAX_COMPOSITE_PAIRS ASI_MAX_LIMIT
#define ASI_MAX_PREF_DESCR_LANGUAGES 10
#define ASI_MAX_DESCRIPTION_BLOCKS (ASI_MAX_GROUPS + ASI_MAX_SWITCH_GROUPS + ASI_MAX_GROUP_PRESETS)
#define ASI_MAX_DESCRIPTION_LEN 256 /* max(mae_bsDescriptionDataLength) + 1 */
#define ASI_MAX_STORED_DESCRIPTION_LEN 64

#define ASI_DIFF_DESCRIPTION 1
#define ASI_DIFF_CONTENT 2
#define ASI_DIFF_GROUP 4
#define ASI_DIFF_SWITCHGRP 8
#define ASI_DIFF_GRPPRESET 16
#define ASI_DIFF_NEEDS_RESET 32
#define ASI_DIFF_LCOMP 64
#define ASI_DIFF_SCRNSIZE 128
#define ASI_DIFF_DRC_INFO 256
#define ASI_DIFF_AVAILABILITY 512

typedef struct {
  UCHAR present;
#ifdef ASI_MAX_DESCRIPTION_LANGUAGES
  SHORT numLanguages;
  SHORT prefLangIdx;
  char language[ASI_MAX_DESCRIPTION_LANGUAGES][3];
  char data[ASI_MAX_DESCRIPTION_LANGUAGES][ASI_MAX_STORED_DESCRIPTION_LEN + 1];
#else
  char language[3];
  char data[ASI_MAX_DESCRIPTION_LEN + 1];
#endif
} ASI_DESCRIPTION;

typedef struct {
  UCHAR contentKind;
  char contentLanguage[3];
} ASI_CONTENT_DATA;

typedef struct {
  UCHAR hasNonStandardScreenSize;
  UCHAR isCenteredInAzimuth;
  SHORT bsScreenSizeAz;
  SHORT bsScreenSizeLeftAz;
  SHORT bsScreenSizeRightAz;
  SHORT bsScreenSizeTopEl;
  SHORT bsScreenSizeBottomEl;
} ASI_PRODUCTION_SCREEN_SIZE_DATA;

typedef struct {
  UCHAR usePredefinedSector;
  UCHAR excludeSectorIndex;
  UCHAR excludeSectorMinAzimuth;
  UCHAR excludeSectorMaxAzimuth;
  UCHAR excludeSectorMinElevation;
  UCHAR excludeSectorMaxElevation;
} ASI_EXCLUSION_SECTOR;

typedef struct {
  UCHAR groupID;
  UCHAR allowOnOff;
  UCHAR defaultOnOff;

  UCHAR allowPositionInteractivity;
  UCHAR interactivityMinAzOffset;
  UCHAR interactivityMaxAzOffset;
  UCHAR interactivityMinElOffset;
  UCHAR interactivityMaxElOffset;
  UCHAR interactivityMinDistFactor;
  UCHAR interactivityMaxDistFactor;

  UCHAR allowGainInteractivity;
  UCHAR interactivityMinGain;
  UCHAR interactivityMaxGain;

  UCHAR isAvailable;
  UCHAR priority;
  UCHAR closestSpeakerPlayout;

  UCHAR numMembers;
  UCHAR hasConjunctMembers;

  UCHAR startID;
  UCHAR metaDataElementID[ASI_MAX_GROUP_MEMBERS];

  /* extension */
  UCHAR extPresent;

  /* content data */
  UCHAR contPresent;
  ASI_CONTENT_DATA contentData;

  /* ID of switch group this group is member of (255 for none) */
  UCHAR switchGroupID;
} ASI_GROUP;

typedef struct {
  UCHAR switchGroupID;
  UCHAR allowOnOff;
  UCHAR defaultOnOff;

  UCHAR numMembers;
  UCHAR memberID[ASI_MAX_SWITCH_GROUP_MEMBERS];

  UCHAR defaultGroupID;

} ASI_SWITCH_GROUP;

typedef struct {
  UCHAR referenceID;
  UCHAR conditionOnOff;

  UCHAR disableGainInteractivity;
  UCHAR gainFlag;
  UCHAR gain;

  UCHAR disablePositionInteractivity;
  UCHAR positionFlag;
  UCHAR azOffset;
  UCHAR elOffset;
  UCHAR distFactor;

  /* extension */
  UCHAR isSwitchGroupCondition;
} ASI_GROUP_PRESET_CONDITION;

typedef struct {
  UCHAR downmixId;
  UCHAR numConditions;
  ASI_GROUP_PRESET_CONDITION conditions[ASI_MAX_GROUP_PRESET_CONDITIONS];
} ASI_DMX_ID_GROUP_PRESET_EXT;

typedef struct {
  UCHAR groupPresetID;
  UCHAR kind;

  UCHAR numConditions;
  ASI_GROUP_PRESET_CONDITION conditions[ASI_MAX_GROUP_PRESET_CONDITIONS];

  /* extension */
  UCHAR extPresent;

  UCHAR hasDownmixIdExtension;
  ASI_DMX_ID_GROUP_PRESET_EXT downmixIdExtension;

  /* production screen size */
  ASI_PRODUCTION_SCREEN_SIZE_DATA productionScreenSize;

} ASI_GROUP_PRESET;

typedef struct {
  UCHAR groupLoudnessPresent;
  UCHAR groupLoudness[ASI_MAX_GROUPS];

  UCHAR defaultParamsPresent;
  UCHAR defaultIncludeGroup[ASI_MAX_GROUPS];

  UCHAR defaultMinMaxGainPresent;
  UCHAR defaultMinGain;
  UCHAR defaultMaxGain;

  UCHAR presetParamsPresent[ASI_MAX_GROUP_PRESETS];
  UCHAR presetIncludeGroup[ASI_MAX_GROUP_PRESETS][ASI_MAX_GROUPS];

  UCHAR presetMinMaxGainPresent[ASI_MAX_GROUP_PRESETS];
  UCHAR presetMinGain[ASI_MAX_GROUP_PRESETS];
  UCHAR presetMaxGain[ASI_MAX_GROUP_PRESETS];
} ASI_LOUDNESS_COMPENSATION_DATA;

typedef struct {
  UCHAR numTargetLoudnessConditions;
  UCHAR bsTargetLoudnessValueUpper[ASI_MAX_TARGET_LOUDNESS_CONDITIONS];
  USHORT drcSetEffectAvailable[ASI_MAX_TARGET_LOUDNESS_CONDITIONS];
} ASI_DRC_UI_INFO;

typedef struct {
  ASI_DESCRIPTION groups[ASI_MAX_GROUPS];
  ASI_DESCRIPTION switchGroups[ASI_MAX_SWITCH_GROUPS];
  ASI_DESCRIPTION groupPresets[ASI_MAX_GROUP_PRESETS];
} ASI_DESCRIPTIONS;

typedef struct {
  UCHAR isMainStream[16];
  UCHAR audioSceneInfoID;

  UCHAR numGroups;
  ASI_GROUP groups[ASI_MAX_GROUPS];

  UCHAR numSwitchGroups;
  ASI_SWITCH_GROUP switchGroups[ASI_MAX_SWITCH_GROUPS];

  UCHAR numGroupPresets;
  ASI_GROUP_PRESET groupPresets[ASI_MAX_GROUP_PRESETS];

  ASI_DESCRIPTIONS* pDescriptions;

  UCHAR metaDataElementIDoffset[16];
  SHORT metaDataElementIDmaxAvail[16]; /* the maximum available metaDataElementID in a Main Stream
                                          or  Sub-Stream. -1 means no IDs. */

  UCHAR compPairsPresent;
  UCHAR numCompositePairs;
  UCHAR compositePairs[ASI_MAX_COMPOSITE_PAIRS][2];

  UCHAR scrnSizePresent, scrnSizeExtPresent;
  ASI_PRODUCTION_SCREEN_SIZE_DATA productionScreenSizeData;

  UCHAR loudCompPresent;
  ASI_LOUDNESS_COMPENSATION_DATA loudnessCompensationData;

  UCHAR drcUiInfoPresent;
  ASI_DRC_UI_INFO drcUiInfo;

  UCHAR sceneDispPresent;

  UINT diffFlags;

  char prefDescrLanguages[ASI_MAX_PREF_DESCR_LANGUAGES][3];
  UCHAR activeDmxId;

  USHORT crcForUid;
} AUDIO_SCENE_INFO;

int asiGroupID2idx(const AUDIO_SCENE_INFO* asi, const int groupID);
int asiSwitchGroupID2idx(const AUDIO_SCENE_INFO* asi, const int switchGroupID);
int asiGroupPresetID2idx(const AUDIO_SCENE_INFO* asi, const int groupPresetID);
/* The three letter language code (ISO 639-2) in buf is replaced with an unambiguous representation:
   - The T (terminology) version is replaced with with the B (bibliographic) version.
   - All letters are converted to lower case.
*/
void asiMapISO639_2_T2B_and_tolower(char* buf);
/* If buf contains an invalid ISO 639-2 language code replace it with "und" (undetermined) and
   return 1 else return 0. Note that only a simple check is done (isalpha()). This avoids that
   invalid characters like "\0" end up in the XML output.
*/
int asiCheckISO639_2(char* buf);

#define EARCON_MAX_NUM_SIGNALS 2
#define EARCON_NUM_LANGUAGES (1 << 4)
/*Buffer size= 2(stereo Earcon) * (3 (maximum resampling ratio for MPEGH) * 1024 (MPEGH base
 * length)) + 775 (delayed samples before the limiter) ) */
#define EARCON_BUFFER_SIZE (EARCON_MAX_NUM_SIGNALS * (3 * 1024 + 775))

typedef struct {
  UINT m_numEarcons;
} EarconInfo;

typedef struct {
  UINT m_numPcmSignals;
  UINT m_bsPcmFrameSize_index;
  UINT m_bsPcmFrameSize;
  UINT m_bsPcmLoudnessValue;
  UINT m_pcmHasAttenuationGain;
  UINT m_bsPcmAttenuationGain;
  UINT m_bsPcmTruncationLength;
  UINT m_bsMHASTruncationLength;
  UINT sampling_frequency;
  UINT TruncationFlag;
  UINT EarconFlag;
  UINT EarconParseError;
} EarconConfig;

typedef INT (*cbUpdateConfig_t)(void*, const CSAudioSpecificConfig*, const UCHAR configMode,
                                UCHAR* configChanged);
typedef INT (*cbDecodeFrame_t)(void*);
typedef INT (*cbFreeMem_t)(void*, const CSAudioSpecificConfig*);
typedef INT (*cbCtrlCFGChange_t)(void*, const CCtrlCFGChange*);
typedef INT (*cbTruncationMsg_t)(void*, INT nTruncSamples, INT truncFromBegin);

typedef INT (*cbUniDrc_t)(void* self, HANDLE_FDK_BITSTREAM hBs, const INT fullPayloadLength,
                          const INT payloadType, const INT subStreamIndex, const INT payloadStart,
                          const AUDIO_OBJECT_TYPE);

typedef INT (*cbParseDmxMatrix_t)(void* self, HANDLE_FDK_BITSTREAM hBs, CSUsacConfig* usc);

typedef INT (*cbUserInteract_t)(void* self, HANDLE_FDK_BITSTREAM hBs, const mha_pactyp_t type,
                                const UINT MhasPacketLength);

typedef INT (*cbEarconBS_t)(void* self, HANDLE_FDK_BITSTREAM bs);
typedef INT (*cbEarconConfig_t)(void* self, EarconConfig* cf);
typedef INT (*cbEarconInfo_t)(void* self, EarconInfo* info);
typedef INT (*cbEarconUpdate_t)(void* self);

typedef struct {
  cbUpdateConfig_t cbUpdateConfig;   /*!< Function pointer for Config change notify callback.  */
  void* cbUpdateConfigData;          /*!< User data pointer for Config change notify callback. */
  cbDecodeFrame_t cbDecodeFrame;     /*!< Function pointer for Decode frame callback.  */
  void* cbDecodeFrameData;           /*!< User data pointer for Decode frame callback. */
  cbFreeMem_t cbFreeMem;             /*!< Function pointer for free memory callback.  */
  void* cbFreeMemData;               /*!< User data pointer for free memory callback. */
  cbCtrlCFGChange_t cbCtrlCFGChange; /*!< Function pointer for config change control callback. */
  void* cbCtrlCFGChangeData;         /*!< User data pointer for config change control callback. */
  cbTruncationMsg_t cbTruncationMsg; /*!< Function pointer for truncation message callback. */
  void* cbTruncationMsgData;         /*!< User data pointer for truncation message callback. */
  cbUniDrc_t
      cbUniDrc; /*!< Function pointer for uniDrcConfig and loudnessInfoSet parser callback. */
  void*
      cbUniDrcData; /*!< User data pointer for uniDrcConfig and loudnessInfoSet parser callback. */
  cbUserInteract_t cbUserInteract; /*!< Function pointer for user interaction parser callback. */
  void* cbUserInteractData;        /*!< User data pointer for user interaction parser callback. */
  cbParseDmxMatrix_t cbParseDmxMatrix; /*!< Function pointer for downmix matrix parser callback. */
  void* cbParseDmxMatrixData;          /*!< User data pointer for downmix matrix parser callback. */
  cbEarconBS_t cbEarconBS; /*!< Function pointer for earcon decoder set bitstream callback. */
  void* cbEarconBSData;    /*!< User data pointer for earcon decoder set bitstream data callback. */

  cbEarconConfig_t cbEarconConfig; /*!< Function pointer for earcon decoder set config callback. */
  void* cbEarconConfigData; /*!< User data pointer for earcon decoder set config data callback. */

  cbEarconInfo_t cbEarconInfo; /*!< Function pointer for earcon decoder set info callback. */
  void* cbEarconInfoData;      /*!< User data pointer for earcon decoder set info data callback. */
} CSTpCallBacks;

static const UINT SamplingRateTable[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
                                         16000, 12000, 11025, 8000,  7350,  0,     0,     57600,
                                         51200, 40000, 38400, 34150, 28800, 25600, 20000, 19200,
                                         17075, 14400, 12800, 9600,  0,     0,     0,     0};

#endif /* TP_DATA_H */
