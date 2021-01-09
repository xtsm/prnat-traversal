#include "common/ip_address.h"
#include "common/udp_socket.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

struct ClientOptions {
    IpAddress server_addr;
};

ClientOptions ParseCmdline(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error("wrong number of parameters");
    }
    ClientOptions ret;
    ret.server_addr = IpAddress::FromString(argv[1]);
    return ret;
}

struct ClientDatagram {
    bool need_ack;
    std::vector<uint8_t> payload;
    
    std::vector<uint8_t> ToBytes() const {
        std::vector<uint8_t> ret(payload.size() + 1);
        if (need_ack) {
            ret[0] |= 0x1;
        }
        std::copy(payload.begin(), payload.end(), ret.begin() + 1);
        return ret;
    }
    
    static ClientDatagram FromBytes(const std::vector<uint8_t>& data) {
        if (data.empty() || (data[0] & 0xfe) != 0) {
            throw std::runtime_error("invalid client datagram");
        }
        ClientDatagram ret;
        ret.need_ack = data[0] & 0x1;
        ret.payload.resize(data.size() - 1);
        std::copy(data.begin() + 1, data.end(), ret.payload.begin());
        return ret;
    }
};

int main(int argc, char** argv) {
    ClientOptions opts;
    try {
        opts = ParseCmdline(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "usage: " << argv[0] << " SERVER-ADDR" << std::endl;
        return 1;
    }

    UdpSocket socket;

    socket.SendTo({}, opts.server_addr);

    IpAddress bob_addr;
    while (true) {
        auto [data, addr] = socket.RecvFrom();
        if (addr == opts.server_addr) {
            bob_addr = IpAddress::FromBytes(data);
            break;
        }
    }

    socket.SetRecvTimeout(1);
    bool can_send = false;
    while (!can_send) {
        socket.SendTo({}, bob_addr);
        try {
            while (true) {
                auto [data, addr] = socket.RecvFrom();
                if (addr == bob_addr) {
                    std::cout.write(reinterpret_cast<char*>(data.data()), data.size());
                    can_send = true;
                    break;
                }
            }
        } catch (const SocketTimeoutException& e) {
        }
    }
    socket.SetRecvTimeout(0);
    
    std::thread recv_thread([&socket, bob_addr]() {
        while (true) {
            auto [data, addr] = socket.RecvFrom();
            if (addr == bob_addr) {
                std::cout.write(reinterpret_cast<char*>(data.data()), data.size());
            }
        }
    });
    
    std::thread send_thread([&socket, bob_addr]() {
        while (true) {
            socket.SendTo({'H', 'e', 'l', 'l', 'o', '\n'}, bob_addr);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    recv_thread.join();
    send_thread.join();
}
