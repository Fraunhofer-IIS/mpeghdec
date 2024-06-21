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

   Description: MPEG Transport decoder

*******************************************************************************/

#ifndef TPDEC_LIB_H
#define TPDEC_LIB_H

#include "tp_data.h"

#include "FDK_bitstream.h"

typedef enum {
  TRANSPORTDEC_OK = 0, /*!< All fine.                                                  */

  /* Synchronization errors. Wait for new input data and try again. */
  tpdec_sync_error_start = 0x100,
  TRANSPORTDEC_NOT_ENOUGH_BITS, /*!< Out of bits. Provide more bits and try again.              */
  TRANSPORTDEC_SYNC_ERROR,      /*!< No sync was found or sync got lost. Keep trying.           */
  tpdec_sync_error_end,

  /* Decode errors. Mostly caused due to bit errors. */
  tpdec_decode_error_start = 0x400,
  TRANSPORTDEC_PARSE_ERROR, /*!< Bitstream data showed inconsistencies (wrong syntax).      */
  TRANSPORTDEC_UNSUPPORTED_FORMAT, /*!< Unsupported format or feature found in the bitstream data.
                                    */
  TRANSPORTDEC_CRC_ERROR, /*!< CRC error encountered in bitstream data.                   */
  tpdec_decode_error_end,

  /* Fatal errors. Stop immediately on one of these errors! */
  tpdec_fatal_error_start = 0x200,
  TRANSPORTDEC_UNKOWN_ERROR,      /*!< An unknown error occured.                                  */
  TRANSPORTDEC_INVALID_PARAMETER, /*!< An invalid parameter was passed to a function.             */
  TRANSPORTDEC_NEED_TO_RESTART,   /*!< The decoder needs to be restarted, since the requiered
                                       configuration change cannot be performed.                  */
  TRANSPORTDEC_TOO_MANY_BITS,     /*!< In case of packet based formats: Supplied number of bits
                                       exceed the size of the internal bit buffer.                */
  tpdec_fatal_error_end

} TRANSPORTDEC_ERROR;

/** Macro to identify decode errors. */
#define TPDEC_IS_DECODE_ERROR(err) \
  (((err >= tpdec_decode_error_start) && (err <= tpdec_decode_error_end)) ? 1 : 0)
/** Macro to identify fatal errors. */
#define TPDEC_IS_FATAL_ERROR(err) \
  (((err >= tpdec_fatal_error_start) && (err <= tpdec_fatal_error_end)) ? 1 : 0)

/**
 * \brief Parameter identifiers for transportDec_SetParam()
 */
typedef enum {
  TPDEC_PARAM_MINIMIZE_DELAY =
      1, /** Delay minimization strategy. 0: none, 1: discard as many frames as possible. */
  TPDEC_PARAM_EARLY_CONFIG,          /** Enable early config discovery. */
  TPDEC_PARAM_IGNORE_BUFFERFULLNESS, /** Ignore buffer fullness. */
  TPDEC_PARAM_SET_BITRATE,         /** Set average bit rate for bit stream interruption frame misses
                                      estimation. */
  TPDEC_PARAM_RESET,               /** Reset transport decoder instance status. */
  TPDEC_PARAM_BURST_PERIOD,        /** Set data reception burst period in mili seconds. */
  TPDEC_PARAM_TARGETLAYOUT,        /** Set CICP target layout */
  TPDEC_PARAM_FORCE_CONFIG_CHANGE, /** Force config change for next received config */
  TPDEC_PARAM_USE_ELEM_SKIPPING
} TPDEC_PARAM;

/*!
  \brief               Reset Program Config Element.
  \param pPce          Program Config Element structure.
  \return              void
*/
void CProgramConfig_Reset(CProgramConfig* pPce);

/*!
  \brief               Initialize Program Config Element.
  \param pPce          Program Config Element structure.
  \return              void
*/
void CProgramConfig_Init(CProgramConfig* pPce);

