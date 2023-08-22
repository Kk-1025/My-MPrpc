#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include <iostream>
#include <unistd.h>
#include <string>


// mprpc框架的基础类，负责框架的一些初始化操作
// 单类，包含整个框架所共享的信息，包括配置、日志信息，供其他类使用并获取共享信息
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);    // 框架的初始化操作
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;     // 删除拷贝构造相关的函数
    MprpcApplication(MprpcApplication&&) = delete;
};