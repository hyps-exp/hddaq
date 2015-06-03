import socket

def tcp_send(ip_address, port, line):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1)
    try:
        sock.connect((ip_address, port))
    except:
        print 'connection faild to '+str(ip_address)+':'+str(port)
        return
   
    sock.send(line)
    sock.close()

def tcp_send_and_read(ip_address, port, line):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1)
    try:
        sock.connect((ip_address, port))
    except:
        print 'connection faild to '+str(ip_address)+':'+str(port)
        return
    
    sock.send(line)
    data = sock.recv(4096)
    sock.close()
    return data

if __name__ == '__main__':
    
    host = '127.0.0.1'
    port = 22222

    tcp_send(host, port, 'aaaa\n')