/*!
  \brief               Inquire state of present Program Config Element structure.
  \param pPce          Program Config Element structure.
  \return              1 if the PCE structure is filled correct,
                       0 if no valid PCE present.
*/
int CProgramConfig_IsValid(const CProgramConfig* pPce);

/**
 * \brief Lookup and verify a given element. The decoder calls this
 *        method with every new element ID found in the bitstream.
 *
 * \param pPce        A valid Program config structure.
 * \param chConfig    MPEG-4 channel configuration.
 * \param tag         Tag of the current element to be looked up.
 * \param channelIdx  The current channel count of the decoder parser.
 * \param chMapping   Array to store the canonical channel mapping indexes.
 * \param chType      Array to store the audio channel type.
 * \param chIndex     Array to store the individual audio channel type index.
 * \param chDescrLen  Length of the output channel description array.
 * \param elMapping   Pointer where the canonical element index is stored.
 * \param elType      The element id of the current element to be looked up.
 *
 * \return            Non-zero if the element belongs to the current program, zero
 *                    if it does not.
 */
int CProgramConfig_LookupElement(CProgramConfig* pPce, UINT chConfig, const UINT tag,
                                 const UINT channelIdx, UCHAR chMapping[],
                                 AUDIO_CHANNEL_TYPE chType[], UCHAR chIndex[],
                                 const UINT chDescrLen, UCHAR* elMapping, MP4_ELEMENT_ID elList[],
                                 MP4_ELEMENT_ID elType);

/**
 * \brief             Get table of channel indices in the order of their appearance in by the
 * program config field.
 * \param pPce        A valid program config structure.
 * \param pceChMap    Array to store the channel mapping indices like they appear in the PCE.
 * \param pceChMapLen Lenght of the channel mapping index array (pceChMap).
 *
 * \return            Non-zero if any error occured otherwise zero.
 */
int CProgramConfig_GetPceChMap(const CProgramConfig* pPce, UCHAR pceChMap[],
                               const UINT pceChMapLen);

/**
 * \brief             Get table of elements in canonical order from a
 *                    give program config field.
 * \param pPce        A valid program config structure.
 * \param table       An array where the element IDs are stored.
 * \param elListSize  The length of the table array.
 * \param pChMapIdx   Pointer to a field receiving the corresponding
 *                    implicit channel configuration index of the given
 *                    PCE. If none can be found it receives the value 0.
 * \return            Total element count including all SCE, CPE and LFE.
 */
int CProgramConfig_GetElementTable(const CProgramConfig* pPce, MP4_ELEMENT_ID table[],
                                   const INT elListSize, UCHAR* pChMapIdx);

/**
 * \brief          Get channel description (type and index) for implicit
                   configurations (chConfig > 0) in MPEG canonical order.
 * \param chConfig MPEG-4 channel configuration.
 * \param chType   Array to store the audio channel type.
 * \param chIndex  Array to store the individual audio channel type index.
 * \return         void
 */
void CProgramConfig_GetChannelDescription(const UINT chConfig, const CProgramConfig* pPce,
                                          AUDIO_CHANNEL_TYPE chType[], UCHAR chIndex[]);

/**
 * \brief       Initialize a given AudioSpecificConfig structure.
 * \param pAsc  A pointer to an allocated CSAudioSpecificConfig struct.
 * \return      void
 */
void AudioSpecificConfig_Init(CSAudioSpecificConfig* pAsc);

/**
 * \brief   Parse a AudioSpecificConfig from a given bitstream handle.
 *
 * \param pAsc                         A pointer to an allocated CSAudioSpecificConfig struct.
 * \param hBs                          Bitstream handle.
 * \param fExplicitBackwardCompatible  Do explicit backward compatibility parsing if set (flag).
 * \param cb pointer to structure holding callback information
 * \param configMode Config modes: memory allocation mode or config change detection mode.
 * \param configChanged Indicates a config change.
 * \param m_aot in case of unequal AOT_NULL_OBJECT only the specific config is parsed.
 *
 * \return  Total element count including all SCE, CPE and LFE.
 */
