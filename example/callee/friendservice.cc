#include <iostream>
#include <string>
#include <vector>
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "logger.h"
#include "friend.pb.h"


class FriendService : public fixbug::FriendServiceRpc
{
public:
    // 本地函数
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service! userid:" << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 获取请求参数
        uint32_t userid = request->userid();

        // 调用本地函数，完成本地业务，获取函数调用结果
        std::vector<std::string> friendsList = GetFriendsList(userid);

        // 设置响应结果
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friendsList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }

        // 调用回调函数，向rpc请求方 返回调用结果
        done->Run();
    }
};


int main(int argc, char **argv)
{
    // 日志测试
    //LOG_ERR("ddddd");
    //LOG_INFO("ddddd");

    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象；将服务对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}