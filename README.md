# Audio Spectrum Analyzer

A real-time, cross-platform audio spectrum analyzer built with C++. This application captures live microphone input or plays back audio files, processes the signal using a Discrete Fourier Transform (DFT), and visualizes the frequency spectrum with a modern, hardware-accelerated GUI.

<p align="center">
  <video src="https://github.com/user-attachments/assets/2947f3d0-d6d4-4368-b02d-f31c3021cfc2" width="100%" controls></video>
</p>

## Features
* **Dual Modes:** Seamlessly switch between live microphone capture and audio file playback.
* **Real-time Signal Processing:** Implements a custom Discrete Fourier Transform (DFT) to convert time-domain audio signals into the frequency domain with adjustable exponential smoothing.
* **Device Management:** Dynamically scan and switch capture devices (microphones) on the fly without interrupting the application.
* **File Playback Controls:** Fully functional transport controls (Play, Pause, Stop) for `.wav`, `.mp3`, and `.flac` files.
* **Modern GUI:** Built with `ImGui` and `GLFW`, featuring a clean, responsive, and distraction-free panel.
* **Native File Dialogs:** Asynchronous native file browsing using `portable-file-dialogs` to prevent UI freezing.

## Project Structure
* `src/`: C++ source code files (`main.cpp`, `gui.cpp`, `audio.cpp`, `math_utils.cpp`).
* `include/`: Header files for the modules.
* `thirdparty/`: External dependencies (ImGui, GLFW, miniaudio).
* `assets/`: Bundled fonts and demo audio files, automatically copied to the build directory via CMake.

## Requirements
To build this project, you will need:
* A C++17 compatible compiler (GCC, Clang, or MSVC)
* CMake (3.20 or higher)
* Git

### Linux Specific Dependencies
If you are on Linux, you may need to install standard development packages for audio and window management (X11/Wayland, ALSA/PulseAudio). For Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install xorg-dev libgl1-mesa-dev
# For portable-file-dialogs (depending on your DE):
sudo apt-get install zenity # or kdialog
```

## Build Instructions
This project uses Git submodules for managing dependencies (`glfw` and `imgui`). Please make sure to clone the repository recursively.
### 1. Clone the repository
```bash
git clone --recursive https://github.com/tanapatnamsomboon/audio-spectrum-analyzer.git
cd audio-spectrum-analyzer
```
(if you already cloned without `--recursive`, you can run: `git submodule update --init --recursive`)
### 2. Generate build files and compile
```bash
mkdir build
cd build
cmake ..
make
```
### 3. Run the application
```bash
cd ..
./build/AudioSpectrumAnalyzer
```

## Modules Overview
- **GUI Module:** Handles window creation, OpenGL context, and ImGui rendering boilerplate.
- **Audio Module:** Runs a background thread via miniaudio callback to safely capture and buffer raw audio samples.
- **Math Module:** Computes the magnitude of frequency bins using the DFT equation: 
$$X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-i \frac{2\pi}{N} kn}$$

## License
[MIT License](https://github.com/tanapatnamsomboon/audio-spectrum-analyzer/blob/master/LICENSE)
