#include "mac.h"

Mac::Mac(const char* r) {
    int i;
    uint8_t* p;
    uint8_t ch1, ch2;

    p = reinterpret_cast<u_char*>(const_cast<char*>(r));
    for (i = 0 ; i < SIZE; i++) {
        ch1 = *p++;
        if (ch1 >= 'a')
            ch1 = ch1 - 'a' + 10;
        else
            if (ch1 >= 'A')
                ch1 = ch1 - 'A' + 10;
            else
                ch1 = ch1 - '0';
        ch2 = *p++;
        if (ch2 >= 'a')
            ch2 = ch2 - 'a' + 10;
        else if (ch2 >= 'A')
            ch2 = ch2 - 'A' + 10;
        else
            ch2 = ch2 - '0';
        mac_[i] = static_cast<uint8_t>((ch1 << 4) + ch2);
        while (*p == '-' || *p == ':') p++;
    }
}

Mac::operator std::string() const {
    uint8_t ch1, ch2;
    int i, index;
    char buf[SIZE * 3]; // enough size

    index = 0;
    for (i = 0; i < SIZE; i++) {
        ch1 = mac_[i] & 0xF0;
        ch1 = ch1 >> 4;
        if (ch1 > 9)
            ch1 = ch1 + 'A' - 10;
        else
            ch1 = ch1 + '0';
        ch2 = mac_[i] & 0x0F;
        if (ch2 > 9)
            ch2 = ch2 + 'A' - 10;
        else
            ch2 = ch2 + '0';
        buf[index++] = char(ch1);
        buf[index++] = char(ch2);
        if (i == 2)
            buf[index++] = '-';
    }
    buf[index] = '\0';
    return (std::string(reinterpret_cast<char*>(buf)));
}
