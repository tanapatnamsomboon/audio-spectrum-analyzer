#pragma once

#include <vector>
#include <string>

namespace audio {

struct DeviceInfo {
    int index;
    std::string name;
};

enum class InputSource {
    Microphone,
    File
};

bool init();
void cleanup();
std::vector<float> get_latest_samples();

bool load_file(const std::string& filepath);
void play();
void pause();
void stop();
void stop_file();
bool is_playing();
bool is_paused();

std::vector<DeviceInfo> get_capture_devices();
bool switch_capture_device(int device_index); // -1 for Default Device

std::vector<DeviceInfo> get_playback_devices();
bool switch_playback_device(int device_index);

InputSource get_current_source();

} // namespace audio
