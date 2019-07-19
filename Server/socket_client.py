# -*- coding=utf-8
import socket
import time

TCP_SERVER_IP, TCP_SERVER_PORT = "localhost", 1923
TCP_SERVER_SOCKET = (TCP_SERVER_IP, TCP_SERVER_PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(TCP_SERVER_SOCKET)
while True:
	try:
		s.send("heart beat".encode("utf-8"))
		buff = s.recv(128)
		print(buff)
	except ConnectionResetError:
		print("Connection Reset")
		s.close()
	time.sleep(2)