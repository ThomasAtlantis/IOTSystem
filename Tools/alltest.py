# -*- coding=utf-8

forms = [
	"/portal?type=command&name=light-on",
	"/portal?number=0",
	"/portal?",
	"/portal",
]

def parseParam(form):
	params = {}
	index = form.find('?') + 1
	if index and index != len(form):
		try:
			pairs = form[index:].split('&')
			for pair in pairs:
				pair = pair.split('=')
				params[pair[0]] = pair[1]
		except Exception:
			print("Parameter syntax error")
	return params

if __name__ == '__main__':
	for form in forms:
		print(parseParam(form))