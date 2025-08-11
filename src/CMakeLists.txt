aux_source_directory(. SRC_LIST)

# 编译成一个动态库
add_library(mprpc ${SRC_LIST})

# 添加库依赖
target_link_libraries(mprpc muduo_net muduo_base pthread)