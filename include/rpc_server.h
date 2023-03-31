
#include "tcp/tcp_server.h"
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <unordered_map>

namespace rpc {

using tcp::TcpServer;


class BaseCommand {
public:
    virtual ~BaseCommand() = default;
};

template <class... ArgTypes>
class Command : public BaseCommand {
    typedef std::function<void(ArgTypes...)> FuncType;
    FuncType f_;
public:
    Command() {}
    Command(FuncType f) : f_(f) {}
    void operator()(ArgTypes... args) { if(f_) f_(args...); }
};


class RpcServer: public TcpServer {
private:
    typedef std::shared_ptr<BaseCommand> BaseCommandPtr;
    typedef std::unordered_map<std::string, BaseCommandPtr> FMap;
    FMap procedures_;

public:
    explicit RpcServer(const std::string& name, uint16_t port);
    RpcServer(const RpcServer &) = delete;
    RpcServer& operator=(const RpcServer&) = delete;
    ~RpcServer() override;

    template<typename Func>
    bool Register(const std::string& name, const Func &f, bool force_cover=false);

    template <class... ArgTypes>
    void Execute(const std::string& name, ArgTypes&&... args) {
        using TmpType = typename std::remove_reference<ArgTypes...>::type;
//        typedef Command<ArgTypes...> CommandType;
        typedef Command<TmpType> CommandType;
        auto it = procedures_.find(name);
        if(it != procedures_.end()) {
            CommandType* c = dynamic_cast<CommandType*>(it->second.get());
            if(c) {
                (*c)(std::forward<ArgTypes>(args)...);
//                (*c)(args...);
            }
        }
    }

    bool Start() override;

};

template<typename T>
bool RpcServer::Register(const std::string& name, const T &f, bool force_cover) {
    procedures_.insert({name, BaseCommandPtr(new T(f))});
    return true;
}


}