import SocketTools

host = '192.168.10.3'
port = 24

def trig_on():
    command = chr(0x1)+chr(0x0)+chr(0x0)+chr(0x82)
    SocketTools.tcp_send(host, port, command)
    
def trig_off():
    command = chr(0x0)+chr(0x0)+chr(0x0)+chr(0x82)
    SocketTools.tcp_send(host, port, command)

def get_trig_state():
    command = chr(0x0)+chr(0x0)+chr(0x0)+chr(0x42)
    data = SocketTools.tcp_send_and_read(host, port, command)
    if(data is not None):
      status = ord(data[0]) & 0x1
      if status == 1 : return 'ON'
      else           : return 'OFF'
    else:
      return 'UNKNOWN'

def mtm_reset():
    command = chr(0x1)+chr(0x0)+chr(0x1)+chr(0x82)
    SocketTools.tcp_send(host, port, command)

if __name__ == '__main__':
    ret = get_trig_state()
    print ret
