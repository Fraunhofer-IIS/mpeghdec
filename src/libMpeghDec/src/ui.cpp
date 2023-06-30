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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Matthias Neusinger, Bernhard Neugebauer

   Description: MPEG-H User Interactivity

*******************************************************************************/

#include "ui.h"

#include "tpdec_lib.h"
#include "FDK_drcDecLib.h"

/* apply default fallbacks for requested DRC effect types */
static LONG getDrcEffectTypeFallback(const int drcEffectTypeRequested) {
  switch (drcEffectTypeRequested) {
    case -1: /* DRC OFF*/
      return UI_MANAGER_DRC_OFF;
    case 0: /* NONE */
      return 0x00000000;
    case 1: /* NIGHT */
      return 0x00543261;
    case 2: /* NOISY */
      return 0x00543162;
    case 3: /* LIMITED */
      return 0x00542163;
    case 4: /* LOWLEVEL */
      return 0x00531264;
    case 5: /* DIALOG */
      return 0x00432165;
    case 6: /* GENERAL */
      return 0x00543216;
    default:
      return 0x00000000;
  }
}

/* Loudness Compensation after Gain Interactivity according to Amendmend 3 to MPEG-H standard:
 Keep output loudness after user interaction on groups. */
static FIXP_DBL getLoudnessCompensationGainDb(AUDIO_SCENE_INFO* pASI,
                                              USER_INTERACTIVITY_STATUS* pUiStatus,
                                              HANDLE_SEL_PROC_OUTPUT pUniDrcSelProcOutput);

