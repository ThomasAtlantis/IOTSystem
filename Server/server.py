# -*- coding=utf-8
# 服务器节点端
import threading
import socket
import json
import re
import sys
import time

# Global arguments
# WEB_SERVER_IP, WEB_SERVER_PORT = "172.16.252.95", 8000
WEB_SERVER_IP, WEB_SERVER_PORT = "localhost", 80
WEB_SERVER_SOCKET = (WEB_SERVER_IP, WEB_SERVER_PORT)
# TCP_SERVER_IP, TCP_SERVER_PORT = "172.16.252.95", 8266
TCP_SERVER_IP, TCP_SERVER_PORT = "localhost", 8266
TCP_SERVER_SOCKET = (TCP_SERVER_IP, TCP_SERVER_PORT)
HTML_ROOT_DIR = "./html"  # 设置静态文件根目录
WSGI_ROOT_DIR = "./wsgi"  # 设置动态文件根目录

class Client():

	def __init__(self, name, conn, addr):
		self.name = name
		self.conn = conn
		self.addr = addr

	def send(self, text):
		if self.conn:
			print("Send to [{}]: {}".format(self.name, text.encode('utf-8')))
			try:
				self.conn.send(("#" + text).encode('utf-8'))
			except ConnectionResetError:
				print("ERROR 1: ConnectionResetError")

	def recv(self, byte):
		text = ""
		while self.conn and not text:
			tmp = gateway.conn.recv(byte)
			if tmp:
				print("Recv from [{}]: {}".format(self.name, tmp))
				text = tmp.decode('utf-8')
				break
		return text

# Global variables
gateway = Client("Gateway", None, None)

class TCPServer(threading.Thread):

	def __init__(self):
		threading.Thread.__init__(self)

	def run(self):
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.sock.bind(TCP_SERVER_SOCKET)
		self.sock.settimeout(None)
		self.sock.listen(10)
		print("TCP Server listening on {}:{} ...".format(*TCP_SERVER_SOCKET))
		while True:
			gateway.conn, gateway.addr = self.sock.accept()
			print(gateway.addr, "Joined TCP Server")

