#include "security_udp_session.h"
#include<chrono>
// return microseconds
uint64_t getCurTime()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

namespace network {

security_udp::security_udp(int local_port)
    : io_context_(),
      sock_(io_context_, udp::endpoint(udp::v4(), local_port)),
      local_port_(local_port),
      thread_(nullptr) {

      }

security_udp::~security_udp() {
  io_context_.stop();
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
}

void security_udp::start() {
  do_receive();
  thread_.reset(new std::thread([this] { io_context_.run(); }));
}

void security_udp::send(char* data, size_t len, const char* ip, int port) {
  auto channel_name = generateSessionName(ip, port);
  auto it = send_session_list_.find(channel_name);

  if (it == send_session_list_.end()) {
    udp_endpoint remoteEndpoint(asio::ip::make_address(ip), port);
    Session_info sessionInfo;
    sessionInfo.local_endpoint = sock_.local_endpoint();
    sessionInfo.remote_endpoint = remoteEndpoint;
    sessionInfo.session_name = channel_name;
    sessionInfo.sock = &sock_;

    // callback set
    sessionInfo.send_callback_sendEndpoint=std::bind(&security_udp::do_send,this,
    std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);

    send_session_list_[channel_name] =
        std::make_shared<udp_conn_send>(sessionInfo);
  }
  
  send_session_list_[channel_name]->onMsgSend(data, len);
}



void security_udp::do_receive() {
  sock_.async_receive_from(
      asio::buffer(data_, max_length), sender_endpoint_,
      [this](std::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd >= sizeof(MsgFrame)) {
          MsgFrame* header = reinterpret_cast<MsgFrame*>(data_);
          auto channel_name = generateSessionName(
              sender_endpoint_.address().to_string().c_str(),
              sender_endpoint_.port());
            
          if (header->Flag == (uint8_t)MsgType::MSG_ack) {
            auto it=send_session_list_.find(channel_name);
            
            if(it!=send_session_list_.end())
            {
              it->second->onMsgACk(header->Ack);
            }
          } 
          else {

            auto it = receive_session_list_.find(channel_name);

            if (it == receive_session_list_.end()) {
              Session_info sessionInfo;
              sessionInfo.local_endpoint = sock_.local_endpoint();
              sessionInfo.remote_endpoint = sender_endpoint_;
              sessionInfo.session_name = channel_name;
              sessionInfo.sock = &sock_;

              // todo set callback
              sessionInfo.msg_handle_callback = onMsgReceiveFunc_;

              sessionInfo.send_callback_recEndpoint =
                  std::bind(&security_udp::send, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3,
                            std::placeholders::_4);
              sessionInfo.send_callback_sendEndpoint =
                  std::bind(&security_udp::do_send, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3);

              receive_session_list_[channel_name] =
                  std::make_shared<udp_conn_recceive>(sessionInfo);
            }
            receive_session_list_[channel_name]->onMsgReceive(data_,
                                                              bytes_recvd);
          }
        }
        do_receive();
      });
}

void security_udp::do_send(BUFFERPTR data, size_t len,udp_endpoint remoteEndipoint) {
    sock_.async_send_to(
      asio::buffer(*data, len), remoteEndipoint,
      [this, data](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
        //   do_receive();
      });

    
}


std::string security_udp::generateSessionName(const char* ip, int port) {
  char buf[32];
  sprintf(buf, "%s:%d", ip, port);
  return std::string(buf);
}

udp_conn::udp_conn() {}

udp_conn::udp_conn(Session_info session, Conn_type conn_type)
    : session_(session), conn_type_(conn_type) {}

udp_conn::~udp_conn() {}



udp_conn_recceive::udp_conn_recceive() {}

udp_conn_recceive::udp_conn_recceive(Session_info session)
    : udp_conn(session, Conn_type::Receive_conn) {
      expect_suquence_id_=1;
    }

udp_conn_recceive::~udp_conn_recceive() {}

void udp_conn_recceive::onMsgReceive(const char* data, size_t receive_bytes) {
  MsgFrame* header=reinterpret_cast<MsgFrame*>(const_cast<char*>(data));
  
  size_t datalen=receive_bytes-sizeof(MsgFrame);
  if(datalen!=header->MsgLength)
  {
    //err occur;
    return;
  }

  if(header->Flag==(uint8_t)MsgType::MSG_send)
  {
    send_ack(header->SequenceNumber);
  }
  
  auto msg=std::make_shared<std::vector<char>>(datalen);
  std::memcpy(msg->data(),data+sizeof(MsgFrame),datalen);
  if(header->SequenceNumber==expect_suquence_id_)
  {
    if(session_.msg_handle_callback)
    {
      auto self=shared_from_this();
      session_.msg_handle_callback(self,msg,datalen);
    }
    expect_suquence_id_+=1;
  }

}