/* parse user interaction packet (mpegh3daElementInteraction) */
static INT mpegh3daElementInteraction(HANDLE_AACDECODER self, HANDLE_FDK_BITSTREAM hBs) {
  USER_INTERACTIVITY_STATUS uiStatus;
  int interactionSignatureDataLength, grpIdx, swGrpIdx, numGroups;
  AUDIO_SCENE_INFO* pASI = UI_Manager_GetAsiPointer(self->hUiManager);

  FDK_ASSERT(self != NULL);

  uiStatus = self->uiStatus;

  interactionSignatureDataLength = FDKreadBits(hBs, 8);
  if (interactionSignatureDataLength > 0) {
    /* skip signature for now */
    FDKpushFor(hBs, 8 + interactionSignatureDataLength * 8);
  }

  /* ElementInteractionData */
  uiStatus.interactionMode = FDKreadBit(hBs);
  numGroups = FDKreadBits(hBs, 7);
  if (numGroups > ASI_MAX_GROUPS) {
    return AAC_DEC_PARSE_ERROR;
  }

  if (uiStatus.interactionMode) {
    uiStatus.groupPresetID = FDKreadBits(hBs, 5);
    if (asiGroupPresetID2idx(pASI, uiStatus.groupPresetID) < 0) return AAC_DEC_PARSE_ERROR;
  }

  /* init on/off states to ASI defaults */
  uiStatus.numGroups = pASI->numGroups;

  for (grpIdx = 0; grpIdx < pASI->numGroups; grpIdx++) {
    uiStatus.groupData[grpIdx].groupID = pASI->groups[grpIdx].groupID;

    if (pASI->groups[grpIdx].switchGroupID == 255) {
      uiStatus.groupData[grpIdx].onOff = pASI->groups[grpIdx].defaultOnOff;
    }
  }

  for (swGrpIdx = 0; swGrpIdx < pASI->numSwitchGroups; swGrpIdx++) {
    if (pASI->switchGroups[swGrpIdx].defaultOnOff) {
      grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].defaultGroupID);
      if (grpIdx >= 0) uiStatus.groupData[grpIdx].onOff = 1;
    }
  }

  /* read group data */
  for (int i = 0; i < numGroups; i++) {
    USER_INTERACTIVITY_GROUP_STATUS* pGroup = NULL;
    ASI_GROUP* pAsiGroup = NULL;
    UCHAR groupID, onOff;

    /* read group ID */
    groupID = FDKreadBits(hBs, 7);

    /* get group index and pointer */
    grpIdx = asiGroupID2idx(pASI, groupID);
    if (grpIdx < 0) return AAC_DEC_PARSE_ERROR;

    pGroup = &uiStatus.groupData[grpIdx];
    pAsiGroup = &pASI->groups[grpIdx];

    /* on/off state */
    onOff = FDKreadBit(hBs);
    if (pAsiGroup->allowOnOff || (pAsiGroup->switchGroupID != 255)) {
      if (onOff && (pAsiGroup->switchGroupID != 255)) {
        swGrpIdx = asiSwitchGroupID2idx(pASI, pAsiGroup->switchGroupID);

        for (int j = 0; j < pASI->switchGroups[swGrpIdx].numMembers; j++) {
          int memGrpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[j]);
          if (memGrpIdx >= 0) uiStatus.groupData[memGrpIdx].onOff = 0;
        }
      }

      pGroup->onOff = onOff;
    }

    /* route to WIRE */
    pGroup->routeToWIRE = FDKreadBit(hBs);
    if (pGroup->routeToWIRE) {
      pGroup->routeToWireID = FDKreadBits(hBs, 16);
    }

    /* interactivity */
    if (onOff) {
      /* position */
      pGroup->changePosition = FDKreadBit(hBs);
      if (pGroup->changePosition) {
        pGroup->azOffset = FDKreadBits(hBs, 8);
        pGroup->elOffset = FDKreadBits(hBs, 6);
        pGroup->distFact = FDKreadBits(hBs, 4);
      }

      if (!pAsiGroup->allowPositionInteractivity) pGroup->changePosition = 0;

      if (pGroup->changePosition) {
        pGroup->azOffset =
            fMax(pGroup->azOffset, (UCHAR)(-(int)pAsiGroup->interactivityMinAzOffset + 128));
        pGroup->azOffset =
            fMin(pGroup->azOffset, (UCHAR)(pAsiGroup->interactivityMaxAzOffset + 128));

        pGroup->elOffset =
            fMax(pGroup->elOffset, (UCHAR)(-(int)pAsiGroup->interactivityMinElOffset + 32));
        pGroup->elOffset =
            fMin(pGroup->elOffset, (UCHAR)(pAsiGroup->interactivityMaxElOffset + 32));
      }

      /* gain */
      pGroup->changeGain = FDKreadBit(hBs);
      if (pGroup->changeGain) {
        pGroup->gain = FDKreadBits(hBs, 7);
      }

      if (!pAsiGroup->allowGainInteractivity) pGroup->changeGain = 0;

      if (pGroup->changeGain) {
        if (pAsiGroup->interactivityMinGain) {
          pGroup->gain = fMax(pGroup->gain,
                              (UCHAR)(pAsiGroup->interactivityMinGain + 1)); /* minGain - 63 + 64 */
        }

        pGroup->gain = fMin(pGroup->gain, (UCHAR)(pAsiGroup->interactivityMaxGain + 64));
      }
    }
  }

  if (FDKreadBit(hBs)) {
    /* skip local zoom area for now */
    FDKpushFor(hBs, 10 + 10 + 9 + 10);
  }

  /* apply preset conditions */
  if (uiStatus.interactionMode) {
    int presetIdx = asiGroupPresetID2idx(pASI, uiStatus.groupPresetID);
    int numConditions;
    ASI_GROUP_PRESET_CONDITION* conditions;

    /* get normal or downmix ID conditions */
    if (presetIdx < 0) {
      numConditions = 0;
    } else if (pASI->groupPresets[presetIdx].hasDownmixIdExtension) {
      numConditions = pASI->groupPresets[presetIdx].downmixIdExtension.numConditions;
      conditions = pASI->groupPresets[presetIdx].downmixIdExtension.conditions;
    } else {
      numConditions = pASI->groupPresets[presetIdx].numConditions;
      conditions = pASI->groupPresets[presetIdx].conditions;
    }

    /* apply conditions */
    for (int i = 0; i < numConditions; i++) {
      if (!conditions[i].isSwitchGroupCondition) { /* group condition */
        grpIdx = asiGroupID2idx(pASI, conditions[i].referenceID);
        if (grpIdx < 0) continue;

        /* on/off flag */
        if (conditions[i].conditionOnOff && (pASI->groups[grpIdx].switchGroupID != 255)) {
          swGrpIdx = asiSwitchGroupID2idx(pASI, pASI->groups[grpIdx].switchGroupID);

          for (int j = 0; j < pASI->switchGroups[swGrpIdx].numMembers; j++) {
            int memGrpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[j]);
            if (memGrpIdx >= 0) uiStatus.groupData[memGrpIdx].onOff = 0;
          }
        }

        uiStatus.groupData[grpIdx].onOff = conditions[i].conditionOnOff;

        /* disable interactivity flags */
        if (conditions[i].disableGainInteractivity) uiStatus.groupData[grpIdx].changeGain = 0;
        if (conditions[i].disablePositionInteractivity)
          uiStatus.groupData[grpIdx].changePosition = 0;
      } else { /* switch group condition */
        swGrpIdx = asiSwitchGroupID2idx(pASI, conditions[i].referenceID);
        if (swGrpIdx < 0) continue;
        int numOn = 0;

        for (int j = 0; j < pASI->switchGroups[swGrpIdx].numMembers; j++) {
          grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[j]);
          if (grpIdx < 0) continue;

          /* if switch group turned off by condition, set all members to off */
          if (!conditions[i].conditionOnOff) uiStatus.groupData[grpIdx].onOff = 0;

          /* count active members */
          if (uiStatus.groupData[grpIdx].onOff) numOn++;

          /* disable interactivity flags */
          if (conditions[i].disableGainInteractivity) uiStatus.groupData[grpIdx].changeGain = 0;
          if (conditions[i].disablePositionInteractivity)
            uiStatus.groupData[grpIdx].changePosition = 0;
        }

        /* if switch group turned on by condition and no member active, activate default member */
        if (conditions[i].conditionOnOff && (numOn == 0)) {
          grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].defaultGroupID);
          if (grpIdx >= 0) uiStatus.groupData[grpIdx].onOff = 1;
        }
      }
    }
  }

  /* validate switch group states */
  for (swGrpIdx = 0; swGrpIdx < pASI->numSwitchGroups; swGrpIdx++) {
    int numOn = 0;

    /* count active members */
    for (int i = 0; i < pASI->switchGroups[swGrpIdx].numMembers; i++) {
      grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[i]);
      if ((grpIdx >= 0) && uiStatus.groupData[grpIdx].onOff) numOn++;
    }

    /* if more than one active member set to default state */
    if (numOn > 1) {
      for (int i = 0; i < pASI->switchGroups[swGrpIdx].numMembers; i++) {
        grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[i]);
        if (grpIdx >= 0) uiStatus.groupData[grpIdx].onOff = 0;
      }

      if (pASI->switchGroups[swGrpIdx].defaultOnOff) {
        grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].defaultGroupID);
        if (grpIdx >= 0) uiStatus.groupData[grpIdx].onOff = 1;
      }
    }
    /* if no member active and turning switch group off not allowed, activate default member */
    else if (numOn == 0) {
      if (!pASI->switchGroups[swGrpIdx].allowOnOff && pASI->switchGroups[swGrpIdx].defaultOnOff) {
        grpIdx = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].defaultGroupID);
        if (grpIdx >= 0) uiStatus.groupData[grpIdx].onOff = 1;
      }
    }
  }

  for (grpIdx = 0; grpIdx < pASI->numGroups; grpIdx++) {
    USER_INTERACTIVITY_GROUP_STATUS* pGroup = &uiStatus.groupData[grpIdx];

    /* for turned off groups copy previous interactivity state to avoid transition to default during
     * fade-out frame */
    if (!pGroup->onOff && self->uiStatusNext.groupData[grpIdx].onOff) {
      pGroup->changePosition = self->uiStatusNext.groupData[grpIdx].changePosition;
      pGroup->azOffset = self->uiStatusNext.groupData[grpIdx].azOffset;
      pGroup->elOffset = self->uiStatusNext.groupData[grpIdx].elOffset;
      pGroup->distFact = self->uiStatusNext.groupData[grpIdx].distFact;

      pGroup->changeGain = self->uiStatusNext.groupData[grpIdx].changeGain;
      pGroup->gain = self->uiStatusNext.groupData[grpIdx].gain;
    }

    /* Copy parameters to all other switch group members which are inactive */
    if (pGroup->onOff) {
      int found = 0;
      for (swGrpIdx = 0; swGrpIdx < pASI->numSwitchGroups; swGrpIdx++) {
        /* find switch group */
        for (int i = 0; i < pASI->switchGroups[swGrpIdx].numMembers; i++) {
          if (pGroup->groupID == pASI->switchGroups[swGrpIdx].memberID[i]) {
            found = 1;
            break;
          }
        }
        if (found) {
          for (int i = 0; i < pASI->switchGroups[swGrpIdx].numMembers; i++) {
            if (pGroup->groupID != pASI->switchGroups[swGrpIdx].memberID[i]) {
              int swgrp = asiGroupID2idx(pASI, pASI->switchGroups[swGrpIdx].memberID[i]);
              USER_INTERACTIVITY_GROUP_STATUS* pG = &uiStatus.groupData[swgrp];

              pG->changePosition = pGroup->changePosition;
              pG->azOffset = pGroup->azOffset;
              pG->elOffset = pGroup->elOffset;
              pG->distFact = pGroup->distFact;

              pG->changeGain = pGroup->changeGain;
              pG->gain = pGroup->gain;
            }
          }
          break;
        }
      }
    }

    /* check if group on/off states changed (for element skipping) */
    if (pGroup->onOff != self->uiStatusNext.groupData[grpIdx].onOff) {
      self->uiSignalChanged = 1; /* signals have changed, requires forced config change and delaying
                                    UI change until next RAP */
    }
  }

  self->uiStatusNext = uiStatus;
  self->uiStatusValid = 1;

  return 0;
}

