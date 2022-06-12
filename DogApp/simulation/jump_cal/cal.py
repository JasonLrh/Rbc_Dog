
# y = a * x ^ 2 + b * x (a<0, b>0)

import numpy as np

import matplotlib.pyplot as plt

# gravity = 9.81
# vx = 1.2
# vy = 1.3
# L = 0.5


# a = - gravity / ((vx**2) * 2)
# b = vy / vx
# h_max = ( a*L**2 + (b**2/a**2) - 2 * L * b / a + 2 * L * b - 2 * b ** 2 / a ) / 4

# print(int(h_max * 1000))

a = 0.15
b = 0.2

theta = 45 * np.pi / 180
thetas = []
v = 1
delta_t = 0.002
# vs = []

for cnt in range(36) :
    sa = a * np.sin(theta)
    ca = a * np.cos(theta)
    param = sa * (1 + ca / np.sqrt((b**2) - (sa**2)))
    w = v / param
    theta -= w * delta_t
    thetas.append(theta)

f_thetas = []
for i in range(len(thetas) - 1):
    f_thetas.append( (thetas[i+1] - thetas[i]) / delta_t )

plt.plot(np.array(thetas) * 180 / np.pi)
plt.plot(- np.array(f_thetas) )
plt.show()