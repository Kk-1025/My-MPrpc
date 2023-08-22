#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc, char **argv)
{
    // 想使用mprpc框架 来享受rpc服务调用，要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 调用远程发布的rpc方法
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    
    // 生成 rpc方法的请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);
    
    // 生成 rpc方法的响应参数
    fixbug::GetFriendsListResponse response;
    
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 完成一次rpc调用，读取调用的结果
    if (controller.Failed())    // rpc调用出错
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendsList response success!" << std::endl;

            // 输出 rpc远程调用返回的结果
            int size = response.friends_size();
            std::cout << size << "           tttttttttttttttt\n";

            for (int i=0; i < size; ++i)
            {
                std::cout << "index:" << (i + 1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error : " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}