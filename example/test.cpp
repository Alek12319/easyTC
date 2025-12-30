#include"src/network/security_udp_session.h"
#include<iostream>
using namespace network;
int main()
{
security_udp clc(6060);
clc.setMsgHandleFunc([&](std::shared_ptr<udp_conn> conn,BUFFERPTR vec,size_t len)
{
    std::cout<<"receive: ";
    std::cout.write(vec->data(),len);
    std::cout<<"\n";
    if(conn)
    {
        conn->sendMsg(vec->data(),len);

    }
    
    // clc.send(vec->data(),len,"127.0.0.1",6060);

});
clc.start();
clc.send("sddssd 1",9,"127.0.0.1",6060);
// clc.send("sddssd 2",9,"127.0.0.1",6060);
// clc.send("sddssd 3",9,"127.0.0.1",6060);

  
test();
int cnt=0;
while(1)
{

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    char a[32];
    memset(a,0,sizeof(a));
    cnt+=1;
    sprintf(a, "test msg:%d", cnt);
    // std::cout<<a<<"\n";
    clc.send(a,sizeof(a),"127.0.0.1",6060);
}
// std::this_thread::sleep_for(std::chrono::milliseconds(500));
return 0;


}