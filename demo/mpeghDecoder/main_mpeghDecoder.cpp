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
#include <iomanip>

// external includes
#include "ilo/memory.h"
#include "mmtisobmff/types.h"
#include "mmtisobmff/logging.h"
#include "mmtisobmff/reader/input.h"
#include "mmtisobmff/reader/reader.h"
#include "mmtisobmff/helper/printhelpertools.h"
#include "mmtisobmff/reader/trackreader.h"

// project includes
#include "mpeghdecoder.h"
#include "cmdl_parser.h"
#include "wav_file.h"

using namespace mmt::isobmff;

/********************* decoder configuration structure **********************/
typedef struct {
  MPEGH_DECODER_PARAMETER param;
  const char* desc;
  const char* swText;
  const TEXTCHAR* sw;  // Command line parameter switch for cmdl parsing
} PARAMETER_ASSIGNMENT_TAB;

/*************************** function declarations ***************************/
static void cmdlHelp(const char* progname);
static uint64_t calcTimestampNs(uint32_t pts, uint32_t timescale) {
  return (uint64_t)((double)pts * (double)1e9 / (double)timescale + 0.5);
}

// I/O buffers
#define IN_BUF_SIZE (65536) /*!< Size of decoder input buffer in bytes. */
#define MAX_RENDERED_CHANNELS (24)
#define MAX_RENDERED_FRAME_SIZE (3072)

/******************************* static memory ******************************/
static PARAMETER_ASSIGNMENT_TAB paramList[] = {
    // name, description, switchText, switch
    {MPEGH_DEC_PARAM_TARGET_REFERENCE_LEVEL,
     "DRC target loudness (reference level) in steps of -0.25 LU using values [40..127]", "-rl",
     "(-rl %d)"},
    {MPEGH_DEC_PARAM_EFFECT_TYPE,
     "MPEG-D DRC effect type request, e.g.: 0 = None (default), 6 = General", "-dse", "(-dse %d)"},
    {MPEGH_DEC_PARAM_BOOST_FACTOR,
     "DRC boost scale factor, where 0 is no boost and 127 is max boost", "-db", "(-db %d)"},
    {MPEGH_DEC_PARAM_ATTENUATION_FACTOR,
     "DRC attenuation scale factor, where 0 is no compression and 127 is max compression", "-dc",
     "(-dc %d)"},
    {MPEGH_DEC_PARAM_ALBUM_MODE, "MPEG-D DRC: album mode, 0: disabled (default), 1: enabled",
     "-dam", "(-dam %d)"},
};

static constexpr int32_t defaultCicpSetup = 6;

class CProcessor {
 private:
  std::string m_wavFilename;
  HANDLE_WAV m_wavFile;
  CIsobmffReader m_reader;
  HANDLE_MPEGH_DECODER_CONTEXT m_decoder;

 public:
  CProcessor(const std::string& inputFilename, const std::string& outputFilename, int32_t cicpSetup)
      : m_wavFilename(outputFilename),
        m_wavFile(nullptr),
        m_reader(ilo::make_unique<CIsobmffFileInput>(inputFilename)) {
    m_decoder = mpeghdecoder_init(cicpSetup);
    if (m_decoder == nullptr) {
      throw std::runtime_error("Error: Unable to create MPEG-H decoder");
    }
  }

  ~CProcessor() {
    if (m_decoder != nullptr) {
      mpeghdecoder_destroy(m_decoder);
    }

    if (m_wavFile != nullptr) {
      WAV_OutputClose(&m_wavFile);
    }
  }

  void configureDecoder(int argc, char* argv[]) {
    // Update decoder instance with all user parameters.
    for (uint32_t i = 0; i < (uint32_t)(sizeof(paramList) / sizeof(PARAMETER_ASSIGNMENT_TAB));
         i++) {
      // Set param if different to the escape value
      int value;
      int argsFound = IIS_ScanCmdl(argc, argv, paramList[i].sw, &value);
      if (!argsFound) {
        continue;
      }
      MPEGH_DECODER_ERROR error = mpeghdecoder_setParam(m_decoder, paramList[i].param, value);
      if (error != MPEGH_DEC_OK) {
        std::cout << "Warning: Failed to set decoder parameter '" << paramList[i].desc << "' to "
                  << value << std::endl;
      }
    }
  }

