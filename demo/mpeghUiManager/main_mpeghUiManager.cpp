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

// system includes

// external includes
#include "ilo/memory.h"
#include "mmtisobmff/types.h"
#include "mmtisobmff/logging.h"
#include "mmtisobmff/helper/commonhelpertools.h"
#include "mmtisobmff/helper/printhelpertools.h"
#include "mmtisobmff/reader/input.h"
#include "mmtisobmff/reader/reader.h"
#include "mmtisobmff/reader/trackreader.h"
#include "mmtisobmff/writer/writer.h"
#include "mmtisobmff/writer/trackwriter.h"

// project includes
#include "mpeghUIManager.h"
#include "interactivityScriptParser.h"
#include "sys/cmdl_parser.h"
#include "sys/genericStds.h"

using namespace mmt::isobmff;

// Buffer size in bytes to hold one MPEG-H frame (sequence of MHAS packages) + overhead
// for MPEG-H Level 4
#define MAX_MPEGH_FRAME_SIZE 65536

// Size of persistence memory in bytes
#define PERSISTENCE_BUFFER_SIZE 2048

// Size of the XML buffer in number of chars
#define XML_BUFFER_SIZE 1024

/*************************** function declarations ***************************/
static void cmdlHelp(const char* progname);

class CProcessor {
 private:
  std::string m_persistenceFilename;
  CIsobmffReader m_reader;
  HANDLE_MPEGH_UI_MANAGER m_uiManager;
  std::vector<uint16_t> m_persistenceMemory;
  std::unique_ptr<CIsobmffFileWriter> m_writer;
  std::ofstream m_sceneStateFile;
  CInteractivityScriptParser m_parser;

 public:
  CProcessor(const std::string& inputFilename, const std::string& outputFilename,
             const std::string& scriptFilename, const std::string& persistFilename,
             const std::string& xmlSceneStateFilename)
      : m_persistenceFilename(persistFilename),
        m_reader(ilo::make_unique<CIsobmffFileInput>(inputFilename)),
        m_uiManager(nullptr),
        m_writer(nullptr),
        m_parser() {
    // Configure the output
    CIsobmffFileWriter::SOutputConfig outputConfig;
    outputConfig.outputUri = outputFilename;
    // Optional: Path to tmp file. If not set, a unique tmp file
    //           will be generated in system specific tmp dir.
    outputConfig.tmpUri = "";

    SMovieConfig movieConfig;
    movieConfig.majorBrand = ilo::toFcc("mp42");

    // Create a non-fragmented (plain) MP4 file writer
    m_writer = ilo::make_unique<CIsobmffFileWriter>(outputConfig, movieConfig);

    // Open XML command script file (optional)
    if (!scriptFilename.empty()) {
      m_parser = CInteractivityScriptParser{scriptFilename};
    }

    // Open XML scene state output file (optional)
    if (!xmlSceneStateFilename.empty()) {
      m_sceneStateFile.open(xmlSceneStateFilename, std::ios::out);
      if (!m_sceneStateFile.is_open()) {
        throw std::ios_base::failure("Warning: Failed to open XML output file!");
      }
    }

    m_persistenceMemory.resize(PERSISTENCE_BUFFER_SIZE / sizeof(uint16_t));

    // Load persistence data from file (optional)
    if (!persistFilename.empty()) {
      std::ifstream persistenceFile;
      persistenceFile.open(m_persistenceFilename, std::ios::in | std::ios::binary);
      if (persistenceFile.is_open()) {
        persistenceFile.read(reinterpret_cast<char*>(m_persistenceMemory.data()),
                             PERSISTENCE_BUFFER_SIZE);
        if (persistenceFile.gcount() != PERSISTENCE_BUFFER_SIZE) {
          std::cout << "Warning: Unable to read enough data from persistence file!" << std::endl;
          m_persistenceMemory.clear();
          m_persistenceMemory.resize(PERSISTENCE_BUFFER_SIZE / sizeof(uint16_t));
        }
        persistenceFile.close();
      } else {
        std::cout
            << "Warning: Persistence input file not found! Creating a new one after processing."
            << std::endl;
      }
    }

    // Open an UI manager instance
    m_uiManager = mpegh_UI_Manager_Open();
    if (m_uiManager == nullptr) {
      throw std::runtime_error("Error: Unable to create MPEG-H UI manager");
    }

    // Set persistence data memory
    MPEGH_UI_ERROR err = mpegh_UI_SetPersistenceMemory(m_uiManager, m_persistenceMemory.data(),
                                                       PERSISTENCE_BUFFER_SIZE);
    if (err != MPEGH_UI_OK) {
      std::cout << "Warning: Unable to set persistence memory!" << std::endl;
    }
  }

