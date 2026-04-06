# Audio Spectrum Analyzer

A real-time audio spectrum analyzer built with C++. This project captures live microphone input, processes the audio signal using a Discrete Fourier Transform (DFT), and visualizes the frequency spectrum in real-time.

## Features
* **Real-time Audio Capture:** Utilizes `miniaudio` for low-latency, cross-platform microphone input.
* **Signal Processing:** Implements a custom Discrete Fourier Transform (DFT) to convert time-domain audio signals into the frequency domain.
* **Interactive GUI:** Built with `ImGui` and `GLFW` for a responsive, hardware-accelerated user interface.
* **Cross-Platform:** Supports Linux, Windows, and MacOS.

## Project Structure
* `src/`: C++ source code files (`main.cpp`, `gui.cpp`, `audio.cpp`, `math_utils.cpp`).
* `include/`: Header files for the modules.
* `thirdparty/`: External dependencies (ImGui, GLFW, miniaudio).

## Requirements

To build this project, you will need:
* A C++23 compatible compiler (GCC, Clang, or MSVC)
* CMake (3.20 or higher)
* Git

### Linux Specific Dependencies
If you are on Linux, you may need to install standard development packages for audio and window management (X11/Wayland, ALSA/PulseAudio). For Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install xorg-dev libgl1-mesa-dev
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
./AudioSpectrumAnalyzer
```

## Modules Overview
- **GUI Module:** Handles window creation, OpenGL context, and ImGui rendering boilerplate.
- **Audio Module:** Runs a background thread via miniaudio callback to safely capture and buffer raw audio samples.
- **Math Module:** Computes the magnitude of frequency bins using the DFT equation: 
$$X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-i \frac{2\pi}{N} kn}$$

## License
[MIT License](https://github.com/tanapatnamsomboon/audio-spectrum-analyzer/blob/master/LICENSE)
