#include <iostream>

#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <scope_guard.h>

void printGlfwError(const std::string &message, std::ostream &out = std::cerr);

int main(int argc, char **argv) {
    
    // Initialize GLFW
    if(!glfwInit()) {
        printGlfwError("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }
    scope_exit(glfwTerminate());

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(1024, 768, "Night Fleet client", nullptr, nullptr);
    if(window == nullptr) {
        printGlfwError("Failed to create window");
        return EXIT_FAILURE;
    }
    scope_exit(glfwDestroyWindow(window));

    // Load OpenGL
    glfwMakeContextCurrent(window);
    if(!gladLoadGL()) {
        std::cerr << "Failed to load OpenGL." << std::endl;
        return EXIT_FAILURE;
    }
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    scope_exit(ImGui::DestroyContext());
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    scope_exit(ImGui_ImplGlfw_Shutdown());
    ImGui_ImplOpenGL3_Init("#version 330");
    scope_exit(ImGui_ImplOpenGL3_Shutdown());

    // Main loop
    while(!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(nullptr);

        // Rendering code
        glm::ivec2 framebufferSize;
        glfwGetFramebufferSize(window, &framebufferSize.x, &framebufferSize.y);
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        glClearColor(1,1,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    return EXIT_SUCCESS;
}

void printGlfwError(const std::string &message, std::ostream &out) {
    const char *errorDescription;
    int errorCode = glfwGetError(&errorDescription);
    out << message << " (GLFW error code " << errorCode << ": " << errorDescription << ")" << std::endl;
}