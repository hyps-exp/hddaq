// -*- C++ -*-
/**
 *  @file   cmsgd.cc
 *  @brief
 *  @author Kenji Hosomi <hosomi@lambda.phys.tohoku.ac.jp>
 *  @date 2007/08/10
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <list>
#include <vector>
#include <algorithm>

#include <cstring>
#include <csignal>
#include <ctime>
#include <cerrno>

#include "kol/koltcp.h"
#include "Message/Message.h"
#include "Message/MessageSocket.h"
#include "Message/MessageClient.h"
#include "Message/cmsgd.h"

typedef std::vector<std::string>::iterator ArgItr;

kol::ThreadController g_thread;
kol::Mutex            g_mutex; // global mutex (for std::cout,std::cerr)

int          g_port            = 8882;
bool         g_verbose         = false;
unsigned int g_retry_interval  = 1; // [sec]
unsigned int g_timeout         = 6; // [sec]
// if the value of timeout is very small (<=5 sec),
// calling of recvMessage() always throws an exception

const std::string k_opt_DR      = "-DR";
const std::string k_opt_UR      = "-UR";
const std::string k_opt_retry   = "--retry=";
const std::string k_opt_timeout = "--timeout=";
const std::string k_opt_port    = "--port=";
const std::string k_opt_v       = "-v";
const std::string k_opt_h       = "-h";
const std::string k_opt_help    = "-help";

const std::string k_red           = "\033[0;31m";
const std::string k_green         = "\033[0;32m";
const std::string k_light_red     = "\033[1;31m";
const std::string k_light_green   = "\033[1;32m";
const std::string k_yellow        = "\033[1;33m";
const std::string k_bg_brown      = "\033[7;33m";
const std::string k_bg_purple     = "\033[7;35m";
const std::string k_bg_cyan       = "\033[7;36m";
const std::string k_bg_white      = "\033[7;37m";
const std::string k_default_color = "\033[0m";

kol::Mutex               g_upsock_mutex; // mutex for upstream socket list
kol::Mutex               g_dnsock_mutex; // mutex for downstream socket list
std::list<MessageSocket> g_upsocklist;
std::list<MessageClient> g_dnsocklist;
std::list<msgnode_t>     g_nodelist;

//______________________________________________________________________________
const std::string k_socket_exception_gcount_0 = "gcount() = 0";

//______________________________________________________________________________
// node connection status
//______________________________________________________________________________
// good
const std::string k_node_connect   = " connect ";

// connection with msgd is not established
const std::string k_node_unconnect = "unconnect";

// msgd is running but frontend process is down
// cable link is down
const std::string k_node_timeout   = " timeout ";

//______________________________________________________________________________
void
sigpipehandler(int signum)
{
  if (g_verbose)
    std::cerr << "#E Got SIGPIPE! : sig = " << signum << std::endl;
  return;
}

//______________________________________________________________________________
int
set_signal()
{
  struct sigaction act;

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = sigpipehandler;
  act.sa_flags |= SA_RESTART;

  if (sigaction(SIGPIPE, &act, NULL) != 0 )
    {
      std::cerr << "#E sigaction(2) error!" << std::endl;
      return -1;
    }

  return 0;
}

//______________________________________________________________________________// struct msgnode_t
//______________________________________________________________________________
void
msgnode_t::show_status() const
{
  std::string color;
  if (status==k_node_connect)
    color = k_green;
  else if (status==k_node_unconnect)
    color = k_red;
  else if (status==k_node_timeout)
    color = k_yellow;

  std::cout.setf(std::ios_base::left);
  std::cout << "   connection: "
	    << std::setfill(' ') << std::setw(16) << hostname
	    << " " << std::setfill(' ') << std::setw(6) << port
	    << " [ " << color << status << k_default_color << " ]"
	    << std::endl;
  std::cout.unsetf(std::ios_base::left);
  return;
}

//______________________________________________________________________________
// class CMessageUpstreamThread
//______________________________________________________________________________
CMessageUpstreamThread::CMessageUpstreamThread(msgnode_t& node,
					       int ref)
  : kol::Thread(),
    m_node(node),
    m_sock(0),
    m_ref_flag(ref)
{
}

//______________________________________________________________________________
CMessageUpstreamThread::~CMessageUpstreamThread()
{
}

//______________________________________________________________________________
void
CMessageUpstreamThread::close()
{
  if (!m_sock)
    return;

  g_dnsock_mutex.lock();
  for (std::list<MessageClient>::iterator i=g_dnsocklist.begin();
       i!=g_dnsocklist.end();)
    {
      if (m_sock!=&(*i))
	++i;
      else
	{
	  g_mutex.lock();
	  std::cout << "#D CMessageUpstreamThread::close() "
		    << " erase dnsock "
		    << m_node.hostname
		    << std::endl;
	  g_mutex.unlock();
	  i = g_dnsocklist.erase(i);
	}
    }
  g_dnsock_mutex.unlock();
  m_sock = 0;

  if (g_verbose)
    {
      std::ostringstream oss;
      oss << "cmsgd: " << m_node.hostname << " " << m_node.port << " "
	  << k_node_unconnect;
      Message msg_unconnect;
      sendMessage(msg_unconnect, oss.str());
    }

  return;
}

//______________________________________________________________________________
bool
CMessageUpstreamThread::connect()
{
  try
    {
      if (!m_sock)
	{
	  MessageClient sock(m_node.hostname.c_str(), m_node.port);
	  g_dnsock_mutex.lock();
	  g_dnsocklist.push_back(sock);
	  m_sock = &g_dnsocklist.back();
	  g_dnsock_mutex.unlock();

	  timeval timeoutv;
	  timeoutv.tv_sec  = g_timeout;
	  timeoutv.tv_usec = 0;
	  m_sock->setsockopt(SOL_SOCKET, SO_RCVTIMEO,
			     &timeoutv, sizeof(timeoutv));
	}
    }
  catch (const std::exception& e)
    {
      if (g_verbose)
	{
	  g_mutex.lock();
	  std::cerr << k_bg_purple<< "#E CMessageUpstreamThread:"
		    << k_default_color << " error @ connect()\n"
		    << " " << m_node.hostname << " " << m_node.port << ": "
		    << "\n" << e.what() << std::endl;
	  g_mutex.unlock();
	}
      return false;
    }

  g_mutex.lock();
  m_node.status = k_node_connect;
  m_node.show_status();
  g_mutex.unlock();

  if (g_verbose)
    {
      std::ostringstream oss;
      oss << "cmsgd: " << m_node.hostname << " " << m_node.port << " "
	  << k_node_connect;
      Message msg_connect;
      sendMessage(msg_connect, oss.str());
    }
  return true;
}

//______________________________________________________________________________
bool
CMessageUpstreamThread::forwardMessage()
{
  Message msg;
  try
    {
      recvMessage(msg);
      if (m_node.status==k_node_timeout)
	return true;

      if (msg.getHeader()==g_MESSAGE_MAGIC)
	{
	  if (m_ref_flag == 1)
	    reflectMessage(msg);

	  sendMessage(msg);
	}
      else
	{
	  g_mutex.lock();
	  std::cout << "#E CMessageUpstreamThread::forwardMessage()"
		    << " bad header " << std::endl;
	  g_mutex.unlock();
	  return false;
	}
    }
  catch (const std::exception& e)
    {
      g_mutex.lock();
      std::cerr << k_bg_purple<< "#E CMessageUpstreamThread:"
		<< " msgd -> cmsgd: "
		<< k_default_color << " error @ forwardMessage()\n"
		<< " " << m_node.hostname << " " << m_node.port << ": "
		<< "\n" << e.what() << std::endl;
      g_mutex.unlock();
      return false;
    }

  return true;
}

//______________________________________________________________________________
void
CMessageUpstreamThread::recvMessage(Message& msg)
{
  try
    {
      msg  = m_sock->recvMessage();
      if (m_node.status==k_node_timeout)
	{
	  g_mutex.lock();
	  m_node.status = k_node_connect;
	  m_node.show_status();
	  g_mutex.unlock();

	  if (g_verbose)
	    {
	      std::ostringstream oss;
	      oss << "cmsgd: " << m_node.hostname << " " << m_node.port
		  << " recovered from timeout";
	      Message msg_recover;
	      sendMessage(msg_recover, oss.str());
	    }
	}
    }
  catch (const kol::SocketException& e)
    {
      std::string err_msg = e.what();
      std::ostringstream oss;
      oss << k_bg_purple << "\n#E CMessageUpstreamThread:"
	  << " msgd -> cmsgd:" << k_default_color
	  << " " << m_node.hostname << ": " << e.what();

      if (err_msg.find(k_socket_exception_gcount_0.c_str())!=std::string::npos)
	{
	  err_msg = ": connection closed by peer";
	  g_mutex.lock();
	  std::cerr << oss.str() << err_msg << std::endl;
	  g_mutex.unlock();
	}
      else if (e.reason()==EWOULDBLOCK)
	{
	  m_sock->iostate_good();
	  err_msg = ": timeout";
	  m_node.status = k_node_timeout;

	  g_mutex.lock();
	  std::cerr << oss.str() << err_msg <<  std::endl;
	  m_node.show_status();
	  g_mutex.unlock();

	  if (g_verbose)
	    {
	      oss.str("");
	      oss << "cmsgd: " << m_node.hostname << " " << m_node.port << " "
		  << k_node_timeout;
	      Message msg_timeout;
	      sendMessage(msg_timeout, oss.str());
	    }
 	  return;
	}
      throw;
    }
  return;
}

//______________________________________________________________________________
void
CMessageUpstreamThread::reflectMessage(const Message& msg)
{
  try
    {
      m_sock->sendMessage(msg);
    }
  catch(const std::exception& e)
    {
      g_mutex.lock();
      std::cerr << k_bg_purple<< "#E CMessageUpstreamThread:"
		<< " cmsgd -> msgd: "
		<< k_default_color << " error @ reflectMessage()\n"
		<< " " << m_node.hostname << " " << m_node.port << ": "
		<< "\n" << e.what() << std::endl;
      g_mutex.unlock();
    }
  return;
}

//______________________________________________________________________________
int
CMessageUpstreamThread::run()
{
  int n_retry = 0;
  try
    {
      while (true) //connection loop
	{
	  if (n_retry==0 || g_verbose)
	    {
	      g_mutex.lock();
	      std::cout << "#D trying new connection to "
			<< m_node.hostname << std::endl;
	      g_mutex.unlock();
	    }

	  if (connect())
	    {
	      n_retry = 0;
	      while (forwardMessage());
	      close();
	    }
	  else
	    {
	      m_node.status = k_node_unconnect;
	      if (n_retry==0)
		m_node.show_status();
	      ++n_retry;
	      sleep(g_retry_interval);
	    }
	}
    }
  catch (...)
    {
      g_mutex.lock();
      std::cerr << k_bg_purple << "#E CMessageUpstreamThread:"
		<< k_default_color
		<< " " << m_node.hostname << " " << m_node.port << ": "
		<< "\n unknown error occurred" << std::endl;
      g_mutex.unlock();
      close();
      throw;
    }

  close();
  g_mutex.lock();
  std::cout << "#D CMessageUpstreamThreadd::run() " << m_node.hostname
	    << ", " << m_node.port << " exit" << std::endl;
  g_mutex.unlock();
  return 0;
};

//______________________________________________________________________________
void
CMessageUpstreamThread::sendMessage(Message& msg,
				    const std::string& str)
{
  g_upsock_mutex.lock();
  for (std::list<MessageSocket>::iterator it=g_upsocklist.begin();
       it!=g_upsocklist.end();)
    {
      MessageSocket& ctrl_sock = *it;
      try
	{
	  if (!str.empty()) // change message
	    {
	      msg.setHeader(g_MESSAGE_MAGIC);
	      msg.setMessage(str);
	    }

	  ctrl_sock.sendMessage(msg);
	  ++it;
	}
      catch (const std::exception& e)
	{
	  if (g_verbose)
	    {
	      g_mutex.lock();
	      std::cerr << k_bg_purple << "#E CMessageUpstreamThread:"
			<< " cmsgd -> ctrl:" << k_default_color
			<< " " << m_node.hostname << " " << e.what()
			<< " erase a DAQ controller from list"
			<< std::endl;
	      g_mutex.unlock();
	    }
	  it = g_upsocklist.erase(it);
	}
    }
  g_upsock_mutex.unlock();
  return;
}


//______________________________________________________________________________
// class CMessageDownstreamThread
//______________________________________________________________________________
CMessageDownstreamThread::CMessageDownstreamThread(MessageSocket& daq_ctrl,
		 int ref)
  : kol::Thread(),
    m_ctrl(daq_ctrl),
    m_ref_flag(ref)
{
}

//______________________________________________________________________________
CMessageDownstreamThread::~CMessageDownstreamThread()
{
}

//______________________________________________________________________________
void
CMessageDownstreamThread::close()
{
  g_upsock_mutex.lock();
  for (std::list<MessageSocket>::iterator i=g_upsocklist.begin();
       i!=g_upsocklist.end();)
    {
      if (&m_ctrl!=&(*i))
	++i;
      else
	{
	  g_mutex.lock();
	  std::cout << "#D CMessageDownstreamThread::close() "
		    << " erase upsock" << std::endl;
	  g_mutex.unlock();
	  i = g_upsocklist.erase(i);
	}
    }
  g_upsock_mutex.unlock();
  return;
}

//______________________________________________________________________________
bool
CMessageDownstreamThread::forwardMessage()
{
  Message msg;
  try
    {
      recvMessage(msg);
      const std::string& msg_str = msg.getMessage();

      g_mutex.lock();
      std::cout << "\n#D cmsgd: received Message from DAQ controller = "
		<< k_bg_cyan << msg_str	<< k_default_color << std::endl;
      g_mutex.unlock();

      if (msg_str == "status")
	printStatus();

      if (msg.getHeader()==g_MESSAGE_MAGIC)
	{
	  if (m_ref_flag == 1)
	    reflectMessage(msg);

	  sendMessage(msg);
	}
      else
	return false;
    }
  catch (const std::exception& e)
    {
      g_mutex.lock();
      std::cerr << k_bg_brown << "#E CMessageDownstreamThread:"
		<< k_default_color << " error @ forwardMessage()\n"
		<< e.what() << std::endl;
      g_mutex.unlock();
      return false;
    }
  return true;
}

//______________________________________________________________________________
void
CMessageDownstreamThread::printStatus() const
{
  g_mutex.lock();
  int n_connect   = 0;
  int n_unconnect = 0;
  int n_timeout   = 0;

  for (std::list<msgnode_t>::iterator i=g_nodelist.begin();
       i!=g_nodelist.end(); ++i)
    {
      i->show_status();
      if (i->status==k_node_connect)
	++n_connect;
      else if (i->status==k_node_unconnect)
	++n_unconnect;
      else if (i->status==k_node_timeout)
	++n_timeout;
    }

  g_upsock_mutex.lock();
  int n_upsock = g_upsocklist.size();
  g_upsock_mutex.unlock();
  g_dnsock_mutex.lock();
  int n_dnsock = g_dnsocklist.size();
  g_dnsock_mutex.unlock();

  std::cout << "\n entries in upstream-list  : "
	    << std::setfill(' ') << std::setw(4) << n_upsock
	    << "\n entries in downstream-list: "
	    << std::setfill(' ') << std::setw(4) << n_dnsock
	    << "\n" << std::setfill(' ') << std::setw(5) << n_connect
	    << " connect"
	    << "\n" << std::setfill(' ') << std::setw(5) << n_unconnect
	    << " unconnect (closed by peer, msgd is down, etc.)"
	    << "\n" << std::setfill(' ') << std::setw(5) << n_timeout
	    << " timeout   (cable link is down, frontend-process is down, etc.)"
	    << "\n" << std::endl;
  g_mutex.unlock();
  return;
}

//______________________________________________________________________________
void
CMessageDownstreamThread::recvMessage(Message& msg)
{
  try
    {
      msg =  m_ctrl.recvMessage();
    }
  catch (const std::exception& e)
    {
      if (g_verbose)
	{
	  g_mutex.lock();
	  std::cerr << k_bg_brown
		    << "#E CMessageDownstreamThread:"
		    << " ctrl -> cmsgd: " << k_default_color
		    << " error @ recvMessage()\n" << e.what() << std::endl;
	  g_mutex.unlock();
	}
      throw;
    }
  return;
}


//______________________________________________________________________________
void
CMessageDownstreamThread::reflectMessage(const Message& msg)
{
  try
    {
      m_ctrl.sendMessage(msg);
    }
  catch (const std::exception& e)
    {
      g_mutex.lock();
      std::cerr << k_bg_brown
		<< "#E CMessageDownstreamThread:"
		<< " cmsgd -> ctrl: " << k_default_color
		<< " error @ reflectMessage()\n" << e.what() << std::endl;
      g_mutex.unlock();
    }
  return;
}

//______________________________________________________________________________
int
CMessageDownstreamThread::run()
{
  try
    {
      while (forwardMessage());
    }
  catch (...)
    {
      g_mutex.lock();
      std::cerr << k_bg_brown
		<< "#E CMessageDownstreamThread:"
		<< k_default_color << "  unknown error occurred in run()"
		<< std::endl;
      g_mutex.unlock();
      close();
      throw;
    }

  close();
  g_mutex.lock();
  std::cout << "#D downstream thread exit run()" << std::endl;
  g_mutex.unlock();

  return 0;
}

//______________________________________________________________________________
void
CMessageDownstreamThread::sendMessage(const Message& msg)
{
  if (g_verbose) {
    std::cout << "#D DOWN send" << std::endl;
  }
  g_dnsock_mutex.lock();
  for (std::list<MessageClient>::iterator it = g_dnsocklist.begin();
       it != g_dnsocklist.end();)
    {
      MessageClient& node_sock = *it;
      try
	{
	  node_sock.sendMessage(msg);
	  ++it;
	}
      catch(const std::exception& e)
	{
	  g_mutex.lock();
	  std::cerr << k_bg_brown
		    << "#E CMessageDownstreamThread:"
		    << " cmsgd -> msgd: " << k_default_color
		    << " error @ sendMessage()" << " "
		    << e.what() << std::endl;
	  std::cout << "#D erase dnsock " << std::endl;
	  g_mutex.unlock();
	  it = g_dnsocklist.erase(it);
	}
    }
  g_dnsock_mutex.unlock();
  return;
}

//______________________________________________________________________________
int main(int argc, char* argv[])
{

  set_signal();
  std::vector<std::string> argvs(argv, argv + argc);

  try
    {
      //get argv
      if (argc < 2)
 	throw std::invalid_argument("See usage");

      int URflag = 0;
      int DRflag = 0;
      std::vector<ArgItr> itrlist;

      for (ArgItr i = argvs.begin(); i!=argvs.end(); ++i)
	{
	  const std::string& val(*i);
	  if ( val == "-h" || val == "--help")
	    throw std::invalid_argument("See usage");
	  if (val==k_opt_UR)
	    {
	      std::cout << argvs[0] << ": REFLECT option to Node" << std::endl;
	      URflag = 1;
	      itrlist.push_back(i);
	    }
	  if (val==k_opt_DR)
	    {
	      std::cout << argvs[0] << ": REFLECT option to Host" << std::endl;
	      DRflag = 1;
	      itrlist.push_back(i);
	    }
	  if (val.find(k_opt_retry)!=std::string::npos)
	    {
	      std::stringstream ss(val.substr(k_opt_retry.size()));
	      ss >> g_retry_interval;
	      itrlist.push_back(i);
	    }
	  if (val.find(k_opt_timeout)!=std::string::npos)
	    {
	      std::stringstream ss(val.substr(k_opt_timeout.size()));
	      ss >> g_timeout;
	      itrlist.push_back(i);
	    }
	  if (val.find(k_opt_port)!=std::string::npos)
	    {
	      std::stringstream ss(val.substr(k_opt_port.size()));
	      ss >> g_port;
	      itrlist.push_back(i);
	    }
	  if (val==k_opt_v)
	    {
	      g_verbose = true;
	      itrlist.push_back(i);
	    }
	}

      for (std::vector<ArgItr>::iterator i=itrlist.begin();
	   i!=itrlist.end(); ++i)
	{
	  ArgItr itr = *i;
	  if (itr!=argvs.end())
	    argvs.erase(std::remove(argvs.begin(), argvs.end(), *itr));
	}

      std::cout << " retrying to connect every " << g_retry_interval
		<< " [sec]" << std::endl;
      if (g_timeout<6)
	{
	  std::cout << " timeout = " << g_timeout
		    << ": too short " << std::endl;
	  g_timeout = 6;
	}
      std::cout << " timeout = "
		<< g_timeout << " [sec]" << std::endl;


      //node list
      std::ifstream ifs(argvs.back().c_str(), std::ios::in);
      if (!ifs)
	{
	  std::cout  <<  "cmsgd: unable to open msgnode file : "
		     << argvs.back().c_str()
		     << std::endl;
	  return -1;
	}

      // read message node file
      std::string line;
      while (ifs.good())
	{
	  std::getline(ifs, line, '\n');
	  std::istringstream ss(line);
	  if ((line.find('#')== std::string::npos) && (!line.empty()))
	    {
	      std::string host;
	      int         port;
	      ss >> host >> port;
	      msgnode_t node = { host, port, k_node_unconnect };
	      g_nodelist.push_back(node);
	    }
	}
      ifs.close();

      // run upstream thread
      for (std::list<msgnode_t>::iterator i=g_nodelist.begin();
	   i!=g_nodelist.end(); ++i)
	g_thread.post(new CMessageUpstreamThread(*i, URflag));

      //run server
      std::cout << "#D server port for DAQ controller = " << g_port << std::endl;
      kol::TcpServer server(g_port);
      while (true)
	{
	  MessageSocket daq_ctrl(server.accept());

	  g_mutex.lock();
	  std::cout  << "\n" << k_bg_white
		     << " cmsgd: accept new connection from DAQ controller "
		     << k_default_color << std::endl;
	  g_mutex.unlock();

	  // run downstream thread
	  g_upsock_mutex.lock();
	  g_upsocklist.push_back(daq_ctrl);
	  g_thread.post(new CMessageDownstreamThread(g_upsocklist.back(),
						     DRflag));
	  g_upsock_mutex.unlock();
	}
    }
  catch (const std::invalid_argument& e)
    {
      const std::string msg = e.what();
      if (msg == "See usage")
	{
	std::cout << "Usage: " << argv[0]
		  << " [" << k_opt_h << "," << k_opt_help << "]"
		  << " [" << k_opt_DR << "]"
		  << " [" << k_opt_UR << "]"
		  << " [" << k_opt_retry << "sec.]"
		  << " [" << k_opt_timeout << "sec.]"
		  << " [" << k_opt_port << "integer]"
		  << " [" << k_opt_v << "]"
		  << " nodefile"
		  << std::endl;

	std::cout << "\n description\n"
		  << std::setw(12)
		  << " option :\n"

		  << std::setw(12)
		  <<k_opt_DR
		  << " : reflect message to downside"
		  << "\n"

		  << std::setw(12)
		  << k_opt_UR
		  << " : reflect message to upside"
		  << "\n"

		  << std::setw(12)
		  << k_opt_retry
		  << " : retry interval [sec] for new connection to msgd"
		  << " (default = " << g_retry_interval << ")"
		  << "\n"

		  << std::setw(12)
		  << k_opt_timeout
		  << " : timeout value [sec] for receiving from msgd"
		  << " (default = " << g_timeout << ")"
		  << "\n"

		  << std::setw(12)
		  << k_opt_port
		  << " : server port for DAQ controller "
		  << " (default = " << g_port << ")"
		  << "\n"

		  << std::setw(12)
		  << k_opt_v
		  << " : verbose output mode"
		  << "\n"

		  << std::setw(12)
		  << (k_opt_h +", "+ k_opt_help)
		  << " : print this message "
		  << std::endl;
	}
      else
	std::cerr << "#E main()\n" << msg << std::endl;
    }
  catch (const std::exception& e)
    {
      std::cerr << "#E main()\n" << e.what() << std::endl;
    }
  catch(...)
    {
      std::cerr << "#E main() \n unknown error occurred" << std::endl;
    }

  return 0;
}
