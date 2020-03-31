#include "ip.h"
#include <string.h>
#include <arpa/inet.h>

Ip::Ip(const char* r) {
    int res = inet_pton(AF_INET, r, &ip_);
    switch (res) {
        case 0:
            fprintf(stderr, "inet_pton return zero");
            break;
        case 1: // succeed
            ip_ = ntohl(ip_);
            break;
        default:
            fprintf(stderr, "inet_pton return %d error=%d(%s)\n", res, errno, strerror(errno));
    }
}

Ip::operator std::string() const {
    uint32_t ip = htonl(ip_);
    char s[INET_ADDRSTRLEN];
    const char* res = inet_ntop(AF_INET, &ip, s, INET_ADDRSTRLEN);
    if (res == nullptr) {
        fprintf(stderr, "inet_ntop return null error=%d(%s)\n", errno, strerror(errno));
        return std::string();
    }
    return std::string(s);
}
