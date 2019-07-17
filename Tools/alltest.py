import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("1.2.3.4", 80))
print(s.getsockname()[0])
s.close()