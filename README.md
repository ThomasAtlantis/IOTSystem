## 智能盆景物联网系统
### TODO LIST
+ **小程序**
	+ 连接网关AP点对点配置信息功能
+ **服务器**
	+ None

+ **网  关**
	+ WiFi和Zigbee联调

+ **终  端**
	+ 传感器节点
		+ None

	+ 鱼缸节点
		+ 水位控制抽水接触不良解决及回归测试
		+ 土壤湿度传感器、花盆和土的获取
		+ 组装

+ **总体联调**
	+ 网关的WiFi和Zigbee ->
	+ 鱼缸节点自己 ->
		+ 网关和服务器 ->
			+ 网关、服务器和鱼缸节点 ->
				+ 小程序和服务器 ->
					+ 总体

### 接口文档
#### 小程序配置网关信息
|用途|格式|
|----|----|
|IP地址|IPx.x.x.x\r\n|
|端口|PORTxxxx\r\n|
|WiFi名称|SSIDabcd\r\n|
|WiFi密码|PSWDabcd\r\n|
|配置完毕|OK\r\n|
#### 小程序请求服务器端
|METHOD|URL|FORM|BACK|NOTE|
|---|---|---|---|---|
|GET|liushangyu.xyz/gateway-info|-|{ip:"x.x.x.x",port:xxxx}|获取网关socket未连接返回空值|
|GET|liushangyu.xyz/portal|type=heartbeat|{"type":"received"}|发送心跳包检测连接活性|
|GET|liushangyu.xyz/portal|type=request&name=temperature&number=%x|{"type":"response","name": "temperature","number": %x,"result": %y}|获取%x号传感器温度|
|GET|liushangyu.xyz/portal|type=request&name=humidity&number=%x|{"type":"response","name": "humidity","number": %x,"result": %y}|获取%x号传感器湿度|
|GET|liushangyu.xyz/portal|type=command&name=light-on|{"type":"command","name":"light-on","back": "OK"}|开鱼缸LED灯|
|GET|liushangyu.xyz/portal|type=command&name=drain-water|{"type":"command","name":"drain-water","back": "OK"}|排水模式：将会屏蔽水位传感器|
|GET|liushangyu.xyz/portal|type=command&name=draw-water|{"type":"command","name":"draw-water","back": "OK"}|抽水|
|GET|liushangyu.xyz/portal|type=command&name=change-water|{"type":"command","name":"change-water","back": "OK"}|换水|
|GET|liushangyu.xyz/portal|type=command&name=humidify|{"type":"command","name":"humidify","back": "OK"}|加湿|
|GET|liushangyu.xyz/portal|type=command&name=stop-humidify|{"type":"command","name":"stop-humidify","back": "OK"}|停止加湿|

注意：
+ %x，%y仅表示变量，数据格式中不包含%
+ 所有方法的返回数据均为json格式，以双引号引起的为字符串，否则为数值类型
+ get请求格式示例：`liushangyu.xyz/portal?type=request&name=temperature&number=1`，但js的get方法可以把URL（liushangyu.xyz/portal）和表单（type=...）分开写