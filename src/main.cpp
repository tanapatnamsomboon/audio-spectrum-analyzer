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

    while(!glfwWindowShouldClose(window)) {
        gui::begin_frame();
        
        std::vector<float> samples = audio::get_latest_samples();
        std::vector<float> spectrum = math::calculate_dft(samples);

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1260, 400), ImGuiCond_FirstUseEver);

        ImGui::Begin("Real-time Spectrum Analyzer");

        if (!spectrum.empty()) {
            // Draw Histogram
            ImGui::PlotHistogram("##Spectrum",
                                 spectrum.data(),
                                 spectrum.size(),
                                 0,
                                 "Frequency Bins",
                                 0.0f,
                                 max_magnitude,
                                 ImVec2(ImGui::GetContentRegionAvail().x, 300));
        }

        ImGui::Separator();

        ImGui::Text("Settings");
        ImGui::SliderFloat("Scale Max", &max_magnitude, 1.0f, 200.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::End();

        gui::end_frame(window);
    }

    audio::cleanup();
    gui::cleanup(window);

    return 0;
}
