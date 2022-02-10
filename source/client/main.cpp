#include <iostream>

#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <colors.h>
#include <scope_guard.h>
#include <network/defaults.h>
#include <network/message.h>
#include <network/protocol.h>
#include <clientlogin.h>
#include <engine/content.h>
#include <engine/map.h>
#include <engine/game.h>

class NFClientProtocolEntity : public NFProtocolEntity {
    private:
    enum {
        DISCONNECTED,
        LOGIN_SCREEN,
        GAME_LOBBY,
        WAITING_ROOM,
        INGAME
    } guiFsm = DISCONNECTED;

    char username[32] = {'\0'}, password[32] = {'\0'};
    bool waitingForLoginResponse = false;
    std::string loginRejectionReason;
    const Map *selectedMap = nullptr;
    char enteredGameID[32] = {'\0'};

    public:

    NFClientProtocolEntity(int sockfd) : NFProtocolEntity(sockfd) {}

    void onVersionHandshake(const Version &version) override {

        blacklist.insert(MessageType::VERSION);
        whitelist = {MessageType::LOGIN_RESPONSE};

        if(applicationVersion.isCompatibleWith(version)) {
            std::cerr << "Succesfully connected to server." << std::endl;
            guiFsm = LOGIN_SCREEN;
        } else {
            std::cerr << "Error: mismatched client/server version." << std::endl;
            halt();
        }
    }

    void onLoginResponse(LoginResponse r) override {

        switch(r) {

            case LoginResponse::OK: 
                std::cerr << "Login succesful!" << std::endl;
                whitelist.clear();
                blacklist.insert(MessageType::LOGIN_RESPONSE);
                guiFsm = GAME_LOBBY;
                break;

            case LoginResponse::E_ALREADY_LOGGED_IN:
                std::cerr << "Error: user is already logged in." << std::endl;
                loginRejectionReason = "This user is already logged in.";
                break;

            default:
                std::cerr << "Unknown login error." << std::endl;
                halt();
                break;
        }

        waitingForLoginResponse = false;
    }

    void onAlertRequest(const AlertRequest &r) override {
        std::cerr << "ALERT: " << r.message << std::endl;
    }

    void onUpdate(const Duration &dt) override {

        switch(guiFsm) {

            case DISCONNECTED: {
                ImGui::Begin("Info");
                ImGui::Text("Connecting to server...");
                ImGui::End();
            }
            break;

            case LOGIN_SCREEN: {
                ImGui::Begin("Login");
                ImGui::InputText("Username", username, sizeof username);
                ImGui::InputText("Password", password, sizeof password);

                if(ImGui::Button("Login") && !waitingForLoginResponse) {
                    LoginRequest credentials;
                    credentials.username = username;
                    sendLoginRequest(credentials);
                    setTimeout(5s);
                    waitingForLoginResponse = true;
                    loginRejectionReason = "";
                }
                if(!loginRejectionReason.empty())
                    ImGui::TextColored(Colors::red, loginRejectionReason.c_str());

                ImGui::End();
            }
            break;

            case GAME_LOBBY: {

                ImGui::Begin("Lobby");

                ImGui::InputText("Game ID", enteredGameID, sizeof enteredGameID);
                int64_t gameID = atoll(enteredGameID);

                ImGui::SameLine();

                if(
                    (gameID == 0 && strlen(enteredGameID) > 0) || //atoll failed
                    gameID < 0                                    //negative IDs are illegal
                )
                    ImGui::TextColored(Colors::red, "Invalid game ID!");
                else if(ImGui::Button("Join game")) {
                    sendJoinGameRequest({gameID});
                    guiFsm = WAITING_ROOM;
                }

                ImGui::Text("Advanced");
                if(ImGui::BeginListBox("Map")) {
                    for(int i=0; i<Map::registry.size(); ++i) {
                        const Map *map = &Map::registry[i];
                        if(ImGui::Selectable(map->id.c_str(), selectedMap == map))
                            selectedMap = map;
                    }
                    ImGui::EndListBox();
                }
                if(ImGui::Button("Host a new game") && selectedMap != nullptr) {
                    sendHostGameRequest({selectedMap});
                    guiFsm = WAITING_ROOM;
                }

                ImGui::End();
            }
            break;

            case WAITING_ROOM: {
                ImGui::Begin("Info");
                ImGui::Text("Waiting for other players to join.");
                if(ImGui::Button("Leave game")) {
                    sendLeaveGameRequest({});
                    guiFsm = GAME_LOBBY;
                }
                ImGui::End();
            }
            break;

            case INGAME: {

            }
            break;
        }
    }

    void onFullSync(const Game &gameState) override {
        std::cerr << "Received full sync from server!";
        if(guiFsm == WAITING_ROOM) {
            //todo start the game
            guiFsm = INGAME;
        }
    }

    void onProtocolError(const ProtocolError &e) override {
        std::cerr << "Protocol error: " << e.what() << std::endl;
        halt();
    }

    void onTimeout() override {
        std::cerr << "Connection timed out." << std::endl;
        halt();
    }

    void onDisconnect() override {
        std::cerr << "Connection closed by remote host." << std::endl;
        halt();
    }
};

void printGlfwError(const std::string &message, std::ostream &out = std::cerr);
int createClientSocket(const std::string &serverAddress, uint16_t serverPort);

volatile sig_atomic_t interrupted = 0;
void signalHandler(int signum) {
    interrupted = 1;
}

int main(int argc, char **argv) {

    TimePoint t0 = Clock::now();

    signal(SIGINT, signalHandler);

    //Initialize GLFW
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

    // Initialize game content
    initGameContent();

    // Connect to server
    // TODO this should be moved somewhere else
    int sockfd = createClientSocket("127.0.0.1", defaultServerPort);
    if(sockfd == -1) {
        perror("Failed to connect to server"); 
        return EXIT_FAILURE;
    }

    NFClientProtocolEntity entity(sockfd);
    entity.onInit();

    while(entity.isRunning() && !glfwWindowShouldClose(window) && !interrupted) {

        TimePoint t1 = Clock::now();
        Duration dt = t1 - t0;
        t0 = t1;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        entity.runNetworkEvents();
        entity.onUpdate(dt);

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
        glfwPollEvents();
    }

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