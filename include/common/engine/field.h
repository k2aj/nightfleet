#pragma once

#include <cassert>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vector_relational.hpp>

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