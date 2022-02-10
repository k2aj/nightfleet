#pragma once

#include <cassert>
#include <vector>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <network/exceptions.h>

// T should be a subclass of ContentType
// writing an appropriate template constraint is left as an exercise for the reader,
// because I prefer to keep my sanity intact
template<typename T>
class Registry {

    private:
    std::vector<const T *> objects;
    std::map<std::string, int> idToNumericID;

    public:

    void add(T &object) {
        assert(!contains(object.id));
        int numericID = static_cast<int>(objects.size());
        object.numericID = numericID;
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


template<typename Derived>
class ContentType {
    public:

    const std::string id;
    int32_t numericID = -1;

    ContentType(const std::string &id) : id(id) {}

    static Registry<Derived> registry;
};

template<typename T>
RxBuffer &operator>>(RxBuffer &rx, const ContentType<T> *value) {
    auto numericID = rx.read<decltype(ContentType<T>::numericID)>();
    auto &registry = ContentType<T>::registry;
    if(!registry.contains(numericID))
        throw ProtocolError("Unknown numeric ID.");
    value = &(registry[numericID]);
    return rx;
}

template<typename T>
TxBuffer &operator<<(TxBuffer &tx, const ContentType<T> &value) {
    return (tx << value.numericID);
}

template<typename T>
TxBuffer &operator<<(TxBuffer &tx, const ContentType<T> *value) {
    return operator<<(tx, *value);
}