static INT mpegh3daLoudnessDrcInterface(HANDLE_AACDECODER self, HANDLE_FDK_BITSTREAM hBs) {
  int targetLoudness = -1;
  ULONG drcEffectType = 0x80000000; /* not set */
  int boost = -1;                   /* Q8 representation. 256 .. 0 --> 1.0 .. 0.0  */
  int compress = -1;                /* Q8 representation. 256 .. 0 --> 1.0 .. 0.0  */
  int albumMode = -1;

  FDK_ASSERT(self != NULL);

  /* uniDrcInterfaceSignature */
  if (FDKreadBit(hBs)) {
    int uniDrcInterfaceSignatureDataLength;
    FDKreadBits(hBs, 8); /* uniDrcInterfaceSignatureType */
    uniDrcInterfaceSignatureDataLength =
        FDKreadBits(hBs, 8); /* uniDrcInterfaceSignatureDataLength */
    FDKpushFor(hBs, uniDrcInterfaceSignatureDataLength * 8); /* uniDrcInterfaceSignatureData */
  }

  /* systemInterface */
  if (FDKreadBit(hBs)) {
    int targetConfigRequestType;

    targetConfigRequestType = FDKreadBits(hBs, 2);

    switch (targetConfigRequestType) {
      case 0: {
        int numDownmixIdRequests;

        numDownmixIdRequests = FDKreadBits(hBs, 4);

        for (int i = 0; i < numDownmixIdRequests; i++) {
          FDKreadBits(hBs, 7); /* downmixIdRequested[i] */
        }
      } break;
      case 1:
        FDKreadBits(hBs, 8); /* targetLayoutRequested */
        break;
      case 2:
        /* targetChannelCountRequested  = */ FDKreadBits(hBs, 7) /* + 1 */;
        break;
      default:
        return (1);
    }
  }

  /* loudnessNormalizationControlInterface */
  if (FDKreadBit(hBs)) {
    int loudnessNormalizationOn;

    loudnessNormalizationOn = FDKreadBit(hBs);

    if (loudnessNormalizationOn == 1) {
      /* targetLoudness = -  FDKreadBits(hBs, 12) * 0.03125f; */
      targetLoudness = FDKreadBits(hBs, 12);
    }
  }

  /* loudnessNormalizationParameterInterface */
  if (FDKreadBit(hBs)) {
    albumMode = FDKreadBit(hBs); /* albumMode */
    FDKreadBit(hBs);             /* peakLimiterPresent */

    if (FDKreadBit(hBs) /* changeLoudnessDeviationMax */) {
      FDKreadBits(hBs, 6); /* loudnessDeviationMax */
    }

    if (FDKreadBit(hBs) /* changeLoudnessMeasurementMethod */) {
      FDKreadBits(hBs, 3); /* loudnessMeasurementMethod */
    }

    if (FDKreadBit(hBs) /* changeLoudnessMeasurementSystem */) {
      FDKreadBits(hBs, 4); /*  loudnessMeasurementSystem*/
    }

    if (FDKreadBit(hBs) /* changeLoudnessMeasurementPreProc */) {
      FDKreadBits(hBs, 2); /* loudnessMeasurementPreProc */
    }

    if (FDKreadBit(hBs) /* changeDeviceCutOffFrequency */) {
      FDKreadBits(hBs, 6); /* deviceCutOffFrequency */
      /* deviceCutOffFrequency = max(min(tmp*10,500),20); */
    }

    if (FDKreadBit(hBs) /* changeLoudnessNormalizationGainDbMax */) {
      FDKreadBits(hBs, 6); /* loudnessNormalizationGainDbMax */
      /*if (tmp<63) {
        loudnessNormalizationParameterInterface->loudnessNormalizationGainDbMax = tmp*0.5f;
      } else {
        loudnessNormalizationParameterInterface->loudnessNormalizationGainDbMax =
      LOUDNESS_NORMALIZATION_GAIN_MAX_DEFAULT;
      }*/
    }

    if (FDKreadBit(hBs) /* changeLoudnessNormalizationGainModificationDb */) {
      FDKreadBits(hBs, 10); /* loudnessNormalizationGainModificationDb */
      // loudnessNormalizationGainModificationDb = -16+tmp*0.03125f;
    }

    if (FDKreadBit(hBs) /* changeOutputPeakLevelMax */) {
      FDKreadBits(hBs, 6); /* outputPeakLevelMax */
      // loudnessNormalizationParameterInterface->outputPeakLevelMax = tmp*0.5f;
    }
  }

  /* dynamicRangeControlInterface */
  if (FDKreadBit(hBs)) {
    if (FDKreadBit(hBs) /* dynamicRangeControlOn */) {
      int numDrcFeatureRequests;

      numDrcFeatureRequests = FDKreadBits(hBs, 3); /* numDrcFeatureRequests */

      for (int i = 0; i < numDrcFeatureRequests; i++) {
        int drcFeatureRequestType;
        drcFeatureRequestType = FDKreadBits(hBs, 2); /* drcFeatureRequestType */

        switch (drcFeatureRequestType) {
          case 0: {
            int numDrcEffectTypeRequests;
            numDrcEffectTypeRequests = FDKreadBits(hBs, 4); /* numDrcEffectTypeRequests */
            FDKreadBits(hBs, 4); /* numDrcEffectTypeRequestsDesired, forced to 1 */
            if (numDrcEffectTypeRequests > 0) {
              for (int j = 0; j < numDrcEffectTypeRequests; j++) {
                int drcEffectTypeRequest = FDKreadBits(hBs, 4); /* drcEffectTypeRequest */
                if (j == 0) {
                  drcEffectType = drcEffectTypeRequest;
                } else if (j < 8) {
                  drcEffectType |= drcEffectTypeRequest << (4 * j);
                }
                /* further drcEffectTypeRequest entries are ignored */
              }
            } else {
              /* numDrcEffectTypeRequests == 0 means: use DRC selection of decoder API */
              drcEffectType = UI_MANAGER_USE_DEFAULT_DRC_SELECTED;
            }
          } break;
          case 1:
            FDKreadBits(hBs, 2); /* dynRangeMeasurementRequestType */

            if (FDKreadBit(hBs) /* dynRangeRequestedIsRange */) {
              int tmp;

              tmp = FDKreadBits(hBs, 8);
              if (tmp == 0) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValue[i] = 0.0f;*/
              } else if (tmp <= 128) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValue[i] = tmp * 0.25f;*/
              } else if (tmp <= 204) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValue[i] = 0.5f * tmp - 32.0f;*/
              } else {
                /*dynamicRangeControlInterface->dynamicRangeRequestValue[i] = tmp - 134.0f;*/
              }
            } else {
              int tmp;

              tmp = FDKreadBits(hBs, 8);
              if (tmp == 0) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMin[i] = 0.0f;*/
              } else if (tmp <= 128) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMin[i] = tmp * 0.25f;*/
              } else if (tmp <= 204) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMin[i] = 0.5f * tmp
                 * - 32.0f;*/
              } else {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMin[i] = tmp - 134.0f;*/
              }
              tmp = FDKreadBits(hBs, 8);
              if (tmp == 0) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMax[i] = 0.0f;*/
              } else if (tmp <= 128) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMax[i] = tmp * 0.25f;*/
              } else if (tmp <= 204) {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMax[i] = 0.5f * tmp
                 * - 32.0f;*/
              } else {
                /*dynamicRangeControlInterface->dynamicRangeRequestValueMax[i] = tmp - 134.0f;*/
              }
            }
            break;
          case 2:
            FDKreadBits(hBs, 7); /* drcCharacteristicRequest */
            break;
          default:
            return (1);
        }
      }
    } else {                              /* dynamicRangeControlOn = 0 */
      drcEffectType = UI_MANAGER_DRC_OFF; /* signal "DRC off" */
    }
  }

  /* dynamicRangeControlParameterInterface */
  if (FDKreadBit(hBs)) {
    int changeCompress, changeBoost;
    changeCompress = FDKreadBit(hBs);
    changeBoost = FDKreadBit(hBs);

    if (changeCompress) {
      int tmp;

      tmp = FDKreadBits(hBs, 8);
      if (tmp < 255) {
        /*dynamicRangeControlParameterInterface->compress = 1 - tmp * 0.00390625f;*/
        compress = 256 - tmp;
      } else {
        /*dynamicRangeControlParameterInterface->compress = 0.f;*/
        compress = 0;
      }
    }

    if (changeBoost) {
      int tmp;

      tmp = FDKreadBits(hBs, 8);
      if (tmp < 255) {
        /*dynamicRangeControlParameterInterface->boost = 1 - tmp * 0.00390625f;*/
        boost = 256 - tmp;
      } else {
        /*dynamicRangeControlParameterInterface->boost = 0.f;*/
        boost = 0;
      }
    }

    if (FDKreadBit(hBs) /* changeDrcCharacteristicTarget */) {
      FDKreadBits(hBs, 8); /* drcCharacteristicTarget */
    }
  }

  /* uniDrcInterfaceExtension */
  if (FDKreadBit(hBs)) {
    int i = 0, uniDrcInterfaceExtType;

    uniDrcInterfaceExtType = FDKreadBits(hBs, 4); /* uniDrcInterfaceExtType[i] */

    while (uniDrcInterfaceExtType != 0x0) /* UNIDRCINTERFACEEXT_TERM = 0x0 */
    {
      int tmp, extBitSize;

      tmp = FDKreadBits(hBs, 4) + 4;
      extBitSize = FDKreadBits(hBs, tmp) + 1; /* extSizeBits */

      switch (uniDrcInterfaceExtType) {
          /* add future extensions here */
        default:
          FDKpushFor(hBs, extBitSize);
          break;
      }

      i++;
      if (i >= 8) { /* 8 = 8 */
        return 1;
      }
      uniDrcInterfaceExtType = FDKreadBits(hBs, 4); /* uniDrcInterfaceExtType[i] */
    }
  }

  /* transfer DRC parameters */
  if (targetLoudness != -1) {
    self->drcStatus.targetLoudness = (SCHAR)(-targetLoudness >> 5);
  } else {
    /* use target loudness selection of decoder API. Loudness normalization is mandatory */
    self->drcStatus.targetLoudness = UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS;
  }

  if (drcEffectType != 0x80000000) {
    self->drcStatus.drcSelected = drcEffectType;
  } else { /* drcEffectType not set */
    self->drcStatus.drcSelected = UI_MANAGER_USE_DEFAULT_DRC_SELECTED;
  }

  if (albumMode != -1) {
    self->drcStatus.albumMode = (SCHAR)albumMode;
  } else {
    self->drcStatus.albumMode = -1;
  }
  if (boost != -1) {
    self->drcStatus.boost = (SHORT)(boost << 6);
  } else {
    self->drcStatus.boost = -1;
  }
  if (compress != -1) {
    self->drcStatus.compress = (SHORT)(compress << 6);
  } else {
    self->drcStatus.compress = -1;
  }

  self->drcStatusValid = 1;

  return 0;
}

