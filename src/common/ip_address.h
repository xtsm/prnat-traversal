#pragma once
#include <string>
#include <vector>

struct IpAddress {
    uint32_t addr;
    uint16_t port;

    static IpAddress FromString(const std::string& s);
    
    std::string ToString() const;

    static IpAddress FromBytes(const std::vector<uint8_t>& data);

    std::vector<uint8_t> ToBytes() const;

    bool operator==(const IpAddress& oth) const;
    
    bool operator!=(const IpAddress& oth) const;
};