  void process(int32_t startSample, int32_t stopSample, int32_t seekFromSample,
               int32_t seekToSample) {
    uint32_t frameSize = 0;       // Current audio frame size
    uint32_t sampleRate = 0;      // Current samplerate
    int32_t numChannels = -1;     // Current amount of output channels
    int32_t outputLoudness = -2;  // Audio output loudness

    uint32_t sampleCounter = 0;  // mp4 sample counter
    uint32_t frameCounter = 0;   // output frame counter
    uint64_t timestamp = 0;      // current sample timestamp in nanoseconds

    int32_t outData[MAX_RENDERED_CHANNELS * MAX_RENDERED_FRAME_SIZE] = {0};

    // Only the first MPEG-H track will be processed. Further MPEG-H tracks will be skipped!
    bool mpeghTrackAlreadyProcessed = false;

    // Getting some information about the available tracks
    std::cout << "\nFound " << m_reader.trackCount() << " track(s) in input file." << std::endl;

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

      if (trackInfo.codec != Codec::mpegh_mhm && trackInfo.codec != Codec::mpegh_mha) {
        std::cout << "Skipping unsupported codec: " << ilo::toString(trackInfo.codingName)
                  << std::endl;
        std::cout << std::endl;
        continue;
      }

      if (mpeghTrackAlreadyProcessed) {
        std::cout << "Skipping further mhm1 and mha1 tracks!" << std::endl;
        std::cout << std::endl;
        continue;
      }

      std::cout << "Creating reader for track with ID " << trackInfo.trackId << " ... ";

      // Create a generic track reader for track number i
      std::unique_ptr<CMpeghTrackReader> mpeghTrackReader =
          m_reader.trackByIndex<CMpeghTrackReader>(trackInfo.trackIndex);

      if (mpeghTrackReader == nullptr) {
        std::cout << "Error: Track reader could not be created! Skipping track!" << std::endl;
        continue;
      } else {
        std::cout << "Done!" << std::endl;
      }

      std::unique_ptr<config::CMhaDecoderConfigRecord> mhaDcr = nullptr;
      if (trackInfo.codec == Codec::mpegh_mha) {
        mhaDcr = mpeghTrackReader->mhaDecoderConfigRecord();
        if (mhaDcr == nullptr) {
          std::cout << "Warning: No MHA config available! Skipping track!" << std::endl;
          continue;
        }
        if (mhaDcr->mpegh3daConfig().empty()) {
          std::cout << "Warning: MHA config is empty! Skipping track!" << std::endl;
          continue;
        }

        MPEGH_DECODER_ERROR err = mpeghdecoder_setMhaConfig(
            m_decoder, mhaDcr->mpegh3daConfig().data(), mhaDcr->mpegh3daConfig().size());
        if (err != MPEGH_DEC_OK) {
          throw std::runtime_error("Error: Unable to set mpeghDecoder MHA configuration");
        }
      }

      std::cout << std::endl;
      std::cout << "Sample Info:" << std::endl;
      std::cout << "########################################" << std::endl;
      std::cout << "Max Sample Size        : " << trackInfo.maxSampleSize << " Bytes" << std::endl;
      std::cout << "Total number of samples: " << trackInfo.sampleCount << std::endl;
      std::cout << std::endl;

      std::cout << "Reading all samples of this track" << std::endl;
      std::cout << "########################################" << std::endl;

      // check if enough samples are available to start at requested sample
      if (startSample >= 0 && static_cast<uint32_t>(startSample) >= trackInfo.sampleCount) {
        throw std::runtime_error(
            "[" + std::to_string(sampleCounter) + "] Error: Too few samples (" +
            std::to_string(trackInfo.sampleCount) + ") in track for starting at sample " +
            std::to_string(startSample));
      }

      // check if enough samples are available to seek to requested sample
      if (seekToSample >= 0 && static_cast<uint32_t>(seekToSample) >= trackInfo.sampleCount) {
        throw std::runtime_error(
            "[" + std::to_string(sampleCounter) + "] Error: Too few samples (" +
            std::to_string(trackInfo.sampleCount) + ") in track for seeking to sample " +
            std::to_string(seekToSample));
      }

      // Preallocate the sample with max sample size to avoid reallocation of memory.
      // Sample can be re-used for each nextSample call.
      CSample sample{trackInfo.maxSampleSize};

      // Get samples starting with the provided start sample
      sampleCounter = startSample;
      SSampleExtraInfo sampleInfo = mpeghTrackReader->sampleByIndex(sampleCounter, sample);
      SSeekConfig seekConfig(
          CTimeDuration(sampleInfo.timestamp.timescale(), sampleInfo.timestamp.ptsValue()),
          ESampleSeekMode::nearestSyncSample);
      sampleInfo = mpeghTrackReader->sampleByTimestamp(seekConfig, sample);

      bool seekPerformed = false;
      // Calculate the timestamp in nano seconds
      timestamp =
          calcTimestampNs(sampleInfo.timestamp.ptsValue(), sampleInfo.timestamp.timescale());
      while (!sample.empty() && sampleCounter <= static_cast<uint32_t>(stopSample)) {
        MPEGH_DECODER_ERROR err = MPEGH_DEC_OK;
        // Feed the sample data to the decoder.
        err = mpeghdecoder_process(m_decoder, sample.rawData.data(), sample.rawData.size(),
                                   timestamp);
        if (err != MPEGH_DEC_OK) {
          throw std::runtime_error("[" + std::to_string(sampleCounter) +
                                   "] Error: Unable to process data");
        }

        sampleCounter++;
        std::cout << "Samples processed: " << sampleCounter << "\r" << std::flush;

        if (!seekPerformed && sampleCounter == static_cast<uint32_t>(seekFromSample)) {
          std::cout << "Performing seek from sample " << seekFromSample << " to sample "
                    << seekToSample << std::endl;
          err = mpeghdecoder_flush(m_decoder);
          if (err != MPEGH_DEC_OK) {
            throw std::runtime_error("[" + std::to_string(sampleCounter) +
                                     "] Error: Unable to flush decoder");
          }
          seekPerformed = true;
          sampleCounter = seekToSample;
          sampleInfo = mpeghTrackReader->sampleByIndex(sampleCounter, sample);
          seekConfig = SSeekConfig(
              CTimeDuration(sampleInfo.timestamp.timescale(), sampleInfo.timestamp.ptsValue()),
              ESampleSeekMode::nearestSyncSample);
          sampleInfo = mpeghTrackReader->sampleByTimestamp(seekConfig, sample);
        } else {
          // Get the next sample.
          sampleInfo = mpeghTrackReader->nextSample(sample);
        }

        // Check if EOF or the provided stop sample is reached.
        if (!sample.empty() && sampleCounter <= static_cast<uint32_t>(stopSample)) {
          // Stop sample is not reached; get the sample's timestamp in nano seconds
          timestamp =
              calcTimestampNs(sampleInfo.timestamp.ptsValue(), sampleInfo.timestamp.timescale());
        } else {
          // Stop sample is reached. -> Flush the remaining output frames from the decoder.
          std::cout << std::endl;
          std::cout << "Flushing the decoder!" << std::endl;
          err = mpeghdecoder_flushAndGet(m_decoder);
          if (err != MPEGH_DEC_OK) {
            throw std::runtime_error("Error: Unable to flush data");
          }
        }

        // Obtain decoded audio frames.
        MPEGH_DECODER_ERROR status = MPEGH_DEC_OK;
        MPEGH_DECODER_OUTPUT_INFO outInfo;
        while (status == MPEGH_DEC_OK) {
          status = mpeghdecoder_getSamples(m_decoder, outData, sizeof(outData) / sizeof(int32_t),
                                           &outInfo);

          if (status != MPEGH_DEC_OK && status != MPEGH_DEC_FEED_DATA) {
            throw std::runtime_error("[" + std::to_string(frameCounter) +
                                     "] Error: Unable to obtain output");
          } else if (status == MPEGH_DEC_OK) {
            if (outInfo.sampleRate != 48000 && outInfo.sampleRate != 44100) {
              throw std::runtime_error("Error: Unsupported sampling rate");
            }
            if (outInfo.numChannels <= 0) {
              throw std::runtime_error("Error: Unsupported number of channels");
            }
            if (sampleRate != 0 && sampleRate != static_cast<uint32_t>(outInfo.sampleRate)) {
              throw std::runtime_error("Error: Unsupported change of sampling rate");
            }
            if (numChannels != -1 && numChannels != outInfo.numChannels) {
              throw std::runtime_error("Error: Unsupported change of number of channels");
            }
            frameSize = outInfo.numSamplesPerChannel;
            sampleRate = outInfo.sampleRate;
            numChannels = outInfo.numChannels;

            if (outputLoudness != outInfo.loudness) {
              outputLoudness = outInfo.loudness;
              if (outputLoudness >= 0) {
                std::cout << "[" << frameCounter << "] Info: audio output loudness "
                          << outputLoudness << " (" << std::setprecision(2) << std::fixed
                          << (-outputLoudness * 0.25f) << " dBFS)" << std::endl;
              } else {
                std::cout << "[" << frameCounter << "] Info: audio output loudness "
                          << outputLoudness << "(no loudness metadata present)" << std::endl;
              }
            }

            if (!m_wavFile) {
              if (!m_wavFilename.empty() != 0 && sampleRate && numChannels) {
                if (WAV_OutputOpen(&m_wavFile, m_wavFilename.c_str(), sampleRate, numChannels,
                                   WAV_BITS)) {
                  throw std::runtime_error("[" + std::to_string(frameCounter) +
                                           "] Error: Unable to create output file " +
                                           m_wavFilename);
                }
              }
            }

            if (m_wavFile && WAV_OutputWrite(m_wavFile, outData, numChannels * frameSize,
                                             SAMPLE_BITS, SAMPLE_BITS)) {
              throw std::runtime_error("[" + std::to_string(frameCounter) +
                                       "] Error: Unable to write to output file " + m_wavFilename);
            }
            frameCounter++;
          }
        }
      }

      mpeghTrackAlreadyProcessed = true;
      std::cout << std::endl << "Written audio frames: " << frameCounter << std::endl;
    }
  }
};

