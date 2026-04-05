#pragma once

struct GLFWwindow;

namespace gui {

    GLFWwindow* setup_window(int width, int height, const char* title);
    void setup_imgui(GLFWwindow* window);

    void begin_frame();
    void end_frame(GLFWwindow* window);

    void cleanup(GLFWwindow* window);

} // namespace gui