TRANSPORTDEC_ERROR AudioSpecificConfig_Parse(CSAudioSpecificConfig* pAsc, HANDLE_FDK_BITSTREAM hBs,
                                             int fExplicitBackwardCompatible, CSTpCallBacks* cb,
                                             UCHAR configMode, UCHAR configChanged,
                                             AUDIO_OBJECT_TYPE m_aot);

/**
 * \brief   Parse a Mpegh3daConfig from a given bitstream handle.
 *
 * \param pAsc                         A pointer to an allocated CSAudioSpecificConfig struct.
 * \param hBs                          Bitstream handle.
 * \param fExplicitBackwardCompatible  Do explicit backward compatibility parsing if set (flag).
 * \param cb pointer to structure holding callback information
 * \param configMode Config modes: memory allocation mode or config change detection mode.
 * \param configChanged Indicates a config change.
 *
 * \return TPdec error code
 */
TRANSPORTDEC_ERROR Mpegh3daConfig_Parse(CSAudioSpecificConfig* pAsc, AUDIO_SCENE_INFO* pASI,
                                        HANDLE_FDK_BITSTREAM hBs,
                                        const int fExplicitBackwardCompatible,
                                        const CSTpCallBacks* cb, const UCHAR configMode,
                                        const UCHAR configChanged, const INT targetLayout,
                                        const INT subStreamIndex, INT* pLoudnessInfoSetPosition);

TRANSPORTDEC_ERROR earconInfo(HANDLE_FDK_BITSTREAM bs, EarconInfo* info);
TRANSPORTDEC_ERROR pcmDataConfig(HANDLE_FDK_BITSTREAM bs, EarconConfig* config);

/* CELP stuff */
enum { MPE = 0, RPE = 1, fs8KHz = 0, fs16KHz = 1 };

/* Defintion of flags that can be passed to transportDecOpen() */
#define TP_FLAG_MPEG4 1

#define CSAUDIOSPECIFICCONFIG_SIZE (TPDEC_MAX_TRACKS + 1)

typedef struct TRANSPORTDEC* HANDLE_TRANSPORTDEC;

/**
 * \brief Configure Transport Decoder via a binary coded AudioSpecificConfig or StreamMuxConfig.
 *        The previously requested configuration callback will be called as well. The buffer conf
 *        must containt a SMC in case of LOAS/LATM transport format, and an ASC elseways.
 *
 * \param hTp     Handle of a transport decoder.
 * \param conf    UCHAR buffer of the binary coded config (ASC or SMC).
 * \param length  The length in bytes of the conf buffer.
 *
 * \return        Error code.
 */
TRANSPORTDEC_ERROR transportDec_OutOfBandConfig(const HANDLE_TRANSPORTDEC hTp, UCHAR* conf,
                                                const UINT length, const UINT layer);

/**
 * \brief Configure Transport Decoder via a binary coded USAC/MPEGH3DA Config.
 *        The buffer newConfig contains a binary coded USAC/MPEGH3DA config of length
 * newConfigLength bytes. If the new config and the previous config are different configChanged is
 * set to 1 otherwise it is set to 0.
 *
 * \param hTp              Handle of a transport decoder.
 * \param newConfig        buffer of the binary coded config.
 * \param newConfigLength  Length of new config in bytes.
 * \param buildUpStatus    Indicates build up status: off|on|idle.
 * \param configChanged    Indicates if config changed.
 * \param layer            Instance layer.
 *
 * \return        Error code.
 */
TRANSPORTDEC_ERROR transportDec_InBandConfig(const HANDLE_TRANSPORTDEC hTp, UCHAR* newConfig,
                                             const UINT newConfigLength, const UCHAR buildUpStatus,
                                             UCHAR* configChanged, const UINT layer);

/**
 * \brief Open Transport medium for reading.
 *
 * \param transportDecFmt Format of the transport decoder medium to be accessed.
 * \param flags           Transport decoder flags. Currently only TP_FLAG_MPEG4, which signals a
 *                        MPEG4 capable decoder (relevant for ADTS only).
 *
 * \return   A pointer to a valid and allocated HANDLE_TRANSPORTDEC or a null pointer on failure.
 */
