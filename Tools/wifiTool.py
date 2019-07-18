# -*- coding=utf-8
from pywifi import PyWiFi, const, Profile
import time

WIFI_SSID, WIFI_PSWD = "liuchen", "liuchen88"
AUTH_TYPE = "ENCRYPTED"
# WIFI_SSID, WIFI_PSWD = "ESP8266", "123456"
# AUTH_TYPE = "OPEN"

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
	else:
		print("WiFi: %s 连接失败" % wifi_name)
if __name__ == '__main__':
	connect(WIFI_SSID, WIFI_PSWD, AUTH_TYPE)