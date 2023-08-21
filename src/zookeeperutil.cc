#include "zookeeperutil.h"


// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  		// 如果回调的消息类型是 和会话相关的消息类型
	{
		if (state == ZOO_CONNECTED_STATE)  	// 如果zkclient和zkserver连接成功
		{
			// 发布信号量
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	}
}


ZkClient::ZkClient() : m_zhandle(nullptr)
{

}


ZkClient::~ZkClient()
{
	// 关闭句柄，释放资源
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}


void ZkClient::Start()
{
	// 获取zk服务器的ip地址
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connAddress = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	API调用线程 
	网络I/O线程  pthread_create  poll
	watcher回调线程 pthread_create
	*/

	// 初始化watcher
    m_zhandle = zookeeper_init(connAddress.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

	// 等待信号量
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}


void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
	// 临时缓冲区
    char pathBuf[128];
    int bufLen = sizeof(pathBuf);

	// 判断znode节点是否存在；如果存在，就无需再重复创建
	int flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) 	// znode节点不存在
	{
		// 创建指定的znode节点
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, pathBuf, bufLen);
		
		if (flag == ZOK)	// znode节点创建成功
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}


std::string ZkClient::GetData(const char *path)
{
	// 获取znode节点数据
    char buf[64];
	int bufLen = sizeof(buf);
	int flag = zoo_get(m_zhandle, path, 0, buf, &bufLen, nullptr);
	if (flag != ZOK)	// 获取节点数据失败
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else				// 获取节点数据成功
	{
		return buf;
	}
}