#pragma once
#include <iostream>
#include <string>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include "mprpcapplication.h"


// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();

    // zkclient启动，连接zkserver
    void Start();

    // 根据指定的路径，创建znode节点
    void Create(const char *path, const char *data, int datalen, int state = 0);

    // 根据指定的路径，获取znode节点的值
    std::string GetData(const char *path);

private:
    zhandle_t *m_zhandle;   // zk的客户端句柄
};