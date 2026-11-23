# SSLoader V1 — Windows + FL Studio VST3

Starter implementation using C++20, JUCE 8.0.8, and CMake. Produces **Standalone** and **VST3** targets.

## Implemented
- Dark UI with paste URL and download action
- Direct HTTP(S) audio file downloads (.mp3, .wav, .flac, .m4a, .ogg, .aac)
- Background worker thread (does not block audio processing)
- Stores downloaded files under `Documents/SSLoader/Downloads`
- Maximum download size 500 MB, temporary file and incomplete download check
- GitHub Actions Windows build workflow

## Not yet implemented
- YouTube/TikTok/SoundCloud page extraction or platform integrations
- FFmpeg conversion, WAV sample rate conversion, metadata, waveform preview
- SQLite library, progress bar, drag/drop to FL Studio, automatic import
- Authentication, download permissions management, installer

**Important:** Only download media you have the rights to access and save. This V1 downloader expects a direct file URL, not a video page. It does not validate media content type; use trusted URLs.

## Build locally (Windows)
Install Visual Studio 2022 with Desktop development with C++ and CMake, and Git.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 2
```

The first configuration fetches JUCE from GitHub. The artifacts appear under `build/SSLoader_artefacts/Release/Standalone/` and `build/SSLoader_artefacts/Release/VST3/`.

For GitHub Actions, push the complete project to a GitHub repository, open **Actions > Windows Build > Run workflow**, then download **SSLoader-Windows-V1** from the successful run.

## Known limitations
This is an initial code delivery, not a verified release. Compilation and FL Studio host behavior still need to be validated in Windows CI. The VST3 is currently an instrument-type utility and outputs silence; do not place it on an existing audio track expecting pass-through.
