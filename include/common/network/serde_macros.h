#pragma once

#define DECLARE_SERDE(Type) \
    RxBuffer &operator>>(RxBuffer &rx, Type &value); \
    TxBuffer &operator<<(TxBuffer &tx, const Type &value);

#define DEFINE_ENUM_SERDE(Enum) \
    RxBuffer &operator>>(RxBuffer &rx, Enum &enumValue) { \
        uint32_t rawValue; \
        rx >> rawValue; \
        if(rawValue >= 0 && rawValue < static_cast<uint32_t>(Enum::COUNT)) \
            enumValue = static_cast<Enum>(rawValue); \
        else \
            throw ProtocolError("Invalid enum value."); \
        return rx; \
    } \
    TxBuffer &operator<<(TxBuffer &tx, const Enum &enumValue) { \
        return (tx << static_cast<uint32_t>(enumValue)); \
    }

//dummy comment to prevent "\ at the end of file" warnings
//I know what I'm doing 