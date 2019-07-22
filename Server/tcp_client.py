# -*- coding=utf-8
# 模拟网关节点端
import socket
import time
import random

# TCP_SERVER_IP, TCP_SERVER_PORT = "39.105.70.105", 8000
TCP_SERVER_IP, TCP_SERVER_PORT = "localhost", 8000
TCP_SERVER_SOCKET = (TCP_SERVER_IP, TCP_SERVER_PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(TCP_SERVER_SOCKET)
print("TCP server connected")
while True:
	try:
		buff = s.recv(128).decode('utf-8')
		if buff:
			print("received:", buff.encode('utf-8'))
		if buff == "heartbeat\r\n":
			s.send("received".encode('utf-8'))
		elif buff.startswith("temprature"):
			s.send("temprature{}".format(random.randint(0, 40)).encode('utf-8'))
		elif buff.startswith("humidity"):
			s.send("humidity{}".format(random.randint(0, 40)).encode('utf-8'))
		elif buff == "light-on\r\n":
			s.send("OK".encode('utf-8'))

	except ConnectionResetError:
		print("Connection Reset")
		s.close()
	time.sleep(2)