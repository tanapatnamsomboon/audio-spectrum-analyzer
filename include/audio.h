#pragma once

#include <vector>
#include <string>

namespace audio {

struct DeviceInfo {
    int index;
    std::string name;
};

bool init();
void cleanup();
std::vector<float> get_latest_samples();

std::vector<DeviceInfo> get_capture_devices();
bool switch_device(int device_index); // -1 for Default Device

} // namespace audio
