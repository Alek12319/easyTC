#ifndef _security_udp_H
#define _security_udp_H

#pragma once

#include<asio.hpp>
#include<thread>
#include<cstdlib>
#include<vector>
#include<memory>
#include<string>
#include<map>

namespace network
{


class security_udp_session;

struct MsgFrame
{
    uint16_t FrameHeader;
    uint16_t MsgType;
    uint64_t SequenceNumber;
    uint32_t MsgLength;
    std::shared_ptr<std::vector<char>> Data;
};

/**
 * @brief  a reliable udp warp base to asio
 * 
 */
class security_udp {
    using udp=asio::ip::udp;
    using io_context=asio::io_context;
    using udp_socket=udp::socket;
    using udp_endpoint=udp::endpoint;
 public:
    using BUFFERPTR=std::shared_ptr<std::vector<char>>;
   
  explicit security_udp(int local_port);
  ~security_udp();

  void send(BUFFERPTR data,size_t len,const char* ip,int port);

 private:
    void do_receive();
    void do_send(BUFFERPTR data,size_t len);
    std::string generateSession(const char* ip,int port);
    
    enum{max_length=1024*1024};
    io_context io_context_;
    udp_endpoint sender_endpoint_;
    udp_socket sock_;
    int local_port_;
    std::unique_ptr<std::thread> thread_;
    char data_[max_length];
    std::map<std::string,std::shared_ptr<security_udp_session>> session_list_;
};

class security_udp_session {
  using udp = asio::ip::udp;
  using io_context = asio::io_context;
  using udp_socket = udp::socket;
  using udp_endpoint = udp::endpoint;

 public:
  struct SessionInfo {
    SessionInfo(const udp_endpoint& locl,const udp_endpoint& reme)
    :local_endpoint_(),remote_endpoint_(reme)
    {
    }
    udp_endpoint local_endpoint_;
    udp_endpoint remote_endpoint_;
  };

  explicit security_udp_session(SessionInfo info);
  ~security_udp_session();


 private:
 SessionInfo sessionInfo_;
};

}//end namespace network

#endif