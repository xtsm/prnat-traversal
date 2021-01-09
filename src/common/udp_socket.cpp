#include "udp_socket.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <sstream>

const char* SocketTimeoutException::what() const noexcept {
    return "socket timeout";
}

UdpSocket::UdpSocket() : UdpSocket(0) {
}

UdpSocket::UdpSocket(uint16_t port) : sockfd(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)),
        size_limit(1024) {
    if (sockfd < 0) {
        throw std::runtime_error("socket creation failed");
    }
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) < 0) {
        throw std::runtime_error("socket binding failed");
    }
}

UdpSocket::~UdpSocket() {
    close(sockfd);
}

std::vector<uint8_t> UdpSocket::Recv() const {
    std::vector<uint8_t> buf(size_limit);
    ssize_t data_size = recv(sockfd, buf.data(), buf.size(), 0);
    if (data_size < 0) {
        throw std::runtime_error(std::strerror(errno));
    }
    buf.resize(data_size);
    return buf;
}

std::pair<std::vector<uint8_t>, IpAddress> UdpSocket::RecvFrom() const {
    std::vector<uint8_t> buf(size_limit);
    sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    ssize_t data_size = recvfrom(sockfd, buf.data(), buf.size(), 0,
            reinterpret_cast<sockaddr*>(&sin), &sin_len);
    if (data_size < 0) {
        throw std::runtime_error(std::strerror(errno));
    }
    buf.resize(data_size);
    return {buf, IpAddress{ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port)}};
}

void UdpSocket::SendTo(const std::vector<uint8_t>& data, IpAddress addr) const {
    if (data.size() > size_limit) {
        std::ostringstream err;
        err << "can't send " << data.size() << " bytes of data, limit is " << size_limit;
        throw std::runtime_error(err.str());
    }
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr.addr);
    sin.sin_port = htons(addr.port);
    ssize_t sent_size = sendto(sockfd, data.data(), data.size(), 0,
            reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
    if (sent_size < 0) {
        throw std::runtime_error(std::strerror(errno));
    }
    if (sent_size != data.size()) {  // this should never be true
        std::ostringstream err;
        err << "sent data size doesn't match: " << sent_size << " != " << data.size();
        throw std::runtime_error(err.str());
    }
}

size_t UdpSocket::GetSizeLimit() {
    return size_limit;
}

void UdpSocket::SetSizeLimit(size_t limit) {
    size_limit = limit;
}

void UdpSocket::SetRecvTimeout(size_t seconds) {
    timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<timeval*>(&tv), sizeof(tv));
    if (ret < 0) {
        throw std::runtime_error(std::strerror(errno));
    }
}
