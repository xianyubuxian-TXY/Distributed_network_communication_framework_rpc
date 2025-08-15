# 项目介绍
通过muduo网络库，protobuf序列化工具，zookeeper的"服务注册中心"，实现的rpc分布式网络通信框架

# 项目的使用
(1)项目可通过autobuild.sh进行一键编译，编译后会在lib目录下生成相应的静态库与涉及的所有头文件
(2)将lib文件夹下的静态库放到主机的/usr/lib或/usr/local/lib, lib文件夹下的所有头文件放到/usr/include或/usr/local/include下
(3)项目的使用样例在example目录下


# 项目代码工程目录
bin：可执行文件
build：项目编译文件
lib：项目库文件
src：源文件
test：测试代码
example：框架代码的使用例子
CMakeLists.txt：cmake文件
autobuild.sh：一键编译脚本

# 项目依赖环境
1.muduo网络库
2.protobuf
3.zookeeper