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

    // Send a coupling request to server.
    socket.SendTo({}, opts.server_addr);

    // Receive Bob's address from server.
    // TODO: add heartbeat because NAT can forget about us while we're waiting.
    IpAddress bob_addr;
    while (true) {
        auto [data, addr] = socket.RecvFrom();
        if (addr == opts.server_addr) {
            bob_addr = IpAddress::FromBytes(data);
            break;
        }
    }

    // Probe Bob's NAT while opening our own to him
    socket.SetRecvTimeout(1);
    bool can_send = false;
    while (!can_send) {
        socket.SendTo({}, bob_addr);
        try {
            while (true) {
                auto [data, addr] = socket.RecvFrom();
                if (addr == bob_addr) {
                    // Non-empty message means Bob has received one of our probes
                    // and decided to send useful data so we should process it.
                    std::cout.write(reinterpret_cast<char*>(data.data()), data.size());
                    can_send = true;
                    break;
                }
            }
        } catch (const SocketTimeoutException& e) {
        }
    }

    // Now that we know that Bob's NAT lets out data through, we can send useful stuff.
    // TODO: replace this with netcat-like behavior.
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