int main(int argc, char* argv[]) {
  // Parse command line arguments.
  uint32_t helpMode = 0;
  int32_t stopSample = std::numeric_limits<int32_t>::max();  // number of last sample which output
                                                             // is written to output file
  int32_t startSample = 0;  // number of first sample which output is written to output file
  int32_t seekFromSample = -1;
  int32_t seekToSample = -1;
  int32_t cicpSetup = defaultCicpSetup;
  char inputFilename[CMDL_MAX_STRLEN] = {0};  /*!< Name of input bitstream file */
  char outputFilename[CMDL_MAX_STRLEN] = {0}; /*!< Name of audio output file */

  // Configure mmtisobmff logging to your liking (logging to file, system, console or disable)
  disableLogging();

  IIS_ScanCmdl(argc, argv, "(-of %s) (-tl %d) (-y %d) (-z %d) (-sf %d) (-st %d) (-h %1)",
               outputFilename, &cicpSetup, &startSample, &stopSample, &seekFromSample,
               &seekToSample, &helpMode);

  // Check if helpMode was set.
  if (helpMode) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_OK;
  }

  // Check if no arguments were given.
  if (argc == 0 || argc == 1) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_USAGE;
  }

  uint32_t i = IIS_ScanCmdl(argc, argv, "-if %s", inputFilename);
  // Check if we got the mandatory input and output parameters.
  if (i < 1) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_USAGE;
  }

  // Check if from and to sample for seeking are defined
  if ((seekFromSample != -1 && seekToSample == -1) ||
      (seekFromSample == -1 && seekToSample != -1)) {
    cmdlHelp(argv[0]);
    return FDK_EXITCODE_USAGE;
  }

  std::cout << "Input file:  " << inputFilename << std::endl;
  std::cout << "Output file: " << outputFilename << std::endl;

  // initialize and configure
  try {
    // initialize
    CProcessor processor(inputFilename, outputFilename, cicpSetup);
    // configure decoder
    processor.configureDecoder(argc, argv);
    // process
    processor.process(startSample, stopSample, seekFromSample, seekToSample);
  } catch (const std::exception& e) {
    std::cout << "std::runtime_error caught: " << e.what() << std::endl;
    return FDK_EXITCODE_SOFTWARE;
  } catch (...) {
    std::cout << "Unknown error occured!" << std::endl;
    return FDK_EXITCODE_UNAVAILABLE;
  }

  return FDK_EXITCODE_OK;
}

