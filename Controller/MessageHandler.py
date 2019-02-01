# -*- coding: utf-8 -*-

import socket
import sys
import struct
import Queue
import time
import threading
import Message

#_______________________________________________________________________________
class MessageHandler():
  #_____________________________________________________________________________
  def __init__(self, ip_address, port):
    self.ip_address = ip_address
    self.port = port
    self.sock_status = False
    self.reader_q = Queue.Queue()
    reader_thread = threading.Thread(target=self.__read_message)
    reader_thread.setDaemon(True)
    reader_thread.start()
  #_____________________________________________________________________________
  def connect(self):
    try:
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((self.ip_address, self.port))
      self.sock_status = True
      print('connect')
    except:
      self.sock.close()
      print('MessageHandler: connection faild to {0} : {1}'
            .format(self.ip_address, self.port))
      #sys.exit()
  #_____________________________________________________________________________
  def send_message(self, line):
    if self.sock_status:
      seq_num = 1
      src_id  = 1
      dst_id  = 0
      ms_type = Message.MT_CONTROL
      header = struct.pack('IIIIII',
                           Message.magic,
                           Message.header_size + len(line),
                           src_id,
                           dst_id,
                           seq_num,
                           ms_type)
      self.sock.send(header+line)
  #_____________________________________________________________________________
  def get_message(self):
    linebuf = []
    while not self.reader_q.empty():
      linebuf.append( self.reader_q.get() )
    return linebuf
  #_____________________________________________________________________________
  def __read_message(self):
    while True:
      if self.sock_status == True:
        try:
          header = self.sock.recv(Message.header_size)
          if len(header) == Message.header_size :
            magic, length, src_id, dst_id, seq_num, ms_type \
                = struct.unpack('IIIIII', header)
            body_len = length - Message.header_size
            if magic == Message.magic and body_len > 0 :
              body = self.sock.recv(body_len)
              now  = time.time()
              self.reader_q.put((now, ms_type, src_id, body))
          elif len(header) == 0 :
            self.sock.close()
            self.sock_status = False
        except:
          self.sock.close()
          self.sock_status = False
      else :
        time.sleep(1)
        self.connect()

#_______________________________________________________________________________
if __name__ == '__main__':
  ip_address = '127.0.0.1'
  port = 8882
  app = MessageHandler(ip_address, port)
  time.sleep(1)
  aaa = app.get_message()
  print(aaa)
