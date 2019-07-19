# -*- coding=utf-8
import socket
import time

SSID = "liuchen"
PSWD = "liuchen88"

# 在实际的小程序中还需要加入超时检测，计算重发次数，超过五次提示用户出错
# 需要格外注意的是，给网关发消息必须以\r\n结尾

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))
s.send("SSIDliuchen\r\n".encode('utf-8'))
s.close()
time.sleep(0.8)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))
s.send("PSWDliuchen88\r\n".encode('utf-8'))
s.close()
time.sleep(0.8)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))
s.send("IP192.168.1.104\r\n".encode('utf-8'))
s.close()
time.sleep(0.8)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))
s.send("PORT8000\r\n".encode('utf-8'))
s.close()
time.sleep(0.8)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.4.1", 8266))
s.send("OK\r\n".encode('utf-8'))
s.close()
# while (True):
# 	s.send("RTS\r\n".encode('utf-8'))
# 	buff = s.recv(128)
# 	if buff == "CTS\r\n":
# 		break
# 	else:
# 		time.sleep(0.8)
# while (True):
# 	s.send("SSID:{}\r\n".format(SSID).encode('utf-8'))
# 	buff = s.recv(128)
# 	if buff == "CTS SSID\r\n":
# 		break
# 	else:
# 		time.sleep(0.8)

# while (True):
# 	s.send("PSWD:{}\r\n".format(PSWD).encode('utf-8'))
# 	buff = s.recv(128)
# 	if buff == "CTS PSWD\r\n":
# 		break
# 	else:
# 		time.sleep(0.8)

# while (True):
# 	s.send("CONFIRM\r\n".encode('utf-8'))
# 	buff = s.recv(128)
# 	if buff == "CTS CONFIRM\r\n":
# 		break
# 	else:
# 		time.sleep(0.8)
