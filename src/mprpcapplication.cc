#include "mprpcapplication.h"


// 静态变量的类外初始化
MprpcConfig MprpcApplication::m_config;


// 显示命令行的正确使用信息
void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>" << std::endl;
}


void MprpcApplication::Init(int argc, char **argv)
{
    // 输入命令时缺少参数
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    // 获取配置文件路径
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)     // 获取命令行参数
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 加载配置文件 rpcserver_ip=  rpcserver_port=  zookeeper_ip=  zookepper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
}


MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}


MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}