#pragma once

#include "tcp/tcp_server.h"
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace rpc {

using tcp::TcpServer;


class RpcServer: public TcpServer {
private:
    using ErasedType = void(*)();
    std::unordered_map<std::string, ErasedType> funcs_;
    std::mutex mu_;

public:
    explicit RpcServer(const std::string& name, uint16_t port);
    RpcServer(const RpcServer &) = delete;
    RpcServer& operator=(const RpcServer&) = delete;
    ~RpcServer() override;

    template <typename... Args>
    bool Register(const std::string &name, void(*func)(Args...)) {
        std::lock_guard<std::mutex> lock(mu_);
        if (funcs_.find(name) != funcs_.end()) {
            return false;
        }
        funcs_[name] = reinterpret_cast<ErasedType>(func);
        return true;
    }

    template <typename... Args>
    bool Call(const std::string &name, Args... args) {
        std::lock_guard<std::mutex> lock(mu_);
        using FuncType = void(*)(Args...);
        auto it = funcs_.find(name);
        if (it == funcs_.end()) {
            return false;
        }
        auto func = reinterpret_cast<FuncType>(it->second);
        func(args...);
        return true;
    }


};


}