void udp_conn_recceive::sendMsg(char* data, size_t msgLen) {

  if(session_.send_callback_recEndpoint)
  {
    session_.send_callback_recEndpoint(data, msgLen,
      session_.remote_endpoint.address().to_string().c_str(),session_.remote_endpoint.port());
  }

}

void udp_conn_recceive::send_ack(uint32_t ack) {
  MsgFrame header;
  header.FrameHeader = FIXHEADER;
  header.Ack = ack;
  header.MsgLength = 0;
  header.SequenceNumber = 0;
  header.Flag = (uint8_t)MsgType::MSG_ack;
  uint32_t totalSize=sizeof(header);
  auto ACKFrame=std::make_shared<std::vector<char>>(totalSize);
  std::memcpy(ACKFrame->data(),&header,sizeof(MsgFrame));
  
  if(session_.send_callback_sendEndpoint)
  {
    session_.send_callback_sendEndpoint(ACKFrame,totalSize,session_.remote_endpoint);
  }
  std::cout<<"send ack:"<<ack<<"\n";
}

udp_conn_send::udp_conn_send() {}

udp_conn_send::udp_conn_send(Session_info session)
    : udp_conn(session, Conn_type::Send_conn) {}

udp_conn_send::~udp_conn_send() {}

void udp_conn_send::onMsgSend(char* data, size_t msg_len) {
  if (data == nullptr || msg_len == 0 ||
      conn_type_ == Conn_type::Receive_conn) {
    return;
  }

  auto header = std::move(generateHeader(data, msg_len));
  size_t total_size = msg_len + sizeof(header);
  BUFFERPTR completeData = std::make_shared<std::vector<char>>(total_size);
  std::memcpy(completeData->data(), &header, sizeof(header));
  std::memcpy(completeData->data() + sizeof(header), data, msg_len);
  auto cahes = std::make_shared<MsgCahes>();
  cahes->data = completeData;
  cahes->MsgLength = total_size;
  cahes->SequenceNumber = header.SequenceNumber;
  cahes->sendTimeTamp = getCurTime();
  cahes->retry_cnt = 0;
  cahes->timer.reset(new asio::steady_timer(session_.sock->get_executor()));
  cahes->timer->expires_after(std::chrono::microseconds(retry_interval_time_));
  send_msg_cahes_[header.SequenceNumber] = cahes;

  auto weak_conn = std::weak_ptr<MsgCahes>(cahes);
  cahes->timer->async_wait([this, weak_conn](const asio::error_code& ec) {
    if (!ec) {
      onSendTimeout(weak_conn);
    }
  });

  if (session_.send_callback_sendEndpoint) {
    session_.send_callback_sendEndpoint(completeData, total_size,
                                        session_.remote_endpoint);
  }
}

void udp_conn_send::onMsgACk(uint32_t ack) {

  auto it=send_msg_cahes_.find(ack);
  std::cout<<"receive ack: "<<ack<<"\n";
  if(it!=send_msg_cahes_.end())
  {
    auto cahes=it->second;
    auto curtime=getCurTime();
    auto msg_time_diff=curtime-cahes->sendTimeTamp;
    retry_interval_time_=msg_time_diff/2;
    // retry_interval_time_=(retry_interval_time_*(ack-1)/ack)+msg_time_diff/ack/2;
    std::cout<<"avarage time:"<<retry_interval_time_<<" "<< msg_time_diff<<"\n";
    send_msg_cahes_.erase(ack);
    
  }

}

void udp_conn_send::sendMsg(char* data, size_t msgLen) {
  // null
  //  no implement
}

uint64_t udp_conn_send::generateMsgSuquence() { return ++sendSuquenceId_; }

MsgFrame udp_conn_send::generateHeader(char* data, size_t msg_len) {
  size_t id = generateMsgSuquence();
  MsgFrame header;
  header.FrameHeader = FIXHEADER;
  header.Ack = 0;
  header.MsgLength = msg_len;
  header.SequenceNumber = id;
  header.Flag = (uint8_t)MsgType::MSG_send;
  return header;
}

void udp_conn_send::onSendTimeout(std::weak_ptr<MsgCahes> cahes) {
  auto conn = cahes.lock();
  if (!conn) return;

  if (session_.send_callback_sendEndpoint) {
    session_.send_callback_sendEndpoint(conn->data, conn->MsgLength,
                                        session_.remote_endpoint);
  }
  
  conn->retry_cnt += 1;
  std::cout<<"sequence: "<< conn->SequenceNumber<<" retry: "<<conn->retry_cnt<<" interval:"<<retry_interval_time_<<"\n";
  if (conn->retry_cnt >= MAX_RETRY_TIMES) {
    send_msg_cahes_.erase(conn->SequenceNumber);
    return;
  }

  conn->timer->expires_after(std::chrono::microseconds(retry_interval_time_*conn->retry_cnt));
  conn->timer->async_wait([this, cahes](const asio::error_code& ec) {
    if (!ec) {
      onSendTimeout(cahes);
    }
  });
}

void test() {
  // std::cout<<"sddssd\n";
}

}  // end namespace network
