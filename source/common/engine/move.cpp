#include <engine/move.h>

#include <network/exceptions.h>

Move Move::moveUnit(const std::vector<glm::ivec2> &unitPath) {
    Move result;
    result.type = MoveType::MOVE_UNIT;
    for(auto pos : unitPath) {
        result.args.push_back(pos.x);
        result.args.push_back(pos.y);
    }
    return result;
}
Move Move::attackUnit(const Unit &attacker, const Unit &target) {
    Move result;
    result.type = MoveType::MOVE_UNIT;
    result.args = {attacker.position.x, attacker.position.y, target.position.x, target.position.y};
    return result;
}
Move Move::endTurn() {
    return {MoveType::END_TURN};
}
Move Move::surrender() {
    return {MoveType::SURRENDER};
}

DEFINE_ENUM_SERDE(MoveType)

RxBuffer &operator>>(RxBuffer &rx, Move &result) {
    return (rx >> result.type >> result.args);
}

TxBuffer &operator<<(TxBuffer &tx, const Move &result) {
    return (tx << result.type << result.args);
}

void RecordingMoveAcceptorProxy::makeMove(const Move &m) {
    downstream.makeMove(m);
    moveList.push_back(m);
}