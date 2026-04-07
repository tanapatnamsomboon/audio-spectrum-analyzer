#include "gui.h"
#include "audio.h"
#include "math_utils.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <portable-file-dialogs.h>
#include <filesystem>

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

    std::string selected_file = "assets/sounds/sine_440hz_44k_6dB";

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

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar
                                      | ImGuiWindowFlags_NoCollapse
                                      | ImGuiWindowFlags_NoResize
                                      | ImGuiWindowFlags_NoMove
                                      | ImGuiWindowFlags_NoBringToFrontOnFocus
                                      | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
        ImGui::Begin("MainWorkspace", nullptr, window_flags);
        ImGui::PopStyleVar();

        // Left Panel: Controller
        float left_panel_width = ImGui::GetWindowWidth() * 0.35f;

        ImGui::BeginChild("LeftPanel", ImVec2(left_panel_width, 0), true);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        ImGui::Text("  INPUT CONFIGURATION");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("  Mode: %s", (audio::get_current_source() == audio::InputSource::Microphone) ? "Microphone" : "File Playback");
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Microphone", ImGuiTreeNodeFlags_DefaultOpen)) {
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
            if (ImGui::Button("Refresh Mic List")) devices = audio::get_capture_devices();
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Audio File", ImGuiTreeNodeFlags_DefaultOpen)) {
            std::string filename = std::filesystem::path(selected_file).filename().string();
            ImGui::TextWrapped("File: %s", filename.c_str());

            if (ImGui::Button("Browse File...")) {
                auto selection = pfd::open_file("Select audio file", ".",
                                                {"Audio Files", "*.wav *.mp3 *.flac"}).result();
                if (!selection.empty()) {
                    selected_file = selection[0];
                    audio::load_file(selected_file);
                }
            }

            ImGui::Spacing();

            if (audio::is_playing()) {
                if (ImGui::Button("PAUSE")) audio::pause();
            } else {
                if (ImGui::Button("PLAY")) {
                    if (audio::get_current_source() != audio::InputSource::File) {
                        audio::load_file(selected_file);
                    }
                    audio::play();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("STOP")) audio::stop();
        }

        ImGui::Spacing();
        ImGui::Text("VISUALIZER SETTINGS");
        ImGui::Separator();
        ImGui::SliderFloat("Scale Max", &max_magnitude, 1.0f, 200.0f);
        ImGui::SliderFloat("Smoothing", &smoothing_factor, 0.01f, 1.0f);

        ImGui::EndChild();

        ImGui::SameLine();

        // Right Panel: Histogram
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        ImGui::Text("  SPECTRUM ANALYZER");
        ImGui::Separator();
        ImGui::Spacing();

        ImVec2 graph_size = ImGui::GetContentRegionAvail();
        graph_size.y -= 10.0f;

        ImGui::PlotHistogram("##Spectrum",
                             smooth_spectrum.data(),
                             smooth_spectrum.size(),
                             0,
                             NULL,
                             0.0f,
                             max_magnitude,
                             graph_size);

        ImGui::EndChild();

        ImGui::End();

        gui::end_frame(window);
    }

    audio::cleanup();
    gui::cleanup(window);

    return 0;
}
