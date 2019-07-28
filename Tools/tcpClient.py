# -*- coding=utf-8
import threading
import socket
import time

SSID = "Atlantis"
PSWD = "21396878335"
# MYIP = "192.168.43.2"
MYIP = "39.105.70.105"
PORT = 8266

# 需要格外注意的是，给网关发消息必须以\r\n结尾

buff = ""
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# connect handshake
s.connect(("192.168.4.1", 8266))
print("connecting ... ", end="", flush=True)
while not buff == "CTS\r\n":
	buff = s.recv(128).decode("utf-8")
print("done")

time.sleep(4)

# set ip handshake
while not buff == "GOT IP\r\n":
	s.send("IP{}\r\n".format(MYIP).encode('utf-8'))
	time.sleep(0.5)
	buff = s.recv(128).decode("utf-8")

time.sleep(2)

# set port handshake
while not buff == "GOT PORT\r\n":
	s.send("PORT{}\r\n".format(PORT).encode('utf-8'))
	time.sleep(0.5)
	buff = s.recv(128).decode("utf-8")

time.sleep(2)

# set ssid handshake
while not buff == "GOT SSID\r\n":
	s.send("SSID{}\r\n".format(SSID).encode('utf-8'))
	time.sleep(0.5)
	buff = s.recv(128).decode("utf-8")

time.sleep(2)

# set pswd handshake
while not buff == "GOT PSWD\r\n":
	s.send("PSWD{}\r\n".format(PSWD).encode('utf-8'))
	time.sleep(0.5)
	buff = s.recv(128).decode("utf-8")

time.sleep(3)

s.send("OK\r\n".encode('utf-8'))
time.sleep(0.1)
s.send("OK\r\n".encode('utf-8'))
time.sleep(0.1)
s.send("OK\r\n".encode('utf-8'))
time.sleep(0.5)

buff = s.recv(128).decode("utf-8")

s.close()
