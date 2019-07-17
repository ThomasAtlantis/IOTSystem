import sys
import socket
import threading
import inspect 
import ctypes
import platform
import re

# Global arguments
SERVER_IP, SERVER_PORT = "192.168.43.2", 8000
SERVER_SOCKET = (SERVER_IP, SERVER_PORT)

# Global variables
thread_list = []

def get_local_ip():
	tmp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	tmp.connect(("8.8.8.8", 80))
	local_ip = tmp.getsockname()[0]
	tmp.close()
	return local_ip

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
			conn.send("received\r\n".encode("utf-8"))
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

if __name__ == '__main__':
	thisIP = get_local_ip()
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.bind((thisIP, SERVER_PORT))
	sock.settimeout(None)
	sock.listen(10)
	print("listening on {}:{} ...".format(thisIP, SERVER_PORT))
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