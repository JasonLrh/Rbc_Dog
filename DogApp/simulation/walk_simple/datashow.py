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


with open('data.csv', 'r') as f:
    lins = f.readlines()
    t, the1, the2, d, theta = [],[],[],[],[]
    for line in lins:
        m = line.split(',')
        t.append(float(m[0]))
        the1.append(float(m[1]))
        the2.append(float(m[2]))
        d.append(float(m[3]))
        theta.append(float(m[4]))
        # height.append(float(m[3]))
    
    plt.plot(t,the1,label='p1')
    plt.plot(t,the2,label='p2')

    plt.legend()
    plt.savefig('motor_angle_t.png')

    plt.clf()
    plt.cla()

    x = []
    y = []
    for i in range(len(the1)):
        y.append( - d[i] * np.cos(theta[i]))
        x.append(   d[i] * np.sin(theta[i]))

    plt.plot(x, y)
    plt.gca().set_aspect(1)
    # plt.legend()
    # plt.show()
    plt.savefig('foot_path.png')
    print(min(x))
    top_index = y.index(max(y))
    print( "center height: %.2f cm"%((max(y) - y[0])*100) )