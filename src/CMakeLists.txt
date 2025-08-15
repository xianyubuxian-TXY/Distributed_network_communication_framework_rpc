# 定义宏 THREADED：为了是zookeeper的zoo_exist函数生效
add_definitions(-DTHREADED)

#aux_source_directory(. SRC_LIST)
set(SRC_LIST mprpcapplication.cc mprpcconfig.cc rpcheader.pb.cc mprpcprovider.cc mprpcchannel.cc mprpccontroller.cc logger.cc zookeeperutil.cc)

# 编译成一个动态库
add_library(mprpc ${SRC_LIST})

# 添加库依赖
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt) #zookeeper_mt:多线程版本  zookeeper_st:多线程版本