  ~CProcessor() {
    // Save persistence data to file (optional)
    if (!m_persistenceFilename.empty()) {
      void* pMemory = nullptr;
      uint16_t size = 0;

      MPEGH_UI_ERROR err = mpegh_UI_GetPersistenceMemory(m_uiManager, &pMemory, &size);
      if (err != MPEGH_UI_OK) {
        std::cout << "Warning: Unable to get persistence memory!" << std::endl;
      }

      if (err == MPEGH_UI_OK && pMemory && size) {
        std::ofstream persistenceFile;
        persistenceFile.open(m_persistenceFilename, std::ios::out | std::ios::binary);
        if (persistenceFile.is_open()) {
          persistenceFile.write(reinterpret_cast<const char*>(m_persistenceMemory.data()), size);
          persistenceFile.close();
        } else {
          std::cout << "Warning: Failed to open/create persistence data file!" << std::endl;
        }
      } else {
        std::cout << "Warning: Obtaining persistence memory failed!" << std::endl;
      }
    }

    // Close UI manager
    if (m_uiManager) {
      mpegh_UI_Manager_Close(m_uiManager);
    }

    // Finish the file, delete temp files, close the file library
    try {
      m_writer->close();
    } catch (...) {
      std::cout << "Error: Closing the MP4 file writer failed!" << std::endl;
    }
  }

  void process() {
    // Only the first MPEG-H mhm1 track will be processed. Further MPEG-H mhm1 tracks will be
    // skipped!
    bool mhmTrackAlreadyProcessed = false;

    // Getting some information about the available tracks
    std::cout << "Found " << m_reader.trackCount() << " tracks in input file." << std::endl;
    const auto& movieInfo = m_reader.movieInfo();

    for (const auto& trackInfo : m_reader.trackInfos()) {
      std::cout << "########################################" << std::endl;
      std::cout << "-TrackInfo: " << std::endl;
      std::cout << "-- ID       : " << trackInfo.trackId << std::endl;
      std::cout << "-- Handler  : " << ilo::toString(trackInfo.handler) << std::endl;
      std::cout << "-- Type     : " << tools::trackTypeToString(trackInfo.type) << std::endl;
      std::cout << "-- Codec    : " << ilo::toString(trackInfo.codingName) << std::endl;
      std::cout << "-- Duration : " << trackInfo.duration << std::endl;
      std::cout << "-- Timescale: " << trackInfo.timescale << std::endl;
      std::cout << std::endl;

      if (trackInfo.codec != Codec::mpegh_mhm) {
        if (trackInfo.codec == Codec::mpegh_mha) {
          std::cout << "MPEG-H UI handling is not applicable to MHA content!" << std::flush;
        }
        std::cout << "Copying Track: " << std::flush;
        tools::SCopyConfig cConfig;
        cConfig.keepFragNumber = false;
        cConfig.fragmentDuration = 0;
        cConfig.oldMovieTimescale = movieInfo.timeScale;
        cConfig.newMovieTimescale = movieInfo.timeScale;
        cConfig.trackInfo = trackInfo;
        tools::copyTrack(m_reader, *m_writer, cConfig);
        std::cout << "done" << std::endl << std::endl;
        continue;
      }

      if (mhmTrackAlreadyProcessed) {
        std::cout << "Skipping further mhm1 track!" << std::endl;
        std::cout << std::endl;
        continue;
      }

      std::cout << "Creating reader for track with ID " << trackInfo.trackId << " ... ";

      // Create an MPEG-H track reader for track number i
      std::unique_ptr<CMpeghTrackReader> mpeghTrackReader =
          m_reader.trackByIndex<CMpeghTrackReader>(trackInfo.trackIndex);

      if (mpeghTrackReader == nullptr) {
        std::cout << "Failed!" << std::endl;
        continue;
      } else {
        std::cout << "Done!" << std::endl;
      }

      // Adjust MPEG-H configuration
      SMpeghMhm1TrackConfig mpeghConfig;
      mpeghConfig.mediaTimescale = trackInfo.timescale;
      mpeghConfig.sampleRate = mpeghTrackReader->sampleRate();

      // Create MPEG-H track writer
      std::unique_ptr<CMpeghTrackWriter> mpeghTrackWriter =
          m_writer->trackWriter<CMpeghTrackWriter>(mpeghConfig);

      std::cout << std::endl;
      std::cout << "########################################" << std::endl;
      std::cout << "ISOBMFF/MP4 Samples Info:" << std::endl;
      std::cout << "Max ISOBMFF/MP4 Sample Size        : " << trackInfo.maxSampleSize << " Bytes"
                << std::endl;
      std::cout << "Total number of ISOBMFF/MP4 Samples: " << trackInfo.sampleCount << std::endl;
      std::cout << std::endl;
      std::cout << "########################################" << std::endl;
      std::cout << "Reading all ISOBMFF/MP4 Samples of this track" << std::endl;

      // Preallocate the sample with max MPEG-H frame size to avoid reallocation of memory.
      // Sample can be re-used for each nextSample call.
      CSample sample = CSample{MAX_MPEGH_FRAME_SIZE};

      uint64_t sampleCounter = 0;

      // Get all samples in order. Each call fetches the next sample.
      mpeghTrackReader->nextSample(sample);

      while (!sample.empty()) {
        // Feed data to the UI manager
        uint32_t mhasLength = sample.rawData.size();
        MPEGH_UI_ERROR feedErr = mpegh_UI_FeedMHAS(m_uiManager, sample.rawData.data(), mhasLength);
        if (feedErr != MPEGH_UI_OK) {
          std::cout << "Warning: Unable to feed MHAS for ISOBMFF/MP4 Sample " << sampleCounter
                    << std::endl;
        }

        // process XML actions
        std::vector<std::string> commands = m_parser.getCommands(sampleCounter - 1);
        uint32_t flagsOut;
        for (const std::string& c : commands) {
          MPEGH_UI_ERROR err =
              mpegh_UI_ApplyXmlAction(m_uiManager, c.c_str(), c.length(), &flagsOut);
          if (err != MPEGH_UI_OK) {
            std::cout << "Warning: Failed to apply XML action for ISOBMFF/MP4 Sample "
                      << sampleCounter << " with command:" << std::endl;
            std::cout << c << std::endl;
          }
        }

        if (feedErr == MPEGH_UI_OK) {
          sample.rawData.resize(MAX_MPEGH_FRAME_SIZE);
          uint32_t newMhasLength = mhasLength;
          // Update the MPEG-H frame
          MPEGH_UI_ERROR err = mpegh_UI_UpdateMHAS(m_uiManager, sample.rawData.data(),
                                                   MAX_MPEGH_FRAME_SIZE, &newMhasLength);
          if (err != MPEGH_UI_OK) {
            sample.rawData.resize(mhasLength);
            std::cout << "Warning: Failed to update MHAS for ISOBMFF/MP4 Sample " << sampleCounter
                      << std::endl;
          } else {
            sample.rawData.resize(newMhasLength);
          }
        }

        // Write the sample to the track
        mpeghTrackWriter->addSample(sample);

        // Write XML scene state
        if (m_sceneStateFile) {
          uint32_t flagsOut = 0;
          bool frameCounterWritten = false;
          std::vector<char> xmlSceneStateBuf(XML_BUFFER_SIZE);

          while (!(flagsOut & MPEGH_UI_NO_CHANGE)) {
            MPEGH_UI_ERROR err = mpegh_UI_GetXmlSceneState(m_uiManager, xmlSceneStateBuf.data(),
                                                           XML_BUFFER_SIZE, 0, &flagsOut);
            if (err != MPEGH_UI_OK) {
              std::cout << "Warning: Failed to get XML scene state for ISOBMFF/MP4 Sample "
                        << sampleCounter << std::endl;
              break;
            }

            if (!(flagsOut & MPEGH_UI_NO_CHANGE)) {
              if (!frameCounterWritten) {
                m_sceneStateFile << "[" << sampleCounter << "]" << std::endl;
                frameCounterWritten = true;
              }
              m_sceneStateFile << xmlSceneStateBuf.data();
            }
          }
        }

        sampleCounter++;
        std::cout << "ISOBMFF/MP4 Samples processed: " << sampleCounter << "\r" << std::flush;

        mpeghTrackReader->nextSample(sample);
      }

      mhmTrackAlreadyProcessed = true;

      std::cout << std::endl;
    }
  }
};