HANDLE_TRANSPORTDEC transportDec_Open(TRANSPORT_TYPE transportDecFmt, const UINT flags,
                                      const UINT nrOfLayer);

/**
 * \brief                Register configuration change callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle audio config changes.
 * \param user_data      void pointer for user data passed to the callback as first parameter.
 * \return               0 on success.
 */
int transportDec_RegisterAscCallback(HANDLE_TRANSPORTDEC hTp, const cbUpdateConfig_t cbUpdateConfig,
                                     void* user_data);

/**
 * \brief                Register decode frame callback.
 * \param hTp            Handle of transport decoder.
 * \param cbDecodeFrame  Pointer to a callback function to handle decode frame call.
 * \param user_data      void pointer for user data passed to the callback as first parameter.
 * \return               0 on success.
 */
int transportDec_RegisterDecodeFrameCallback(HANDLE_TRANSPORTDEC hTp,
                                             const cbDecodeFrame_t cbDecodeFrame, void* user_data);

/**
 * \brief                Register free memory callback.
 * \param hTp            Handle of transport decoder.
 * \param cbFreeMem      Pointer to a callback function to free config dependent memory.
 * \param user_data      void pointer for user data passed to the callback as first parameter.
 * \return               0 on success.
 */
int transportDec_RegisterFreeMemCallback(HANDLE_TRANSPORTDEC hTp, const cbFreeMem_t cbFreeMem,
                                         void* user_data);

/**
 * \brief                 Register config change control callback.
 * \param hTp             Handle of transport decoder.
 * \param cbCtrlCFGChange Pointer to a callback function for config change control.
 * \param user_data       void pointer for user data passed to the callback as first parameter.
 * \return                0 on success.
 */
int transportDec_RegisterCtrlCFGChangeCallback(HANDLE_TRANSPORTDEC hTp,
                                               const cbCtrlCFGChange_t cbCtrlCFGChange,
                                               void* user_data);

/**
 * \brief                 Register truncation message callback.
 * \param hTp             Handle of transport decoder.
 * \param cbCtrlCFGChange Pointer to a callback function for truncation message.
 * \return                0 on success.
 */
int transportDec_RegisterTruncationMsgCallback(HANDLE_TRANSPORTDEC hTpDec,
                                               const cbTruncationMsg_t cbTruncationMsg,
                                               void* user_data);

/**
 * \brief                Register uniDrcConfig and loudnessInfoSet parser callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle uniDrcConfig and loudnessInfoSet
 * parsing.
 * \param user_data      void pointer for user data passed to the callback as first parameter.
 * \return               0 on success.
 */
int transportDec_RegisterUniDrcConfigCallback(HANDLE_TRANSPORTDEC hTpDec, const cbUniDrc_t cbUniDrc,
                                              void* user_data, INT* pLoudnessInfoSetPosition);

/**
 * \brief                Register user interaction parser callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle user interaction parsing.
 * \param user_data      void pointer for user data passed to the callback as first parameter.
 * \return               0 on success.
 */
int transportDec_RegisterUserInteractCallback(HANDLE_TRANSPORTDEC hTpDec,
                                              const cbUserInteract_t cbUserInteract,
                                              void* user_data);

/**
 * \brief                  Register user Dmx Matrix parser callback.
 * \param hTp              Handle of transport decoder.
 * \param cbParseDmxMatrix Pointer to a callback function to handle user Dmx Matrix parser.
 * \param user_data        void pointer for user data passed to the callback as first parameter.
 * \return                 0 on success.
 */
int transportDec_RegisterParseDmxMatrixCallback(HANDLE_TRANSPORTDEC hTpDec,
                                                const cbParseDmxMatrix_t cbParseDmxMatrix,
                                                void* user_data);

