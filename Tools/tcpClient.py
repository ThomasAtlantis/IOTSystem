import socket 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))  
s.send("micropython".encode('utf-8'))
s.close()
