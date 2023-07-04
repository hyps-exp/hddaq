import os

import SocketTools

g_host = 'localhost'
g_port = 24

#______________________________________________________________________________
def set_host(host):
  global g_host
  g_host = host

#______________________________________________________________________________
def set_port(port):
  global g_port
  g_port = port

#______________________________________________________________________________
def trig_on():
  command = chr(0x0)+chr(0x0)+chr(0x0)+chr(0x82)
  # command = chr(0x1)+chr(0x0)+chr(0x0)+chr(0x82)
  SocketTools.tcp_send(g_host, g_port, command)

#______________________________________________________________________________
def trig_off():
  command = chr(0x1)+chr(0x0)+chr(0x0)+chr(0x82)
  # command = chr(0x0)+chr(0x0)+chr(0x0)+chr(0x82)
  SocketTools.tcp_send(g_host, g_port, command)

#______________________________________________________________________________
def get_trig_state():
  command = chr(0x0)+chr(0x0)+chr(0x0)+chr(0x42)
  data = SocketTools.tcp_send_and_read(g_host, g_port, command)
  if data is not None:
    status = ord(data[0]) & 0x1
    if status == 1: return 'ON'
    else          : return 'OFF'
  else:
    return 'UNKNOWN'

#______________________________________________________________________________
def mtm_reset():
  command = chr(0x1)+chr(0x0)+chr(0x1)+chr(0x82)
  SocketTools.tcp_send(g_host, g_port, command)

#______________________________________________________________________________
def force_L2():
  command = chr(0x1)+chr(0x0)+chr(0x2)+chr(0x82)
  SocketTools.tcp_send(g_host, g_port, command)

#______________________________________________________________________________
if __name__ == '__main__':
  ret = get_trig_state()
  print(ret)
