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

#ifndef UIMANAGERINTERNAL_H
#define UIMANAGERINTERNAL_H

#include "common_fix.h"
#include "tpdec_lib.h"

#define PRESET_ID_AUTO 255
#define INVALID_ID 255
#define INVALID_IDX 255
#define NUM_PREF_LANGUAGES 10
#define NUM_ACCESSIBILITY_MODES 5

#define XML_START_INDEX (-1)
#define XML_END_INDEX 127

#define MAX_XML_TEXT_LENGTH 40
#define FLAG_XML_PARAM_INT 1
#define FLAG_XML_PARAM_FLOAT 2
#define FLAG_XML_PARAM_BOOL 4
#define FLAG_XML_PARAM_TEXT 8

/* UI commands */
typedef enum {
  UI_MANAGER_COMMAND_RESET = 0,

  UI_MANAGER_COMMAND_DRC_SELECTED = 10,
  UI_MANAGER_COMMAND_DRC_BOOST = 11,
  UI_MANAGER_COMMAND_DRC_COMPRESS = 12,
  UI_MANAGER_COMMAND_TARGET_LOUDNESS = 20,
  UI_MANAGER_COMMAND_ALBUM_MODE = 21,

  UI_MANAGER_COMMAND_PRESET_SELECTED = 30,
  UI_MANAGER_COMMAND_ACCESSIBILITY_PREFERENCE = 31,

  UI_MANAGER_COMMAND_AUDIO_ELEMENT_MUTING_CHANGED = 40,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_BALANCE_CHANGED = 41,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_AZIMUTH_CHANGED = 42,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_ELEVATION_CHANGED = 43,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_DISTANCE_CHANGED = 44,

  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_SELECTED = 60,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_MUTING_CHANGED = 61,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_BALANCE_CHANGED = 62,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_AZIMUTH_CHANGED = 63,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_ELEVATION_CHANGED = 64,
  UI_MANAGER_COMMAND_AUDIO_ELEMENT_SWITCH_DISTANCE_CHANGED = 65,

  UI_MANAGER_COMMAND_AUDIO_LANGUAGE_SELECTED = 70,
  UI_MANAGER_COMMAND_INTERFACE_LANGUAGE_SELECTED = 71,

  UI_MANAGER_COMMAND_SET_GUID = 90,

  UI_MANAGER_COMMAND_PRESET_SELECTED_NO_UUID = 95,

  UI_MANAGER_COMMAND_INVALID = 255
} UI_MANAGER_COMMAND;

/* group state */
typedef struct {
  UCHAR onOff;
  UCHAR allowOnOff;

  SHORT gain;
  SHORT defaultGain;
  UCHAR allowGainInteractivity;

  UCHAR azOffset, elOffset, distFactor;
  UCHAR defaultAzOffset, defaultElOffset, defaultDistFactor;
  UCHAR allowPositionInteractivity;

  UCHAR isAvailable;
} UI_STATE_GROUP;

/* switch group state */
typedef struct {
  UCHAR onOff;
  UCHAR activeMemberIndex;
  UCHAR allowSwitch;
  UCHAR isAvailable;
} UI_STATE_SWITCH_GROUP;

/* group preset state */
typedef struct {
  UCHAR isAvailable;
} UI_STATE_GROUP_PRESET;

/* UI state */
typedef struct {
  UCHAR uuid[16];

  UI_STATE_GROUP groups[ASI_MAX_GROUPS];
  UI_STATE_SWITCH_GROUP switchGroups[ASI_MAX_SWITCH_GROUPS];
  UI_STATE_GROUP_PRESET groupPresets[ASI_MAX_GROUP_PRESETS];
  UCHAR activePresetIndex;
  char prefAudioLanguages[NUM_PREF_LANGUAGES][3];
  UCHAR accessibilityPreference;

  SCHAR targetLoudness;
  ULONG drcSelected;
  SCHAR albumMode;
  SHORT boost;
  SHORT compress;
} UI_STATE;

/* XML writer state */
typedef struct {
  char* pOut;
  UINT nLeft;
  char* pLastValidPos;
  SCHAR nextPresetIdx;
  SCHAR nextGroupIdx;
  SCHAR nextSwitchGroupMemberIdx;
} UI_MANAGER_XML_WRITER;

/* persistence */
struct UI_PERSISTENCE_MANAGER;
typedef struct UI_PERSISTENCE_MANAGER* HANDLE_UI_PERSISTENCE_MANAGER;

/* UI manager */
typedef struct UI_MANAGER {
  AUDIO_SCENE_INFO asi;
  UI_STATE uiState;
  UCHAR xmlStateChanged;
  UCHAR uiStateChanged;
  UCHAR drcStateChanged;
  UCHAR configChanged;
  UCHAR isActive;
  UI_MANAGER_XML_WRITER xmlWriter;
  HANDLE_UI_PERSISTENCE_MANAGER hPersistence;
} UI_MANAGER;

/* UI action params */
typedef struct {
  UCHAR uuid[16];
  UCHAR actionType;
  LONG paramInt;
  FIXP_DBL paramFloat;
  UCHAR paramBool;
  char paramText[MAX_XML_TEXT_LENGTH + 1];
  UCHAR presentFlags;
} UI_MANAGER_ACTION;

int getMinPresetID(UI_MANAGER* hUiManager);
void simulatePreset(UI_MANAGER* hUiManager, UCHAR presetID, UI_STATE* pUiState);

#endif
