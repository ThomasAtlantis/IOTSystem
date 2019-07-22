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
	}
	, {
		"type": "request", "name": "temprature", "number": 0
	}, {
		"type": "request", "name": "temprature", "number": 2
	}, {
		"type": "request", "name": "humidity", "number": 1
	}, {
		"type": "request", "name": "humidity", "number": 2
	}, {
		"type": "command", "name": "light-on"
	}, {
		"type": "foo"
	}, {
	
	}
]
try:
	while True:
		buff = request.urlopen(
			"http://localhost/portal?{}".format(
			# "http://liushangyu.xyz/portal?{}".format(
				urlencode(random.sample(forms, 1)[0])
		)).read().decode("utf-8")
		print(json.loads(buff))
		time.sleep(3)
except Exception as err:
	print(err)
	sys.exit(1)