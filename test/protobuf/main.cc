#include "test.pb.h"
#include <iostream>
#include<string>
using namespace fixbug;

int main()
{

    /*
    1.有 mutable_* 方法的字段：只有当字段的类型是嵌套的 消息类型 时（例如 ResultCode 类型字段），protobuf 才会生成 mutable_* 方法，允许修改这个字段的内容。
    2.没有 mutable_* 方法的字段：对于简单类型字段（如 int32、string 等），protobuf 会生成标准的 getter 和 setter 方法，而没有 mutable_* 方法。
    */

    LoginResponse rsp;
    //嵌套消息（message）字段的使用
    //mutable_* 方法返回的是字段的引用，可以直接对其进行修改
    ResultCode* rc = rsp.mutable_result();
    rc->set_code(200);
    rc->set_msg("OK");

    std::string str1;
    if(rsp.SerializeToString(&str1))
    {
        std::cout<<str1<<std::endl;
    }

    GetFriendListsResponse rsp2;
    ResultCode* rc2= rsp2.mutable_result();
    rc2->set_code(404);
    rc2->set_msg("Not Found");

    //列表字段的使用
    //第一个好友
    User *user1=rsp2.add_friend_list();
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);
    //第二个好友
    User *user2=rsp2.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(22);
    user2->set_sex(User::MAN);
    //输出好友列表的大小
    std::cout<<rsp2.friend_list_size()<<std::endl;

    User user=rsp2.friend_list(0);//获取第一个好友的副本
    User* userptr=rsp2.mutable_friend_list(0);//获取第一个好友的指针，可以直接修改
    userptr->set_name("wangwu");

    std::string str2;
    if(rsp2.SerializeToString(&str2))
    {
        std::cout<<str2<<std::endl;
    }

    return 0;
}