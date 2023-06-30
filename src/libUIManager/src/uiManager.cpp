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

/**************** MPEG-H 3DA User Interaction manager library ******************

   Author(s):   Matthias Neusinger

   Description: MPEG-H UI Manager

*******************************************************************************/

#include "uiManager.h"

#include "uiManagerInternal.h"
#include "uiXml.h"
#include "uiPersistence.h"

static void reset(HANDLE_UI_MANAGER hUiManager, int keepSwitchGroups);
static UI_MANAGER_ERROR selectSwitchGroup(HANDLE_UI_MANAGER hUiManager, int switchGrpIdx,
                                          UCHAR groupID);
static UI_MANAGER_ERROR applyPreset(HANDLE_UI_MANAGER hUiManager, UCHAR presetID);
static void persistenceRestore(HANDLE_UI_MANAGER hUiManager);

/* set group on/off */
static UI_MANAGER_ERROR setGroupOnOff(HANDLE_UI_MANAGER hUiManager, UCHAR groupID, UCHAR onOff) {
  /* get group index */
  int grpIdx = asiGroupID2idx(&hUiManager->asi, groupID);
  if (grpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowOnOff) return UI_MANAGER_NOT_ALLOWED;
  if (hUiManager->asi.groups[grpIdx].switchGroupID != INVALID_ID) return UI_MANAGER_NOT_ALLOWED;

  /* set on/off */
  hUiManager->uiState.groups[grpIdx].onOff = onOff;

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set switch group on/off */
static UI_MANAGER_ERROR setSwitchGroupOnOff(HANDLE_UI_MANAGER hUiManager, UCHAR switchGroupID,
                                            UCHAR onOff) {
  int swgrpIdx, grpIdx, memberIdx;

  /* get switch group index */
  swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, switchGroupID);
  if (swgrpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* get active group index */
  memberIdx = hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex;
  grpIdx =
      asiGroupID2idx(&hUiManager->asi, hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
  if (grpIdx < 0) return UI_MANAGER_INVALID_STATE;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowOnOff) return UI_MANAGER_NOT_ALLOWED;

  /* set on/off */
  hUiManager->uiState.switchGroups[swgrpIdx].onOff = onOff;
  hUiManager->uiState.groups[grpIdx].onOff = onOff;

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set group gain */
static UI_MANAGER_ERROR setGroupGain(HANDLE_UI_MANAGER hUiManager, UCHAR groupID, SHORT gain) {
  /* get group index */
  int grpIdx = asiGroupID2idx(&hUiManager->asi, groupID);
  if (grpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowGainInteractivity) return UI_MANAGER_NOT_ALLOWED;
  if (hUiManager->asi.groups[grpIdx].switchGroupID != INVALID_ID) return UI_MANAGER_NOT_ALLOWED;

  /* limit to allowed range */
  if (hUiManager->asi.groups[grpIdx].interactivityMinGain) {
    gain =
        fMax(gain, (SHORT)(((SHORT)hUiManager->asi.groups[grpIdx].interactivityMinGain - 63) << 1));
  }
  gain = fMin(gain, (SHORT)(hUiManager->asi.groups[grpIdx].interactivityMaxGain << 1));

  /* set gain */
  hUiManager->uiState.groups[grpIdx].gain = gain;

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set switch group gain */
static UI_MANAGER_ERROR setSwitchGroupGain(HANDLE_UI_MANAGER hUiManager, UCHAR switchGroupID,
                                           SHORT gain) {
  int swgrpIdx, grpIdx, memberIdx;

  /* get switch group index */
  swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, switchGroupID);
  if (swgrpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* get active group index */
  memberIdx = hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex;
  grpIdx =
      asiGroupID2idx(&hUiManager->asi, hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
  if (grpIdx < 0) return UI_MANAGER_INVALID_STATE;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowGainInteractivity) return UI_MANAGER_NOT_ALLOWED;

  /* set gain for all members */
  for (memberIdx = 0; memberIdx < hUiManager->asi.switchGroups[swgrpIdx].numMembers; memberIdx++) {
    SHORT tmpGain = gain;

    grpIdx = asiGroupID2idx(&hUiManager->asi,
                            hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
    if (grpIdx < 0) continue;

    /* check if allowed */
    if (!hUiManager->asi.groups[grpIdx].allowGainInteractivity) continue;

    /* limit to allowed range */
    if (hUiManager->asi.groups[grpIdx].interactivityMinGain) {
      tmpGain =
          fMax(tmpGain, (SHORT)((hUiManager->asi.groups[grpIdx].interactivityMinGain - 63) << 1));
    }
    tmpGain = fMin(tmpGain, (SHORT)(hUiManager->asi.groups[grpIdx].interactivityMaxGain << 1));

    hUiManager->uiState.groups[grpIdx].gain = tmpGain;
  }

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set group azimuth offset */
static UI_MANAGER_ERROR setGroupAzOffset(HANDLE_UI_MANAGER hUiManager, UCHAR groupID,
                                         UCHAR azOffset) {
  /* get group index */
  int grpIdx = asiGroupID2idx(&hUiManager->asi, groupID);
  if (grpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowPositionInteractivity) return UI_MANAGER_NOT_ALLOWED;
  if (hUiManager->asi.groups[grpIdx].switchGroupID != INVALID_ID) return UI_MANAGER_NOT_ALLOWED;

  /* limit to allowed range */
  azOffset = fMax(azOffset, (UCHAR)(128 - hUiManager->asi.groups[grpIdx].interactivityMinAzOffset));
  azOffset = fMin(azOffset, (UCHAR)(128 + hUiManager->asi.groups[grpIdx].interactivityMaxAzOffset));

  /* set offset */
  hUiManager->uiState.groups[grpIdx].azOffset = azOffset;

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set switch group azimuth offset */
static UI_MANAGER_ERROR setSwitchGroupAzOffset(HANDLE_UI_MANAGER hUiManager, UCHAR switchGroupID,
                                               UCHAR azOffset) {
  int swgrpIdx, grpIdx, memberIdx;

  /* get switch group index */
  swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, switchGroupID);
  if (swgrpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* get active group index */
  memberIdx = hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex;
  grpIdx =
      asiGroupID2idx(&hUiManager->asi, hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
  if (grpIdx < 0) return UI_MANAGER_INVALID_STATE;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowPositionInteractivity) return UI_MANAGER_NOT_ALLOWED;

  /* set offset for all members */
  for (memberIdx = 0; memberIdx < hUiManager->asi.switchGroups[swgrpIdx].numMembers; memberIdx++) {
    UCHAR tmpAzOffset = azOffset;

    grpIdx = asiGroupID2idx(&hUiManager->asi,
                            hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
    if (grpIdx < 0) continue;

    /* check if allowed */
    if (!hUiManager->asi.groups[grpIdx].allowPositionInteractivity) continue;

    /* limit to allowed range */
    tmpAzOffset =
        fMax(tmpAzOffset, (UCHAR)(128 - hUiManager->asi.groups[grpIdx].interactivityMinAzOffset));
    tmpAzOffset =
        fMin(tmpAzOffset, (UCHAR)(128 + hUiManager->asi.groups[grpIdx].interactivityMaxAzOffset));

    hUiManager->uiState.groups[grpIdx].azOffset = tmpAzOffset;
  }

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set group elevation offset */
static UI_MANAGER_ERROR setGroupElOffset(HANDLE_UI_MANAGER hUiManager, UCHAR groupID,
                                         UCHAR elOffset) {
  /* get group index */
  int grpIdx = asiGroupID2idx(&hUiManager->asi, groupID);
  if (grpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowPositionInteractivity) return UI_MANAGER_NOT_ALLOWED;
  if (hUiManager->asi.groups[grpIdx].switchGroupID != INVALID_ID) return UI_MANAGER_NOT_ALLOWED;

  /* limit to allowed range */
  elOffset = fMax(elOffset, (UCHAR)(32 - hUiManager->asi.groups[grpIdx].interactivityMinElOffset));
  elOffset = fMin(elOffset, (UCHAR)(32 + hUiManager->asi.groups[grpIdx].interactivityMaxElOffset));

  /* set offset */
  hUiManager->uiState.groups[grpIdx].elOffset = elOffset;

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* set switch group elevation offset */
static UI_MANAGER_ERROR setSwitchGroupElOffset(HANDLE_UI_MANAGER hUiManager, UCHAR switchGroupID,
                                               UCHAR elOffset) {
  int swgrpIdx, grpIdx, memberIdx;

  /* get switch group index */
  swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, switchGroupID);
  if (swgrpIdx < 0) return UI_MANAGER_INVALID_PARAM;

  /* get active group index */
  memberIdx = hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex;
  grpIdx =
      asiGroupID2idx(&hUiManager->asi, hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
  if (grpIdx < 0) return UI_MANAGER_INVALID_STATE;

  /* check if allowed */
  if (!hUiManager->uiState.groups[grpIdx].allowPositionInteractivity) return UI_MANAGER_NOT_ALLOWED;

  /* set offset for all members */
  for (memberIdx = 0; memberIdx < hUiManager->asi.switchGroups[swgrpIdx].numMembers; memberIdx++) {
    UCHAR tmpElOffset = elOffset;

    grpIdx = asiGroupID2idx(&hUiManager->asi,
                            hUiManager->asi.switchGroups[swgrpIdx].memberID[memberIdx]);
    if (grpIdx < 0) continue;

    /* check if allowed */
    if (!hUiManager->asi.groups[grpIdx].allowPositionInteractivity) continue;

    /* limit to allowed range */
    tmpElOffset =
        fMax(tmpElOffset, (UCHAR)(32 - hUiManager->asi.groups[grpIdx].interactivityMinElOffset));
    tmpElOffset =
        fMin(tmpElOffset, (UCHAR)(32 + hUiManager->asi.groups[grpIdx].interactivityMaxElOffset));

    hUiManager->uiState.groups[grpIdx].elOffset = tmpElOffset;
  }

  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* select audio language */
static void selectAudioLang(HANDLE_UI_MANAGER hUiManager) {
  int swgrpIdx, grpId, grpIdx, memberIdx;

  for (swgrpIdx = 0; swgrpIdx < hUiManager->asi.numSwitchGroups; swgrpIdx++) {
    ASI_SWITCH_GROUP* pSwitchGroup = &(hUiManager->asi.switchGroups[swgrpIdx]);
    ASI_GROUP* pGroup;
    int prio, currPrio = NUM_PREF_LANGUAGES, selGrpId = pSwitchGroup->defaultGroupID;
    int hasLang = 0;

    grpId = pSwitchGroup->memberID[hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex];
    grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
    if (grpIdx < 0) continue;
    pGroup = &(hUiManager->asi.groups[grpIdx]);

    for (memberIdx = 0; memberIdx < pSwitchGroup->numMembers; memberIdx++) {
      grpId = pSwitchGroup->memberID[memberIdx];
      grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
      if (grpIdx < 0) continue;
      pGroup = &(hUiManager->asi.groups[grpIdx]);

      if (pGroup->contPresent && pGroup->contentData.contentLanguage[0]) {
        hasLang = 1;

        /* get language priority of member */
        for (prio = 0; prio < NUM_PREF_LANGUAGES; prio++) {
          if (FDKstrncmp(pGroup->contentData.contentLanguage,
                         &hUiManager->uiState.prefAudioLanguages[prio][0], 3) == 0)
            break;
        }

        /* select member if it has higher priority */
        if (prio < currPrio) {
          selGrpId = grpId;
          currPrio = prio;
        }
      }
    }

    if (hasLang && (selGrpId != INVALID_ID)) {
      selectSwitchGroup(hUiManager, swgrpIdx, selGrpId);
      hUiManager->xmlStateChanged = 1;
    }
  }
}

static int setAccessibilityIndex(HANDLE_UI_MANAGER hUiManager, const UCHAR accessibilityIndex,
                                 UCHAR* groupPresetId) {
  int i;
  int found = 0, minId = 256;
  int map[NUM_ACCESSIBILITY_MODES] = {
      0, 7, 5, 6, 7}; /* map 0..4 to corresponding mae_groupPresetKind for none, visual_impaired,
                         light_hearing_impaired, heavy_hearing_impaired, visual_impaired */

  for (i = 0; i < hUiManager->asi.numGroupPresets; i++) {
    if (hUiManager->asi.groupPresets[i].kind == map[accessibilityIndex]) {
      if (hUiManager->asi.groupPresets[i].groupPresetID < minId) {
        minId = hUiManager->asi.groupPresets[i].groupPresetID;
        found = 1;
      }
    }
  }

  if (found) *groupPresetId = minId;

  return found;
}

/* get presetId according to accessibility preference index */
static int getAccessibilityPreset(HANDLE_UI_MANAGER hUiManager, UCHAR accessibilityIndex,
                                  UCHAR* groupPresetId) {
  *groupPresetId = getMinPresetID(hUiManager); /* initialize with default preset, which is the
                                                  preset with the lowest presetId */

  if (!accessibilityIndex) {
    return asiGroupPresetID2idx(&hUiManager->asi, *groupPresetId) >= 0;
  }

  int retVal = setAccessibilityIndex(hUiManager, accessibilityIndex, groupPresetId);

  /* try again with heavy hearing impaired setting */
  if ((retVal == 0) && ((accessibilityIndex == 2) || (accessibilityIndex == 4))) {
    retVal = setAccessibilityIndex(hUiManager, 3, groupPresetId);
  }

  /* try again with light hearing impaired setting */
  if ((retVal == 0) && ((accessibilityIndex == 3) || (accessibilityIndex == 4))) {
    retVal = setAccessibilityIndex(hUiManager, 2, groupPresetId);
  }

  return retVal;
}

/* reset state to default */
static void resetAvailability(HANDLE_UI_MANAGER hUiManager) {
  const AUDIO_SCENE_INFO* pASI = &hUiManager->asi;
  int i, memberIdx;

  FDK_ASSERT(hUiManager->asi.numGroups <= ASI_MAX_GROUPS);

  /* groups */
  for (i = 0; i < hUiManager->asi.numGroups; i++) {
    UI_STATE_GROUP* pGroupState = &(hUiManager->uiState.groups[i]);

    pGroupState->isAvailable = hUiManager->asi.groups[i].isAvailable;
  }

  /* switch groups */
  for (i = 0; i < hUiManager->asi.numSwitchGroups; i++) {
    UI_STATE_SWITCH_GROUP* pSwitchGroupState = &(hUiManager->uiState.switchGroups[i]);

    pSwitchGroupState->isAvailable = 1;

    for (memberIdx = 0; memberIdx < hUiManager->asi.switchGroups[i].numMembers; memberIdx++) {
      int grpId = hUiManager->asi.switchGroups[i].memberID[memberIdx];
      int grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
      if (grpIdx < 0) continue;

      /* Check if all members are available. */
      if (hUiManager->asi.groups[grpIdx].isAvailable == 0) {
        pSwitchGroupState->isAvailable = 0;
      }
    }
  }

  /* presets */
  for (int p = 0; p < hUiManager->asi.numGroupPresets; p++) {
    hUiManager->uiState.groupPresets[p].isAvailable = 1;

    for (memberIdx = 0; memberIdx < hUiManager->asi.groupPresets[p].numConditions; memberIdx++) {
      int numConditions;
      const ASI_GROUP_PRESET_CONDITION* conditions;

      if (pASI->groupPresets[memberIdx].hasDownmixIdExtension) {
        numConditions = pASI->groupPresets[memberIdx].downmixIdExtension.numConditions;
        conditions = pASI->groupPresets[memberIdx].downmixIdExtension.conditions;
      } else {
        numConditions = pASI->groupPresets[memberIdx].numConditions;
        conditions = pASI->groupPresets[memberIdx].conditions;
      }
      /* check conditions */
      for (i = 0; i < numConditions; i++) {
        if (!conditions[i].isSwitchGroupCondition) { /* group condition */
          int grpIdx = asiGroupID2idx(pASI, conditions[i].referenceID);
          if (grpIdx < 0) continue;

          if (hUiManager->uiState.groups[grpIdx].isAvailable == 0) {
            hUiManager->uiState.groupPresets[p].isAvailable = 0;
          }
        } else { /* switch group condition */
          int swGrpIdx = asiSwitchGroupID2idx(pASI, conditions[i].referenceID);
          if (swGrpIdx < 0) continue;

          if (hUiManager->uiState.switchGroups[swGrpIdx].isAvailable == 0) {
            hUiManager->uiState.groupPresets[p].isAvailable = 0;
          }
        }
      }
    }
  }

  hUiManager->xmlStateChanged = 1;
  hUiManager->uiStateChanged = 1;
  hUiManager->configChanged = 1;
}

/* reset state to default */
static void reset(HANDLE_UI_MANAGER hUiManager, int keepSwitchGroups) {
  const AUDIO_SCENE_INFO* pASI = &hUiManager->asi;
  int i, memberIdx;

  FDK_ASSERT(hUiManager->asi.numGroups <= ASI_MAX_GROUPS);

  /* groups */
  for (i = 0; i < hUiManager->asi.numGroups; i++) {
    UI_STATE_GROUP* pGroupState = &(hUiManager->uiState.groups[i]);

    /* on/off*/
    pGroupState->onOff = hUiManager->asi.groups[i].defaultOnOff;
    pGroupState->allowOnOff = hUiManager->asi.groups[i].allowOnOff;

    /* gain */
    pGroupState->gain = 0;
    pGroupState->defaultGain = 0;
    pGroupState->allowGainInteractivity = hUiManager->asi.groups[i].allowGainInteractivity;

    /* position */
    pGroupState->azOffset = 128;
    pGroupState->elOffset = 32;
    pGroupState->distFactor = 12;
    pGroupState->defaultAzOffset = 128;
    pGroupState->defaultElOffset = 32;
    pGroupState->defaultDistFactor = 12;
    pGroupState->allowPositionInteractivity = hUiManager->asi.groups[i].allowPositionInteractivity;

    pGroupState->isAvailable = hUiManager->asi.groups[i].isAvailable;
  }

  /* switch groups */
  for (i = 0; i < hUiManager->asi.numSwitchGroups; i++) {
    UI_STATE_SWITCH_GROUP* pSwitchGroupState = &(hUiManager->uiState.switchGroups[i]);

    pSwitchGroupState->onOff = hUiManager->asi.switchGroups[i].defaultOnOff;
    pSwitchGroupState->allowSwitch = 1;

    if (!keepSwitchGroups) {
      pSwitchGroupState->activeMemberIndex = 0;
      selectSwitchGroup(hUiManager, i, hUiManager->asi.switchGroups[i].defaultGroupID);
    } else {
      selectSwitchGroup(
          hUiManager, i,
          hUiManager->asi.switchGroups[i].memberID[pSwitchGroupState->activeMemberIndex]);
    }

    pSwitchGroupState->isAvailable = 1;

    for (memberIdx = 0; memberIdx < hUiManager->asi.switchGroups[i].numMembers; memberIdx++) {
      int grpId = hUiManager->asi.switchGroups[i].memberID[memberIdx];
      int grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
      if (grpIdx < 0) continue;

      hUiManager->uiState.groups[grpIdx].allowOnOff = hUiManager->asi.switchGroups[i].allowOnOff;

      /* Check if all members are available. */
      if (hUiManager->asi.groups[grpIdx].isAvailable == 0) {
        pSwitchGroupState->isAvailable = 0;
      }
    }
  }

  /* presets */
  for (int p = 0; p < hUiManager->asi.numGroupPresets; p++) {
    hUiManager->uiState.groupPresets[p].isAvailable = 1;

    for (memberIdx = 0; memberIdx < hUiManager->asi.groupPresets[p].numConditions; memberIdx++) {
      int numConditions;
      const ASI_GROUP_PRESET_CONDITION* conditions;

      if (pASI->groupPresets[memberIdx].hasDownmixIdExtension) {
        numConditions = pASI->groupPresets[memberIdx].downmixIdExtension.numConditions;
        conditions = pASI->groupPresets[memberIdx].downmixIdExtension.conditions;
      } else {
        numConditions = pASI->groupPresets[memberIdx].numConditions;
        conditions = pASI->groupPresets[memberIdx].conditions;
      }
      /* check conditions */
      for (i = 0; i < numConditions; i++) {
        if (!conditions[i].isSwitchGroupCondition) { /* group condition */
          int grpIdx = asiGroupID2idx(pASI, conditions[i].referenceID);
          if (grpIdx < 0) continue;

          if (hUiManager->uiState.groups[grpIdx].isAvailable == 0) {
            hUiManager->uiState.groupPresets[p].isAvailable = 0;
          }
        } else { /* switch group condition */
          int swGrpIdx = asiSwitchGroupID2idx(pASI, conditions[i].referenceID);
          if (swGrpIdx < 0) continue;

          if (hUiManager->uiState.switchGroups[swGrpIdx].isAvailable == 0) {
            hUiManager->uiState.groupPresets[p].isAvailable = 0;
          }
        }
      }
    }
  }

  hUiManager->uiState.activePresetIndex = INVALID_IDX;

  if (!keepSwitchGroups) {
    /* select audio language */
    selectAudioLang(hUiManager);
  }

  hUiManager->xmlStateChanged = 1;
  hUiManager->uiStateChanged = 1;
}

/* compare asi group properties: 0 = no diff, 1 = different */
static int cmpGroupProperties(AUDIO_SCENE_INFO* asi, int groupIndex1, int groupIndex2) {
  ASI_GROUP* grp1;
  ASI_GROUP* grp2;

  if (asi == NULL || (groupIndex1 == groupIndex2) || (groupIndex1 >= asi->numGroups) ||
      (groupIndex2 >= asi->numGroups))
    return 0;

  grp1 = &(asi->groups[groupIndex1]);
  grp2 = &(asi->groups[groupIndex2]);

  if (grp1->allowPositionInteractivity != grp2->allowPositionInteractivity) return 1;
  if (grp1->allowGainInteractivity != grp2->allowGainInteractivity) return 1;

  /* ignore allowOnOff and defaultOnOff because this is not relevant for switchgroup members */

  if (grp1->interactivityMinAzOffset != grp2->interactivityMinAzOffset) return 1;
  if (grp1->interactivityMaxAzOffset != grp2->interactivityMaxAzOffset) return 1;
  if (grp1->interactivityMinElOffset != grp2->interactivityMinElOffset) return 1;
  if (grp1->interactivityMaxElOffset != grp2->interactivityMaxElOffset) return 1;
  if (grp1->interactivityMinDistFactor != grp2->interactivityMinDistFactor) return 1;
  if (grp1->interactivityMaxDistFactor != grp2->interactivityMaxDistFactor) return 1;
  if (grp1->interactivityMinGain != grp2->interactivityMinGain) return 1;
  if (grp1->interactivityMaxGain != grp2->interactivityMaxGain) return 1;

  return 0;
}

/* select active switch group member */
static UI_MANAGER_ERROR selectSwitchGroup(HANDLE_UI_MANAGER hUiManager, int switchGrpIdx,
                                          UCHAR groupID) {
  /* get switch group pointer */
  ASI_SWITCH_GROUP* switchGroup = &(hUiManager->asi.switchGroups[switchGrpIdx]);
  int i, found = 0;
  int grpIdx = 0;
  int prevGrpIdx = 0;

  /* check if allowed */
  if (!hUiManager->uiState.switchGroups[switchGrpIdx].allowSwitch) return UI_MANAGER_NOT_ALLOWED;

  /* get currently active group id */
  prevGrpIdx = asiGroupID2idx(
      &hUiManager->asi,
      switchGroup->memberID[hUiManager->uiState.switchGroups[switchGrpIdx].activeMemberIndex]);
  if (prevGrpIdx < 0) return UI_MANAGER_INVALID_STATE;

  /* find member to be activated */
  for (i = 0; i < switchGroup->numMembers; i++) {
    /* get group index */
    grpIdx = asiGroupID2idx(&hUiManager->asi, switchGroup->memberID[i]);
    if (grpIdx < 0) continue;

    /* turn selected member on, all others off */
    if (switchGroup->memberID[i] == groupID) {
      hUiManager->uiState.switchGroups[switchGrpIdx].activeMemberIndex = i;
      hUiManager->uiState.groups[grpIdx].onOff =
          hUiManager->uiState.switchGroups[switchGrpIdx].onOff;
      found = 1;
    } else {
      hUiManager->uiState.groups[grpIdx].onOff = 0;
    }
  }

  /* if ID not found */
  if (!found) {
    if (switchGroup->allowOnOff) {
      /* switch group turned off */
      hUiManager->uiState.switchGroups[switchGrpIdx].onOff = 0;
    } else {
      /* turn previous active member back on */
      int grpId =
          switchGroup->memberID[hUiManager->uiState.switchGroups[switchGrpIdx].activeMemberIndex];
      grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
      hUiManager->uiState.groups[grpIdx].onOff = 1;
    }
  }

  /* get active group id */
  grpIdx = asiGroupID2idx(
      &hUiManager->asi,
      switchGroup->memberID[hUiManager->uiState.switchGroups[switchGrpIdx].activeMemberIndex]);
  if (grpIdx < 0) return UI_MANAGER_INVALID_STATE;

  if (cmpGroupProperties(&hUiManager->asi, prevGrpIdx, grpIdx)) {
    hUiManager->xmlStateChanged = 1;
  }

  hUiManager->uiStateChanged = 1;

  if (!found) return UI_MANAGER_INVALID_PARAM;

  return UI_MANAGER_OK;
}

/* apply preset condition to group */
static void applyConditionToGroup(HANDLE_UI_MANAGER hUiManager,
                                  ASI_GROUP_PRESET_CONDITION* pCondition, int grpIdx) {
  /* on/off (do not set for switch group members)  */
  if (hUiManager->asi.groups[grpIdx].switchGroupID == INVALID_ID) {
    hUiManager->uiState.groups[grpIdx].onOff = pCondition->conditionOnOff;
  }

  /* disable on/off */
  hUiManager->uiState.groups[grpIdx].allowOnOff = 0;

  if (pCondition->conditionOnOff) {
    /* gain interactivity */
    if (pCondition->disableGainInteractivity) {
      hUiManager->uiState.groups[grpIdx].allowGainInteractivity = 0;
    }

    /* default gain */
    if (pCondition->gainFlag) {
      hUiManager->uiState.groups[grpIdx].gain = hUiManager->uiState.groups[grpIdx].defaultGain =
          ((SHORT)pCondition->gain - 255) + 64;
    }

    /* position interactivity */
    if (pCondition->disablePositionInteractivity) {
      hUiManager->uiState.groups[grpIdx].allowPositionInteractivity = 0;
    }

    /* default position */
    if (pCondition->positionFlag) {
      hUiManager->uiState.groups[grpIdx].azOffset =
          hUiManager->uiState.groups[grpIdx].defaultAzOffset = pCondition->azOffset;
      hUiManager->uiState.groups[grpIdx].elOffset =
          hUiManager->uiState.groups[grpIdx].defaultElOffset = pCondition->elOffset;
      hUiManager->uiState.groups[grpIdx].distFactor =
          hUiManager->uiState.groups[grpIdx].defaultDistFactor = pCondition->distFactor;
    }
  }
}

/* apply preset */
static UI_MANAGER_ERROR applyPreset(HANDLE_UI_MANAGER hUiManager, UCHAR presetID) {
  ASI_GROUP_PRESET* preset;
  ASI_GROUP_PRESET_CONDITION* conditions;

  int i, numConditions;

  if (presetID == PRESET_ID_AUTO) {
    /* find appropriate preset (default preset is returned for accessibility preference 0) */
    getAccessibilityPreset(hUiManager, hUiManager->uiState.accessibilityPreference, &presetID);
  }

  /* get preset index and pointer */
  int presetIdx = asiGroupPresetID2idx(&hUiManager->asi, presetID);
  if (presetIdx < 0) return UI_MANAGER_INVALID_PARAM;

  preset = &hUiManager->asi.groupPresets[presetIdx];

  /* set active preset index */
  hUiManager->uiState.activePresetIndex = presetIdx;

  /* replace conditions if downmixIdGroupPresetExtension applies */
  if (preset->hasDownmixIdExtension) {
    numConditions = preset->downmixIdExtension.numConditions;
    conditions = preset->downmixIdExtension.conditions;
  } else {
    numConditions = preset->numConditions;
    conditions = preset->conditions;
  }

  /* apply preset conditions */
  for (i = 0; i < numConditions; i++) {
    if (!conditions[i].isSwitchGroupCondition) { /* apply group condition */
      /* get group index */
      int grpIdx = asiGroupID2idx(&hUiManager->asi, conditions[i].referenceID);
      if (grpIdx < 0) continue;

      /* check if group is member of a switch group */
      if (hUiManager->asi.groups[grpIdx].switchGroupID == INVALID_ID) {
        applyConditionToGroup(hUiManager, &(conditions[i]), grpIdx);
      } else {
        int swgrpIdx;
        ASI_SWITCH_GROUP* switchGroup;
        int grpId, memberIdx;

        /* get switch group index and pointer */
        swgrpIdx =
            asiSwitchGroupID2idx(&hUiManager->asi, hUiManager->asi.groups[grpIdx].switchGroupID);
        if (swgrpIdx < 0) continue;
        switchGroup = &(hUiManager->asi.switchGroups[swgrpIdx]);

        /* apply condition to switch group */
        if (conditions[i].conditionOnOff) {
          /* on/off */
          hUiManager->uiState.switchGroups[swgrpIdx].onOff = 1;

          /* disable switch */
          hUiManager->uiState.switchGroups[swgrpIdx].allowSwitch = 0;

          /* apply condition to all switch group members */
          for (memberIdx = 0; memberIdx < switchGroup->numMembers; memberIdx++) {
            grpId = switchGroup->memberID[memberIdx];

            /* get group index */
            grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
            if (grpIdx < 0) continue;

            /* on/off */
            if (grpId == conditions[i].referenceID) {
              hUiManager->uiState.groups[grpIdx].onOff = 1;
              hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex = memberIdx;
            } else {
              hUiManager->uiState.groups[grpIdx].onOff = 0;
            }

            applyConditionToGroup(hUiManager, &(conditions[i]), grpIdx);
          }
        }
      }
    } else { /* switch group condition */
      int swgrpIdx;
      ASI_SWITCH_GROUP* switchGroup;
      int grpId, grpIdx, memberIdx;

      /* get switch group index and pointer */
      swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, conditions[i].referenceID);
      if (swgrpIdx < 0) continue;
      switchGroup = &(hUiManager->asi.switchGroups[swgrpIdx]);

      /* on/off */
      hUiManager->uiState.switchGroups[swgrpIdx].onOff = conditions[i].conditionOnOff;

      /* set active member on/off */
      grpId = switchGroup->memberID[hUiManager->uiState.switchGroups[swgrpIdx].activeMemberIndex];
      grpIdx = asiGroupID2idx(&hUiManager->asi, grpId);
      if (grpIdx < 0) continue;
      hUiManager->uiState.groups[grpIdx].onOff = conditions[i].conditionOnOff;

      /* apply condition to members */
      for (memberIdx = 0; memberIdx < switchGroup->numMembers; memberIdx++) {
        /* get group index */
        grpIdx = asiGroupID2idx(&hUiManager->asi, switchGroup->memberID[memberIdx]);
        if (grpIdx < 0) continue;

        applyConditionToGroup(hUiManager, &(conditions[i]), grpIdx);
      }
    }
  }

  hUiManager->xmlStateChanged = 1;
  hUiManager->uiStateChanged = 1;

  return UI_MANAGER_OK;
}

/* check/update state */
static void update(HANDLE_UI_MANAGER hUiManager) {
  if (hUiManager->asi.diffFlags == ASI_DIFF_AVAILABILITY) {
    resetAvailability(hUiManager);

  } else if (hUiManager->asi.diffFlags == ASI_DIFF_DESCRIPTION) {
    hUiManager->xmlStateChanged = 1;
  } else if (hUiManager->asi.diffFlags) {
    int i;

    for (i = 0; i < 15; i++) hUiManager->uiState.uuid[i] = 0;

    /* set last byte of UUID to ASI ID (0..255) */
    hUiManager->uiState.uuid[15] = hUiManager->asi.audioSceneInfoID;

    /* xor upper 16 bits of UUID with partial ASI CRC */
    hUiManager->uiState.uuid[0] ^= hUiManager->asi.crcForUid >> 8;
    hUiManager->uiState.uuid[1] ^= hUiManager->asi.crcForUid & 0xFF;

    /* reset */
    reset(hUiManager, 0);
    applyPreset(hUiManager, PRESET_ID_AUTO);

    /* restore state */
    persistenceRestore(hUiManager);

    hUiManager->configChanged = 1;
  }

  hUiManager->asi.diffFlags = 0;
}

/* compare UUID */
static int compUUID(const UCHAR* uuid1, const UCHAR* uuid2) {
  int i;

  for (i = 0; i < 16; i++) {
    if (uuid1[i] != uuid2[i]) return 0;
  }

  return 1;
}

/* apply default fallbacks for requested DRC effect types */
static ULONG getDrcEffectTypeFallback(const LONG drcEffectTypeRequested) {
  ULONG retVal = (ULONG)drcEffectTypeRequested;
  if (retVal <= 0xF) {
    switch (drcEffectTypeRequested) {
      case 0x00000000: /* NONE */
        retVal = 0x00000000;
        break;

      case 0x00000001: /* NIGHT */
        retVal = 0x00543261;
        break;

      case 0x00000002: /* NOISY */
        retVal = 0x00543162;
        break;

      case 0x00000003: /* LIMITED */
        retVal = 0x00542163;
        break;

      case 0x00000004: /* LOWLEVEL */
        retVal = 0x00531264;
        break;

      case 0x00000005: /* DIALOG */
        retVal = 0x00432165;
        break;

      case 0x00000006: /* GENERAL */
        retVal = 0x00543216;
        break;

      default:
        retVal = 0x00000000;
        break;
    }
  }

  return retVal;
}

/* check validity of DRC effect type fallback code */
static int checkDrcEffectTypeFallback(const LONG drcEffectTypeRequested) {
  ULONG ulDrcEffectTypeRequested = (ULONG)drcEffectTypeRequested;
  int i;
  if (ulDrcEffectTypeRequested == UI_MANAGER_DRC_OFF) return 0;
  if (ulDrcEffectTypeRequested == UI_MANAGER_USE_DEFAULT_DRC_SELECTED) return 0;
  for (i = 0; i < 8; i++) { /* Each byte, representing a fallback DRC effect type, may not be >=
                               EFFECT_TYPE_REQUESTED_COUNT (9) */
    if (((ulDrcEffectTypeRequested >> (4 * i)) & 0xF) >= 9) return -1;
  }
  return 0;
}

/* get lowest groupPresetID */
int getMinPresetID(HANDLE_UI_MANAGER hUiManager) {
  int i;
  int minID = 31;

  if (hUiManager->asi.numGroupPresets == 0) return 0;

  for (i = 0; i < hUiManager->asi.numGroupPresets; i++) {
    if (hUiManager->asi.groupPresets[i].groupPresetID < minID) {
      minID = hUiManager->asi.groupPresets[i].groupPresetID;
    }
  }
  return minID;
}

/* perform UI action */
static UI_MANAGER_ERROR performAction(HANDLE_UI_MANAGER hUiManager,
                                      const UI_MANAGER_ACTION* action) {
  UI_MANAGER_ERROR err = UI_MANAGER_OK;

  switch (action->actionType) {
    case UI_MANAGER_COMMAND_RESET:
      reset(hUiManager, 0);
      applyPreset(hUiManager, PRESET_ID_AUTO);
      break;

    case UI_MANAGER_COMMAND_DRC_SELECTED:
      if (!(action->presentFlags & FLAG_XML_PARAM_INT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if (checkDrcEffectTypeFallback(action->paramInt)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      hUiManager->uiState.drcSelected = getDrcEffectTypeFallback(action->paramInt);
      hUiManager->drcStateChanged = 1;
      break;

    case UI_MANAGER_COMMAND_DRC_BOOST:
      if (!(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if ((action->paramFloat < (FIXP_DBL)0) || (action->paramFloat > (FIXP_DBL)(1 << 16))) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      hUiManager->uiState.boost = (SHORT)(LONG)(action->paramFloat >> 2); /* Q16 to Q14 */
      hUiManager->drcStateChanged = 1;
      break;

    case UI_MANAGER_COMMAND_DRC_COMPRESS:
      if (!(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if ((action->paramFloat < (FIXP_DBL)0) || (action->paramFloat > (FIXP_DBL)(1 << 16))) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      hUiManager->uiState.compress = (SHORT)(LONG)(action->paramFloat >> 2); /* Q16 to Q14 */
      hUiManager->drcStateChanged = 1;
      break;

    case UI_MANAGER_COMMAND_TARGET_LOUDNESS:
      if (!(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if ((action->paramFloat < (FIXP_DBL)(-63 * (1 << 16))) ||
          (action->paramFloat > (FIXP_DBL)(-10 * (1 << 16)))) {
        if (action->paramFloat != (FIXP_DBL)(UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS * (1 << 16))) {
          err = UI_MANAGER_INVALID_PARAM;
          break;
        }
      }
      hUiManager->uiState.targetLoudness = (SCHAR)(LONG)(action->paramFloat >> 16);
      hUiManager->drcStateChanged = 1;
      break;

    case UI_MANAGER_COMMAND_ALBUM_MODE:
      if (!(action->presentFlags & FLAG_XML_PARAM_BOOL)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      hUiManager->uiState.albumMode = (SCHAR)action->paramBool;
      hUiManager->drcStateChanged = 1;
      break;

    case UI_MANAGER_COMMAND_PRESET_SELECTED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if (asiGroupPresetID2idx(&hUiManager->asi, action->paramInt) < 0) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      reset(hUiManager, 0);
      err = applyPreset(hUiManager, (UCHAR)action->paramInt);
      break;

    case UI_MANAGER_COMMAND_PRESET_SELECTED_NO_UUID:
      if (!(action->presentFlags & FLAG_XML_PARAM_INT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if (asiGroupPresetID2idx(&hUiManager->asi, action->paramInt) < 0) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      reset(hUiManager, 0);
      err = applyPreset(hUiManager, (UCHAR)action->paramInt);
      break;

    case UI_MANAGER_COMMAND_ACCESSIBILITY_PREFERENCE:
      if (!(action->presentFlags & FLAG_XML_PARAM_INT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      if ((action->paramInt < 0) || (action->paramInt >= NUM_ACCESSIBILITY_MODES)) {
        err = UI_MANAGER_INVALID_PARAM;
      } else {
        UCHAR groupPresetId;
        hUiManager->uiState.accessibilityPreference = (UCHAR)action->paramInt;
        getAccessibilityPreset(hUiManager, hUiManager->uiState.accessibilityPreference,
                               &groupPresetId);
        reset(hUiManager, 0);
        applyPreset(hUiManager, groupPresetId);
        hUiManager->drcStateChanged = 1;
      }
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_MUTING_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_BOOL)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setGroupOnOff(hUiManager, (UCHAR)action->paramInt, !action->paramBool);
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_BALANCE_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setGroupGain(hUiManager, (UCHAR)action->paramInt,
                         (SHORT)(LONG)(action->paramFloat >> 15));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_AZIMUTH_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setGroupAzOffset(hUiManager, (UCHAR)action->paramInt,
                             (UCHAR)(((action->paramFloat * 2 / 3) >> 16) + 128));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_ELEVATION_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setGroupElOffset(hUiManager, (UCHAR)action->paramInt,
                             (UCHAR)((((LONG)action->paramFloat / 3) >> 16) + 32));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_DISTANCE_CHANGED:
      /* currently not supported */
      return UI_MANAGER_INVALID_PARAM;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_SELECTED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      } else {
        int swgrpIdx;

        swgrpIdx = asiSwitchGroupID2idx(&hUiManager->asi, action->paramInt);
        if (swgrpIdx < 0) return UI_MANAGER_INVALID_PARAM;

        err = selectSwitchGroup(hUiManager, swgrpIdx, (UCHAR)(LONG)(action->paramFloat >> 16));

        break;
      }

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_MUTING_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_BOOL)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setSwitchGroupOnOff(hUiManager, (UCHAR)action->paramInt, !action->paramBool);
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_BALANCE_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setSwitchGroupGain(hUiManager, (UCHAR)action->paramInt,
                               (SHORT)(LONG)(action->paramFloat >> 15));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_AZIMUTH_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setSwitchGroupAzOffset(hUiManager, (UCHAR)action->paramInt,
                                   (UCHAR)(((action->paramFloat * 2 / 3) >> 16) + 128));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_ELEVATION_CHANGED:
      if (!compUUID(hUiManager->uiState.uuid, action->uuid) ||
          !(action->presentFlags & FLAG_XML_PARAM_INT) ||
          !(action->presentFlags & FLAG_XML_PARAM_FLOAT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }
      err = setSwitchGroupElOffset(hUiManager, (UCHAR)action->paramInt,
                                   (UCHAR)((((LONG)action->paramFloat / 3) >> 16) + 32));
      break;

    case UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_DISTANCE_CHANGED:
      /* currently not supported */
      return UI_MANAGER_INVALID_PARAM;

    case UI_MANAGER_COMMAND_AUDIO_LANGUAGE_SELECTED:
      if ((action->paramInt < 0) || (action->paramInt >= NUM_PREF_LANGUAGES) ||
          !(action->presentFlags & FLAG_XML_PARAM_TEXT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }

      {
        char language[3];
        language[0] = action->paramText[0];
        language[1] = action->paramText[1];
        language[2] = action->paramText[2];
        if (asiCheckISO639_2(language)) {
          err = UI_MANAGER_INVALID_PARAM;
          break;
        }
        asiMapISO639_2_T2B_and_tolower(language);
        hUiManager->uiState.prefAudioLanguages[action->paramInt][0] = language[0];
        hUiManager->uiState.prefAudioLanguages[action->paramInt][1] = language[1];
        hUiManager->uiState.prefAudioLanguages[action->paramInt][2] = language[2];
      }

      selectAudioLang(hUiManager);

      break;

    case UI_MANAGER_COMMAND_INTERFACE_LANGUAGE_SELECTED:
      if ((action->paramInt < 0) || (action->paramInt >= ASI_MAX_PREF_DESCR_LANGUAGES) ||
          !(action->presentFlags & FLAG_XML_PARAM_TEXT)) {
        err = UI_MANAGER_INVALID_PARAM;
        break;
      }

      {
        char language[3];
        language[0] = action->paramText[0];
        language[1] = action->paramText[1];
        language[2] = action->paramText[2];
        if (asiCheckISO639_2(language)) {
          err = UI_MANAGER_INVALID_PARAM;
          break;
        }
        asiMapISO639_2_T2B_and_tolower(language);
        hUiManager->asi.prefDescrLanguages[action->paramInt][0] = language[0];
        hUiManager->asi.prefDescrLanguages[action->paramInt][1] = language[1];
        hUiManager->asi.prefDescrLanguages[action->paramInt][2] = language[2];
      }

      break;

    case UI_MANAGER_COMMAND_SET_GUID: {
      int i, chg = 0;

      for (i = 0; i < 16; i++) {
        if (hUiManager->uiState.uuid[i] != action->uuid[i]) chg = 1;
        hUiManager->uiState.uuid[i] = action->uuid[i];
      }
      /* restore state after change */
      if (chg) persistenceRestore(hUiManager);

      break;
    }

    default:
      return UI_MANAGER_INVALID_PARAM;
  }

  return err;
}

/* persistence manager restore */
static void persistenceRestore(HANDLE_UI_MANAGER hUiManager) {
  UI_MANAGER_ACTION action;
  UCHAR result;

  /* get first action */
  result =
      persistenceManagerGetCommand(hUiManager->hPersistence, hUiManager->uiState.uuid, &action);

  /* loop over remaining actions */
  while (result) {
    /* perform action */
    if (action.actionType == UI_MANAGER_COMMAND_ACCESSIBILITY_PREFERENCE) {
      reset(hUiManager, 0);
      applyPreset(hUiManager, PRESET_ID_AUTO);
    } else if (action.actionType == UI_MANAGER_COMMAND_AUDIO_LANGUAGE_SELECTED) {
      selectAudioLang(hUiManager);
    } else {
      performAction(hUiManager, &action);
    }

    /* get next action */
    result = persistenceManagerGetCommand(hUiManager->hPersistence, NULL, &action);
  }
}

/* create instance */
INT UI_Manager_Create(HANDLE_UI_MANAGER* phUiManager) {
  *phUiManager = (HANDLE_UI_MANAGER)FDKcalloc(1, sizeof(UI_MANAGER));
  if (*phUiManager == NULL) return UI_MANAGER_OUT_OF_MEMORY;

  (*phUiManager)->asi.activeDmxId = INVALID_ID;
  (*phUiManager)->uiState.targetLoudness = UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS;
  (*phUiManager)->uiState.drcSelected = UI_MANAGER_USE_DEFAULT_DRC_SELECTED;
  (*phUiManager)->uiState.albumMode = -1;
  (*phUiManager)->uiState.boost = -1;
  (*phUiManager)->uiState.compress = -1;
  (*phUiManager)->xmlStateChanged = 1;
  (*phUiManager)->uiStateChanged = 1;
  (*phUiManager)->drcStateChanged = 1;
  (*phUiManager)->configChanged = 1;
  (*phUiManager)->isActive = 1;

  (*phUiManager)->hPersistence = persistenceManagerCreate();

  return UI_MANAGER_OK;
}

/* get ASI pointer */
AUDIO_SCENE_INFO* UI_Manager_GetAsiPointer(HANDLE_UI_MANAGER hUiManager) {
  return &(hUiManager->asi);
}

UI_MANAGER_ERROR UI_Manager_SetIsActive(HANDLE_UI_MANAGER hUiManager, UCHAR isActive) {
  if (hUiManager) {
    if (hUiManager->isActive != isActive) {
      hUiManager->isActive = isActive;
      hUiManager->xmlStateChanged = 1;
      hUiManager->uiStateChanged = 1;
      hUiManager->drcStateChanged = 1;
      if (isActive) {
        hUiManager->configChanged = 1;
        reset(hUiManager, 0);
        applyPreset(hUiManager, PRESET_ID_AUTO);
      } else {
        hUiManager->configChanged = 0;
      }
    }
  }
  return UI_MANAGER_OK;
}

/* get XML scene data */
UI_MANAGER_ERROR UI_Manager_GetXmlSceneState(HANDLE_UI_MANAGER hUiManager, char* xmlOut,
                                             UINT xmlOutSize, UINT flagsIn, UINT* flagsOut) {
  UI_MANAGER_ERROR err = UI_MANAGER_OK;

  xmlOut[0] = 0;

  *flagsOut = 0;

  /* check/update state */
  update(hUiManager);

  /* check if we have to output XML */
  if (!hUiManager->xmlStateChanged && !(flagsIn & UI_MANAGER_FORCE_UPDATE) &&
      (hUiManager->xmlWriter.nextPresetIdx == XML_START_INDEX)) {
    *flagsOut |= UI_MANAGER_NO_CHANGE;
    return UI_MANAGER_OK;
  }

  /* if something has changed we have to restart XML output */
  if (hUiManager->xmlStateChanged) {
    flagsIn |= UI_MANAGER_FORCE_RESTART_XML;
  }

  /* write XML string */
  err = uiManagerWriteXML(hUiManager, xmlOut, xmlOutSize, flagsIn, flagsOut);

  if (err == UI_MANAGER_OK) {
    if (!(*flagsOut & UI_MANAGER_SHORT_OUTPUT)) {
      /* reset changed flag */
      hUiManager->xmlStateChanged = 0;
    }

    /* reset config change flag */
    hUiManager->configChanged = 0;
  }

  return err;
}

/* apply XML action */
UI_MANAGER_ERROR UI_Manager_ApplyXmlAction(HANDLE_UI_MANAGER hUiManager, const char* xmlIn,
                                           UINT xmlInSize, UINT* flagsOut) {
  UI_MANAGER_ACTION action;
  UI_MANAGER_ERROR err = UI_MANAGER_OK;

  *flagsOut = 0;

  if (!hUiManager->isActive) {
    return UI_MANAGER_INVALID_STATE;
  }

  update(hUiManager);

  uiManagerParseXmlAction(xmlIn, xmlInSize, &action);

  err = performAction(hUiManager, &action);

  if ((err == UI_MANAGER_OK) && (hUiManager->hPersistence)) {
    persistenceManagerSaveCommand(hUiManager->hPersistence, &action);
  }

  if (err != UI_MANAGER_OK) hUiManager->xmlStateChanged = 1;

  return err;
}

/* set UUID */
UI_MANAGER_ERROR UI_Manager_SetUUID(HANDLE_UI_MANAGER hUiManager, UCHAR uuid[16],
                                    UCHAR applyAsiCrc) {
  int i, chg = 0;

  if (!hUiManager->isActive) {
    return UI_MANAGER_INVALID_STATE;
  }

  update(hUiManager);

  if (applyAsiCrc) {
    /* xor upper 16 bits of UUID with partial ASI CRC */
    UCHAR tmp0, tmp1;

    tmp0 = uuid[0] ^ (hUiManager->asi.crcForUid >> 8);
    tmp1 = uuid[1] ^ (hUiManager->asi.crcForUid & 0xFF);

    if (hUiManager->uiState.uuid[0] != tmp0) chg = 1;
    if (hUiManager->uiState.uuid[1] != tmp1) chg = 1;

    hUiManager->uiState.uuid[0] = tmp0;
    hUiManager->uiState.uuid[1] = tmp1;

    i = 2;
  } else {
    i = 0;
  }

  for (; i < 16; i++) {
    if (hUiManager->uiState.uuid[i] != uuid[i]) chg = 1;
    hUiManager->uiState.uuid[i] = uuid[i];
  }
  /* restore state */
  if (chg) persistenceRestore(hUiManager);

  return UI_MANAGER_OK;
}

/* get interactvity status */
UI_MANAGER_ERROR UI_Manager_GetInteractivityStatus(HANDLE_UI_MANAGER hUiManager,
                                                   USER_INTERACTIVITY_STATUS* pUiStatus,
                                                   UCHAR* pSignalsChanged) {
  int grp;

  if (!hUiManager->isActive) {
    return UI_MANAGER_INVALID_STATE;
  }

  update(hUiManager);

  if (hUiManager->uiState.activePresetIndex == INVALID_IDX) {
    pUiStatus->interactionMode = 0;
    pUiStatus->groupPresetID = INVALID_ID;
  } else {
    pUiStatus->interactionMode = 1;
    pUiStatus->groupPresetID =
        hUiManager->asi.groupPresets[hUiManager->uiState.activePresetIndex].groupPresetID;
  }

  pUiStatus->numGroups = hUiManager->asi.numGroups;

  for (grp = 0; grp < pUiStatus->numGroups; grp++) {
    pUiStatus->groupData[grp].groupID = hUiManager->asi.groups[grp].groupID;
    if (pSignalsChanged &&
        (pUiStatus->groupData[grp].onOff != hUiManager->uiState.groups[grp].onOff)) {
      *pSignalsChanged = 1;
    }
    pUiStatus->groupData[grp].onOff = hUiManager->uiState.groups[grp].onOff;
    pUiStatus->groupData[grp].routeToWIRE = 0;

    pUiStatus->groupData[grp].changePosition = 0;
    {
      UCHAR azOffset = hUiManager->uiState.groups[grp].azOffset;
      UCHAR elOffset = hUiManager->uiState.groups[grp].elOffset;
      UCHAR distFactor = hUiManager->uiState.groups[grp].distFactor;

      if ((azOffset != hUiManager->uiState.groups[grp].defaultAzOffset) ||
          (elOffset != hUiManager->uiState.groups[grp].defaultElOffset) ||
          (distFactor != hUiManager->uiState.groups[grp].defaultDistFactor)) {
        pUiStatus->groupData[grp].changePosition = 1;
      }

      pUiStatus->groupData[grp].azOffset = azOffset;
      pUiStatus->groupData[grp].elOffset = elOffset;
      pUiStatus->groupData[grp].distFact = distFactor;
    }

    pUiStatus->groupData[grp].changeGain = 0;
    {
      SHORT gain = hUiManager->uiState.groups[grp].gain;

      if (gain != hUiManager->uiState.groups[grp].defaultGain) {
        pUiStatus->groupData[grp].changeGain = 1;
      }

      pUiStatus->groupData[grp].gain = (UCHAR)fMax(0, (gain >> 1) + 64);
    }
  }

  hUiManager->uiStateChanged = 0;

  return UI_MANAGER_OK;
}

/* get DRC/loudness status*/
UI_MANAGER_ERROR UI_Manager_GetDrcLoudnessStatus(HANDLE_UI_MANAGER hUiManager,
                                                 UI_DRC_LOUDNESS_STATUS* pDrcLoudnessStatus) {
  if (!hUiManager->isActive) {
    return UI_MANAGER_INVALID_STATE;
  }

  pDrcLoudnessStatus->targetLoudness = hUiManager->uiState.targetLoudness;

  /* if accessibility preference is set to hearing impaired, force dialog DRC mode, otherwise return
   * selected mode */
  if (hUiManager->uiState.accessibilityPreference >= 2) {
    pDrcLoudnessStatus->drcSelected = getDrcEffectTypeFallback(5);
  } else {
    pDrcLoudnessStatus->drcSelected = hUiManager->uiState.drcSelected;
  }

  pDrcLoudnessStatus->albumMode = hUiManager->uiState.albumMode;
  pDrcLoudnessStatus->boost = hUiManager->uiState.boost;
  pDrcLoudnessStatus->compress = hUiManager->uiState.compress;

  hUiManager->drcStateChanged = 0;

  return UI_MANAGER_OK;
}

/* check status change */
UI_MANAGER_ERROR UI_Manager_GetStatusChanged(HANDLE_UI_MANAGER hUiManager,
                                             UCHAR* pInteractivityStatusChanged,
                                             UCHAR* pDrcLoudnessStatusChanged) {
  update(hUiManager);

  if (pInteractivityStatusChanged) *pInteractivityStatusChanged = hUiManager->uiStateChanged;
  if (pDrcLoudnessStatusChanged) *pDrcLoudnessStatusChanged = hUiManager->drcStateChanged;

  return UI_MANAGER_OK;
}

/* set persistence memory */
UI_MANAGER_ERROR UI_Manager_SetPersistenceMemory(HANDLE_UI_MANAGER hUiManager,
                                                 void* persistenceMemoryBlock,
                                                 USHORT persistenceMemorySize) {
  INT r = persistenceManagerSetMemory(hUiManager->hPersistence, persistenceMemoryBlock,
                                      persistenceMemorySize);

  if (r == -1) return UI_MANAGER_BUFFER_TOO_SMALL;
  if (r == -2) return UI_MANAGER_INVALID_PARAM;
  if (r == 0) return UI_MANAGER_OK_BUT_NO_VALID_DATA;
  return UI_MANAGER_OK;
}

/* get persistence memory */
UI_MANAGER_ERROR UI_Manager_GetPersistenceMemory(HANDLE_UI_MANAGER hUiManager,
                                                 void** persistenceMemoryBlock,
                                                 USHORT* persistenceMemorySize) {
  persistenceManagerGetMemory(hUiManager->hPersistence, persistenceMemoryBlock,
                              persistenceMemorySize);

  return UI_MANAGER_OK;
}

/* delete instance */
INT UI_Manager_Delete(HANDLE_UI_MANAGER* phUiManager) {
  if (*phUiManager && (*phUiManager)->hPersistence)
    persistenceManagerDelete((*phUiManager)->hPersistence);

  FDKfree(*phUiManager);
  *phUiManager = NULL;

  return UI_MANAGER_OK;
}
