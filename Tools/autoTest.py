# -*- coding=utf-8
from pywifi import PyWiFi, const, Profile
import socket
import time
import sys
import threading
import inspect 
import ctypes
import platform
import re

# Global arguments
# WIFI_SSID_AP, WIFI_PSWD_AP = "liuchen", "liuchen88"
WIFI_SSID_AP, WIFI_PSWD_AP = "Atlantis", "21396878335"
AUTH_TYPE_AP = "ENCRYPTED"
WIFI_SSID_ESP, WIFI_PSWD_ESP = "ESP8266", "123456"
AUTH_TYPE_ESP = "OPEN"
SERVER_IP, SERVER_PORT = "192.168.1.102", 8000
SERVER_SOCKET = (SERVER_IP, SERVER_PORT)
GATEWAY_IP, GATEWAY_PORT = "192.168.4.1", 8266
GATEWAY_SOCKET = (GATEWAY_IP, GATEWAY_PORT)

# Global variables
thread_list = []

def disconnect():
	'''Disconnect wireless network card'''
	wifi = PyWiFi()
	ifaces = wifi.interfaces()[0]
	ifaces.disconnect()
	if ifaces.status() in [const.IFACE_DISCONNECTED, const.IFACE_INACTIVE]:
		print("无线网卡 %s 未连接！" % ifaces.name())
	else:
		print("无线网卡 %s 已连接！" % ifaces.name())

def connect(wifi_name, wifi_password, auth_type):
	'''Connect WiFi'''
	wifi = PyWiFi()
	iface = wifi.interfaces()[0]
	iface.disconnect()
	time.sleep(1)
	profile_info = Profile()
	profile_info.ssid = wifi_name
	profile_info.auth = const.AUTH_ALG_OPEN
	if auth_type == "OPEN":
		profile_info.akm.append(const.AKM_TYPE_NONE)
		profile_info.cipher = const.CIPHER_TYPE_NONE
	else:
		profile_info.akm.append(const.AKM_TYPE_WPA2PSK)
		profile_info.cipher = const.CIPHER_TYPE_CCMP
	profile_info.key = wifi_password
	iface.remove_all_network_profiles()
	tmp_profile = iface.add_network_profile(profile_info)
	iface.connect(tmp_profile)
	time.sleep(1)
	if iface.status() == const.IFACE_CONNECTED:
		print("WiFi: %s 连接成功" % wifi_name)
		return True
	else:
		print("WiFi: %s 连接失败" % wifi_name)
		return False

def configure_WiFi():
	# 需要格外注意的是，给网关发消息必须以\r\n结尾
	buff = ""
	print("configure WiFi")
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	# connect handshake
	s.connect(GATEWAY_SOCKET)
	print("connecting ... ", end="", flush=True)
	while not buff == "CTS\r\n":
		buff = s.recv(128).decode("utf-8")
	print("done")

	time.sleep(2)
	# set ip handshake
	while not buff == "GOT IP\r\n":
		s.send("IP{}\r\n".format(SERVER_IP).encode('utf-8'))
		time.sleep(0.5)
		buff = s.recv(128).decode("utf-8")

	time.sleep(2)
	# set port handshake
	while not buff == "GOT PORT\r\n":
		s.send("PORT{}\r\n".format(SERVER_PORT).encode('utf-8'))
		time.sleep(0.5)
		buff = s.recv(128).decode("utf-8")

	time.sleep(2)
	# set ssid handshake
	while not buff == "GOT SSID\r\n":
		s.send("SSID{}\r\n".format(WIFI_SSID_AP).encode('utf-8'))
		time.sleep(0.5)
		buff = s.recv(128).decode("utf-8")

	time.sleep(2)
	# set pswd handshake
	while not buff == "GOT PSWD\r\n":
		s.send("PSWD{}\r\n".format(WIFI_PSWD_AP).encode('utf-8'))
		time.sleep(0.5)
		buff = s.recv(128).decode("utf-8")
	
	time.sleep(2)
	for i in range(10):
		s.send("OK\r\n".encode('utf-8'))
		time.sleep(0.1)
	buff = s.recv(128).decode("utf-8")
	s.close()
	print("configure WiFi Finished")


def get_local_ip():
	try:
		tmp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		tmp.connect(("8.8.8.8", 80))
		local_ip = tmp.getsockname()[0]
		tmp.close()
		return local_ip
	except OSError:
		return SERVER_IP

def _async_raise(tid, exctype):
	"""raises the exception, performs cleanup if needed"""
	if not inspect.isclass(exctype):
		raise TypeError("Only types can be raised (not instances)")
	res = ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, ctypes.py_object(exctype))
	if res == 0:
		raise ValueError("invalid thread id")
	elif res != 1:
		# """if it returns a number greater than one, you're in trouble, 
		# and you should call it again with exc=NULL to revert the effect"""
		ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, 0)
		raise SystemError("PyThreadState_SetAsyncExc failed")

class Client(threading.Thread):

	def __init__(self, conn, addr):
		threading.Thread.__init__(self)
		self.conn = conn
		self.addr = addr

	def run(self):
		while True:
			szBuf = self.conn.recv(128)
			self.conn.send("received\r\n".encode("utf-8"))
			print(szBuf)
	
	def _get_my_tid(self):
		"""determines this (self's) thread id"""
		if not self.isAlive():
			raise threading.ThreadError("the thread is not active")
		# do we have it cached?
		if hasattr(self, "_thread_id"):
			return self._thread_id
		# no, look for it in the _active dict
		for tid, tobj in threading._active.items():
			if tobj is self:
				self._thread_id = tid
				return tid
		raise AssertionError("could not determine the thread's id")
	
	def raise_exc(self, exctype):
		"""raises the given exception type in the context of this thread"""
		_async_raise(self._get_my_tid(), exctype)

	def terminate(self):
		"""raises SystemExit in the context of the given thread, which should 
		cause the thread to exit silently (unless caught)"""
		self.raise_exc(SystemExit)

def run_server():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.bind(SERVER_SOCKET)
	sock.settimeout(None)
	sock.listen(10)
	print("listening on {}:{} ...".format(*SERVER_SOCKET))
	try:
		while True:
			conn, addr = sock.accept()
			conn.settimeout(None)
			print(addr, "Joined")
			client = Client(conn, addr)
			thread_list.append(client)
			client.start()
			for thread in thread_list[:-1]:
				if thread.isAlive():
					thread.conn.close()
					thread.terminate()
					thread.join()
			thread_list.clear()
	except Exception as err:
		print(err)
		if (sock):
			sock.close()
		sys.exit()

if __name__ == '__main__':
	while not connect(WIFI_SSID_AP, WIFI_PSWD_AP, AUTH_TYPE_AP):
		pass
	SERVER_IP = get_local_ip()
	SERVER_SOCKET = (SERVER_IP, SERVER_PORT)
	time.sleep(5)
	while not connect(WIFI_SSID_ESP, WIFI_PSWD_ESP, AUTH_TYPE_ESP):
		pass
	time.sleep(5)
	configure_WiFi()
	time.sleep(2)
	while not connect(WIFI_SSID_AP, WIFI_PSWD_AP, AUTH_TYPE_AP):
		pass
	run_server()