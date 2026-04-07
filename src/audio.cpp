#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <mutex>
#include <iostream>

namespace audio {

ma_context g_context;
ma_device g_device;
ma_decoder g_decoder;

bool g_is_context_initialized = false;
bool g_is_device_initialized = false;
bool g_is_file_playing = false;

InputSource g_current_source = InputSource::Microphone;

const int SAMPLE_SIZE = 512;
std::vector<float> g_sample_buffer(SAMPLE_SIZE, 0.0f);
std::mutex g_buffer_mutex;

bool g_is_paused = false;
bool g_is_loaded = false;
bool g_is_eof = false;

void data_callback(ma_device* p_device, void* p_output, const void* p_input, ma_uint32 frame_count) {
    std::lock_guard<std::mutex> lock(g_buffer_mutex);
    
    if (g_current_source == InputSource::Microphone) {
        if (p_input == nullptr) return;
        const float* p_input_f32 = (const float*)p_input;
        for (ma_uint32 i = 0; i < frame_count && i < SAMPLE_SIZE; ++i) {
            g_sample_buffer[i] = p_input_f32[i];
        }
    } else if (g_current_source == InputSource::File && g_is_file_playing) {
        if (p_output == nullptr) return;
        float* p_output_f32 = (float*)p_output;

        if (g_is_paused) {
            std::fill_n(p_output_f32, frame_count, 0.0f);
            return;
        }

        // 1. Read data from an audio file and send it to the speaker (p_output)
        ma_uint64 frames_read = 0;
        ma_decoder_read_pcm_frames(&g_decoder, p_output_f32, frame_count, &frames_read);

        // 2. Copy the same data for graph analysis
        for (ma_uint32 i = 0; i < frames_read && i < SAMPLE_SIZE; ++i) {
            g_sample_buffer[i] = p_output_f32[i];
        }

        if (frames_read < frame_count) {
            g_is_file_playing = false;
            g_is_eof = true;
        }
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
    stop_file();
    if (g_is_device_initialized) ma_device_uninit(&g_device);
    if (g_is_context_initialized) ma_context_uninit(&g_context);
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

    stop_file();
    g_current_source = InputSource::Microphone;

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

bool load_file(const std::string& filepath) {
    // 1. Stop the microphone
    if (g_is_device_initialized) {
        ma_device_stop(&g_device);
        g_is_device_initialized = false;
    }
    stop_file();

    // 2. Load audio file and force-convert all formats to Mono, Float32, 44100 Hz
    ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 1, 44100);
    if (ma_decoder_init_file(filepath.c_str(), &decoder_config, &g_decoder) != MA_SUCCESS) {
        std::cerr << "Failed to load file: " << filepath << "\n";
        switch_device(-1);
        return false;
    }

    g_is_loaded = true;
    g_is_file_playing = false;
    g_is_paused = false;
    g_current_source  = InputSource::File;

    // 3. Initialize a new device, this time in "Playback" mode (speaker)
    ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = ma_format_f32;
    device_config.playback.channels = 1;
    device_config.sampleRate        = 44100;
    device_config.dataCallback      = data_callback;

    if (ma_device_init(&g_context, &device_config, &g_device) != MA_SUCCESS) {
        std::cerr << "Failed to init playback device.\n";
        return false;
    }
    
    ma_device_start(&g_device);
    g_is_device_initialized = true;

    return true;
}

void play() {
    if (g_is_loaded && g_is_eof) {
        ma_decoder_seek_to_pcm_frame(&g_decoder, 0);
        g_is_eof = false;
    }

    if (g_is_loaded) {
        g_is_file_playing = true;
        g_is_paused = false;
    }
}

void pause() {
    g_is_paused = true;
}

void stop() {
    stop_file();
    switch_device(-1);
}

void stop_file() {
    if (g_is_loaded) {
        ma_decoder_uninit(&g_decoder);
        g_is_loaded = false;
    }
    g_is_file_playing = false;
    g_is_paused = false;
    g_is_eof = false;
}

bool is_playing() {
    return g_is_file_playing && !g_is_paused;
}

bool is_paused() {
    return g_is_paused;
}

InputSource get_current_source() {
    return g_current_source;
}

} // namespace audio
