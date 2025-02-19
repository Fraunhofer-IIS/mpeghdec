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
#include <iostream>

// external includes
#include "ilo/memory.h"
#include "mmtisobmff/logging.h"

// project includes
#include "mpeghUiManagerProcessor.h"
#include "mpeghUIManager.h"
#include "sys/cmdl_parser.h"
#include "sys/genericStds.h"

/*************************** function declarations ***************************/
static void cmdlHelp(const char* progname);

int main(int argc, char** argv) {
  // Configure mmtisobmff logging to your liking (logging to file, system, console or disable)
  mmt::isobmff::disableLogging();

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
    CUIManagerProcessor processor(inputFilename, outputFilename, scriptFilename, persistFilename,
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
