#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <mutex>
#include <iostream>

namespace audio {

ma_context g_context;
ma_device g_device;
bool g_is_context_initialized = false;
bool g_is_device_initialized = false;

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
    if (ma_context_init(NULL, 0, NULL, &g_context) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio context.\n";
        return false;
    }
    g_is_context_initialized = true;

    // Start with Default Device (-1)
    return switch_device(-1);
}

void cleanup() {
    ma_device_uninit(&g_device);
}

std::vector<float> get_latest_samples() {
    std::lock_guard<std::mutex> lock(g_buffer_mutex);
    return g_sample_buffer;
}

std::vector<DeviceInfo> get_capture_devices() {
    std::vector<DeviceInfo> devices;
    if (!g_is_context_initialized) return devices;

    ma_device_info* p_playback_infos;
    ma_uint32 playback_count;
    ma_device_info* p_capture_infos;
    ma_uint32 capture_count;

    if (ma_context_get_devices(&g_context, &p_playback_infos, &playback_count, &p_capture_infos, &capture_count) == MA_SUCCESS) {
        for (ma_uint32 i = 0; i < capture_count; ++i) {
            devices.push_back({(int)i, p_capture_infos[i].name});
        }
    }
    return devices;
}

bool switch_device(int device_index) {
    if (g_is_device_initialized) {
        ma_device_uninit(&g_device);
        g_is_device_initialized = false;
    }

    ma_device_id* p_selected_id = nullptr;
    if (device_index >= 0) {
        ma_device_info* p_playback_infos;
        ma_uint32 playback_count;
        ma_device_info* p_capture_infos;
        ma_uint32 capture_count;

        if (ma_context_get_devices(&g_context, &p_playback_infos, &playback_count, &p_capture_infos, &capture_count) == MA_SUCCESS) {
            if ((ma_uint32)device_index < capture_count) {
                p_selected_id = &p_capture_infos[device_index].id;
            }
        }
    }
    
    ma_device_config device_config = ma_device_config_init(ma_device_type_capture);
    device_config.capture.pDeviceID = p_selected_id;
    device_config.capture.format    = ma_format_f32;
    device_config.capture.channels  = 1;
    device_config.sampleRate        = 44100;
    device_config.dataCallback      = data_callback;

    if (ma_device_init(&g_context, &device_config, &g_device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize capture device.\n";
        return false;
    }

    ma_device_start(&g_device);
    g_is_device_initialized = true;

    std::lock_guard<std::mutex> lock(g_buffer_mutex);
    std::fill(g_sample_buffer.begin(), g_sample_buffer.end(), 0.0f);

    return true;
}

} // namespace audio
