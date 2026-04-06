#include "gui.h"
#include "audio.h"
#include "math_utils.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <algorithm>

int main() {
    if (!audio::init()) return -1;

    GLFWwindow* window = gui::setup_window(1280, 720, "Audio Spectrum Analyzer");
    if (!window) return -1;
    gui::setup_imgui(window);

    float max_magnitude = 50.0f;
    std::vector<float> smooth_spectrum;
    float smoothing_factor = 0.15f;

    std::vector<audio::DeviceInfo> devices = audio::get_capture_devices();
    int current_device_index = -1; // Default

    while(!glfwWindowShouldClose(window)) {
        gui::begin_frame();
        
        std::vector<float> samples = audio::get_latest_samples();
        std::vector<float> raw_spectrum = math::calculate_dft(samples);

        if (smooth_spectrum.size() != raw_spectrum.size()) {
            smooth_spectrum.resize(raw_spectrum.size(), 0.0f);
        }

        for (size_t i = 0; i < raw_spectrum.size(); ++i) {
            smooth_spectrum[i] = (raw_spectrum[i] * smoothing_factor) + (smooth_spectrum[i] * (1.0f - smoothing_factor));
        }

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1260, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Real-time Spectrum Analyzer");

        // Input Source Controller
        ImGui::Text("Audio Input Source");
        
        std::string preview_value = "Default Capture Device";
        for (const auto& dev : devices) {
            if (dev.index == current_device_index) preview_value = dev.name;
        }
        
        if (ImGui::BeginCombo("##DeviceCombo", preview_value.c_str())) {
            // First choice: Default
            if (ImGui::Selectable("Default Capture Device", current_device_index == -1)) {
                current_device_index = -1;
                audio::switch_device(current_device_index);
            }
            
            // Other choice
            for (const auto& dev : devices) {
                bool is_selected = (current_device_index == dev.index);
                if (ImGui::Selectable(dev.name.c_str(), is_selected)) {
                    current_device_index = dev.index;
                    audio::switch_device(current_device_index);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("Refresh List")) {
            devices = audio::get_capture_devices();
        }

        ImGui::Separator();
        
        // Draw Histogram
        ImGui::PlotHistogram("##Spectrum",
                             smooth_spectrum.data(),
                             smooth_spectrum.size(),                                 0,
                             "Frequency Bins",
                             0.0f,
                             max_magnitude,
                             ImVec2(ImGui::GetContentRegionAvail().x, 250));

        ImGui::Separator();

        ImGui::Text("Settings");
        ImGui::SliderFloat("Scale Max", &max_magnitude, 1.0f, 200.0f);
        ImGui::SliderFloat("Smoothing", &smoothing_factor, 0.01f, 1.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::End();
        gui::end_frame(window);
    }

    audio::cleanup();
    gui::cleanup(window);

    return 0;
}
