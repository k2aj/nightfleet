#pragma once

#include <network/message.h>
#include <network/protocol.h>

LoginResponse tryLogin(MessageSocket &server, const LoginRequest &credentials);