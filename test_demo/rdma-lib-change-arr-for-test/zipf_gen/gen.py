# -*- coding: utf-8 -*-
#P(r) = C / r^α zipf分布函数
import numpy as np
a = 2. #float类型，应该比1大
#Samples are drawn from a Zipf distribution with specified parameter a > 1
s = np.random.zipf(a, 2000)
print(s)
import matplotlib.pyplot as plt
from scipy import special
import scipy.stats as stats
count, bins, ignored = plt.hist(s[s<100], 100 , density=True)
x = np.arange(1., 100.)
y = x**(-a) / special.zetac(a)
print(x)
print(y)
plt.plot(x, y, linewidth=2, color='r')
#plt.show()
sum = 0.
for num in range(0, 99):
    sum = sum + y[num]
print ("sum", sum)
for num in range(0, 99):
    y[num] = (1000000 / sum * y[num])
print(y)
plt.plot(x, y, linewidth=2, color='r')
#plt.show()
sx = []
import random
sum = 0
#print random.sample(list(np.arange(0, 700000)), int(y[0]))
for num in range(0, 99):
    rangenum = 700
    last = 1 + sum
    sum = sum + rangenum
    x = list(np.arange(last, sum))
    sx.extend(random.sample(x, int(y[num]))) #append() 方法向列表的尾部添加一个新的元素。extend()方法只接受一个列表作为参数，并将该参数的每个元素都添加到原有的列表中。
sx.sort(key=None, reverse=False) #好用, cmp=None,
#sorted(sx)
#print (sx)
f = open('./test.txt', 'w')
for num in sx:
    f.write(str(num)+",")
f.close()
