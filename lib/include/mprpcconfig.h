#pragma once
#include <iostream>
#include <string>
#include <unordered_map>


// rpc框架读取配置文件类
// 配置文件保存以下信息：rpcserverip   rpcserverport    zookeeperip   zookeeperport
class MprpcConfig
{
public:
    // 解析加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查询配置项信息
    std::string Load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> m_configMap;   // 保存配置项信息
    // 去除字符串前后的空格
    void Trim(std::string &src_buf);
};