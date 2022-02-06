#include <iostream>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <scope_guard.h>
#include <network/defaults.h>
#include <network/message.h>

void printGlfwError(const std::string &message, std::ostream &out = std::cerr);
int createClientSocket(const std::string &serverAddress, uint16_t serverPort);

int main(int argc, char **argv) {

    int sockfd = createClientSocket("127.0.0.1", defaultServerPort);
    if(sockfd == -1) {
        perror("Failed to connect to server"); 
        return EXIT_FAILURE;
    }

    MessageSocket server(sockfd);

    while(server.isConnected()) {
        std::string line;
        std::getline(std::cin, line);
        if(line.empty())
            break;

        TxBuffer request;
        request << line;
        server.sendMessage(request);

        while(true) {

            server.update();

            if(!server.isConnected()) {
                std::cout << "Server disconnected." << std::endl;
                break;
            }

            if(server.hasMessage()) {
                RxBuffer response = server.receiveMessage();
                std::string recvLine;
                response >> recvLine;
                std:: cout << "Got response: " << recvLine << std::endl;
                break;
            }
        }
    }
    
    /*// Initialize GLFW
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
    }*/

    return EXIT_SUCCESS;
}

void printGlfwError(const std::string &message, std::ostream &out) {
    const char *errorDescription;
    int errorCode = glfwGetError(&errorDescription);
    out << message << " (GLFW error code " << errorCode << ": " << errorDescription << ")" << std::endl;
}

int createClientSocket(const std::string &serverAddress, uint16_t serverPort) {

    sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_addr.s_addr = inet_addr(serverAddress.c_str());
    sa.sin_port = htons(serverPort);
    sa.sin_family = AF_INET;

    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1)
        return -1;
    
    if(connect(sockfd, (sockaddr*)(&sa), sizeof sa) == -1) {
        auto old_errno = errno;
        close(sockfd);
        errno = old_errno;
        return -1;
    }

    return sockfd;
}