static INT mpegh3daSetContentIdentifier(void* handle, HANDLE_FDK_BITSTREAM hBs,
                                        const UINT mhasPacketLength) {
  int err = 0;
  HANDLE_AACDECODER hAacDecoder = (HANDLE_AACDECODER)handle;
  UINT i;
  UINT idBytes = mhasPacketLength - 2;
  UCHAR uuid[16];

  if (idBytes > 16) idBytes = 16;

  for (i = 0; i < 16; i++) {
    if (i >= (16 - idBytes)) {
      uuid[i] = FDKreadBits(hBs, 8);
    } else {
      uuid[i] = 0;
    }
  }

  if (hAacDecoder->hUiManager && hAacDecoder->uiManagerActive) {
    /* ignore error return here (UI error should not trigger a sync error) */
    UI_Manager_SetUUID(hAacDecoder->hUiManager, uuid, 1);
  }

  return err;
}

static INT mpegh3daInteractivity(void* user_data, HANDLE_FDK_BITSTREAM hBs,
                                 const mha_pactyp_t pac_type, const UINT mhasPacketLength) {
  HANDLE_AACDECODER self = (HANDLE_AACDECODER)user_data;
  int err;

  switch (pac_type) {
    case MHA_PACTYP_USERINTERACTION:
      err = mpegh3daElementInteraction(self, hBs);
      break;
    case MHA_PACTYP_LOUDNESS_DRC:
      err = mpegh3daLoudnessDrcInterface(self, hBs);
      break;
    case MHA_PACTYP_MARKER:
      err = mpegh3daSetContentIdentifier(self, hBs, mhasPacketLength);
      break;
    default:
      err = 1;
  }

  return err;
}

/* Table AMD11.2 of Amendmend 3 to MPEG-H standard */
static FIXP_DBL computeLoudnessCompensationGain(
    int numGroups, UCHAR includeGroup[ASI_MAX_GROUPS],
    int groupLoudnessValueMissingFlag, /* group loudness value missing for one or more groups */
    FIXP_DBL groupLoudness[ASI_MAX_GROUPS],
    UCHAR groupGainDefaultDb[ASI_MAX_GROUPS], /* groupPresetGain in dB = 0.5 * (mae_groupPresetGain
                                                 - 255) + 32 */
    UCHAR groupGainInteractivityDb[ASI_MAX_GROUPS], /* interactivity gain in dB =
                                                       uiStatus.groupData[i].gain - 64; */
    UCHAR groupStateDefault[ASI_MAX_GROUPS], UCHAR groupStateInteractivity[ASI_MAX_GROUPS],
    UCHAR minGain, /* loudnessCompPresetMinGain in dB = -3 * mae_bsLoudnessCompPresetMinGain */
    UCHAR maxGain) /* loudnessCompPresetMaxGain in dB = 3 * mae_bsLoudnessCompPresetMaxGain */
{
  FIXP_DBL loudnessCompensationGainDb;
  FIXP_DBL loudnessCompensationGainPow2;
  FIXP_DBL loudnessReference = (FIXP_DBL)0, loudnessAfterInteract = (FIXP_DBL)0;
  FIXP_DBL tmp1, tmp2, minGainDb, maxGainDb;
  int n, tmp1_e, tmp2_e, lr_e = 0, lai_e = 0, lcgp2_e, lcgdb_e;

  /* compute components of loudness compensation gain */
  for (n = 0; n < numGroups; n++) {
    FIXP_DBL tmp1Db =
        ((groupGainDefaultDb[n] - 255) << (31 - 8 - 1)) + (32 << (31 - 8)); /* e = 8 */
    FIXP_DBL tmp2Db = fMin(fMax((INT)groupGainInteractivityDb[n] - 64, -63), 31)
                      << (31 - 8); /* e = 8 */
    if (groupLoudnessValueMissingFlag == 0) {
      tmp1Db += groupLoudness[n];
      tmp2Db += groupLoudness[n];
    }
    /* 10^(dB_val/10) = 2^(log2(10)/10*dB_val) */
    tmp1 = f2Pow(fMult(tmp1Db, FL2FXCONST_DBL(0.3321928f * (float)(1 << 1))), 8 - 1, &tmp1_e);
    tmp2 = f2Pow(fMult(tmp2Db, FL2FXCONST_DBL(0.3321928f * (float)(1 << 1))), 8 - 1, &tmp2_e);

    if (includeGroup[n]) {
      if (groupStateDefault[n]) {
        loudnessReference = fAddNorm(loudnessReference, lr_e, tmp1, tmp1_e, &lr_e);
      }
      if (groupStateInteractivity[n]) {
        loudnessAfterInteract = fAddNorm(loudnessAfterInteract, lai_e, tmp2, tmp2_e, &lai_e);
      }
    }
  }

  if ((loudnessReference <= (FIXP_DBL)0) || (loudnessAfterInteract <= (FIXP_DBL)0))
    return (FIXP_DBL)0;

  if ((lr_e == lai_e) && (loudnessAfterInteract == loudnessReference)) return (FIXP_DBL)0;

  loudnessCompensationGainPow2 = fDivNorm(loudnessReference, loudnessAfterInteract, &lcgp2_e);
  lcgp2_e += lr_e - lai_e;

  /* loudness compensation gain in dB */
  /* 10*log10(lin_val) = 20/log2(10)*log2(lin_val) */
  loudnessCompensationGainDb = fMultDiv2(FL2FXCONST_DBL(3.01029996 / (float)(1 << 2)),
                                         fLog2(loudnessCompensationGainPow2, lcgp2_e, &lcgdb_e));
  lcgdb_e += 2 + 1;
  loudnessCompensationGainDb = scaleValue(loudnessCompensationGainDb, lcgdb_e - 8); /* e = 8 */

  if (minGain == 15)
    minGainDb = (FIXP_DBL)MINVAL_DBL;
  else
    minGainDb = FIXP_DBL(-3 * minGain) << (31 - 8);
  maxGainDb = FIXP_DBL(3 * maxGain) << (31 - 8);

  /* clip loudness compensation gain to min/max gain */
  loudnessCompensationGainDb = fMax(fMin(loudnessCompensationGainDb, maxGainDb), minGainDb);

  return loudnessCompensationGainDb;
}

