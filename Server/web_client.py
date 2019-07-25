# -*- coding=utf-8
# 模拟小程序端
from urllib.parse import urlencode
from urllib import request
import json
import time
import random
import sys

forms = [{
		"type": "heartbeat"
	}, {
		"type": "request", "name": "temperature", "number": 0
	}, {
		"type": "request", "name": "temperature", "number": 2
	}, {
		"type": "request", "name": "humidity", "number": 1
	}, {
		"type": "request", "name": "humidity", "number": 2
	}, {
		"type": "command", "name": "light-on"
	}, {
		"type": "command", "name": "humidify"
	}, {
		"type": "command", "name": "stop-humidify"
	}, {
		"type": "command", "name": "draw-water"
	}, {
		"type": "command", "name": "drain-water"
	}, {
		"type": "command", "name": "change-water"
	}, {
		"type": "request", "name": "environment"
	}, {
		"type": "foo"
	}
]
try:
	while True:
		form = random.sample(forms, 1)[0]
		print("Request:", form)
		buff = request.urlopen(
			# "http://localhost/portal?{}".format(
			"http://liushangyu.xyz/portal?{}".format(urlencode(form)
		)).read().decode("utf-8")
		print("Back:", json.loads(buff))
		time.sleep(3)
except Exception as err:
	print(err)
	sys.exit(1)