class WebServer(threading.Thread):

	def __init__(self):
		threading.Thread.__init__(self)

	def run(self):
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.sock.bind(WEB_SERVER_SOCKET)
		self.sock.settimeout(None)
		self.sock.listen(1)
		sys.path.insert(1, WSGI_ROOT_DIR)
		print("Web Server listening on {}:{} ...".format(*WEB_SERVER_SOCKET))
		while True:
			conn, addr = self.sock.accept()
			print(addr, "Joined Web Server")
			thread = threading.Thread(target=self.handle_client, args=(conn,))
			thread.start()

	def parseParam(self, form):
		params = {}
		index = form.find('?') + 1
		if index and index != len(form):
			try:
				pairs = form[index:].split('&')
				for pair in pairs:
					pair = pair.split('=')
					params[pair[0]] = pair[1]
			except Exception:
				print("ERROR 2: Parameter syntax error")
		return params

	def handle_client(self, client_socket):
		request_data = client_socket.recv(512)
		request_lines = request_data.splitlines()
		if not request_lines:
			client_socket.close()
			return
		print("request: {}".format(request_lines[0]))

		# 解析请求报文
		request_start_line = request_lines[0]
		
		# 提取用户请求的文件名及请求方法
		file_name = re.match(r"\w+ +(/[^ ]*) ", request_start_line.decode("utf-8")).group(1)
		method = re.match(r"(\w+) +/[^ ]* ", request_start_line.decode("utf-8")).group(1)
		
		params = {}
		if method.upper() == "GET":
			params = self.parseParam(file_name)
			index = file_name.find('?')
			if index != -1:
				file_name = file_name[: index]
		elif method.upper() == "POST":
			pass

		# 处理动态文件
		if file_name.endswith(".py"):
			try:
				m = __import__(file_name[1:-3])
			except Exception:
				self.response_headers = "HTTP/1.1 404 Not Found\r\n"
				response_body = "not found"
			else:
				env = {
					"PATH_INFO": file_name,
					"METHOD": method
				}
				response_body = m.application(env, self.start_response)

			response = self.response_headers + "\r\n" + response_body
		# 处理静态文件
		elif file_name:		
			# 静态路由
			file_flag = False
			response_start_line = "HTTP/1.1 200 OK\r\n"
			response_headers = "Server: My server\r\n"
			response_body = ""
			if "/" == file_name:
				file_name = "/index.html"
				file_flag = True
			elif "/gateway-info" == file_name:
				if gateway.addr:
					response_body = json.dumps({
						"ip": gateway.addr[0],
						"port": gateway.addr[1]
					})
			elif "/portal" == file_name:
				if 'type' not in params:
					response_body = json.dumps({
						"type": "error",
						"code": "1"
					})
				else:
					_type = params['type']
					if _type == "heartbeat":
						gateway.send("heartbeat\r\n")
						while True:
							_buff = gateway.recv(512)
							if _buff == "received":
								time.sleep(0.5)
								
								response_body = json.dumps({
									"type": "received"
								})
								break
					elif _type == "request":
						_name = params['name']
						if _name == "environment":
							gateway.send("environment\r\n")
							while True:
								_buff = gateway.recv(512)
								data_dict = {}
								# try:
								for item in _buff.split('&'):
									print(item)
									tmp = item.split('?')
									data_dict[tmp[0]] = []
									for pair in tmp[1].split(','):
										n, value = pair.split('=')
										n = int(n)
										value = int(value) if value else 0
										data_dict[tmp[0]].append([n, value])
								# except Exception as err:
									# print(err)
									# response_body = json.dumps({
										# "type": "error",
										# "code": "3"
									# })
									
									# break
								response_body = json.dumps({
									"type": "response",
									"name": "environment",
									"data": data_dict
								})
								
								break
						elif _name in ["temperature", "humidity"]:
							_number = params['number']
							gateway.send("{}{}\r\n".format(_name, _number))
							while True:
								_buff = gateway.recv(512)
								if _buff.startswith("temperature"):
									response_body = json.dumps({
										"type": "response",
										"name": "temperature",
										"number": _number,
										"result": int(_buff[11:])
									})
									
									break
								elif _buff.startswith("humidity"):
									response_body = json.dumps({
										"type": "response",
										"name": "humidity",
										"number": _number,
										"result": int(_buff[8:])
									})
									
									break
					elif _type == "command":
						_name = params['name']
						if _name in [
								"light-on",
								"light-off", 
								"humidify", 
								"stop-humidify", 
								"drain-water",
								"stop-drain-water", 
								"draw-water", 
								"change-water",
								"water",
								"stop-water",
							]:
							print("Command:", _name)
							gateway.send(_name + "\r\n")
							while True:
								_buff = gateway.recv(512)
								if _buff == "OK":
									response_body = json.dumps({
										"type": "command",
										"name": _name,
										"back": "OK"
									})
									
									break
					else:
						response_body = json.dumps({
							"type": "error",
							"code": "2"
						})
			if file_flag:
				# 打开文件，读取内容
				file_data = ""
				try:
					file = open(HTML_ROOT_DIR + file_name, "rb")
				except IOError:
					response_start_line = "HTTP/1.1 404 Not Found\r\n"
					response_headers = "Server: My server\r\n"
					response_body = "The file is not found!"
				else:
					file_data = file.read().decode("utf-8")
					file.close()
				response_body = file_data

			response = response_start_line + response_headers + '\r\n' + response_body
		
		print("response data:", response.encode("utf-8"))

		# 向客户端返回响应数据
		client_socket.send(bytes(response, "utf-8"))

		# 关闭客户端连接
		client_socket.close()

if __name__ == '__main__':
	tcpServer = TCPServer()
	tcpServer.start()
	webServer = WebServer()
	webServer.start()