# -*- coding=utf-8
import random
data = "humidity?{}&temperature?{}"
humidity = ",".join([str(i) + "=" + str(random.randint(0, 40)) for i in range(3)])
temperature = ",".join([str(i) + "=" + str(random.randint(0, 40)) for i in range(3)])
print(data.format(humidity, temperature))