#include "ip_address.h"
#include <regex>
#include <sstream>

IpAddress IpAddress::FromString(const std::string& s) {
    static const auto regex = std::regex(R"~((\d+)\.(\d+)\.(\d+)\.(\d+):(\d+))~");
    std::smatch match;
    if (!std::regex_match(s, match, regex)) {
        throw std::runtime_error("invalid IPv4 address");
    }
    IpAddress ret;
    ret.addr = 0;
    for (size_t i = 1; i < 5; ++i) {
        ret.addr = (ret.addr << 8) | (std::stoi(match.str(i)) & 0xff);
    }
    ret.port = std::stoi(match.str(5));
    return ret;
}
    
std::string IpAddress::ToString() const {
    std::ostringstream ss;
    ss << ((addr >> 24) & 0xff) << '.'
       << ((addr >> 16) & 0xff) << '.'
       << ((addr >> 8) & 0xff) << '.'
       << (addr & 0xff) << ':'
       << port;
    return ss.str();
}

IpAddress IpAddress::FromBytes(const std::vector<uint8_t>& data) {
    IpAddress ret;
    ret.addr = 0;
    for (size_t i = 0; i < 4; ++i) {
        ret.addr = (ret.addr << 8) | data[i];
    }
    ret.port = 0;
    for (size_t i = 4; i < 6; ++i) {
        ret.port = (ret.port << 8) | data[i];
    }
    return ret;
}

std::vector<uint8_t> IpAddress::ToBytes() const {
    std::vector<uint8_t> ret;
    for (size_t i = 4; i--; ) {
        ret.push_back(addr >> (8 * i));
    }
    for (size_t i = 2; i--; ) {
        ret.push_back(port >> (8 * i));
    }
    return ret;
}

bool IpAddress::operator==(const IpAddress& oth) const {
    return addr == oth.addr && port == oth.port;
}

bool IpAddress::operator!=(const IpAddress& oth) const {
    return !(*this == oth);
}