int main(int argc, char** argv) {
  // Configure mmtisobmff logging to your liking (logging to file, system, console or disable)
  disableLogging();

  char inputFilename[CMDL_MAX_STRLEN] = "";
  char outputFilename[CMDL_MAX_STRLEN] = "";
  char scriptFilename[CMDL_MAX_STRLEN] = "";
  char xmlSceneStateFilename[CMDL_MAX_STRLEN] = "";
  char persistFilename[CMDL_MAX_STRLEN] = "";
  uint32_t helpMode = 0;

  // Check if helpMode was set.
  IIS_ScanCmdl(argc, argv, "(-h %1)", &helpMode);
  if (helpMode) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_OK;
  }

  // Check if we got the mandatory input and output parameters.
  if (IIS_ScanCmdl(argc, argv, "-if %s -of %s", inputFilename, outputFilename) < 2) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_USAGE;
  }

  // Parse optional command line parameters.
  IIS_ScanCmdl(argc, argv, "(-script %s)", scriptFilename);
  IIS_ScanCmdl(argc, argv, "(-xmlSceneState %s)", xmlSceneStateFilename);
  IIS_ScanCmdl(argc, argv, "(-persistFile %s)", persistFilename);

  std::cout << "Input file:  " << inputFilename << std::endl;
  std::cout << "Output file: " << outputFilename << std::endl;

  // Initialize and process.
  try {
    // initialize
    CProcessor processor(inputFilename, outputFilename, scriptFilename, persistFilename,
                         xmlSceneStateFilename);
    // process
    processor.process();
  } catch (const std::exception& e) {
    std::cout << std::endl << "Error: " << e.what() << std::endl << std::endl;
    return FDK_EXITCODE_SOFTWARE;
  } catch (...) {
    std::cout << std::endl
              << "Error: An unknown error happened. The program will exit now." << std::endl
              << std::endl;
    return FDK_EXITCODE_UNAVAILABLE;
  }
  return FDK_EXITCODE_OK;
}

static void cmdlHelp(const char* progname) {
  std::cout << std::endl
            << "Usage: " << progname
            << " -if <input file> -of <output file> [-script <interactivity "
               "script>][-xmlSceneState <XML output>][-persistFile <filename>]"
            << std::endl;
}
