#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <mutex>
#include <iostream>

namespace audio {

ma_device device;
const int SAMPLE_SIZE = 512;
std::vector<float> g_sample_buffer(SAMPLE_SIZE, 0.0f);
std::mutex g_buffer_mutex;

void data_callback(ma_device* p_device, void* p_output, const void* p_input, ma_uint32 frame_count) {
    if (p_input == nullptr) return;
    
    std::lock_guard<std::mutex> lock(g_buffer_mutex);
    const float* p_input_f32 = (const float*)p_input;

    // Copy sound data to buffer
    for (ma_uint32 i = 0; i < frame_count && i < SAMPLE_SIZE; ++i) {
        g_sample_buffer[i] = p_input_f32[i];
    }
}

bool init() {
    ma_device_config device_config = ma_device_config_init(ma_device_type_capture);
    device_config.capture.format   = ma_format_f32;
    device_config.capture.channels = 1;
    device_config.sampleRate      = 44100;
    device_config.dataCallback    = data_callback;

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize capture device.\n";
        return false;
    }
    ma_device_start(&device);
    std::cout << "Audio capture started.\n";
    return true;
}

void cleanup() {
    ma_device_uninit(&device);
}

std::vector<float> get_latest_samples() {
    std::lock_guard<std::mutex> lock(g_buffer_mutex);
    return g_sample_buffer;
}

} // namespace audio
