#include "gui.h"
#include "audio.h"
#include "math_utils.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <print>

int main() {
    if (!audio::init()) return -1;

    GLFWwindow* window = gui::setup_window(1280, 720, "Audio Spectrum Analyzer");
    if (!window) return -1;
    gui::setup_imgui(window);

    int frame_counter = 0;

    while(!glfwWindowShouldClose(window)) {
        gui::begin_frame();
        
        std::vector<float> samples = audio::get_latest_samples();
        std::vector<float> spectrum = math::calculate_dft(samples);

        if (frame_counter % 60 == 0 && !spectrum.empty()) {
            std::println("[Log] Freq Low: {} | Mid: {} | High: {}", 
                         spectrum[2], spectrum[spectrum.size() / 2], 
                         spectrum[spectrum.size() - 2]);
        }
        frame_counter++;

        ImGui::Begin("Analyzer Status");
        ImGui::Text("Audio capturing and Math computing in background...");
        ImGui::End();

        gui::end_frame(window);
    }

    audio::cleanup();
    gui::cleanup(window);

    return 0;
}