static void cmdlHelp(const char* progname) {
  std::cout << "usage: " << progname
            << " [options] -if infile -of outfile\n"
               "       options are:"
            << std::endl;
  std::cout << "       -tl\tCICP index of the desired target layout (default: 6)" << std::endl;
  for (uint32_t i = 0; i < (uint32_t)(sizeof(paramList) / sizeof(PARAMETER_ASSIGNMENT_TAB)); i++) {
    std::cout << "       " << paramList[i].swText << "\t" << paramList[i].desc << std::endl;
  }
  std::cout << "       -y \tstart decoding at the provided sample number\n"
               "          \t  NOTE: The decoding will start at the nearest sync sample!\n"
               "       -z \tstop decoding at the provided sample number\n"
               "       -sf\tseek in the bitstream from the provided sample number to the \n"
               "          \t  sample number provided with '-st'\n"
               "          \t  NOTE: '-st' must be set!\n"
               "       -st\tseek in the bitstream from the sample number provided with '-sf'\n"
               "          \t  to the provided sample number\n"
               "          \t  NOTE: The decoding will resume at the nearest sync sample!\n"
               "          \t  NOTE: '-sf' must be set!\n"
               "\n"
               "         \tSeeking example:\n"
               "         \t  '"
            << progname
            << " -if in.mp4 -of out.wav -sf 50 -st 100'\n"
               "         \t  will start decoding the input file from its first sample until sample"
               " 50 is\n"
               "         \t  reached. Afterwards it will seek to the nearest sync sample around"
               " sample 100\n"
               "         \t  and resume decoding until the end of the input file is reached.\n"
               "       -h\tShow this help"
            << std::endl;
}