/* Loudness Compensation after Gain Interactivity according to Amendmend 3 to MPEG-H standard:
   Keep output loudness after user interaction on groups. */
static FIXP_DBL getLoudnessCompensationGainDb(AUDIO_SCENE_INFO* pASI,
                                              USER_INTERACTIVITY_STATUS* pUiStatus,
                                              HANDLE_SEL_PROC_OUTPUT pUniDrcSelProcOutput) {
  int groupLoudnessValueMissingFlag = 0, c, g, i, j, grpIdx, presetIdx, switchGrpIdx;
  UCHAR includeGroup[ASI_MAX_GROUPS], groupStateDefault[ASI_MAX_GROUPS],
      groupStateInteractivity[ASI_MAX_GROUPS];
  FIXP_DBL groupLoudness[ASI_MAX_GROUPS]; /* e = 8 */
  UCHAR minGain;
  UCHAR maxGain;
  UCHAR groupGainDefaultDb[ASI_MAX_GROUPS];
  UCHAR groupGainInteractivityDb[ASI_MAX_GROUPS];
  FIXP_DBL loudnessCompensationGainDb = (FIXP_DBL)0;
  ASI_LOUDNESS_COMPENSATION_DATA* pLcd = &(pASI->loudnessCompensationData);

  if (!pASI->loudCompPresent) {
    return loudnessCompensationGainDb;
  }

  if (pLcd->groupLoudnessPresent) {
    for (g = 0; g < pASI->numGroups; g++) { /* loudnessCompGroupLoudness in dB = 0.25 *
                                               mae_bsLoudnessCompGroupLoudness - 57.75*/
      groupLoudness[g] = (FIXP_DBL)(pLcd->groupLoudness[g] - 231) << (31 - 8 - 2); /* e = 8 */
    }
  } else {
    int groupLoudnessAvailable = 0;
    for (g = 0; g < pASI->numGroups; g++) {
      groupLoudness[g] =
          (FIXP_DBL)FDK_drcDec_GetGroupLoudness(pUniDrcSelProcOutput, pASI->groups[g].groupID,
                                                &groupLoudnessAvailable) >>
          1; /* e = 8 */
      if (!groupLoudnessAvailable) {
        /* loudness for this group not found */
        groupLoudnessValueMissingFlag = 1;
        break;
      }
    }
  }

  /* fallbacks for non-present cases */
  for (g = 0; g < pASI->numGroups; g++) {
    includeGroup[g] = 1;
    groupGainDefaultDb[g] = 191; /* 0 dB */
  }
  minGain = 15; /* -inf dB */
  maxGain = 7;  /* 21 dB */

  /* get default state and current interactivity gain and state */
  for (g = 0; g < pASI->numGroups; g++) {
    groupStateDefault[g] = pASI->groups[g].defaultOnOff;
    if (pUiStatus->groupData[g].changeGain)
      groupGainInteractivityDb[g] = pUiStatus->groupData[g].gain;
    else
      groupGainInteractivityDb[g] = 64; /* 0 dB */
    if ((pUiStatus->groupData[g].changeGain) && (pUiStatus->groupData[g].gain == 0))
      groupStateInteractivity[g] = 0;
    else
      groupStateInteractivity[g] = pUiStatus->groupData[g].onOff;
  }
  for (i = 0; i < pASI->numSwitchGroups; i++) {
    for (j = 0; j < pASI->switchGroups[i].numMembers; j++) {
      grpIdx = asiGroupID2idx(pASI, (int)pASI->switchGroups[i].memberID[j]);
      if (grpIdx >= 0) /* index to group found */
        groupStateDefault[grpIdx] = 0;
    }
    grpIdx = asiGroupID2idx(pASI, (int)pASI->switchGroups[i].defaultGroupID);
    if (grpIdx >= 0) /* index to group found */
      groupStateDefault[grpIdx] = pASI->switchGroups[i].defaultOnOff;
  }

  if (pUiStatus->interactionMode) /* preset selected */
  {
    presetIdx = asiGroupPresetID2idx(pASI, pUiStatus->groupPresetID);
    if (presetIdx >= 0) /* index to groupPreset found */
    {
      ASI_GROUP_PRESET_CONDITION* conditions;
      int numConditions;

      if (pLcd->presetParamsPresent[presetIdx]) {
        for (g = 0; g < pASI->numGroups; g++) {
          includeGroup[g] = pLcd->presetIncludeGroup[presetIdx][g];
        }
        if (pLcd->presetMinMaxGainPresent[presetIdx]) {
          minGain = pLcd->presetMinGain[presetIdx];
          maxGain = pLcd->presetMaxGain[presetIdx];
        }
      }

      if (pASI->groupPresets[presetIdx].hasDownmixIdExtension) {
        numConditions = pASI->groupPresets[presetIdx].downmixIdExtension.numConditions;
        conditions = pASI->groupPresets[presetIdx].downmixIdExtension.conditions;
      } else {
        numConditions = pASI->groupPresets[presetIdx].numConditions;
        conditions = pASI->groupPresets[presetIdx].conditions;
      }

      for (c = 0; c < numConditions; c++) {
        /* find condition matching to group */
        if (conditions[c].isSwitchGroupCondition) {
          switchGrpIdx = asiSwitchGroupID2idx(pASI, conditions[c].referenceID);
          if (switchGrpIdx >= 0) /* index to switchGroup found */
          {
            for (i = 0; i < pASI->switchGroups[switchGrpIdx].numMembers; i++) {
              grpIdx = asiGroupID2idx(pASI, (int)pASI->switchGroups[switchGrpIdx].memberID[i]);
              if (grpIdx >= 0) { /* index to group found */
                groupStateDefault[grpIdx] = 0;
                if (conditions[c].gainFlag) groupGainDefaultDb[grpIdx] = conditions[c].gain;
              }
            }
            grpIdx = asiGroupID2idx(pASI, (int)pASI->switchGroups[switchGrpIdx].defaultGroupID);
            if (grpIdx >= 0) /* index to group found */
            {
              groupStateDefault[grpIdx] = conditions[c].conditionOnOff;
            }
          }
        } else {
          /* get group index */
          grpIdx = asiGroupID2idx(pASI, conditions[c].referenceID);
          if (grpIdx >= 0) /* index to group found */
          {
            /* if group belongs to a switch group, deactivate all other members of this switch group
             */
            for (i = 0; i < pASI->numSwitchGroups; i++) {
              int partOfSwitchGroup = 0;
              for (j = 0; j < pASI->switchGroups[i].numMembers; j++) {
                if (conditions[c].referenceID == pASI->switchGroups[i].memberID[j])
                  partOfSwitchGroup = 1;
              }
              if (partOfSwitchGroup) {
                for (j = 0; j < pASI->switchGroups[i].numMembers; j++) {
                  int grpIdx2 = asiGroupID2idx(pASI, (int)pASI->switchGroups[i].memberID[j]);
                  if (grpIdx2 >= 0) { /* index to group found */
                    groupStateDefault[grpIdx2] = 0;
                    if (conditions[c].gainFlag) groupGainDefaultDb[grpIdx2] = conditions[c].gain;
                  }
                }
              }
            }
            if (conditions[c].gainFlag) groupGainDefaultDb[grpIdx] = conditions[c].gain;
            groupStateDefault[grpIdx] = conditions[c].conditionOnOff;
          }
        }
      }
    }
  } else /* default scene (no preset selected) */
  {
    if (pLcd->defaultParamsPresent) {
      for (g = 0; g < pASI->numGroups; g++) {
        includeGroup[g] = pLcd->defaultIncludeGroup[g];
      }
      if (pLcd->defaultMinMaxGainPresent) {
        minGain = pLcd->defaultMinGain;
        maxGain = pLcd->defaultMaxGain;
      }
    }
  }

  /* set group gain to default if changeGain is false */
  for (g = 0; g < pASI->numGroups; g++) {
    if (!pUiStatus->groupData[g].changeGain) {
      INT tmp = (((INT)groupGainDefaultDb[g] - 255) >> 1) + 32 + 64;
      tmp = fMax(tmp, 0);
      groupGainInteractivityDb[g] = (UCHAR)tmp;
    }
  }

  loudnessCompensationGainDb =
      computeLoudnessCompensationGain(pASI->numGroups, includeGroup, groupLoudnessValueMissingFlag,
                                      groupLoudness, groupGainDefaultDb, groupGainInteractivityDb,
                                      groupStateDefault, groupStateInteractivity, minGain, maxGain);

  return loudnessCompensationGainDb; /* e = 8 */
}

