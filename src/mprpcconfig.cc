#include "mprpcconfig.h"


void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 以 r 只读方式打开配置文件
    FILE *pF = fopen(config_file, "r");
    if (nullptr == pF)
    {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 要处理的问题：1.注释项   2.正确的配置项 =    3.去掉多余的空格 
    while(!feof(pF))    // 只要还没读到文件末尾
    {
        // 从文件中读取一行数据
        char buf[512] = {0};
        fgets(buf, 512, pF);

        // 去掉字符串前面多余的空格（后面去不了，因为最后一个字符是\n）
        std::string read_buf(buf);
        Trim(read_buf);

        // 忽略 空行、#开头的注释
        if (read_buf.empty() || read_buf[0] == '#')
        {
            continue;
        }

        // 解析配置项   rpcserverip=127.0.0.1\n
        int index = read_buf.find('=');
        if (index == -1)      // 配置项不合法
        {
            continue;
        }

        // 提取key、value 并去除多余空格
        std::string key;
        std::string value;
        key = read_buf.substr(0, index);
        Trim(key);
        
        int endIndex = read_buf.find('\n', index);
        if (endIndex == -1)      // 配置项过长，导致读取不到\n，不合法
        {
            continue;
        }
        value = read_buf.substr(index + 1, endIndex - index - 1);
        Trim(value);

        // 保存配置项信息
        m_configMap.insert({key, value});
    }

    fclose(pF);
}


std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}


void MprpcConfig::Trim(std::string &src_buf)
{
    // 去掉字符串前面多余的空格
    int index = src_buf.find_first_not_of(' ');
    if (index != -1)    // 如果字符串前面有空格
    {
        src_buf = src_buf.substr(index, src_buf.size() - index);
    }

    // 去掉字符串后面多余的空格
    index = src_buf.find_last_not_of(' ');
    if (index != -1)    // 如果字符串后面有空格
    {
        src_buf = src_buf.substr(0, index + 1);
    }
}