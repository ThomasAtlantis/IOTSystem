# -*- coding=utf-8
# 模拟网关节点端
import socket
import time
import random

# TCP_SERVER_IP, TCP_SERVER_PORT = "39.105.70.105", 8266
TCP_SERVER_IP, TCP_SERVER_PORT = "localhost", 8266
TCP_SERVER_SOCKET = (TCP_SERVER_IP, TCP_SERVER_PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(TCP_SERVER_SOCKET)
print("TCP server connected")
while True:
	try:
		buff = s.recv(128).decode('utf-8')
		if buff:
			buff = buff[1:]
			print("received:", buff.encode('utf-8'))
		if buff == "heartbeat\r\n":
			s.send("received".encode('utf-8'))
		elif buff == "environment\r\n":
			# data = "humidity?{}&temperature?{}"
			# humidity = [str(i) + "=" + str(random.randint(0, 40)) for i in range(3)]
			# temperature = [str(i) + "=" + str(random.randint(0, 40)) for i in range(3)]
			# random.shuffle(humidity)
			# random.shuffle(temperature)
			# humidity = ",".join(humidity)
			# temperature = ",".join(temperature)
			# s.send(data.format(humidity, temperature).encode('utf-8'))
			s.send(b'temperature?0=24,1=&humidity?0=56,1=')
		elif buff.startswith("temperature"):
			s.send("temperature{}".format(random.randint(0, 40)).encode('utf-8'))
		elif buff.startswith("humidity"):
			s.send("humidity{}".format(random.randint(0, 40)).encode('utf-8'))
		elif buff in [
				"light-on\r\n", 
				"light-off\r\n",
				"humidify\r\n",
				"stop-humidify\r\n",
				"drain-water\r\n", 
				"stop-drain-water\r\n", 
				"draw-water\r\n", 
				"change-water\r\n",
				"water\r\n",
				"stop-water\r\n"
			]:
			s.send("OK".encode('utf-8'))

	except ConnectionResetError:
		print("Connection Reset")
		s.close()
	time.sleep(2)