AAC_DECODER_ERROR FDK_mpeghUiInitialize(HANDLE_AACDECODER self) {
  int tpErr;

  /* Enabled automatic UI manager mode by default */
  self->uiManagerEnabled = -1;
  self->uiManagerActive = 1;
  self->uiStatusValid = 0;
  self->drcStatusValid = 0;
  self->uiSignalChanged = 0;
  FDKmemclear(&self->uiStatus, sizeof(self->uiStatus));
  FDKmemclear(&self->uiStatusNext, sizeof(self->uiStatusNext));
  updateOnOffFlags(self);

  tpErr =
      transportDec_RegisterUserInteractCallback(self->hInput, mpegh3daInteractivity, (void*)self);
  if (tpErr == 0) {
    return AAC_DEC_OK;
  } else {
    return AAC_DEC_OUT_OF_MEMORY;
  }
}

int updateUiStatus(HANDLE_AACDECODER self) {
  int valid = 0;

  if (self->uiManagerEnabled != 0) {
    if (self->uiManagerEnabled > 0) {
      self->uiManagerActive = 1;
    } else {
      if (self->pUsacConfig[0]) {
        self->uiManagerActive = self->pUsacConfig[0]->uiManagerActive;
      }
    }
  } else {
    self->uiManagerActive = 0;
  }

  UI_Manager_SetIsActive(self->hUiManager, self->uiManagerActive);

  if (self->uiManagerActive) {
    if (UI_Manager_GetInteractivityStatus(self->hUiManager, &self->uiStatusNext,
                                          &self->uiSignalChanged) == UI_MANAGER_OK) {
      valid = 1;
    }
  }
  if (self->uiStatusValid) {
    valid = 1;
  }

  if (!valid) {
    return 1;
  }

  if (!self->uiSignalChanged || !self->useElementSkipping) {
    self->uiStatus = self->uiStatusNext;
    updateOnOffFlags(self);
  }

  return 0;
}

void updateOnOffFlags(HANDLE_AACDECODER self) {
  AUDIO_SCENE_INFO* pASI;
  int grp, asiGrpIdx, sigGrpIdx, sigIdx, maeId, grpMem;

  if (self->pUsacConfig[0] == NULL) {
    return;
  }

  /* clear skipped signals flags */
  FDKmemclear(self->signalSkipped, TP_MPEGH_MAX_SIGNAL_GROUPS * sizeof(UCHAR));

  /* check if element skipping active */
  if (!self->useElementSkipping || !(self->flags[0] & AC_MPEGH3DA) ||
      !self->pUsacConfig[0]->elementLengthPresent)
    return;

  /* get ASI */
  pASI = UI_Manager_GetAsiPointer(self->hUiManager);
  if (!pASI->numGroups) return;

  /* loop over groups */
  for (grp = 0; grp < self->uiStatus.numGroups; grp++) {
    /* if group is off ... */
    if (!self->uiStatus.groupData[grp].onOff) {
      asiGrpIdx = asiGroupID2idx(pASI, self->uiStatus.groupData[grp].groupID);
      if (asiGrpIdx < 0) continue;

      /* ... and group is switch group member ... */
      if (pASI->groups[asiGrpIdx].switchGroupID != 255) {
        /* ... set skipped flag for signals in group */
        for (grpMem = 0; grpMem < pASI->groups[asiGrpIdx].numMembers; grpMem++) {
          /* determine signal index from MAE ID */
          maeId = pASI->groups[asiGrpIdx].metaDataElementID[grpMem];
          sigIdx = 0;

          for (int streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
            if (self->pUsacConfig[streamIndex] == NULL) break;
            for (sigGrpIdx = 0; sigGrpIdx < self->pUsacConfig[streamIndex]->bsNumSignalGroups;
                 sigGrpIdx++) {
              if (self->pUsacConfig[0]->m_signalGroupType[sigGrpIdx].type != 3) { /* not HOA */
                if (maeId >= self->pUsacConfig[streamIndex]->m_signalGroupType[sigGrpIdx].count) {
                  maeId -= self->pUsacConfig[streamIndex]->m_signalGroupType[sigGrpIdx].count;
                  sigIdx += self->pUsacConfig[streamIndex]->m_signalGroupType[sigGrpIdx].count;
                } else {
                  sigIdx += maeId;
                  maeId = 0;
                  break;
                }
              }
            }
          }

          /* set skipped flag */
          if (maeId == 0) {
            self->signalSkipped[sigIdx] = 1;
          }
        }
      }
    }
  }
}

