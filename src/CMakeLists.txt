# 当前目录的所有源文件 添加到SRC_LIST
# 法1：自动将当前目录下所有源文件设为SRC_LIST参与编译，缺点是如果编译过后 CMakeLists.txt不改变、将不再编译
# 可以通过删除build下所有文件解决
#aux_source_directory(. SRC_LIST)

# 法2：将需要编译的文件逐项列出来，新加源文件时添加进来，缺点是工作量大
set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    mprpcheader.pb.cc 
    mprpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc)

# 编译成库，因为后面用到的muduo库暂时只有静态库，所以这里也只编译成静态库
add_library(mprpc ${SRC_LIST})

# 设置 依赖的库，使用了muduo/pthread/zookeeper
# 注意库与库之间有依赖关系，muduo_net是核心网络库，其他模块依赖muduo_net
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)