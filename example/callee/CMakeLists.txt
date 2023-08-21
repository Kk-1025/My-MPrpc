# set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})            # 生成可执行文件 provider
target_link_libraries(provider mprpc protobuf)  # 连接动态库 mprpc、protobuf