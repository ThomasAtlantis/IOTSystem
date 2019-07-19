from flask import Flask, request
from flask_socketio import SocketIO
import socket
import threading
import inspect 
import ctypes
import platform
import re

# Global arguments
WEB_SERVER_IP, WEB_SERVER_PORT = "localhost", 8000
WEB_SERVER_SOCKET = (WEB_SERVER_IP, WEB_SERVER_PORT)
TCP_SERVER_IP, TCP_SERVER_PORT = "localhost", 1923
TCP_SERVER_SOCKET = (TCP_SERVER_IP, TCP_SERVER_PORT)

# Global variables
thread_list = []

app = Flask(__name__)
socketio = SocketIO()
socketio.init_app(app)

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
			try:
				szBuf = self.conn.recv(128)
				self.conn.send("received\r\n".encode("utf-8"))
				print(szBuf)
			except (ConnectionAbortedError, ConnectionResetError):
				print("Client Connection Aborted")
				return
	
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

class SocketServer(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)

	def run(self):
		try:
			sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			sock.bind(TCP_SERVER_SOCKET)
		except OSError:
			sock.close()
			return
		sock.settimeout(None)
		sock.listen(10)
		print("listening on {}:{} ...".format(TCP_SERVER_IP, TCP_SERVER_PORT))
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
			return

"""
对app进行一些路由设置
"""

@app.route('/')
def hello_world():
	word = request.args['word']
	return word.upper()

if __name__ == '__main__':
	socketServer = SocketServer()
	socketServer.start()
	socketio.run(app, debug=True, host=WEB_SERVER_IP, port=WEB_SERVER_PORT)