#include "gui.h"
#include "audio.h"
#include "math_utils.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <portable-file-dialogs.h>
#include <filesystem>
#include <memory>

int main() {
    if (!audio::init()) return -1;

    GLFWwindow* window = gui::setup_window(1280, 720, "Audio Spectrum Analyzer");
    if (!window) return -1;
    gui::setup_imgui(window);

    float max_magnitude = 50.0f;
    std::vector<float> smooth_spectrum;
    float smoothing_factor = 0.15f;

    // Microphone devices list
    std::vector<audio::DeviceInfo> capture_devices = audio::get_capture_devices();
    int current_capture_device_index = -1; // Default

    // Speaker devices list
    std::vector<audio::DeviceInfo> playback_devices = audio::get_playback_devices();
    int current_playback_device_index = -1;

    std::string selected_file = "assets/sounds/sine_440hz_-6db.wav";

    std::shared_ptr<pfd::open_file> open_file_dialog;

    std::string default_dir = std::filesystem::absolute("assets/sounds").string();

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

        if (ImGui::CollapsingHeader("Speaker", ImGuiTreeNodeFlags_DefaultOpen)) {
            std::string preview_value = "Default Capture Device";
            for (const auto& dev : playback_devices) {
                if (dev.index == current_playback_device_index) preview_value = dev.name;
            }
            if (ImGui::BeginCombo("##PlaybackCombo", preview_value.c_str())) {
                // First choice: Default
                if (ImGui::Selectable("Default Playback Device", current_playback_device_index == -1)) {
                    current_playback_device_index = -1;
                    audio::switch_playback_device(current_playback_device_index);
                }

                // Other choice
                for (const auto& dev : playback_devices) {
                    bool is_selected = (current_playback_device_index == dev.index);
                    if (ImGui::Selectable(dev.name.c_str(), is_selected)) {
                        current_playback_device_index = dev.index;
                        audio::switch_playback_device(current_playback_device_index);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (ImGui::Button("Reset Output List")) playback_devices = audio::get_playback_devices();
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Microphone", ImGuiTreeNodeFlags_DefaultOpen)) {
            std::string preview_value = "Default Capture Device";
            for (const auto& dev : capture_devices) {
                if (dev.index == current_capture_device_index) preview_value = dev.name;
            }
            if (ImGui::BeginCombo("##CaptureCombo", preview_value.c_str())) {
                // First choice: Default
                if (ImGui::Selectable("Default Capture Device", current_capture_device_index == -1)) {
                    current_capture_device_index = -1;
                    audio::switch_capture_device(current_capture_device_index);
                }
                
                // Other choice
                for (const auto& dev : capture_devices) {
                    bool is_selected = (current_capture_device_index == dev.index);
                    if (ImGui::Selectable(dev.name.c_str(), is_selected)) {
                        current_capture_device_index = dev.index;
                        audio::switch_capture_device(current_capture_device_index);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Refresh Mic List")) capture_devices = audio::get_capture_devices();
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Audio File", ImGuiTreeNodeFlags_DefaultOpen)) {
            std::string filename = std::filesystem::path(selected_file).filename().string();
            ImGui::TextWrapped("File: %s", filename.c_str());

            if (ImGui::Button("Browse File...")) {
                open_file_dialog = std::make_shared<pfd::open_file>(
                    "Select audio file",
                    default_dir,
                    std::vector<std::string>{"Audio Files", "*.wav *.mp3 *.flac"}
                );
            }

            if (open_file_dialog && open_file_dialog->ready(0)) {
                auto selection = open_file_dialog->result();
                if (!selection.empty()) {
                    selected_file = selection[0];
                    audio::load_file(selected_file);
                }
                open_file_dialog.reset();
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
