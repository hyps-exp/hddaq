/*
 *
 *
 */

#include <string.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "kol/koltcp.h"
#include "kol/kolthread.h"
#include "Message/Message.h"

//using namespace kekonline;

//const int g_MESSAGE_MAGIC = 0x4d455347;

class RThread : public kol::Thread
{

public:
  RThread(kol::TcpSocket* ptcp) {m_ptcp = ptcp;};
protected:
  int run()
  {
    try
      {
	std::cout << "Server: start reading message..."<< std::endl;
	while(1)
	  {     
	    ReadMessage(m_ptcp);
	    std::cout<<"% "<<buf<<std::endl;
	  }
      }
    catch(...)
      {
	std::cout << "###Server: receive thread failed to read"<< std::endl;;
      }
    std::cout << "###Server: receive thread closed socket"<< std::endl;
    m_ptcp->close();
      
    
	return 0;
      };
protected:
  kol::TcpSocket* m_ptcp;
  char buf[4096];
  int ReadMessage(kol::TcpSocket* );
  struct msg_fmt hdr;
  int msg_size;
};


int RThread::ReadMessage(kol::TcpSocket* mestcp)
{
  //initialize
  msg_size=0;
  memset(&hdr,0,sizeof(struct msg_fmt));
  memset(buf,0,sizeof(buf));
 
  //read message
  mestcp->read((char*)&hdr, sizeof(struct msg_fmt));
  if(mestcp->gcount()==0){
    throw kol::SocketException("ReadMessage: gcount() = 0");
  }else if(mestcp->gcount() == sizeof(struct msg_fmt)){
   
    if(hdr.header != g_MESSAGE_MAGIC){
      //throw kol::SocketException("ReadMessage: Read broken message");
      std::cout <<"cmsgd: Unknown Message header"<<std::endl;
      return -1;
    } 
    msg_size = hdr.length - sizeof(struct msg_fmt);
    mestcp->read(buf, msg_size);
    if(mestcp->gcount()!=msg_size){    
      throw kol::SocketException("ReadMessage: Failed to read all message"); 
    }
    buf[msg_size]='\0';
    
  }else{
    throw kol::SocketException("ReadMessage: Unknown Error happend");
  }
  return 1;
}



//-----------------------------------------------------------------------------------------//
class SThread : public kol::Thread
{

public:
  SThread(kol::TcpSocket* ptcp, char* moji);
  ~SThread() {std::cout<<"destructor of SThread"<<std::endl;};
protected:
  int run()
  {
    try
      {
	std::cout << "Server: start sending message..."<< std::endl;
	while(1)
	  {
	    memset(buf,str[0],10);
	    //buf[10]='\n';
	    struct msg_fmt hdr;
	    hdr.header = g_MESSAGE_MAGIC;
	    hdr.length = sizeof(struct msg_fmt)+10;
	    m_ptcp->write((char*)&hdr,sizeof(struct msg_fmt));
	    m_ptcp->write(buf,10);
	    m_ptcp->flush();
	    usleep(1000000);
	  }
      }  
    catch(...)
      {
	std::cout << "###Server: send thread failed to write"<< std::endl;;
      }
    std::cout << "###Server: send thread closed socket"<< std::endl;
    m_ptcp->close();
    return 0;
  };
protected:
  kol::TcpSocket* m_ptcp;
  
  char buf[1024];
  char *str;
};

SThread::SThread(kol::TcpSocket* ptcp, char* moji)
{
  m_ptcp=ptcp;
  str=moji;
}

int main(int argc, char* argv[])
{
  
  try
    {
    
      if(argc != 3)
	throw std::invalid_argument("See usage");
      
    
      std::istringstream ss(argv[1]);
      int port;
      ss >> port;
      
      int th_count=0;
      
      kol::TcpServer server(port);
      kol::ThreadController control;
      kol::TcpSocket sock[100];
      RThread* rthread[100];
      SThread* sthread[100];
      while(1){
	std::cout << "Server: waiting new connection...[port:"<<port<<"]"<< std::endl;
     
	sock[th_count] = server.accept();
	std::cout << "\nServer: accepted connection!! #"<<th_count<< std::endl;
	
	control.post(new  RThread(&sock[th_count]) );
	control.post(new  SThread(&sock[th_count],argv[2]));
	
	th_count++;
      }
    }
  catch(...)
    {
      std::cout << "Usage: " << argv[0] << " <port>  <char>" << std::endl;
    }
  return 0;
}

