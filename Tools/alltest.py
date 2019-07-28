# -*- coding=utf-8
import json
_buff = 'temperature?0=24,1=&humidity?0=57,1='
data_dict = {}
# try:
for item in _buff.split('&'):
	tmp = item.split('?')
	data_dict[tmp[0]] = []
	for pair in tmp[1].split(','):
		n, value = pair.split('=')
		n = int(n)
		value = int(value) if value else 0
		data_dict[tmp[0]].append([n, value])
print(json.dumps({
	"type": "response",
	"name": "environment",
	"data": data_dict
}))