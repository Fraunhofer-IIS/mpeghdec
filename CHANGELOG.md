

# Changelog

## [r2.0.0]

### Changed

- UI Manager changes to support the Brazil TV 3.0 application (XML format v11.0; see wiki page for details)
- Rework CMakeLists.txt and build one consolidated "mpeghdec" library.
- Decoder example program: by default write WAV file as 24-bit integer format for better compatibility with external tools (was 32-bit).
- Heap memory optimizations

### Fixed

- Significantly improve accuracy of FormatConverter/ActiveDownmix fixed-point implementation
- Compiler warnings
- Robustness issues (fuzzing)

## [r1.0.0]

Initial release
