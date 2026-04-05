#include "gui.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

int main() {
    GLFWwindow* window = gui::setup_window(1280, 720, "Audio Spectrum Analyzer");
    if (!window) return -1;

    gui::setup_imgui(window);

    while(!glfwWindowShouldClose(window)) {
        gui::begin_frame();
        
        ImGui::Begin("Analyzer Status");
        ImGui::Text("GUI is working perfectly!");
        ImGui::End();

        gui::end_frame(window);
    }

    gui::cleanup(window);

    return 0;
}
