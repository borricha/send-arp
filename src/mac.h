#pragma once

#include <cstdint>
#include <cstring>
#include <string>

// ----------------------------------------------------------------------------
// Mac
// ----------------------------------------------------------------------------
struct Mac final {
public:
	static constexpr int SIZE = 6;

protected:
	uint8_t mac_[SIZE];

public:
	//
	// constructor
	//
	Mac() {}
	Mac(const Mac& rhs) { memcpy(this->mac_, rhs.mac_, SIZE); }
	Mac(const uint8_t* rhs) { memcpy(this->mac_, rhs, SIZE); }
	Mac(const std::string& rhs);

	// assign operator
	Mac& operator = (const Mac& rhs) { memcpy(this->mac_, rhs.mac_, SIZE); return *this; }

	//
	// casting operator
	//
	explicit operator uint8_t*() const { return const_cast<uint8_t*>(mac_); }
	explicit operator std::string() const;

	//
	// comparison operator
	//
	bool operator == (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) == 0; }
	bool operator != (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) != 0; }
	bool operator < (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) < 0; }
	bool operator > (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) > 0; }
	bool operator <= (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) <= 0; }
	bool operator >= (const Mac& rhs) const { return memcmp(mac_, rhs.mac_, SIZE) >= 0; }
	bool operator == (const uint8_t* rhs) const { return memcmp(mac_, rhs, SIZE) == 0; }

public:
	void clear() {
		*this = nullMac();
	}

	bool isNull() const {
		return *this == nullMac();
	}

	bool isBroadcast() const { // FF:FF:FF:FF:FF:FF
		return *this == broadcastMac();
	}

	bool isMulticast() const { // 01:00:5E:0*
		return mac_[0] == 0x01 && mac_[1] == 0x00 && mac_[2] == 0x5E && (mac_[3] & 0x80) == 0x00;
	}

	static Mac randomMac();
	static Mac& nullMac();
	static Mac& broadcastMac();
};

namespace std {
	template<>
	struct hash<Mac> {
		size_t operator() (const Mac & rhs) const {
			uint8_t* p = (uint8_t*)rhs;
			uint16_t* p1 = (uint16_t*)p;
			uint32_t* p2 = (uint32_t*)(p + 2);
			return *p1 + *p2;
		}
	};
}
