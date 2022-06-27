from matplotlib import pyplot as plt
import numpy as np


# with open('data.txt', 'r') as f:
#     lins = f.readlines()
#     t,theta,d,height = [],[],[],[]
#     for line in lins:
#         m = line.split(',')
#         t.append(float(m[0]))
#         theta.append(float(m[1]))
#         d.append(float(m[2]))
#         height.append(float(m[3]))
    
#     plt.plot(t,theta,label='theta')
#     plt.plot(t,d,label='d')
#     plt.plot(t,height,label='height')
#     plt.legend()
#     plt.savefig('ans.png')


with open('data.txt', 'r') as f:
    lins = f.readlines()
    t, the1, the2 = [],[],[]
    for line in lins:
        m = line.split(',')
        t.append(float(m[0]))
        the1.append(float(m[1]))
        the2.append(float(m[2]))
        # height.append(float(m[3]))
    
    plt.plot(t,the1,label='1')
    plt.plot(t,the2,label='2')
    # plt.plot(t,height,label='height')
    plt.legend()
    plt.savefig('ans.png')