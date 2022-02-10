#pragma once

#include <cassert>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vector_relational.hpp>

#include <network/rxbuffer.h>
#include <network/txbuffer.h>

template<typename T>
class Field {

    private:
    std::vector<std::vector<T>> values;

    public:
    Field(glm::ivec2 size, const T &filler) :
        values(size.x, std::vector<T>(size.y, filler))  
    {}
    Field(glm::ivec2 size) : 
        Field(size, T{})
    {}
    Field() : 
        Field(glm::ivec2(0)) 
    {}
    
    glm::ivec2 size() const {
        return glm::ivec2(
            static_cast<int>(values.size()),
            static_cast<int>(values.front().size())
        );
    }
    const T &get(const glm::ivec2 &position) const {
        assert(inBounds(position));
        return values[position.x][position.y];
    }
    T &get(const glm::ivec2 &position) {
        return const_cast<T &>(const_cast<const Field *>(this)->get(position));
    }
    const T &getOr(const glm::ivec2 &position, const T& defaultValue) const {
        if(inBounds(position))
            return get(position);
        else
            return defaultValue;
    }
    T &getOr(const glm::ivec2 &position, T& defaultValue) {
        return const_cast<T &>(const_cast<const Field *>(this)->getOr(position, defaultValue));
    }
    void set(const glm::ivec2 &position, const T &value) {
        assert(inBounds(position));
        values[position.x][position.y] = std::move(value);
    }
    void fill(const T &value) {
        for(int x=0; x<size().x; ++x)
            for(int y=0; y<size().y; ++y)
                set(glm::ivec2(x,y), value);
    }
    bool trySet(const glm::ivec2 &position, const T &value) {
        if(inBounds(position)) {
            set(position,value);
            return true;
        } else {
            return false;
        }
    }
    bool inBounds(const glm::ivec2 &position) const {
        return glm::all(glm::greaterThanEqual(position, glm::ivec2(0))) &&
               glm::all(glm::lessThan(position, size()));
    }
};

template<typename T>
RxBuffer &operator>>(RxBuffer &rx, Field<T> &field) {
    auto width = rx.read<uint32_t>();
    auto height = rx.read<uint32_t>();
    field = Field<T>(glm::ivec2(width, height));
    for(uint32_t y=0; y<height; ++y)
        for(uint32_t x=0; x<width; ++x)
            field.set(glm::ivec2(x,y), rx.read<T>());
    return rx;
}

template<typename T>
TxBuffer &operator<<(TxBuffer &tx, const Field<T> &field) {
    tx << field.size().x << field.size().y;
    for(int y=0; y<field.size().y; ++y)
        for(int x=0; x<field.size().x; ++x)
            tx << field.get(glm::ivec2(x,y));
    return tx;
}