int getOnOffFlag(HANDLE_AACDECODER self, const int signal) {
  return !self->signalSkipped[signal];
}

/* apply user interactivity */
AAC_DECODER_ERROR applyUserInteractivity(HANDLE_AACDECODER self, PCM_DEC* pTimeData) {
  UI_DRC_LOUDNESS_STATUS drcStatus;
  HANDLE_SEL_PROC_OUTPUT pUniDrcSelProcOutput =
      FDK_drcDec_GetSelectionProcessOutput(self->hUniDrcDecoder);
  FIXP_DBL channelGain[((28) * 2)]; /* e = 8 */
  int metadataElementIDLast = 0;
  UCHAR maeIDstartCh[((28) * 2)], maeIDstopCh[((28) * 2)];
  UCHAR maeIDobjGrp[((28) * 2)], maeIDobjIdx[((28) * 2)], objGrp;
  int grp, valid = 0, ch, streamIndex = 0;

  FDKmemclear(channelGain, sizeof(channelGain));
  FDKmemset(maeIDobjGrp, 255, ((28) * 2));
  FDKmemclear(maeIDobjIdx, sizeof(maeIDobjIdx));
  FDKmemclear(maeIDstartCh, sizeof(maeIDstartCh));
  FDKmemclear(maeIDstopCh, sizeof(maeIDstopCh));

  /* generate lookup tables */
  objGrp = 0;
  int signalsSkipped = 0, signalsPrevStreams = 0;
  for (streamIndex = 0; streamIndex < TPDEC_MAX_TRACKS; streamIndex++) {
    int metadataElementID;
    if (self->pUsacConfig[streamIndex] == NULL) break;
    metadataElementID =
        UI_Manager_GetAsiPointer(self->hUiManager)->metaDataElementIDoffset[streamIndex];
    for (grp = 0; grp < self->pUsacConfig[streamIndex]->bsNumSignalGroups; grp++) {
      switch (self->pUsacConfig[streamIndex]->m_signalGroupType[grp].type) {
        case 0: {
          int id = 0, idIdx;
          for (idIdx = 0; idIdx < self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;
               idIdx++) {
            if (!getOnOffFlag(self,
                              self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                                  signalsPrevStreams + idIdx)) {
              signalsSkipped++;
              metadataElementID++;
              continue;
            }
            if (metadataElementID >= ((28) * 2)) {
              return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            }
            maeIDstartCh[metadataElementID] =
                self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                signalsPrevStreams + id - signalsSkipped;
            maeIDstopCh[metadataElementID] =
                self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                signalsPrevStreams + id + 1 - signalsSkipped;
            metadataElementID++;
            id++;
          }
          break;
        }
        case 1: {
          int id = 0, idIdx, objGrpSkipped = 1;
          for (idIdx = 0; idIdx < self->pUsacConfig[streamIndex]->m_signalGroupType[grp].count;
               idIdx++) {
            if (!getOnOffFlag(self,
                              self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                                  signalsPrevStreams + idIdx)) {
              signalsSkipped++;
              metadataElementID++;
              continue;
            }
            if ((self->hgVBAPRenderer[objGrp] == NULL) ||
                (id >= self->hgVBAPRenderer[objGrp]->numObjects)) {
              return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            }
            if (metadataElementID >= ((28) * 2)) {
              return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            }
            maeIDstartCh[metadataElementID] =
                self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                signalsPrevStreams + id - signalsSkipped;
            maeIDstopCh[metadataElementID] =
                self->pUsacConfig[streamIndex]->m_signalGroupType[grp].firstSigIdx +
                signalsPrevStreams + id + 1 - signalsSkipped;
            maeIDobjGrp[metadataElementID] = objGrp;
            maeIDobjIdx[metadataElementID] = id;
            metadataElementID++;
            id++;
            objGrpSkipped = 0;
          }
          if (!objGrpSkipped) objGrp++;
          break;
        }
        case 2:
          return AAC_DEC_UNSUPPORTED_FORMAT; /* SAOC not supported */
        case 3:
          return AAC_DEC_UNSUPPORTED_FORMAT;
      }
    }
    signalsPrevStreams += self->ascChannels[streamIndex];
    metadataElementIDLast = fMax(metadataElementIDLast, metadataElementID);
  }

  if (metadataElementIDLast > ((28) * 2)) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  if (updateUiStatus(self) == 0) {
    valid = 1;
  }

  if (self->useElementSkipping && self->uiSignalChanged) {
    if ((self->flushStatus == AACDEC_FLUSH_OFF) && (self->buildUpStatus == AACDEC_BUILD_UP_OFF) &&
        (self->truncateStopOffset ==
         -128)) { /* make sure no config change is in progress before forcing one */
      transportDec_SetParam(self->hInput, TPDEC_PARAM_FORCE_CONFIG_CHANGE, 0);
      self->applyCrossfade |= AACDEC_CROSSFADE_BITMASK_UI; /* cross-fade shall be applied */
    }
  }

  if (valid) {
    AUDIO_SCENE_INFO* pASI;
    int i, j;
    FIXP_DBL loudnessCompensationGainDb;

    pASI = UI_Manager_GetAsiPointer(self->hUiManager);

    if (pASI == NULL) {
      return AAC_DEC_INVALID_HANDLE;
    }

    loudnessCompensationGainDb =
        getLoudnessCompensationGainDb(pASI, &self->uiStatus, pUniDrcSelProcOutput);

    for (ch = 0; ch < self->streamInfo.numChannels; ch++) {
      channelGain[ch] = loudnessCompensationGainDb;
    }

    for (i = 0; i < self->uiStatus.numGroups; i++) {
      int grpIdx, onOff;
      FIXP_DBL gainDb = 0;
      INT azOffset = 0, elOffset = 0;
      FIXP_DBL azOffsetConv = 0, elOffsetConv = 0;

      /* get group index */
      grpIdx = asiGroupID2idx(pASI, self->uiStatus.groupData[i].groupID);
      if (grpIdx < 0) continue;

      /* group preset gain and position offset */
      if (self->uiStatus.interactionMode) {
        int presetIdx = asiGroupPresetID2idx(pASI, self->uiStatus.groupPresetID);

        if (presetIdx >= 0) {
          ASI_GROUP_PRESET_CONDITION* conditions;
          int numConditions;

          if (pASI->groupPresets[presetIdx].hasDownmixIdExtension) {
            numConditions = pASI->groupPresets[presetIdx].downmixIdExtension.numConditions;
            conditions = pASI->groupPresets[presetIdx].downmixIdExtension.conditions;
          } else {
            numConditions = pASI->groupPresets[presetIdx].numConditions;
            conditions = pASI->groupPresets[presetIdx].conditions;
          }

          for (j = 0; j < numConditions; j++) {
            if ((!conditions[j].isSwitchGroupCondition &&
                 (conditions[j].referenceID == self->uiStatus.groupData[i].groupID)) ||
                (conditions[j].isSwitchGroupCondition &&
                 (conditions[j].referenceID == pASI->groups[grpIdx].switchGroupID))) {
              if (conditions[j].gainFlag) {
                gainDb = (((FIXP_DBL)conditions[j].gain - (FIXP_DBL)255) << (31 - 8 - 1)) +
                         ((FIXP_DBL)32 << (31 - 8));
              }

              if (conditions[j].positionFlag) {
                azOffset = (INT)conditions[j].azOffset - 128;
                elOffset = (INT)conditions[j].elOffset - 32;
              }
            }
          }
        }
      }

      /* get on/off and gain */
      if (!self->uiStatus.groupData[i].onOff ||
          (self->uiStatus.groupData[i].changeGain && (self->uiStatus.groupData[i].gain == 0))) {
        onOff = 0;
      } else {
        onOff = 1;

        if (self->uiStatus.groupData[i].changeGain) {
          gainDb = (FIXP_DBL)fMin(fMax((INT)self->uiStatus.groupData[i].gain - 64, -63), 31)
                   << (31 - 8);
        }
      }

      if (self->uiStatus.groupData[i].changePosition) {
        azOffset = (INT)self->uiStatus.groupData[i].azOffset - 128;
        elOffset = (INT)self->uiStatus.groupData[i].elOffset - 32;
      }

      /* convert position offset */
      azOffset = fMax(-120, fMin(120, azOffset));
      azOffsetConv = fMult((FIXP_SGL)((LONG)azOffset << 8), FL2FXCONST_SGL(1.5f / 180.f * 64)) << 1;

      elOffset = fMax(-30, fMin(30, elOffset));
      elOffsetConv = fMult((FIXP_SGL)((LONG)elOffset << 10), FL2FXCONST_SGL(3.f / 180.f * 32));

      /* apply to group members */
      for (j = 0; j < pASI->groups[grpIdx].numMembers; j++) {
        int maeID = pASI->groups[grpIdx].metaDataElementID[j];

        if (maeID >= metadataElementIDLast) continue;

        /* gain */
        for (ch = maeIDstartCh[maeID]; ch < maeIDstopCh[maeID]; ch++) {
          if (channelGain[ch] == (FIXP_DBL)MINVAL_DBL) /* sanity check to prevent underflow */
            continue;
          if (onOff) {
            channelGain[ch] = fAddSaturate(channelGain[ch], gainDb);
          } else {
            channelGain[ch] = (FIXP_DBL)MINVAL_DBL; /* -inf dB */
          }
        }

        /* position offset */
        if (maeIDobjGrp[maeID] != 255 && self->hgVBAPRenderer[maeIDobjGrp[maeID]] != NULL) {
          int k;

          for (k = 0; k < self->hgVBAPRenderer[maeIDobjGrp[maeID]]->numOamFrames; k++) {
            LONG tmp;

            tmp = (LONG)self->hgVBAPRenderer[maeIDobjGrp[maeID]]
                      ->oamSamples[k][maeIDobjIdx[maeID]]
                      .sph.azi;
            tmp = (LONG)((INT64)tmp + (LONG)azOffsetConv);
            self->hgVBAPRenderer[maeIDobjGrp[maeID]]->oamSamples[k][maeIDobjIdx[maeID]].sph.azi =
                (FIXP_DBL)tmp;

            tmp = (LONG)self->hgVBAPRenderer[maeIDobjGrp[maeID]]
                      ->oamSamples[k][maeIDobjIdx[maeID]]
                      .sph.ele;
            tmp = (LONG)((INT64)tmp + (LONG)elOffsetConv);
            self->hgVBAPRenderer[maeIDobjGrp[maeID]]->oamSamples[k][maeIDobjIdx[maeID]].sph.ele =
                (FIXP_DBL)tmp;
          }
        }
      }
    }
    {
      /* transfer groupPreset and active groups to uniDrc decoder */
      int numGroupIdsRequested = 0, groupPresetIdRequested = -1;
      int groupIdsRequested[ASI_MAX_GROUPS];

      if (self->uiStatus.interactionMode) {
        groupPresetIdRequested = self->uiStatus.groupPresetID;
      }
      for (i = 0; i < self->uiStatus.numGroups; i++) {
        if (self->uiStatus.groupData[i].onOff) {
          groupIdsRequested[numGroupIdsRequested] = self->uiStatus.groupData[i].groupID;
          numGroupIdsRequested++;
        }
      }
      FDK_drcDec_SetSelectionProcessMpeghParameters_simple(
          self->hUniDrcDecoder, groupPresetIdRequested, numGroupIdsRequested, groupIdsRequested);
    }
  }

  /* transfer selected DRC effect and target loudness to uniDrc decoder */
  valid = 0;
  if (self->uiManagerActive) {
    if (UI_Manager_GetDrcLoudnessStatus(self->hUiManager, &drcStatus) == UI_MANAGER_OK) valid = 1;
  }
  if (self->drcStatusValid) {
    drcStatus = self->drcStatus;
    valid = 1;
  }
  if (valid) {
    DRC_DEC_ERROR drcErr;
    if (drcStatus.targetLoudness == UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS) {
      /* use target loudness that was set with the API */
      if (self->defaultTargetLoudness >= 0) {
        drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_LOUDNESS_NORMALIZATION_ON,
                                     (FIXP_DBL)1);
        if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
        drcErr = FDK_drcDec_SetParam(
            self->hUniDrcDecoder, DRC_DEC_TARGET_LOUDNESS,
            (INT)-self->defaultTargetLoudness * FL2FXCONST_DBL(1.0f / (float)(1 << 9)));
        if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
      } else {
        /* Loudness Normalization off*/
        drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_LOUDNESS_NORMALIZATION_ON,
                                     (FIXP_DBL)0);
        if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
      }
    } else {
      /* use target loudness from UI manager */
      drcErr =
          FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_LOUDNESS_NORMALIZATION_ON, (FIXP_DBL)1);
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
      drcErr = FDK_drcDec_SetParam(
          self->hUniDrcDecoder, DRC_DEC_TARGET_LOUDNESS,
          (INT)drcStatus.targetLoudness * FL2FXCONST_DBL(1.0f / (float)(1 << 7)));
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    }

    if (drcStatus.drcSelected == UI_MANAGER_USE_DEFAULT_DRC_SELECTED) {
      /* use DRC effect type request that was set with the API */
      drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_EFFECT_TYPE_FALLBACK_CODE,
                                   (FIXP_DBL)getDrcEffectTypeFallback(self->defaultDrcSetEffect));
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    } else {
      /* use DRC effect type request from UI manager */
      drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_EFFECT_TYPE_FALLBACK_CODE,
                                   (FIXP_DBL)(LONG)drcStatus.drcSelected);
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    }

    if (drcStatus.albumMode != -1) {
      drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_ALBUM_MODE,
                                   (FIXP_DBL)drcStatus.albumMode);
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    }

    if (drcStatus.boost != -1) {
      drcErr =
          FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_BOOST, FX_SGL2FX_DBL(drcStatus.boost));
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    }

    if (drcStatus.compress != -1) {
      drcErr = FDK_drcDec_SetParam(self->hUniDrcDecoder, DRC_DEC_COMPRESS,
                                   FX_SGL2FX_DBL(drcStatus.compress));
      if (drcErr) return AAC_DEC_SET_PARAM_FAIL;
    }

    self->drcStatus = drcStatus;
  }

  /* set UI gains (in dB) which will be applied in DRC decoder */
  FDK_drcDec_SetChannelGains(self->hUniDrcDecoder, 1, self->streamInfo.numChannels,
                             self->streamInfo.frameSize, channelGain, pTimeData,
                             self->streamInfo.frameSize + 256);

  return AAC_DEC_OK;
}
