#include"security_udp_session.h"


namespace network
{

security_udp::security_udp(int local_port) 
:io_context_(),
local_port_(local_port),
sock_(io_context_, udp::endpoint(udp::v4(), local_port)),
thread_(nullptr)
{
    do_receive();
    thread_.reset(new std::thread([this]{
    io_context_.run();
    }));
}

security_udp::~security_udp() 
{
    if(thread_->joinable())
    {
        thread_->join();
    }
    session_list_.clear();
}

void security_udp::send(BUFFERPTR data, size_t len, const char* ip, int port) 
{

}

void security_udp::do_receive() 
{
    sock_.async_receive_from(asio::buffer(data_, max_length), sender_endpoint_,
        [this](std::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            //do_send(bytes_recvd);
            auto channel_name=generateSession(sender_endpoint_.address().to_string().c_str(),sender_endpoint_.port());
            auto it=session_list_.find(channel_name);
            if(it==session_list_.end())
            {
                security_udp_session::SessionInfo channel_info(sock_.local_endpoint(),sender_endpoint_);
                session_list_[channel_name]=std::make_shared<security_udp_session>(channel_info);
            }
            

          }
          do_receive();

        });
}

void security_udp::do_send(BUFFERPTR data, size_t len) 
{
        sock_.async_send_to(
        asio::buffer(*data, len), sender_endpoint_,
        [this,data](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
        //   do_receive();
        });
}

std::string security_udp::generateSession(const char* ip, int port) 
{
  char buf[32];
  sprintf(buf,"%s:%d",ip,port);
  return std::string(buf);
}

security_udp_session::security_udp_session(SessionInfo info) :
sessionInfo_(info)
{

}

security_udp_session::~security_udp_session() {}

}  // end namespace network