/**
 * \brief                  Register earcon decoder set bistream callback.
 * \param hTp              Handle of transport decoder.
 * \param cbEarconBS       Pointer to a callback function to handle earcon decoder data.
 * \param user_data        void pointer for user data passed to the callback as first parameter.
 * \return                 0 on success.
 */
int transportDec_RegisterEarconSetBSCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                             const cbEarconBS_t cbEarconBS, void* user_data);

/**
 * \brief                  Register earcon decoder set config callback.
 * \param hTp              Handle of transport decoder.
 * \param cbEarconConfig   Pointer to a callback function to handle earcon decoder data.
 * \param user_data        void pointer for user data passed to the callback as first parameter.
 * \return                 0 on success.
 */
int transportDec_RegisterEarconConfigCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                              const cbEarconConfig_t cbEarconConfig,
                                              void* user_data);

/**
 * \brief                  Register earcon decoder set info callback.
 * \param hTp              Handle of transport decoder.
 * \param cbEarconInfo     Pointer to a callback function to handle earcon decoder data.
 * \param user_data        void pointer for user data passed to the callback as first parameter.
 * \return                 0 on success.
 */
int transportDec_RegisterEarconInfoCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                            const cbEarconInfo_t cbEarconInfo, void* user_data);

int transportDec_SetAsiParsing(HANDLE_TRANSPORTDEC hTpDec, AUDIO_SCENE_INFO* pASI);

/**
 * \brief Fill internal input buffer with bitstream data from the external input buffer.
 *  The function only copies such data as long as the decoder-internal input buffer is not full.
 *  So it grabs whatever it can from pBuffer and returns information (bytesValid) so that at a
 *  subsequent call of %transportDec_FillData(), the right position in pBuffer can be determined to
 *  grab the next data.
 *
 * \param hTp         Handle of transportDec.
 * \param pBuffer     Pointer to external input buffer.
 * \param bufferSize  Size of external input buffer. This argument is required because
 * decoder-internally we need the information to calculate the offset to pBuffer, where the next
 *                    available data is, which is then fed into the decoder-internal buffer (as much
 *                    as possible). Our example framework implementation fills the buffer at pBuffer
 *                    again, once it contains no available valid bytes anymore (meaning bytesValid
 * equal 0).
 * \param bytesValid  Number of bitstream bytes in the external bitstream buffer that have not yet
 * been copied into the decoder's internal bitstream buffer by calling this function. The value is
 * updated according to the amount of newly copied bytes.
 * \param layer       The layer the bitstream belongs to.
 * \return            Error code.
 */
TRANSPORTDEC_ERROR transportDec_FillData(const HANDLE_TRANSPORTDEC hTp, const UCHAR* pBuffer,
                                         const UINT bufferSize, UINT* pBytesValid, const INT layer);

/**
 * \brief      Get transportDec bitstream handle.
 * \param hTp  Pointer to a transport decoder handle.
 * \return     HANDLE_FDK_BITSTREAM bitstream handle.
 */
HANDLE_FDK_BITSTREAM transportDec_GetBitstream(const HANDLE_TRANSPORTDEC hTp, const UINT layer);

/**
 * \brief      Get transport format.
 * \param hTp  Pointer to a transport decoder handle.
 * \return     The transport format.
 */
TRANSPORT_TYPE transportDec_GetFormat(const HANDLE_TRANSPORTDEC hTp);

/**
 * \brief       Close and deallocate transportDec.
 * \param phTp  Pointer to a previously allocated transport decoder handle.
 * \return      void
 */
void transportDec_Close(HANDLE_TRANSPORTDEC* phTp);

/**
 * \brief         Read one access unit from the transportDec medium.
 * \param hTp     Handle of transportDec.
 * \param length  On return, this value is overwritten with the actual access unit length in bits.
 *                Set to -1 if length is unknown.
 * \return        Error code.
 */
TRANSPORTDEC_ERROR transportDec_ReadAccessUnit(const HANDLE_TRANSPORTDEC hTp, const UINT layer);

