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
#include<functional>
#include"def.h"

namespace network
{

  void test();
//class struct declare
class udp_conn;
class udp_conn_send;
class udp_conn_recceive;

struct Session_info;


// type define
using udp = asio::ip::udp;
using io_context = asio::io_context;
using udp_socket = udp::socket;
using udp_endpoint = udp::endpoint;

using BUFFERPTR=std::shared_ptr<std::vector<char>>;

using SEND_CALLBACK_RECENDPOINT=std::function<void(char*, size_t,const char*,int)>;
using SEND_CALLBACK_SENDENDPOINT=std::function<void(BUFFERPTR,size_t,udp_endpoint )>;
using MSG_HANDLE_FUNC=std::function<void(std::shared_ptr<udp_conn> ,BUFFERPTR,size_t)>;


//const var define

constexpr uint32_t FIXHEADER=0xcc12ff67;
constexpr int MAX_RETRY_TIMES=3;



enum class MsgType
{
  MSG_send=0x00,
  MSG_ack,
  MSG_heartBeat
};



// class struct implement

struct MsgFrame
{
    uint32_t FrameHeader;
    uint32_t SequenceNumber;
    uint32_t Ack;
    uint32_t MsgLength;
    uint8_t Flag;
    uint8_t Reserved[3];
};

struct MsgCahes
{
  uint32_t SequenceNumber;
  uint32_t MsgLength;
  uint64_t sendTimeTamp;
  BUFFERPTR data;
  int retry_cnt;
  std::unique_ptr<asio::steady_timer> timer;
  ~MsgCahes(){
    std::cout<<"SequenceNumber"<<SequenceNumber<<" destroy\n";

  }
};

struct Session_info
{
  std::string session_name;
  udp_endpoint local_endpoint;
  udp_endpoint remote_endpoint;
  udp_socket* sock;
  SEND_CALLBACK_RECENDPOINT send_callback_recEndpoint;   //wrap send, same with user
  SEND_CALLBACK_SENDENDPOINT send_callback_sendEndpoint; //raw asio send
  MSG_HANDLE_FUNC msg_handle_callback;
  
};

/**
 * @brief  a reliable udp warp base to asio
 * 
 */
class security_udp {
 public:
    
   
  explicit security_udp(int local_port);
  ~security_udp();
  void start();
  void setMsgHandleFunc(MSG_HANDLE_FUNC msg_handle_callback) {onMsgReceiveFunc_=msg_handle_callback;}

  void send(char* data,size_t len,const char* ip,int port);
  

 private:
    void do_receive();
    void do_send(BUFFERPTR data,size_t len,udp_endpoint remoteEndipoint);
    std::string generateSessionName(const char* ip,int port);
    
    enum{max_length=100};
    io_context io_context_;
    udp_endpoint sender_endpoint_;
    udp_socket sock_;
    int local_port_;
    std::unique_ptr<std::thread> thread_;
    char data_[max_length];

    MSG_HANDLE_FUNC onMsgReceiveFunc_;

    std::map<std::string,std::shared_ptr<udp_conn_recceive>> receive_session_list_;
    std::map<std::string,std::shared_ptr<udp_conn_send>> send_session_list_;
};

class udp_conn: public std::enable_shared_from_this<udp_conn>
{
 public:
  enum class Conn_type
  {
    Receive_conn,
    Send_conn
  };
  udp_conn();
  udp_conn(Session_info session,Conn_type conn_type=Conn_type::Receive_conn);
  ~udp_conn();
  virtual void sendMsg(char* data,size_t msgLen)=0;

protected:
  
  Session_info session_;
  Conn_type conn_type_;
};

class udp_conn_recceive:public udp_conn
{
public:
    udp_conn_recceive();
    udp_conn_recceive(Session_info session);
    ~udp_conn_recceive();
    void onMsgReceive(const char* data, size_t receive_bytes);
    virtual void sendMsg(char* data,size_t msgLen);

private:
   uint32_t expect_suquence_id_;
   void send_ack(uint32_t ack);
   
};

class udp_conn_send:public udp_conn
{
public:
   udp_conn_send();
   udp_conn_send(Session_info session);
  ~udp_conn_send();

  void onMsgSend(char* data, size_t msg_len);
  void onMsgACk(uint32_t ack);
  virtual void sendMsg(char* data,size_t msgLen);
   

private:
  uint64_t sendSuquenceId_;
  std::map<uint32_t,std::shared_ptr<MsgCahes>> send_msg_cahes_;

  int retry_interval_time_{1000};//microsec

  uint64_t generateMsgSuquence();
  MsgFrame generateHeader(char* data, size_t msg_len);
  void onSendTimeout(std::weak_ptr<MsgCahes> cahes);
};

}//end namespace network

#endif