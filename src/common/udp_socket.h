#pragma once
#include "ip_address.h"
#include <string>
#include <cstdint>
#include <vector>

struct SocketTimeoutException : public std::exception {
    const char* what() const noexcept override;
};

class UdpSocket {
public:
    UdpSocket();
    
    explicit UdpSocket(uint16_t port);

    size_t GetSizeLimit();

    void SetSizeLimit(size_t limit);

    void SetRecvTimeout(size_t seconds);

    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    std::vector<uint8_t> Recv() const;
    
    std::pair<std::vector<uint8_t>, IpAddress> RecvFrom() const;

    void SendTo(const std::vector<uint8_t>& data, IpAddress addr) const;

private:
    int sockfd;
    size_t size_limit;
};
