set(SRC_LIST userservice.cc ../user.pb.cc)

add_executable(provider ${SRC_LIST})

# 需要链接的库： 我们实现的mprpc动态库、protobuf库
target_link_libraries(provider mprpc protobuf)