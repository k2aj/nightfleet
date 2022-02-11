#pragma once

#include <vector>

#include <glm/vec2.hpp>

#include <network/serde_macros.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <engine/unit.h>

enum class MoveType : uint32_t {
    MOVE_UNIT = 0,
    ATTACK_UNIT = 1,
    END_TURN = 2,
    SURRENDER = 3,
    COUNT = 4
};

struct Move {
    MoveType type;
    std::vector<int32_t> args;

    static Move moveUnit(const std::vector<glm::ivec2> &unitPath);
    static Move attackUnit(const Unit &attacker, const Unit &target);
    static Move endTurn();
    static Move surrender();
};

DECLARE_SERDE(Move)
DECLARE_SERDE(MoveType)

class MoveAcceptor {
    public:
    virtual void makeMove(const Move &) = 0;
};

class RecordingMoveAcceptorProxy : public MoveAcceptor {
    private:
    MoveAcceptor &downstream;

    public:
    std::vector<Move> moveList;

    RecordingMoveAcceptorProxy(MoveAcceptor &downstream) : downstream(downstream) {}
    void makeMove(const Move &) override;
};

class InvalidMoveError : public std::runtime_error {
    public:
    InvalidMoveError(const std::string &what) : std::runtime_error(what) {}
};