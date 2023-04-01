#pragma once

#include "tcp/tcp_server.h"
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <any>

namespace rpc {

using tcp::TcpServer;


class RpcServer: public TcpServer {
private:
    std::unordered_map<std::string, std::any> funcs_;
    std::mutex mu_;

public:
    explicit RpcServer(const std::string& name, uint16_t port);
    RpcServer(const RpcServer &) = delete;
    RpcServer& operator=(const RpcServer&) = delete;
    ~RpcServer() override;

    template <typename F>
    bool Register(const std::string &name, F&& func) {
        std::lock_guard<std::mutex> lock(mu_);
        if (funcs_.find(name) != funcs_.end()) {
            return false;
        }
        funcs_[name] = std::function( std::forward<F>(func) );
        return true;
    }

    template <typename... Args>
    bool Call(const std::string &name, Args&& ... args) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = funcs_.find(name);
        if (it == funcs_.end()) {
            return false;
        }
        std::any_cast< std::function<void(Args...)> >(it->second)(std::forward<Args>(args)...);
        return true;
    }


};


}