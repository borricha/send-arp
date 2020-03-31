#pragma once

#include <cstdint>
#include <cstring>
#include <string>

struct Mac final {
    static const int SIZE = 6;

    //
    // constructor
    //
    Mac() {}
    Mac(const uint8_t* r) { memcpy(this->mac_, r, SIZE); }
    Mac(const char* r);

    //
    // casting operator
    //
    operator uint8_t*() const { return const_cast<uint8_t*>(mac_); } // default
    explicit operator std::string() const;

protected:
    uint8_t mac_[SIZE];
};
