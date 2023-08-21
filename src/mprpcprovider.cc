#include "mprpcprovider.h"


/*
service_name =>  service描述   
                        =》 service* 记录服务对象
                        method_name  =>  method方法对象
*/

// 接收父类指针service作为参数，使得各种服务都能使用
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    // 保存服务信息
    ServiceInfo service_info;
    service_info.m_service = service;   // 保存服务对象

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();
    // 获取服务对象的名字
    std::string service_name = pServiceDesc->name();
    // 获取服务对象的方法数量
    int methodCnt = pServiceDesc->method_count();
    
    LOG_INFO("service_name:%s", service_name.c_str());

    // 获取服务对象的每个方法的描述，并保存（抽象描述）     UserService：Login
    for (int i = 0; i < methodCnt; ++i)
    {
        const google::protobuf::MethodDescriptor* pMethodDesc = pServiceDesc->method(i);
        std::string method_name = pMethodDesc->name();                  // 获取方法名字

        service_info.m_methodMap.insert({method_name, pMethodDesc});    // 保存服务方法

        LOG_INFO("method_name:%s", method_name.c_str());
    }

    m_serviceMap.insert({service_name, service_info});  // 保存 注册成功的服务名字、服务方法的具体描述
}


void RpcProvider::Run()
{
    // 读取配置文件的信息，获取rpcserver的ip/port
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    //uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    uint16_t port = stoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);      // 生成muduo地址对象

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 绑定 连接回调 和 消息读写回调，分离 网络代码和业务代码
    // std::function<void (const TcpConnectionPtr&)>
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    // std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)>
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this,
                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap)               // 创建服务节点
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);             // 创建服务节点 不存数据

        for (auto &mp : sp.second.m_methodMap)  // 创建方法节点
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);   // 创建方法节点 存数据（ip:port）

            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    // LOG_INFO！！！

    // 启动网络服务
    server.start();
    m_eventLoop.loop(); 
}


void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    // rpc服务为短连接，返回响应后将自动关闭连接
    if (!conn->connected())     // 和rpc client的连接断开了，shutdown
    {
        conn->shutdown();
    }
}


/*
在框架内部，RpcProvider和RpcConsumer协商好 之间通信用的protobuf数据类型
service_name method_name args    定义proto的message类型，进行数据头的序列化和反序列化
                                 service_name method_name args_size （最后记录args_size是为了解决tcp的粘包问题，防止与后面的数据混在一起）
16UserServiceLoginzhang san123456   

header_size(4个字节) + header_str + args_str
10 "10"
10000 "1000000"
std::string   insert和copy方法 
*/


void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, 
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流    Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    // 反序列化 获取header数据
    if (rpcHeader.ParseFromString(rpc_header_str))      // 数据头反序列化成功
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else        // 数据头反序列化失败
    {
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息（后续改成日志输出）
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;          // 获取service对象 UserService
    const google::protobuf::MethodDescriptor *method = mit->second;     // 获取method对象  Login

    // 生成 请求参数request
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    // 解析请求的参数
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }

    // 生成 响应参数response
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给method方法的调用 绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}


// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))     // 对response进行序列化
    {
        // 序列化后，把rpc方法执行的结果 发送回rpc的调用方/客户端
        conn->send(response_str);
    }
    else    // 序列化失败
    {
        std::cout << "serialize response_str error!" << std::endl; 
    }
    
    conn->shutdown(); // 模拟http的短连接服务，由rpcprovider主动断开连接
}