/**
 * \brief Get the remaining amount of bits of the current access unit. The result
 *        can be below zero, meaning that too many bits have been read.
 * \param hTp     Handle of transportDec.
 * \return amount of remaining bits.
 */
INT transportDec_GetAuBitsRemaining(const HANDLE_TRANSPORTDEC hTp, const UINT layer);

/**
 * \brief Get the total amount of bits of the current access unit.
 * \param hTp     Handle of transportDec.
 * \return amount of total bits.
 */
INT transportDec_GetAuBitsTotal(const HANDLE_TRANSPORTDEC hTp, const UINT layer);

/**
 * \brief      This function is required to be called when the decoder has finished parsing
 *             one Access Unit for bitstream housekeeping.
 * \param hTp  Transport Handle.
 * \return     Error code.
 */
TRANSPORTDEC_ERROR transportDec_EndAccessUnit(const HANDLE_TRANSPORTDEC hTp);

/**
 * \brief      Obtain the amount of missing access units if applicable in case of
 *             a bit stream synchronization error. Each time transportDec_ReadAccessUnit()
 *             returns TRANSPORTDEC_SYNC_ERROR this function can be called to retrieve an estimate
 *             of the amount of missing access units. This works only in case of constant average
 *             bit rate (has to be known) and if the parameter TPDEC_PARAM_SET_BITRATE has been set
 *             accordingly.
 * \param hTp  Transport Handle.
 * \param pNAccessUnits pointer to a memory location where the estimated lost frame count will be
 * stored into.
 * \return     Error code.
 */
TRANSPORTDEC_ERROR transportDec_GetMissingAccessUnitCount(INT* pNAccessUnits,
                                                          HANDLE_TRANSPORTDEC hTp);

/**
 * \brief        Set a given setting.
 * \param hTp    Transport Handle.
 * \param param  Identifier of the parameter to be changed.
 * \param value  Value for the parameter to be changed.
 * \return       Error code.
 */
TRANSPORTDEC_ERROR transportDec_SetParam(const HANDLE_TRANSPORTDEC hTp, const TPDEC_PARAM param,
                                         const INT value);

/* ADTS CRC support */

/**
 * \brief        Set current bitstream position as start of a new data region.
 * \param hTp    Transport handle.
 * \param mBits  Size in bits of the data region. Set to 0 if it should not be of a fixed size.
 * \return       Data region ID, which should be used when calling transportDec_CrcEndReg().
 */
int transportDec_CrcStartReg(const HANDLE_TRANSPORTDEC hTp, const INT mBits);

/**
 * \brief        Set end of data region.
 * \param hTp    Transport handle.
 * \param reg    Data region ID, opbtained from transportDec_CrcStartReg().
 * \return       void
 */
void transportDec_CrcEndReg(const HANDLE_TRANSPORTDEC hTp, const INT reg);

/**
 * \brief      Calculate ADTS crc and check if it is correct. The ADTS checksum is held internally.
 * \param hTp  Transport handle.
 * \return     Return TRANSPORTDEC_OK if the CRC is ok, or error if CRC is not correct.
 */
TRANSPORTDEC_ERROR transportDec_CrcCheck(const HANDLE_TRANSPORTDEC hTp);

/**
 * \brief Parse MPEG-H Audio Scene Info.
 *
 * \param asi               Pointer to ASI struct receiving read data.
 * \param bs                Bitstream handle.
 * \param numMaxElementIDs  Maximum valid element IDs.
 *
 * \return        Error code.
 */
TRANSPORTDEC_ERROR mae_AudioSceneInfo(AUDIO_SCENE_INFO* asi, HANDLE_FDK_BITSTREAM bs,
                                      int numMaxElementIDs, const int streamIndex);

void asiReset(AUDIO_SCENE_INFO* asi);

TRANSPORTDEC_ERROR checkASI(const AUDIO_SCENE_INFO* asi, int numSignalGroups,
                            const CSSignalGroup* signalGroups);

#endif /* #ifndef TPDEC_LIB_H */
