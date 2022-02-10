#pragma once

/** Thrown when the other side of communication does not obey 
 *  the application layer communication protocol. 
 */
class ProtocolError : public std::runtime_error {
    public:
    ProtocolError(const std::string &what) : std::runtime_error(what) {}
};