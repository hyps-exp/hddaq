// Msg.cc
//#include "Message/MessageClient.h"
//#include "Message/MessageSocket.h"
#include <iostream>
#include "Message/Message.h"

//int Message::m_header = 0x20060815;
//int Message::m_src_id = 200;

Message::Message (const std::string * message, int dst_id, int type)
  : m_fmt(),
    m_message(*message)
{
    m_fmt.header = g_MESSAGE_MAGIC;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
    m_fmt.dst_id = dst_id;
    m_fmt.type = type;
    m_fmt.seq_num = 0;

}


Message::Message (const std::string& message, int dst_id, int type)
  : m_fmt(),
    m_message(message)
{
    m_fmt.header = g_MESSAGE_MAGIC;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
    m_fmt.dst_id = dst_id;
    m_fmt.type = type;
    m_fmt.seq_num = 0;

}

Message::Message (const char *message, int dst_id, int type)
  : m_fmt(),
    m_message(message)
{
    m_fmt.header = g_MESSAGE_MAGIC;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
    m_fmt.dst_id = dst_id;
    m_fmt.type = type;
    m_fmt.seq_num = 0;
}

Message::Message ()
{
}

Message::~Message ()
{
}

void
Message::setHeader (int header)
{
    m_fmt.header = header;
}

void
Message::setSrcId (int src_id)
{
    m_fmt.src_id = src_id;
}

void
Message::setDstId (int dst_id)
{
    m_fmt.dst_id = dst_id;
}

void
Message::setSeqNum (int seq_num)
{
    m_fmt.seq_num = seq_num;
}

void
Message::setType (int type)
{
    m_fmt.type = type;
}

void
Message::setMessage (const std::string * message)
{
    m_message = *message;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
}

void
Message::setMessage (const std::string& message)
{
    m_message = message;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
}

void
Message::setMessage (const char *message)
{
    std::string msg (message);
    m_message = msg;
    m_fmt.length = sizeof(msg_fmt) + m_message.size ();
}

void
Message::setAllHeader (struct msg_fmt *hdr)
{
  m_fmt = *hdr;
//     m_header = hdr->header;
//     m_length = hdr->length;
//     m_src_id = hdr->src_id;
//     m_dst_id = hdr->dst_id;
//     m_seq_num = hdr->seq_num;
//     m_type = hdr->type;
}

int
Message::getHeader ()
{
    return m_fmt.header;
}

int
Message::getLength ()
{
    return m_fmt.length;
}

int
Message::getSrcId ()
{
    return m_fmt.src_id;
}

int
Message::getDstId ()
{
    return m_fmt.dst_id;
}

int
Message::getSeqNum ()
{
    return m_fmt.seq_num;
}

int
Message::getType ()
{
    return m_fmt.type;
}

std::string
Message::getMessage ()
{
    return m_message;
}

void
Message::showAll ()
{
/*
    std::cout << std::endl;
    std::cout << "header  : " << m_header << std::endl;
    std::cout << "length  : " << m_length << std::endl;
    std::cout << "src_id  : " << m_src_id << std::endl;
    std::cout << "dst_id  : " << m_dst_id << std::endl;
    std::cout << "seq_num : " << m_seq_num << std::endl;
    std::cout << "type    : " << m_type << std::endl;
    std::cout << "-----------------------" << std::endl;
    std::cout << "message : " << m_message << std::endl;
    std::cout << std::endl;
*/
    std::cout << std::endl;
    std::cout << "header: " << std::hex << "0x" << m_fmt.header;
    std::cout << ", len: " << std::dec << m_fmt.length;
    std::cout << ", sid: " << std::dec << m_fmt.src_id;
    std::cout << ", did: " << std::dec << m_fmt.dst_id;
    std::cout << ", seq_num: " << std::dec << m_fmt.seq_num;
    std::cout << ", type: " << m_fmt.type << std::endl;
    std::cout << "message: " << m_message << std::endl;
    std::cout << std::endl;
}


int Message::getMessageNodeId(int node_type)
{
	return 0;
}

int Message::setMessageNodeId(int node_type)
{
	return 0;
}
