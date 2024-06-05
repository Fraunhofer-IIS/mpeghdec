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

   Author(s):   Matthias Neusinger

   Description: MPEG Transport decoder

*******************************************************************************/

#include "tpdec_lib.h"

#include "genericStds.h"
#include "FDK_crc.h"

#if (16 < TPDEC_MAX_TRACKS)
#error 16 is smaller than TPDEC_MAX_TRACKS
#endif

#define ID_MAE_GROUP_DESCRIPTION 0
#define ID_MAE_SWITCHGROUP_DESCRIPTION 1
#define ID_MAE_GROUP_CONTENT 2
#define ID_MAE_GROUP_COMPOSITE 3
#define ID_MAE_SCREEN_SIZE 4
#define ID_MAE_GROUPPRESET_DESCRIPTION 5
#define ID_MAE_DRC_UI_INFO 6
#define ID_MAE_SCREEN_SIZE_EXTENSION 7
#define ID_MAE_GROUP_PRESET_EXTENSION 8
#define ID_MAE_LOUDNESS_COMPENSATION 9

#define INVALID_ID 255

/* compare and assign */
static inline int compAssign(UCHAR* dest, UCHAR src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int compAssign(char* dest, char src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int compAssign(SHORT* dest, SHORT src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

static inline int compAssign(USHORT* dest, USHORT src) {
  int diff = 0;
  if (*dest != src) diff = 1;
  *dest = src;
  return diff;
}

/* get group index from group ID */
int asiGroupID2idx(const AUDIO_SCENE_INFO* asi, const int groupID) {
  int i;

  for (i = 0; i < asi->numGroups; i++) {
    if (asi->groups[i].groupID == groupID) {
      return i;
    }
  }

  return -1;
}

/* get switch group index from switch group ID */
int asiSwitchGroupID2idx(const AUDIO_SCENE_INFO* asi, const int switchGroupID) {
  int i;

  for (i = 0; i < asi->numSwitchGroups; i++) {
    if (asi->switchGroups[i].switchGroupID == switchGroupID) {
      return i;
    }
  }

  return -1;
}

/* get preset index from preset ID */
int asiGroupPresetID2idx(const AUDIO_SCENE_INFO* asi, const int groupPresetID) {
  int i;

  for (i = 0; i < asi->numGroupPresets; i++) {
    if (asi->groupPresets[i].groupPresetID == groupPresetID) {
      return i;
    }
  }

  return -1;
}

/* reset asi in case of parse error */
void asiReset(AUDIO_SCENE_INFO* asi) {
  int i;
  char prefDescrLanguages_old[ASI_MAX_PREF_DESCR_LANGUAGES][3];
  UCHAR activeDmxId_old = asi->activeDmxId;
  ASI_DESCRIPTIONS* pDescriptions_old = asi->pDescriptions;

  for (i = 0; i < ASI_MAX_PREF_DESCR_LANGUAGES; i++) {
    FDKstrncpy(prefDescrLanguages_old[i], asi->prefDescrLanguages[i], 3);
  }

  FDKmemclear(asi, sizeof(AUDIO_SCENE_INFO));

  if (pDescriptions_old) FDKmemclear(pDescriptions_old, sizeof(ASI_DESCRIPTIONS));
  asi->pDescriptions = pDescriptions_old;

  asi->activeDmxId = activeDmxId_old;
  for (i = 0; i < ASI_MAX_PREF_DESCR_LANGUAGES; i++) {
    FDKstrncpy(asi->prefDescrLanguages[i], prefDescrLanguages_old[i], 3);
  }
  for (i = 0; i < TPDEC_MAX_TRACKS; i++) {
    asi->metaDataElementIDmaxAvail[i] = -1;
  }
  asi->diffFlags = ASI_DIFF_NEEDS_RESET;

  return;
}

static TRANSPORTDEC_ERROR mae_GroupDefinition(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs,
                                              int numMaxElementIDs) {
  int grp;

  for (grp = 0; grp < asi->numGroups; grp++) {
    ASI_GROUP* group = &(asi->groups[grp]);
    ASI_DESCRIPTION* description = asi->pDescriptions ? &(asi->pDescriptions->groups[grp]) : NULL;

    if (compAssign(&group->groupID, FDKreadBits(bs, 7))) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (compAssign(&group->allowOnOff, FDKreadBit(bs))) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (compAssign(&group->defaultOnOff, FDKreadBit(bs))) asi->diffFlags |= ASI_DIFF_GROUP;

    if (compAssign(&group->allowPositionInteractivity, FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (group->allowPositionInteractivity) {
      if (compAssign(&group->interactivityMinAzOffset, FDKreadBits(bs, 7)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMaxAzOffset, FDKreadBits(bs, 7)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMinElOffset, FDKreadBits(bs, 5)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMaxElOffset, FDKreadBits(bs, 5)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMinDistFactor, FDKreadBits(bs, 4)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMaxDistFactor, FDKreadBits(bs, 4)))
        asi->diffFlags |= ASI_DIFF_GROUP;
    }

    if (compAssign(&group->allowGainInteractivity, FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (group->allowGainInteractivity) {
      if (compAssign(&group->interactivityMinGain, FDKreadBits(bs, 6)))
        asi->diffFlags |= ASI_DIFF_GROUP;
      if (compAssign(&group->interactivityMaxGain, FDKreadBits(bs, 5)))
        asi->diffFlags |= ASI_DIFF_GROUP;
    }

    if (compAssign(&group->numMembers, FDKreadBits(bs, 7) + 1))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (group->numMembers > ASI_MAX_GROUP_MEMBERS) {
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    }
    group->hasConjunctMembers = FDKreadBit(bs);

    if (group->hasConjunctMembers) {
      int obj;

      group->startID = FDKreadBits(bs, 7);
      if (group->startID >= numMaxElementIDs) {
        return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }

      for (obj = 0; obj < group->numMembers; obj++) {
        if (compAssign(&group->metaDataElementID[obj], group->startID + obj))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (group->metaDataElementID[obj] >= numMaxElementIDs) {
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
        }
      }
    } else {
      int obj;

      for (obj = 0; obj < group->numMembers; obj++) {
        if (compAssign(&group->metaDataElementID[obj], FDKreadBits(bs, 7)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (group->metaDataElementID[obj] >= numMaxElementIDs) {
          return TRANSPORTDEC_UNSUPPORTED_FORMAT;
        }
      }
    }

    group->extPresent = 0;
    group->contPresent = 0;

    if (description && (asi->diffFlags & ASI_DIFF_NEEDS_RESET)) description->present = 0;

    group->switchGroupID = INVALID_ID;
    /*Presume that no groups are available initially.*/
    group->isAvailable = 0;
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_GroupAvailabilityCheck(AUDIO_SCENE_INFO* asi) {
  int grp, obj;

  for (grp = 0; grp < asi->numGroups; grp++) {
    ASI_GROUP* group = &(asi->groups[grp]);

    int group_detected = 0;
    for (int streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
      if (asi->metaDataElementIDmaxAvail[streamIndex] >=
          asi->metaDataElementIDoffset[streamIndex]) {
        for (obj = 0; obj < group->numMembers; obj++) {
          /*Pass over previous groups*/
          if ((group->metaDataElementID[obj] < asi->metaDataElementIDoffset[streamIndex]) ||
              (group->metaDataElementID[obj] > asi->metaDataElementIDmaxAvail[streamIndex])) {
            break;
          }
          group_detected++;
        }
      }
    }

    /*A check for the case, when metaDataElementIDs are not consecutive*/
    if ((group_detected != 0) && (group_detected != group->numMembers)) {
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    }

    if (group_detected) {
      group->isAvailable = 1;
    }
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_DrcUserInterfaceInfo(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs) {
  int version, c;

  version = FDKread2Bits(bs);

  if (version == 0) {
    if (compAssign(&asi->drcUiInfo.numTargetLoudnessConditions, FDKreadBits(bs, 3) + 1))
      asi->diffFlags |= ASI_DIFF_DRC_INFO;
    if (asi->drcUiInfo.numTargetLoudnessConditions > ASI_MAX_TARGET_LOUDNESS_CONDITIONS)
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;

    for (c = 0; c < asi->drcUiInfo.numTargetLoudnessConditions; c++) {
      if (compAssign(&asi->drcUiInfo.bsTargetLoudnessValueUpper[c], FDKreadBits(bs, 6)))
        asi->diffFlags |= ASI_DIFF_DRC_INFO;
      if (compAssign(&asi->drcUiInfo.drcSetEffectAvailable[c], FDKreadBits(bs, 16)))
        asi->diffFlags |= ASI_DIFF_DRC_INFO;
    }
  } else {
    if (compAssign(&asi->drcUiInfo.numTargetLoudnessConditions, 0))
      asi->diffFlags |= ASI_DIFF_DRC_INFO;
    /* discard remaining bits signaled by mae_dataLength */
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_SwitchGroupDefinition(AUDIO_SCENE_INFO* asi,
                                                    HANDLE_FDK_BITSTREAM bs) {
  int swgrp, grp;

  for (swgrp = 0; swgrp < asi->numSwitchGroups; swgrp++) {
    ASI_SWITCH_GROUP* switchGroup = &(asi->switchGroups[swgrp]);
    ASI_DESCRIPTION* description =
        asi->pDescriptions ? &(asi->pDescriptions->switchGroups[swgrp]) : NULL;

    if (compAssign(&switchGroup->switchGroupID, FDKreadBits(bs, 5)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

    if (compAssign(&switchGroup->allowOnOff, FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (switchGroup->allowOnOff) {
      if (compAssign(&switchGroup->defaultOnOff, FDKreadBit(bs)))
        asi->diffFlags |= ASI_DIFF_SWITCHGRP;
    } else {
      switchGroup->defaultOnOff = 1;
    }

    if (compAssign(&switchGroup->numMembers, FDKreadBits(bs, 5) + 1))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (switchGroup->numMembers > ASI_MAX_SWITCH_GROUP_MEMBERS)
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    for (grp = 0; grp < switchGroup->numMembers; grp++) {
      int grpIdx;

      if (compAssign(&switchGroup->memberID[grp], FDKreadBits(bs, 7)))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

      grpIdx = asiGroupID2idx(asi, switchGroup->memberID[grp]);
      if (grpIdx < 0) return TRANSPORTDEC_PARSE_ERROR;
      asi->groups[grpIdx].switchGroupID = switchGroup->switchGroupID;
    }
    if (compAssign(&switchGroup->defaultGroupID, FDKreadBits(bs, 7)))
      asi->diffFlags |= ASI_DIFF_SWITCHGRP;

    if (description && (asi->diffFlags & ASI_DIFF_NEEDS_RESET)) description->present = 0;
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_GroupPresetDefinition(AUDIO_SCENE_INFO* asi,
                                                    HANDLE_FDK_BITSTREAM bs) {
  int gp, cnd;

  for (gp = 0; gp < asi->numGroupPresets; gp++) {
    ASI_GROUP_PRESET* groupPreset = &(asi->groupPresets[gp]);
    ASI_DESCRIPTION* description =
        asi->pDescriptions ? &(asi->pDescriptions->groupPresets[gp]) : NULL;

    if (compAssign(&groupPreset->groupPresetID, FDKreadBits(bs, 5)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (compAssign(&groupPreset->kind, FDKreadBits(bs, 5))) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

    if (compAssign(&groupPreset->numConditions, FDKreadBits(bs, 4) + 1))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (groupPreset->numConditions > ASI_MAX_GROUP_PRESET_CONDITIONS)
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    for (cnd = 0; cnd < groupPreset->numConditions; cnd++) {
      if (compAssign(&groupPreset->conditions[cnd].referenceID, FDKreadBits(bs, 7)))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
      if (compAssign(&groupPreset->conditions[cnd].conditionOnOff, FDKreadBit(bs)))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

      if (groupPreset->conditions[cnd].conditionOnOff) {
        if (compAssign(&groupPreset->conditions[cnd].disableGainInteractivity, FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (compAssign(&groupPreset->conditions[cnd].gainFlag, FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (groupPreset->conditions[cnd].gainFlag) {
          if (compAssign(&groupPreset->conditions[cnd].gain, FDKreadBits(bs, 8)))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        }

        if (compAssign(&groupPreset->conditions[cnd].disablePositionInteractivity, FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (compAssign(&groupPreset->conditions[cnd].positionFlag, FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        if (groupPreset->conditions[cnd].positionFlag) {
          if (compAssign(&groupPreset->conditions[cnd].azOffset, FDKreadBits(bs, 8)))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
          if (compAssign(&groupPreset->conditions[cnd].elOffset, FDKreadBits(bs, 6)))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
          if (compAssign(&groupPreset->conditions[cnd].distFactor, FDKreadBits(bs, 4)))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
        }
      }
    }

    groupPreset->extPresent = 0;

    if (description && (asi->diffFlags & ASI_DIFF_NEEDS_RESET)) description->present = 0;

    groupPreset->productionScreenSize.hasNonStandardScreenSize = 0;
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_GroupPresetDefinitionExtension(AUDIO_SCENE_INFO* asi,
                                                             HANDLE_FDK_BITSTREAM bs) {
  int gp, cnd, egp;

  for (gp = 0; gp < asi->numGroupPresets; gp++) {
    ASI_GROUP_PRESET* preset = &(asi->groupPresets[gp]);

    preset->extPresent = 1;

    int hasSwitchGroupConditions = FDKreadBit(bs);
    if (hasSwitchGroupConditions) {
      for (cnd = 0; cnd < preset->numConditions; cnd++) {
        if (compAssign(&preset->conditions[cnd].isSwitchGroupCondition, FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
      }
    } else {
      for (cnd = 0; cnd < preset->numConditions; cnd++) {
        if (compAssign(&preset->conditions[cnd].isSwitchGroupCondition, 0))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
      }
    }

    int hasDownmixIdGroupPresetExtensions = FDKreadBit(bs);
    if (hasDownmixIdGroupPresetExtensions) {
      int numDownmixIdGroupPresetExtensions;
      int foundDmxId = 0;

      numDownmixIdGroupPresetExtensions = FDKreadBits(bs, 5);
      for (egp = 0; egp < numDownmixIdGroupPresetExtensions; egp++) {
        UCHAR tmpDmxId;

        tmpDmxId = FDKreadBits(bs, 7);
        if (tmpDmxId == asi->activeDmxId) {
          foundDmxId = 1;

          if (compAssign(&preset->downmixIdExtension.downmixId, tmpDmxId))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
          if (compAssign(&preset->downmixIdExtension.numConditions, FDKreadBits(bs, 4) + 1))
            asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
          if (preset->downmixIdExtension.numConditions > ASI_MAX_GROUP_PRESET_CONDITIONS)
            return TRANSPORTDEC_UNSUPPORTED_FORMAT;

          for (cnd = 0; cnd < preset->downmixIdExtension.numConditions; cnd++) {
            ASI_GROUP_PRESET_CONDITION* condition = &(preset->downmixIdExtension.conditions[cnd]);

            if (compAssign(&condition->isSwitchGroupCondition, FDKreadBit(bs)))
              asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
            if (condition->isSwitchGroupCondition) {
              if (compAssign(&condition->referenceID, FDKreadBits(bs, 5)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
            } else {
              if (compAssign(&condition->referenceID, FDKreadBits(bs, 7)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
            }

            if (compAssign(&condition->conditionOnOff, FDKreadBit(bs)))
              asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
            if (condition->conditionOnOff) {
              if (compAssign(&condition->disableGainInteractivity, FDKreadBit(bs)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              if (compAssign(&condition->gainFlag, FDKreadBit(bs)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              if (condition->gainFlag) {
                if (compAssign(&condition->gain, FDKreadBits(bs, 8)))
                  asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              }

              if (compAssign(&condition->disablePositionInteractivity, FDKreadBit(bs)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              if (compAssign(&condition->positionFlag, FDKreadBit(bs)))
                asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              if (condition->positionFlag) {
                if (compAssign(&condition->azOffset, FDKreadBits(bs, 8)))
                  asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
                if (compAssign(&condition->elOffset, FDKreadBits(bs, 6)))
                  asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
                if (compAssign(&condition->distFactor, FDKreadBits(bs, 4)))
                  asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
              }
            }
          }
        } else {
          int numConditions = FDKreadBits(bs, 4) + 1;

          for (cnd = 0; cnd < numConditions; cnd++) {
            int isSwitchGroupCondition, conditionOnOff;

            isSwitchGroupCondition = FDKreadBit(bs);
            if (isSwitchGroupCondition) {
              FDKpushFor(bs, 5);
            } else {
              FDKpushFor(bs, 7);
            }

            conditionOnOff = FDKreadBit(bs);
            if (conditionOnOff) {
              int gainFlag, positionFlag;

              FDKpushFor(bs, 1);
              gainFlag = FDKreadBit(bs);
              if (gainFlag) {
                FDKpushFor(bs, 8);
              }

              FDKpushFor(bs, 1);
              positionFlag = FDKreadBit(bs);
              if (positionFlag) {
                FDKpushFor(bs, 8 + 6 + 4);
              }
            }
          }
        }
      }
      if (compAssign(&preset->hasDownmixIdExtension, foundDmxId))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    } else {
      if (compAssign(&preset->hasDownmixIdExtension, 0)) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    }
  }

  return TRANSPORTDEC_OK;
}

#define NUM_SPECIAL_LANG_CODES (20 + 2)
void asiMapISO639_2_T2B_and_tolower(char* buf) {
  /* double possibilities according to ISO 639-2 T/B */
  const char langCodes[NUM_SPECIAL_LANG_CODES][2][3] = {
      /*{ B version},      {T version} */
      {{'a', 'l', 'b'}, {'s', 'q', 'i'}},  {{'a', 'r', 'm'}, {'h', 'y', 'e'}},
      {{'b', 'a', 'q'}, {'e', 'u', 's'}},  {{'t', 'i', 'b'}, {'b', 'o', 'd'}},
      {{'b', 'u', 'r'}, {'m', 'y', 'a'}},  {{'c', 'z', 'e'}, {'c', 'e', 's'}},
      {{'c', 'h', 'i'}, {'z', 'h', 'o'}},  {{'w', 'e', 'l'}, {'c', 'y', 'm'}},
      {{'g', 'e', 'r'}, {'d', 'e', 'u'}},  {{'d', 'u', 't'}, {'n', 'l', 'd'}},
      {{'g', 'r', 'e'}, {'e', 'l', 'l'}},  {{'p', 'e', 'r'}, {'f', 'a', 's'}},
      {{'f', 'r', 'e'}, {'f', 'r', 'a'}},  {{'g', 'e', 'o'}, {'k', 'a', 't'}},
      {{'i', 'c', 'e'}, {'i', 's', 'l'}},  {{'m', 'a', 'c'}, {'m', 'k', 'd'}},
      {{'m', 'a', 'o'}, {'m', 'r', 'i'}},  {{'m', 'a', 'y'}, {'m', 's', 'a'}},
      {{'r', 'u', 'm'}, {'r', 'o', 'n'}},  {{'s', 'l', 'o'}, {'s', 'l', 'k'}},
      {{'k', 'o', 'r'}, {'k', 'o', '\0'}}, /* invalid input but map to valid ISO 639-2 language code
                                            */
      {{'e', 'n', 'g'}, {'e', 'n', '\0'}}  /* invalid input but map to valid ISO 639-2 language code
                                            */
  };

  buf[0] = FDKtolower(buf[0]);
  buf[1] = FDKtolower(buf[1]);
  buf[2] = FDKtolower(buf[2]);

  for (int i = 0; i < NUM_SPECIAL_LANG_CODES; i++) {
    if ((buf[0] == langCodes[i][1][0]) && (buf[1] == langCodes[i][1][1]) &&
        (buf[2] == langCodes[i][1][2])) {
      FDKmemcpy(buf, langCodes[i][0], 3);
      return;
    }
  }

  return;
}

int asiCheckISO639_2(char* buf) {
  if (!FDKisalpha(buf[0]) || !FDKisalpha(buf[1]) || !FDKisalpha(buf[2])) {
    buf[0] = 'u';
    buf[1] = 'n';
    buf[2] = 'd';
    return 1;
  }
  return 0;
}

static TRANSPORTDEC_ERROR mae_Description(int type, AUDIO_SCENE_INFO* asi,
                                          HANDLE_FDK_BITSTREAM bs) {
  int numDescriptionBlocks, n;
  int numDescLanguages, i;
  ASI_DESCRIPTION* descr = NULL;

  numDescriptionBlocks = FDKreadBits(bs, 7) + 1;
  if (numDescriptionBlocks > ASI_MAX_DESCRIPTION_BLOCKS) {
    return TRANSPORTDEC_PARSE_ERROR;
  }

  for (n = 0; n < numDescriptionBlocks; n++) {
    int currPrio = ASI_MAX_PREF_DESCR_LANGUAGES + 1;

    if (type == ID_MAE_GROUP_DESCRIPTION) {
      int groupID = FDKreadBits(bs, 7);

      i = asiGroupID2idx(asi, groupID);
      if (i < 0) return TRANSPORTDEC_PARSE_ERROR;
      if (asi->pDescriptions) descr = &(asi->pDescriptions->groups[i]);
    } else if (type == ID_MAE_SWITCHGROUP_DESCRIPTION) {
      int switchGroupID = FDKreadBits(bs, 5);

      i = asiSwitchGroupID2idx(asi, switchGroupID);
      if (i < 0) return TRANSPORTDEC_PARSE_ERROR;
      if (asi->pDescriptions) descr = &(asi->pDescriptions->switchGroups[i]);
    } else if (type == ID_MAE_GROUPPRESET_DESCRIPTION) {
      int groupPresetID = FDKreadBits(bs, 5);

      i = asiGroupPresetID2idx(asi, groupPresetID);
      if (i < 0) return TRANSPORTDEC_PARSE_ERROR;
      if (asi->pDescriptions) descr = &(asi->pDescriptions->groupPresets[i]);
    }

    /* get priority of current language */
    if (descr && descr->present) {
      for (currPrio = 0; currPrio < ASI_MAX_PREF_DESCR_LANGUAGES; currPrio++) {
#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
        if (descr->language[0] &&
            (FDKstrncmp(descr->language, asi->prefDescrLanguages[currPrio], 3) == 0)) {
#else
        if (descr->language[descr->prefLangIdx][0] &&
            (FDKstrncmp(descr->language[descr->prefLangIdx], asi->prefDescrLanguages[currPrio],
                        3) == 0)) {
#endif
          break;
        }
      }
    }

    numDescLanguages = FDKreadBits(bs, 4) + 1;
#ifdef ASI_MAX_DESCRIPTION_LANGUAGES
    if (numDescLanguages > ASI_MAX_DESCRIPTION_LANGUAGES) return TRANSPORTDEC_PARSE_ERROR;
    if (descr) descr->numLanguages = numDescLanguages;
#endif
    for (i = 0; i < numDescLanguages; i++) {
      char language[3];
      int dataLength;
      int prio;

      language[0] = FDKreadBits(bs, 8);
      language[1] = FDKreadBits(bs, 8);
      language[2] = FDKreadBits(bs, 8);
      asiMapISO639_2_T2B_and_tolower(language);
      asiCheckISO639_2(language); /* ignore error to allow playback of the stream */
      dataLength = FDKreadBits(bs, 8) + 1;

      /* get priority of language */
      if (descr) {
        for (prio = 0; prio < ASI_MAX_PREF_DESCR_LANGUAGES; prio++) {
          if (language[0] && (FDKstrncmp(language, asi->prefDescrLanguages[prio], 3) == 0)) {
            break;
          }
        }
        if (prio == ASI_MAX_PREF_DESCR_LANGUAGES)
          prio += i; /* if not in preferred list select first transmitted */
        if (prio == ASI_MAX_PREF_DESCR_LANGUAGES) {
#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
          if (descr && descr->present && (FDKstrncmp(language, descr->language, 3) != 0)) {
#else
          if (descr && descr->present &&
              (FDKstrncmp(language, descr->language[descr->prefLangIdx], 3) != 0)) {
#endif
            prio++;
          }
        }
      }

#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
      /* select language with highest priority */
      if (descr && (prio <= currPrio)) {
        int c;

        descr->present = 1;

        if (compAssign(&descr->language[0], language[0])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&descr->language[1], language[1])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&descr->language[2], language[2])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;

        for (c = 0; c < dataLength; c++) {
          if (compAssign(&descr->data[c], FDKreadBits(bs, 8)))
            asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        }
        if (compAssign(&descr->data[dataLength], 0))
          asi->diffFlags |= ASI_DIFF_DESCRIPTION; /* terminate string */

        currPrio = prio;
      }
#else
      if (descr) {
        int j;

        if (prio <= currPrio) descr->prefLangIdx = i;

        descr->present = 1;

        if (compAssign(&descr->language[i][0], language[0])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&descr->language[i][1], language[1])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&descr->language[i][2], language[2])) asi->diffFlags |= ASI_DIFF_DESCRIPTION;

        for (j = 0; j < dataLength; j++) {
          char c;

          if (j >= ASI_MAX_STORED_DESCRIPTION_LEN) break;

          c = FDKreadBits(bs, 8);

          /* prevent truncation within UTF-8 character */
          if (((j == ASI_MAX_STORED_DESCRIPTION_LEN - 1) && ((c & 0xC0) == 0xC0)) ||
              ((j == ASI_MAX_STORED_DESCRIPTION_LEN - 2) && ((c & 0xE0) == 0xE0)) ||
              ((j == ASI_MAX_STORED_DESCRIPTION_LEN - 3) && ((c & 0xF0) == 0xF0))) {
            c = 0;
          }

          if (compAssign(&descr->data[i][j], c)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        }
        if (compAssign(&descr->data[i][j], 0))
          asi->diffFlags |= ASI_DIFF_DESCRIPTION; /* terminate string */
        FDKpushFor(bs, 8 * (dataLength - j));

        currPrio = prio;
      }
#endif
      else {
        /* skip string */
        FDKpushFor(bs, 8 * dataLength);
      }
    }
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_ContentData(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs) {
  int numContentDataBlocks, n;

  numContentDataBlocks = FDKreadBits(bs, 7) + 1;

  for (n = 0; n < numContentDataBlocks; n++) {
    int groupID, i;
    UCHAR hasContentLanguage;
    ASI_CONTENT_DATA* contentData = NULL;

    groupID = FDKreadBits(bs, 7);
    i = asiGroupID2idx(asi, groupID);
    if (i < 0) return TRANSPORTDEC_PARSE_ERROR;
    contentData = &(asi->groups[i].contentData);

    asi->groups[i].contPresent = 1;

    if (compAssign(&contentData->contentKind, FDKreadBits(bs, 4)))
      asi->diffFlags |= ASI_DIFF_CONTENT;

    hasContentLanguage = FDKreadBit(bs);
    if (hasContentLanguage) {
      char language[3];
      language[0] = FDKreadBits(bs, 8);
      language[1] = FDKreadBits(bs, 8);
      language[2] = FDKreadBits(bs, 8);
      asiMapISO639_2_T2B_and_tolower(language);
      asiCheckISO639_2(language); /* ignore error to allow playback of the stream */
      if (compAssign(&contentData->contentLanguage[0], language[0]))
        asi->diffFlags |= ASI_DIFF_CONTENT;
      if (compAssign(&contentData->contentLanguage[1], language[1]))
        asi->diffFlags |= ASI_DIFF_CONTENT;
      if (compAssign(&contentData->contentLanguage[2], language[2]))
        asi->diffFlags |= ASI_DIFF_CONTENT;
    } else {
      if (compAssign(&contentData->contentLanguage[0], 0)) asi->diffFlags |= ASI_DIFF_CONTENT;
      if (compAssign(&contentData->contentLanguage[1], 0)) asi->diffFlags |= ASI_DIFF_CONTENT;
      if (compAssign(&contentData->contentLanguage[2], 0)) asi->diffFlags |= ASI_DIFF_CONTENT;
    }
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_CompositePair(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs) {
  int sa;

  if (compAssign(&asi->numCompositePairs, FDKreadBits(bs, 7) + 1))
    asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
  if (asi->numCompositePairs >= ASI_MAX_COMPOSITE_PAIRS) return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  for (sa = 0; sa < asi->numCompositePairs; sa++) {
    if (compAssign(&asi->compositePairs[sa][0], FDKreadBits(bs, 7)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (compAssign(&asi->compositePairs[sa][1], FDKreadBits(bs, 7)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_ProductionScreenSizeData(AUDIO_SCENE_INFO* asi,
                                                       HANDLE_FDK_BITSTREAM bs) {
  if (compAssign(&asi->productionScreenSizeData.hasNonStandardScreenSize, FDKreadBit(bs)))
    asi->diffFlags |= ASI_DIFF_SCRNSIZE;
  if (asi->productionScreenSizeData.hasNonStandardScreenSize) {
    if (compAssign(&asi->productionScreenSizeData.bsScreenSizeAz, FDKreadBits(bs, 9)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    if (compAssign(&asi->productionScreenSizeData.bsScreenSizeTopEl, FDKreadBits(bs, 9)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    if (compAssign(&asi->productionScreenSizeData.bsScreenSizeBottomEl, FDKreadBits(bs, 9)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_ProductionScreenSizeDataExtension(AUDIO_SCENE_INFO* asi,
                                                                HANDLE_FDK_BITSTREAM bs) {
  int overwriteProductionScreenSizeData;
  int numPresetProductionScreens, n;

  overwriteProductionScreenSizeData = FDKreadBit(bs);
  if (overwriteProductionScreenSizeData) {
    if (compAssign(&asi->productionScreenSizeData.bsScreenSizeLeftAz, FDKreadBits(bs, 10)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    if (compAssign(&asi->productionScreenSizeData.bsScreenSizeRightAz, FDKreadBits(bs, 10)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
  }

  numPresetProductionScreens = FDKreadBits(bs, 5);
  for (n = 0; n < numPresetProductionScreens; n++) {
    int groupPresetID, i;
    ASI_PRODUCTION_SCREEN_SIZE_DATA* productionScreen = NULL;

    groupPresetID = FDKreadBits(bs, 5);
    i = asiGroupPresetID2idx(asi, groupPresetID);
    if (i < 0) return TRANSPORTDEC_PARSE_ERROR;
    productionScreen = &(asi->groupPresets[i].productionScreenSize);

    if (compAssign(&productionScreen->hasNonStandardScreenSize, FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    if (productionScreen->hasNonStandardScreenSize) {
      if (compAssign(&productionScreen->isCenteredInAzimuth, FDKreadBit(bs)))
        asi->diffFlags |= ASI_DIFF_SCRNSIZE;
      if (productionScreen->isCenteredInAzimuth) {
        if (compAssign(&productionScreen->bsScreenSizeAz, FDKreadBits(bs, 9)))
          asi->diffFlags |= ASI_DIFF_SCRNSIZE;
      } else {
        if (compAssign(&productionScreen->bsScreenSizeLeftAz, FDKreadBits(bs, 10)))
          asi->diffFlags |= ASI_DIFF_SCRNSIZE;
        if (compAssign(&productionScreen->bsScreenSizeRightAz, FDKreadBits(bs, 10)))
          asi->diffFlags |= ASI_DIFF_SCRNSIZE;
      }
      if (compAssign(&productionScreen->bsScreenSizeTopEl, FDKreadBits(bs, 9)))
        asi->diffFlags |= ASI_DIFF_SCRNSIZE;
      if (compAssign(&productionScreen->bsScreenSizeBottomEl, FDKreadBits(bs, 9)))
        asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    }
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_LoudnessCompensationData(AUDIO_SCENE_INFO* asi,
                                                       HANDLE_FDK_BITSTREAM bs) {
  ASI_LOUDNESS_COMPENSATION_DATA* loudnessComp = &asi->loudnessCompensationData;
  int grp, gp;

  if (compAssign(&loudnessComp->groupLoudnessPresent, FDKreadBit(bs)))
    asi->diffFlags |= ASI_DIFF_LCOMP;
  if (loudnessComp->groupLoudnessPresent) {
    for (grp = 0; grp < asi->numGroups; grp++) {
      if (compAssign(&loudnessComp->groupLoudness[grp], FDKreadBits(bs, 8)))
        asi->diffFlags |= ASI_DIFF_LCOMP;
    }
  }

  if (compAssign(&loudnessComp->defaultParamsPresent, FDKreadBit(bs)))
    asi->diffFlags |= ASI_DIFF_LCOMP;
  if (loudnessComp->defaultParamsPresent) {
    for (grp = 0; grp < asi->numGroups; grp++) {
      if (compAssign(&loudnessComp->defaultIncludeGroup[grp], FDKreadBit(bs)))
        asi->diffFlags |= ASI_DIFF_LCOMP;
    }

    if (compAssign(&loudnessComp->defaultMinMaxGainPresent, FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_LCOMP;
    if (loudnessComp->defaultMinMaxGainPresent) {
      if (compAssign(&loudnessComp->defaultMinGain, FDKreadBits(bs, 4)))
        asi->diffFlags |= ASI_DIFF_LCOMP;
      if (compAssign(&loudnessComp->defaultMaxGain, FDKreadBits(bs, 4)))
        asi->diffFlags |= ASI_DIFF_LCOMP;
    }
  }

  for (gp = 0; gp < asi->numGroupPresets; gp++) {
    if (compAssign(&loudnessComp->presetParamsPresent[gp], FDKreadBit(bs)))
      asi->diffFlags |= ASI_DIFF_LCOMP;
    if (loudnessComp->presetParamsPresent[gp]) {
      for (grp = 0; grp < asi->numGroups; grp++) {
        if (compAssign(&loudnessComp->presetIncludeGroup[gp][grp], FDKreadBit(bs)))
          asi->diffFlags |= ASI_DIFF_LCOMP;
      }

      if (compAssign(&loudnessComp->presetMinMaxGainPresent[gp], FDKreadBit(bs)))
        asi->diffFlags |= ASI_DIFF_LCOMP;
      if (loudnessComp->presetMinMaxGainPresent[gp]) {
        if (compAssign(&loudnessComp->presetMinGain[gp], FDKreadBits(bs, 4)))
          asi->diffFlags |= ASI_DIFF_LCOMP;
        if (compAssign(&loudnessComp->presetMaxGain[gp], FDKreadBits(bs, 4)))
          asi->diffFlags |= ASI_DIFF_LCOMP;
      }
    }
  }

  return TRANSPORTDEC_OK;
}

static TRANSPORTDEC_ERROR mae_Data(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs,
                                   HANDLE_FDK_CRCINFO hCrcInfo) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  int numDataSets, dscr;

  asi->compPairsPresent = 0;
  asi->scrnSizePresent = asi->scrnSizeExtPresent = 0;
  asi->loudCompPresent = 0;
  asi->sceneDispPresent = 0;
  asi->drcUiInfoPresent = 0;

  numDataSets = FDKreadBits(bs, 4);
  for (dscr = 0; dscr < numDataSets; dscr++) {
    int dataType, dataLength, i;
    INT crcReg = 0;

    dataType = FDKreadBits(bs, 4);
    dataLength = FDKreadBits(bs, 16);

    i = (INT)FDKgetValidBits(bs);

    if (dataType == ID_MAE_GROUP_CONTENT) crcReg = FDKcrcStartReg(hCrcInfo, bs, 0);

    switch (dataType) {
      case ID_MAE_GROUP_DESCRIPTION:
      case ID_MAE_SWITCHGROUP_DESCRIPTION:
      case ID_MAE_GROUPPRESET_DESCRIPTION:
        err = mae_Description(dataType, asi, bs);
        break;
      case ID_MAE_GROUP_CONTENT:
        err = mae_ContentData(asi, bs);
        break;
      case ID_MAE_GROUP_COMPOSITE:
        err = mae_CompositePair(asi, bs);
        asi->compPairsPresent = 1;
        break;
      case ID_MAE_SCREEN_SIZE:
        err = mae_ProductionScreenSizeData(asi, bs);
        asi->scrnSizePresent = 1;
        break;
      case ID_MAE_DRC_UI_INFO:
        err = mae_DrcUserInterfaceInfo(asi, bs);
        asi->drcUiInfoPresent = 1;
        break;
      case ID_MAE_SCREEN_SIZE_EXTENSION:
        err = mae_ProductionScreenSizeDataExtension(asi, bs);
        asi->scrnSizeExtPresent = 1;
        break;
      case ID_MAE_GROUP_PRESET_EXTENSION:
        err = mae_GroupPresetDefinitionExtension(asi, bs);
        break;
      case ID_MAE_LOUDNESS_COMPENSATION:
        err = mae_LoudnessCompensationData(asi, bs);
        asi->loudCompPresent = 1;
        break;
    }

    i = dataLength * 8 - (i - (INT)FDKgetValidBits(bs));
    if (i >= 0) {
      FDKpushFor(bs, i);
    } else {
      return TRANSPORTDEC_PARSE_ERROR;
    }

    if (dataType == ID_MAE_GROUP_CONTENT) FDKcrcEndReg(hCrcInfo, bs, crcReg);

    if (err != TRANSPORTDEC_OK) {
      return err;
    }
  }

  /* clear data of elements not present */
  for (int grp = 0; grp < asi->numGroups; grp++) {
    ASI_DESCRIPTION* description = NULL;
    if (asi->pDescriptions) description = &(asi->pDescriptions->groups[grp]);

    if (!asi->drcUiInfoPresent) {
      asi->drcUiInfo.numTargetLoudnessConditions = 0;
    }

    if (description && !description->present) {
#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
      if (compAssign(&description->language[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      if (compAssign(&description->data[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
#else
      if (compAssign(&description->numLanguages, 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      for (int i = 0; i < ASI_MAX_DESCRIPTION_LANGUAGES; i++) {
        if (compAssign(&description->language[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&description->data[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      }
      description->prefLangIdx = 0;
#endif
    }

    if (!asi->groups[grp].contPresent) {
      if (compAssign(&asi->groups[grp].contentData.contentKind, 0))
        asi->diffFlags |= ASI_DIFF_CONTENT;
      if (compAssign(&asi->groups[grp].contentData.contentLanguage[0], 0))
        asi->diffFlags |= ASI_DIFF_CONTENT;
    }
  }

  for (int swgrp = 0; swgrp < asi->numSwitchGroups; swgrp++) {
    ASI_DESCRIPTION* description = NULL;
    if (asi->pDescriptions) description = &(asi->pDescriptions->switchGroups[swgrp]);

    if (description && !description->present) {
#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
      if (compAssign(&description->language[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      if (compAssign(&description->data[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
#else
      if (compAssign(&description->numLanguages, 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      for (int i = 0; i < ASI_MAX_DESCRIPTION_LANGUAGES; i++) {
        if (compAssign(&description->language[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&description->data[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      }
      description->prefLangIdx = 0;
#endif
    }
  }

  for (int gp = 0; gp < asi->numGroupPresets; gp++) {
    ASI_DESCRIPTION* description = NULL;
    if (asi->pDescriptions) description = &(asi->pDescriptions->groupPresets[gp]);

    if (!asi->groupPresets[gp].extPresent) {
      if (compAssign(&asi->groupPresets[gp].hasDownmixIdExtension, 0))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

      for (int cnd = 0; cnd < asi->groupPresets[gp].numConditions; cnd++) {
        if (compAssign(&asi->groupPresets[gp].conditions[cnd].isSwitchGroupCondition, 0))
          asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
      }
    }

    if (description && !description->present) {
#ifndef ASI_MAX_DESCRIPTION_LANGUAGES
      if (compAssign(&description->language[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      if (compAssign(&description->data[0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
#else
      if (compAssign(&description->numLanguages, 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      for (int i = 0; i < ASI_MAX_DESCRIPTION_LANGUAGES; i++) {
        if (compAssign(&description->language[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
        if (compAssign(&description->data[i][0], 0)) asi->diffFlags |= ASI_DIFF_DESCRIPTION;
      }
      description->prefLangIdx = 0;
#endif
    }
  }

  if (!asi->compPairsPresent) {
    if (compAssign(&asi->numCompositePairs, 0)) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
  }

  if (!asi->scrnSizePresent) {
    if (compAssign(&asi->productionScreenSizeData.hasNonStandardScreenSize, 0))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;
  }

  if (!asi->scrnSizeExtPresent) {
    if (compAssign(&asi->productionScreenSizeData.isCenteredInAzimuth, 1))
      asi->diffFlags |= ASI_DIFF_SCRNSIZE;

    for (int gp = 0; gp < asi->numGroupPresets; gp++) {
      if (compAssign(&asi->groupPresets[gp].productionScreenSize.hasNonStandardScreenSize, 0))
        asi->diffFlags |= ASI_DIFF_SCRNSIZE;
    }
  }

  if (!asi->loudCompPresent) {
    if (compAssign(&asi->loudnessCompensationData.groupLoudnessPresent, 0))
      asi->diffFlags |= ASI_DIFF_LCOMP;
    if (compAssign(&asi->loudnessCompensationData.defaultParamsPresent, 0))
      asi->diffFlags |= ASI_DIFF_LCOMP;
    for (int gp = 0; gp < asi->numGroupPresets; gp++) {
      if (compAssign(&asi->loudnessCompensationData.presetParamsPresent[gp], 0))
        asi->diffFlags |= ASI_DIFF_LCOMP;
    }
  }
  return err;
}

TRANSPORTDEC_ERROR mae_AudioSceneInfo(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs,
                                      int numMaxElementIDs, const int streamIndex) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  asi->diffFlags = 0;
  FDK_CRCINFO crcInfo;
  HANDLE_FDK_CRCINFO hCrcInfo = &crcInfo;
  INT crcReg;

  FDKcrcInit(hCrcInfo, 0x8021, 0, 16);
  crcReg = FDKcrcStartReg(hCrcInfo, bs, 0);

  if (compAssign(&asi->isMainStream[streamIndex], FDKreadBit(bs)))
    asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

  if (asi->isMainStream[streamIndex]) {
    UCHAR audioSceneInfoIDPresent = FDKreadBit(bs);
    if (audioSceneInfoIDPresent) {
      if (compAssign(&asi->audioSceneInfoID, FDKreadBits(bs, 8)))
        asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    } else {
      if (compAssign(&asi->audioSceneInfoID, 0)) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    }

    if (compAssign(&asi->numGroups, FDKreadBits(bs, 7))) asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (asi->numGroups > ASI_MAX_GROUPS) {
      asiReset(asi);
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    }
    err = mae_GroupDefinition(asi, bs, numMaxElementIDs);
    if (err != TRANSPORTDEC_OK) {
      asiReset(asi);
      return err;
    }
    if (compAssign(&asi->numSwitchGroups, FDKreadBits(bs, 5)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (asi->numSwitchGroups > ASI_MAX_SWITCH_GROUPS) {
      asiReset(asi);
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    }
    err = mae_SwitchGroupDefinition(asi, bs);
    if (err != TRANSPORTDEC_OK) {
      asiReset(asi);
      return err;
    }
    if (compAssign(&asi->numGroupPresets, FDKreadBits(bs, 5)))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
    if (asi->numGroupPresets > ASI_MAX_GROUP_PRESETS) {
      asiReset(asi);
      return TRANSPORTDEC_UNSUPPORTED_FORMAT;
    }
    err = mae_GroupPresetDefinition(asi, bs);
    if (err != TRANSPORTDEC_OK) {
      asiReset(asi);
      return err;
    }

    FDKcrcEndReg(hCrcInfo, bs, crcReg);

    err = mae_Data(asi, bs, hCrcInfo);
    if (err != TRANSPORTDEC_OK) {
      asiReset(asi);
      return err;
    }
    asi->metaDataElementIDoffset[streamIndex] = 0;
  } else {
    if (compAssign(&asi->metaDataElementIDoffset[streamIndex], FDKreadBits(bs, 7) + 1))
      asi->diffFlags |= ASI_DIFF_NEEDS_RESET;
  }

  if (compAssign(&asi->metaDataElementIDmaxAvail[streamIndex], FDKreadBits(bs, 7)))
    asi->diffFlags |= ASI_DIFF_NEEDS_RESET;

  err = mae_GroupAvailabilityCheck(asi);
  if (err != TRANSPORTDEC_OK) {
    asiReset(asi);
    return err;
  }

  if (asi->isMainStream[streamIndex]) {
    asi->crcForUid = FDKcrcGetCRC(hCrcInfo);
  }

  if (streamIndex > 0) {
    if (asi->diffFlags & ASI_DIFF_NEEDS_RESET) {
      asi->diffFlags = ASI_DIFF_AVAILABILITY;
    }
  }

  return TRANSPORTDEC_OK;
}

TRANSPORTDEC_ERROR checkASI(const AUDIO_SCENE_INFO* asi, int numSignalGroups,
                            const CSSignalGroup* signalGroups) {
  UCHAR maeID, maeID2grpIdx[(2 * 28)];
  int i, j;

  /* create mapping table for MAE ID to ASI group index */
  for (i = 0; i < (2 * 28); i++) maeID2grpIdx[i] = INVALID_ID;

  for (i = 0; i < asi->numGroups; i++) {
    for (j = 0; j < asi->groups[i].numMembers; j++) {
      maeID = asi->groups[i].metaDataElementID[j];

      if (maeID >= (2 * 28)) return TRANSPORTDEC_PARSE_ERROR;

      /* return error if same MAE ID occurs multiple times */
      if (maeID2grpIdx[maeID] != INVALID_ID) return TRANSPORTDEC_PARSE_ERROR;

      maeID2grpIdx[maeID] = i;
    }
  }

  /* check if switch group members are always complete signal groups */
  maeID = 0;
  for (i = 0; i < numSignalGroups; i++) {
    if (signalGroups[i].type == 3) {
      /* HOA signal group (has single MAE ID for whole signal group) */
      maeID++;
    } else {
      /* other signal group types (have MAE IDs for individual signals) */
      for (j = 1; j < signalGroups[i].count; j++) {
        /* check if all signal group members are in same ASI group */
        if (maeID2grpIdx[maeID + j] != maeID2grpIdx[maeID]) {
          /* if not, check if one of them is in a switch group */
          if (maeID2grpIdx[maeID] != INVALID_ID) {
            if (asi->groups[maeID2grpIdx[maeID]].switchGroupID != INVALID_ID)
              return TRANSPORTDEC_PARSE_ERROR;
          }
          if (maeID2grpIdx[maeID + j] != INVALID_ID) {
            if (asi->groups[maeID2grpIdx[maeID + j]].switchGroupID != INVALID_ID)
              return TRANSPORTDEC_PARSE_ERROR;
          }
        }
      }

      maeID += signalGroups[i].count;
    }
  }

  return TRANSPORTDEC_OK;
}
