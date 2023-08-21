#pragma once
#include <google/protobuf/service.h>
#include <string>


class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    // 重置状态、错误信息
    void Reset();
    // 获取状态信息
    bool Failed() const;
    // 获取错误信息
    std::string ErrorText() const;
    // 设置错误信息
    void SetFailed(const std::string& reason);

    // 当前没有具体实现的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;          // rpc方法执行过程中的状态
    std::string m_errText;  // rpc方法执行过程中的错误信息
};