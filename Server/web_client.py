# -*- coding=utf-8
# 模拟小程序端
from urllib.parse import urlencode
from urllib import request
import json
import time
import random
import sys

forms = [{
		# "type": "heartbeat"
	# }, {
		# "type": "request", "name": "temperature", "number": 0
	# }, {
		# "type": "request", "name": "temperature", "number": 2
	# }, {
		# "type": "request", "name": "humidity", "number": 1
	# }, {
	# 	"type": "request", "name": "humidity", "number": 2
	# }, {
		# "type": "command", "name": "light-on"
	# }, {
		# "type": "command", "name": "light-off"
	# }, {
		# "type": "command", "name": "humidify"
	# }, {
	# 	"type": "command", "name": "stop-humidify"
	# }, {
	# 	"type": "command", "name": "draw-water"
	# }, {
	# 	"type": "command", "name": "drain-water"
	# }, {
	# 	"type": "command", "name": "change-water"
	# }, {
		"type": "request", "name": "environment"
	# }, {
	# 	"type": "foo"
	}
]
# host_url = "http://liushangyu.xyz:8000/portal"
host_url = "http://localhost/portal"
try:
	while True:
		form = random.sample(forms, 1)[0]
		print("Request:", "{}?{}".format(host_url, urlencode(form)))
		buff = request.urlopen(
			# "http://localhost/portal?{}".format(
			"{}?{}".format(host_url, urlencode(form)
		)).read().decode("utf-8")
		print("Back:", json.loads(buff))
		# time.sleep(10)
		break
except Exception as err:
	print(err)
	sys.exit(1)