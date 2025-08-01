# Changelog

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
