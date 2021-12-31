#pragma once

#include <cassert>
#include <vector>
#include <map>
#include <stdexcept>

template<typename T>
class Registry {

    private:
    std::vector<const T *> objects;
    std::map<std::string, int> idToNumericID;

    public:

    void add(const T &object) {
        assert(!contains(object.id));
        int numericID = static_cast<int>(objects.size());
        idToNumericID[object.id] = numericID;
        objects.push_back(&object);
    }

    bool contains(int numericID) const {
        return numericID >= 0 && numericID < objects.size();
    }

    bool contains(const std::string &id) const {
        return idToNumericID.find(id) != idToNumericID.end();
    }

    const T &operator[](int numericID) const {
        assert(contains(numericID));
        return *(objects[numericID]);
    }

    const T &operator[](const std::string &id) const {
        assert(contains(id));
        int numericID = idToNumericID.find(id)->second;
        return operator[](numericID);
    }

    const T &getDefault() const {
        return operator[](0);
    }
};