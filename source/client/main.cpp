#include <iostream>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include <glm/glm.hpp>
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
#include <engine/content.h>
#include <engine/map.h>
#include <engine/game.h>
#include <graphics.h>
#include <dgl/debug.h>

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
    std::string loginRejectionReason, usernameStr;
    const Map *selectedMap = nullptr;
    char enteredGameID[32] = {'\0'};
    std::unique_ptr<Game> game;
    GameID gameID = 0;

    glm::mat4 projMatrix{1};
    SpriteRenderer renderer;
    std::vector<AtlasArea> unitSprites, terrainSprites;
    AtlasArea victoryMsg, defeatMsg;
    
    std::map<glm::ivec2, glm::ivec2, IVec2Comparator> selectedUnitMovementRange;
    int playerIndex;
    std::vector<Move> savedMoves;

    public:

    static constexpr glm::ivec2 NO_TILE_SELECTED = glm::ivec2{-1};
    glm::ivec2 windowSize, gridMousePos, selectedTile = NO_TILE_SELECTED;

    NFClientProtocolEntity(int sockfd) : NFProtocolEntity(sockfd) {

        victoryMsg = renderer.loadImage("../textures/victory.png");
        defeatMsg = renderer.loadImage("../textures/defeat.png");

        for(int id = 0; id < UnitType::registry.size(); ++id)
            unitSprites.push_back(renderer.loadImage(std::string("../textures/units/") + UnitType::registry[id].id + std::string(".png")));

        for(int id = 0; id < TerrainType::registry.size(); ++id)
            terrainSprites.push_back(renderer.loadImage(std::string("../textures/terrain/") + TerrainType::registry[id].id + std::string(".png")));
    }

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
                loginRejectionReason = "Unknown error.";
                halt();
                break;
        }

        waitingForLoginResponse = false;
    }

    void onAlertRequest(const AlertRequest &r) override {
        std::cerr << "ALERT: " << r.message << std::endl;
    }

    void makeMove(const Move &m) {
        assert(game->currentPlayer() == usernameStr);
        game->makeMove(m);
        savedMoves.push_back(m);
    }

    void showUnitInfo(const std::string &windowName, const Unit &unit) {
        ImGui::Begin(windowName.c_str());
            ImGui::TextColored(unit.player == playerIndex ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "%s", unit.type->id.c_str());
            ImGui::Text("HP: %d/%d", unit.health, unit.type->maxHealth);
            ImGui::Text("MP: %d/%d", unit.movementPoints, unit.type->movementPointsPerTurn);
            ImGui::Text("AP: %d/%d", unit.actionPoints, unit.type->actionPointsPerTurn);
        ImGui::End();
    }

    void onUpdate(const Duration &dt) override {\

        ImGuiIO &io = ImGui::GetIO();

        glm::vec2 relativeMousePos = (glm::vec2(ImGui::GetMousePos()) / glm::vec2(windowSize) * glm::vec2(2) - glm::vec2(1)) * glm::vec2(1,-1);
        glm::vec4 gp = (glm::inverse(projMatrix) * glm::vec4(relativeMousePos, 0.0, 1.0f));
        gridMousePos = glm::ivec2(std::round(gp.x), std::round(gp.y));

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
                    credentials.username = usernameStr = username;
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
                    gameID = 0;
                    guiFsm = WAITING_ROOM;
                }

                ImGui::End();
            }
            break;

            case WAITING_ROOM: {
                ImGui::Begin("Info");
                ImGui::Text("Waiting for other players to join.");
                if(gameID)
                    ImGui::Text("Game ID = %d", (int)gameID);
                if(ImGui::Button("Leave game")) {
                    sendLeaveGameRequest({});
                    guiFsm = GAME_LOBBY;
                }
                ImGui::End();
            }
            break;

            case INGAME: {

                bool myTurn = game->currentPlayer() == usernameStr;

                auto selectedUnit = game->unitAt(selectedTile);
                auto hoveredUnit = game->unitAt(gridMousePos);

                if(selectedUnit != nullptr) {
                    selectedUnitMovementRange = game->findReachableTiles(*selectedUnit);
                } else
                    selectedUnitMovementRange.clear();

                if(!io.WantCaptureMouse) {
                    if(io.MouseClicked[0]) {

                        if(selectedTile == gridMousePos) //selecting same thing twice = deselection
                            selectedTile = NO_TILE_SELECTED;
                        else if(selectedUnit == nullptr || selectedUnit->player != playerIndex || !myTurn) { 
                            selectedTile = gridMousePos;
                        } else { //friendly unit is selected and we can make a move
                            if(selectedUnitMovementRange.find(gridMousePos) != selectedUnitMovementRange.end()) {
                                //we clicked a location, which can be moved to!
                                std::vector<glm::ivec2> path = {gridMousePos};
                                while(path.back() != selectedUnit->position)
                                    path.push_back(selectedUnitMovementRange[path.back()]);
                                std::reverse(path.begin(), path.end());
                                makeMove(Move::moveUnit(path));

                            } else if (selectedUnit->actionPoints > 0 && hoveredUnit != nullptr && hoveredUnit->player != playerIndex && areTilesAdjacent(selectedTile, gridMousePos)) {
                                //we clicked on an enemy unit in attack range
                                makeMove(Move::attackUnit(*selectedUnit, *hoveredUnit));
                                if(selectedUnit->actionPoints <= 0)
                                    selectedTile = NO_TILE_SELECTED;
                            } else {
                                selectedTile = gridMousePos;
                            }
                        }
                    }
                }

                ImGui::Begin("Game");
                    ImGui::TextColored(myTurn ? Colors::green : Colors::red, "Current player: %s", game->currentPlayer().c_str());
                    if(ImGui::Button("End turn") && myTurn) 
                        makeMove(Move::endTurn());
                    if(ImGui::Button("Surrender") && myTurn)
                        makeMove(Move::surrender());
                    if(ImGui::Button("Quit")) {
                        sendLeaveGameRequest({});
                        guiFsm = GAME_LOBBY;
                    }
                ImGui::End();

                if(selectedUnit != nullptr)
                    showUnitInfo("Selected unit", *selectedUnit);
                if(hoveredUnit != nullptr && hoveredUnit != selectedUnit)
                    showUnitInfo("Hovered unit", *hoveredUnit);

                if(!savedMoves.empty()) {
                    sendIncrementalSync({savedMoves});
                    savedMoves.clear();
                }
            }
            break;
        }
    }

    void render() {

        if(guiFsm != INGAME)
            return;

        renderer.clear();
        
        for(int x=0; x<game->terrain.size().x; ++x)
            for(int y=0; y<game->terrain.size().y; ++y) {
                glm::ivec2 pos = glm::ivec2(x,y);

                auto terrainSprite = terrainSprites[game->terrain.get(pos)->numericID];
                renderer.drawImage(terrainSprite, pos, glm::vec2(0.5f));

                auto unit = game->unitAt(pos);
                if(unit != nullptr) {

                    renderer.mulColor(unit->player == playerIndex ? glm::vec4(0,1,0,0.25) : glm::vec4(1,0,0,0.25));
                    renderer.drawRectangle(pos, glm::vec2(0.5f));
                    renderer.mulColor();

                    auto unitSprite = unitSprites[unit->type->numericID];
                    renderer.drawImage(unitSprite, pos, glm::vec2(0.5f));

                    if(unit->health < unit->type->maxHealth) {
                        glm::vec2 hpBarCenter = glm::vec2(0,-0.45f) + glm::vec2(unit->position), hpBarRadii = glm::vec2(0.4f, 0.025f);
                        float relativeHealthLeft = unit->health / (float) unit->type->maxHealth;

                        renderer.mulColor({0,0,0,1});
                        renderer.drawRectangle(hpBarCenter, hpBarRadii);
                        if(relativeHealthLeft > 0.7)
                            renderer.mulColor({0,1,0,1});
                        else if(relativeHealthLeft > 0.3)
                            renderer.mulColor({1,1,0,1});
                        else renderer.mulColor({1,0,0,1});
                        renderer.drawRectangle(hpBarCenter, hpBarRadii*glm::vec2(relativeHealthLeft, 1));
                        renderer.mulColor();
                    }
                }
            }

        renderer.mulColor(glm::vec4(1,1,0,1) * glm::vec4(sin(glfwGetTime()*8)*0.3300 + 0.3301));
        for(auto [succ,pred] : selectedUnitMovementRange)
            renderer.drawRectangle(succ, glm::vec2(0.5f));
        renderer.mulColor();

        if(game->terrain.inBounds(gridMousePos)) {
            renderer.mulColor({1,1,1,0.33f});
            renderer.drawRectangle(gridMousePos, glm::vec2(0.5f));
            renderer.mulColor();
        }

        if(selectedUnitMovementRange.find(gridMousePos) != selectedUnitMovementRange.end()) {
            auto p1 = gridMousePos;
            for(auto p2 = selectedUnitMovementRange[p1]; p1 != p2; p1 = p2, p2 = selectedUnitMovementRange[p2])
                renderer.drawLine(p1, p2, 0.1);
        }

        glm::vec2 msgCenter = glm::vec2(game->terrain.size())/glm::vec2(2);
        glm::vec2 msgRadii = {msgCenter.x, msgCenter.x*9/16};

        if(game->didPlayerLoose(usernameStr)) {
            renderer.drawImage(defeatMsg, msgCenter, msgRadii);             
        } else if(game->didPlayerWin(usernameStr)) {
            renderer.drawImage(victoryMsg, msgCenter, msgRadii); 
        }

        renderer.render(projMatrix);
    }

    void onFullSync(const Game &gameState) override {
        std::cerr << "Received full sync from server!";
        game = std::make_unique<Game>(gameState);
        playerIndex = game->getPlayerIndex(usernameStr);
        projMatrix[0][0] = 1.8f / game->terrain.size().x;
        projMatrix[1][1] = 1.8f / game->terrain.size().y;
        projMatrix[3] = glm::vec4(-0.9f, -0.9f, 0, 1);
        guiFsm = INGAME;
    }

    void onIncrementalSync(const GameIncrementalSync &sync) override {

        if(guiFsm != INGAME)
            return;

        for(auto move : sync.moveList)
            try {
                game->makeMove(move);
            } catch (InvalidMoveError e) {
                throw ProtocolError(std::string("Invalid move in received IncrementalSync: ") + std::string(e.what()));
            }
    }

    void onHostGameAck(const HostGameAck &ack) {
        gameID = ack.gameID;
    }

    void onLeaveGameRequest(const LeaveGameRequest &) override {
        guiFsm = GAME_LOBBY;
    }

    void onGameJoinError(GameJoinError) override {
        guiFsm = GAME_LOBBY;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
    initOpenGLDebugOutput(std::cerr);

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

    char ipAddrBuf[32] = "127.0.0.1";
    const char *connectionError = nullptr;
    std::unique_ptr<NFClientProtocolEntity> entity;
    
    while(!glfwWindowShouldClose(window) && !interrupted) {

        TimePoint t1 = Clock::now();
        Duration dt = t1 - t0;
        t0 = t1;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(entity == nullptr) {
            ImGui::Begin("Choose your server");
            ImGui::InputText("Server IP Address", ipAddrBuf, sizeof(ipAddrBuf));
            if(ImGui::Button("Connect")) {
                connectionError = nullptr;
                int sockfd = createClientSocket(ipAddrBuf, defaultServerPort);
                if(sockfd == -1)
                    connectionError = strerror(errno);
                else {
                    entity = std::make_unique<NFClientProtocolEntity>(sockfd);
                    glfwGetWindowSize(window, &entity->windowSize.x, &entity->windowSize.y);
                    entity->onInit();
                }
            }
            if(connectionError != nullptr)
                ImGui::TextColored(Colors::red, "%s", connectionError);
            ImGui::End();
        } else {
            glfwGetWindowSize(window, &entity->windowSize.x, &entity->windowSize.y);
            entity->runNetworkEvents();
            entity->onUpdate(dt);
            if(!entity->isRunning())
                entity = {};
        }

        // Rendering code
        glm::ivec2 framebufferSize;
        glfwGetFramebufferSize(window, &framebufferSize.x, &framebufferSize.y);
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        glClearColor(0,0,0.1,1);
        glClear(GL_COLOR_BUFFER_BIT);

        if(entity != nullptr)
            entity->render();

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