
#include "rpc_server.h"
#include "tcp/tcp_socket.h"

#include <string>
#include "spdlog/spdlog.h"
#include <functional>


namespace rpc {

using tcp::TcpSocket;

RpcServer::RpcServer(const std::string& name, uint16_t port)
    : TcpServer(name, port) {

}

RpcServer::~RpcServer() {
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
}



bool RpcServer::Start() {

    bool success = init(); // create socket & bind & listen
    if (!success) {
        spdlog::error("rpc server's initialization failed. ");
        return false;
    }

    is_running_.store(true);

    main_thread_ = std::thread([&](){
        while (is_running_.load()) {
            struct sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(server_fd_, (struct sockaddr*)(&client_addr), &addr_len);
            if (client_fd < 0) {
                spdlog::warn("Failed to accept connection");
                break;
            }

            char ip[32];
            const char *client_ip = inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), ip, sizeof(ip));
            uint16_t port = ntohs(client_addr.sin_port);
            spdlog::debug("Incoming: [{}:{}]", client_ip, port);

            std::shared_ptr<TcpSocket> socket_ptr = std::make_shared<TcpSocket>(client_fd);


        }
    });


    return true;
}

}