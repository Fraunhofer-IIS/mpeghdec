# Changelog

## [r4.0.0] - 2026-07-13

### Added

- Added support for the multi-stream feature in the decoder and UI manager.

### Changed

- Updated the UI manager from XML API v11.0 to v11.1; see the updated wiki page for details.
  - API v11.1 is fully backward-compatible with v11.0.
  - Introduced the new action event `UI_MANAGER_COMMAND_FORCE_SCENESTATE_UPDATE` to explicitly trigger output of the current XML scene information.
  - Prevented XML scene information updates from being emitted when no content changes have occurred.
  - Deprecated the obsolete `DETACHED_UI_SHORT_OUTPUT` case.
  - Updated startup behavior to avoid emitting dummy XML scene information when the UI manager starts on a non-RAP frame or when frame data is invalid.

## [r3.0.3] - 2026-03-30

### Added

- Enable installing of static library on Linux systems and consumption of dependencies

### Changed

- Update mmtisobmff and ilo dependencies

## [r3.0.2] - 2025-10-17

### Changed

- Update mmtisobmff and ilo dependencies

### Fixed

- Fixes for two decoding edge cases

## [r3.0.1] - 2025-08-29

### Changed

- Update mmtisobmff (ISOBMFF reader and writer library) dependency

## [r3.0.0] - 2025-08-01

### Added

- Add optional bitstream output for the mpeghDecoder example program. Now the full functionality of the
  stand-alone UI manager demo program (mpeghUiManager) is also available with "mpeghDecoder".
- Add timescale/samplerate based Presentation Time Stamp (PTS) decoder API.
- Add support for installing as shared library on Linux, including pkg-config support.

### Fixed

- Robustness improvements
- Fixes for several decoding edge cases

## [r2.0.1] - 2025-02-03

### Added

- Integrate UI manager component into mpeghDecoder example program

### Fixed

- Fix fragmented MP4 support for mpeghUiManager demo program

## [r2.0.0] - 2024-06-21

### Changed

- UI Manager changes to support the Brazil TV 3.0 application (XML format v11.0; see wiki page for details)
- Rework CMakeLists.txt and build one consolidated "mpeghdec" library.
- Decoder example program: by default write WAV file as 24-bit integer format for better compatibility with external tools (was 32-bit).
- Heap memory optimizations

### Fixed

- Significantly improve accuracy of FormatConverter/ActiveDownmix fixed-point implementation
- Compiler warnings
- Robustness issues (fuzzing)

## [r1.0.0] - 2023-07-03

Initial release
