import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib        import pyplot as plt
from matplotlib.figure import Figure
from PyQt5.QtWidgets   import QApplication, QWidget, QLabel, QCheckBox, QSlider, QOpenGLWidget
from PyQt5.QtGui       import QPixmap, QPainter, QColor
from PyQt5.QtCore      import Qt, QLineF, QTimer

from PyQt5             import QtGui, QtWidgets
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from OpenGL.GL         import *
from OpenGL.GLU        import *
from ui_utils.quat     import *
import json
import numpy as np
import time


# class pltWindow(QWidget):
#     def __init__(self, parent=None):
#         super().__init__()
#         self._main = QWidget()
#         # self.setCentralWidget(self._main)
#         layout = QtWidgets.QVBoxLayout(self._main)

#         static_canvas = FigureCanvas(Figure(figsize=(5, 3)))
#         # Ideally one would use self.addToolBar here, but it is slightly
#         # incompatible between PyQt6 and other bindings, so we just add the
#         # toolbar as a plain widget instead.
#         # layout.addWidget(NavigationToolbar(static_canvas, self))
#         layout.addWidget(static_canvas)

#         dynamic_canvas = FigureCanvas(Figure(figsize=(5, 3)))
#         layout.addWidget(dynamic_canvas)
#         # layout.addWidget(NavigationToolbar(dynamic_canvas, self))

#         self._static_ax = static_canvas.figure.subplots()
#         t = np.linspace(0, 10, 501)
#         self._static_ax.plot(t, np.tan(t), ".")

#         self._dynamic_ax = dynamic_canvas.figure.subplots()
#         t = np.linspace(0, 10, 101)
#         # Set up a Line2D.
#         self._line, = self._dynamic_ax.plot(t, np.sin(t + time.time()))
#         self._timer = dynamic_canvas.new_timer(50)
#         self._timer.add_callback(self._update_canvas)
#         self._timer.start()
        
    
#     def _update_canvas(self):
#         t = np.linspace(0, 10, 101)
#         # Shift the sinusoid as a function of time.
#         self._line.set_data(t, np.sin(t + time.time()))
#         self._line.figure.canvas.draw()

class pltCanvas(FigureCanvas):
    def __init__(self):
        self.fig  = Figure()
        self.ax = self.fig.add_subplot(111)
        FigureCanvas.__init__(self, self.fig)
        FigureCanvas.setSizePolicy(self, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

class pltWidget(QWidget):
    def __init__(self, parent=None) -> None:
        QtWidgets.QWidget.__init__(self, parent=parent)
        self.canvases = {'p':pltCanvas(), 'v':pltCanvas(), 't':pltCanvas()}
        # self.canvasP = pltCanvas()
        # self.canvasV = pltCanvas()
        # self.canvasT = pltCanvas()
        self.vbl = QtWidgets.QVBoxLayout()
        self.vbl.addWidget(self.canvases['p'])
        self.vbl.addWidget(self.canvases['v'], alignment=Qt.AlignBottom)
        self.vbl.addWidget(self.canvases['t'], alignment=Qt.AlignBottom)

        self.setLayout(self.vbl)

        self.motor = {}
        # self.current_item = []
    
    # def drawing(self, canv:pltCanvas, )
    
    def update_motor(self, data:dict):
        ci = []
        for m in data:
            id = str(m['id'])
            if id in self.motor.keys():
                ci.append(id)
                for item in ['p', 'v', 't']:
                    self.motor[id][item].append(m[item])
                    while len(self.motor[id][item]) > 200:
                        # self.motor[id][item] = self.motor[id][item][-201:-1]
                        self.motor[id][item].pop(0)
            else:
                self.motor[id] = {'p':[], 'v':[], 't':[]}
        
        # drawing
        for item in ['p', 'v', 't']:
            self.canvases[item].ax.clear()
            self.canvases[item].ax.set_title(item)
            for id in ci:
                # x = np.arange(0, 1, 1/len(self.motor[id][item]))
                self.canvases[item].ax.plot(self.motor[id][item])
            self.canvases[item].draw()


class GLWindow(QOpenGLWidget):
    def __init__(self, parent=None):
        self.__axis_points = ((-6.0, 0.0, 0.0), (6.0, 0.0, 0.0), (0.0, -6.0, 0.0),
                (0.0, 6.0, 0.0), (0.0, 0.0, -6.0), (0.0, 0.0, 6.0))

        self.__axes = ((0, 1), (2, 3), (4, 5))

        self.__verticies = (
            (-3.0, -3.0, 3.0),
            (-3.0, 3.0, 3.0),
            (3.0, 3.0, 3.0),
            (3.0, -3.0, 3.0),
            (-3.0, -3.0, -3.0),
            (-3.0, 3.0, -3.0),
            (3.0, 3.0, -3.0),
            (3.0, -3.0, -3.0),
        )

        self.__edges = ((0, 1), (0, 3), (0, 4), (2, 1), (2, 3), (2, 6), (5, 1), (5, 4),
                (5, 6), (7, 3), (7, 4), (7, 6))

        self.__surfaces = (
            (0, 1, 2, 3),
            (3, 2, 6, 7),
            (7, 6, 5, 4),
            (4, 5, 1, 0),
            (1, 5, 6, 2),
            (4, 0, 3, 7),
        )

        self.__colors = (
            (0.769, 0.118, 0.227),  # Red 
            (0.0, 0.318, 0.729),  # Blue 
            (1.0, 0.345, 0.0),  # Orange 
            (0.0, 0.62, 0.376),  # Green 
            (1.0, 1.0, 1.0),  # White 
            (1.0, 0.835, 0.0),  # Yellow 
        )
        super(GLWindow, self).__init__(parent=parent)

        q = [0,0,0,1]
        # q = axisangle_to_q(q[0:3], -2 * acos(q[3]))
        self.__q = [0,0,0,1]
        self.__isinitial = True

        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update)
        self.update_timer.start(1)

    def Axis(self):
        glBegin(GL_LINES)
        glColor3f(1, 1, 1)
        for axis in self.__axes:
            for point in axis:
                glVertex3fv(self.__axis_points[point])
        glEnd()

    def Cube(self):
        glBegin(GL_QUADS)
        for color, surface in zip(self.__colors, self.__surfaces):
            glColor3fv(color)
            for vertex in surface:
                glVertex3fv(self.__verticies[vertex])
        glEnd()

        glBegin(GL_LINES)
        glColor3fv((0, 0, 0))
        for edge in self.__edges:
            for vertex in edge:
                glVertex3fv(self.__verticies[vertex])
        glEnd()
    
    def update_quat(self, q:list):
        # q = axisangle_to_q(q[0:3], -2 * acos(q[3]))
        # q_mult(q, (-2 * acos(q3)))
        self.__q = list(q)
        # self.tot_rot = q_to_mat4(q_mult(q, (0,0,1,0)))
        # print(self.tot_rot)
        
    def resized(self) -> None:
        return super().resized()
    
    def paintGL(self, coordinates=None) -> None:
        if self.__isinitial == True:
            self.__isinitial = False
        else:
            glPopMatrix()
        glPushMatrix()
        tot_rot = q_to_mat4(q_mult(self.__q, (0,0,1,0)))
        glMultMatrixf(tot_rot)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        self.Cube()
        self.Axis()
    
    def initializeGL(self) -> None:
        glEnable(GL_DEPTH_TEST)
        glEnable(GL_TEXTURE_2D)
        glDepthFunc(GL_LESS)
        size = self.size()
        gluPerspective(45, (size.width() / size.height()), 0.05, 50.0)
        glTranslatef(0.0, 0.0, -25)


class logView(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setGeometry(0, 0, 400, 450)

        ## glx window
        self.glWindow = GLWindow(self)
        self.glWindow.setGeometry(0, 230*3, 100, 100)
        ## plt window
        self.pltView = pltWidget(self)
        self.pltView.setGeometry(0, 0, 600, 230 * 3)

        self.cb_motor_en = []
        for i in range(8):
            self.cb_motor_en.append( QCheckBox('&'+str(i+1), self) )
        for i in range(len( self.cb_motor_en )):
            # self.vbl.addWidget(i, alignment=Qt.AlignRight)
            self.cb_motor_en[i].setChecked(False)
            self.cb_motor_en[i].setGeometry(110 + i*32 , 230 * 3 + 10 , 28 , 20)
            self.cb_motor_en[i].stateChanged.connect(self.cb_motor_en_function_generator(i))
        
        self.send_tx = None
        self.show()
        self.__total_pack = 0
        self.__error_pack = 0
    
    def __call_upper_serial_print(self, data):
        # print(data) # todo
        if self.send_tx != None:
            self.send_tx(data.encode())
    
    def cb_motor_en_function_generator(self, num):
        def g():
            state = self.cb_motor_en[num].isChecked()
            st = 'M' + str(num + 1)
            if state == False:
                st = 'LC' + st
            else:
                st = 'LO' + st 
            self.__call_upper_serial_print(st)
        return g
    
    def connect_tx(self, f):
        self.send_tx = f
    
    def update_data(self, data):
        self.__total_pack += 1
        try:
            m = json.loads(data)
            # self.quat = list(m['imu']['quat'])
            self.glWindow.update_quat(list(m['imu']['quat']))
            self.pltView.update_motor(list(m['motor']))
        except Exception as e:
            self.__error_pack += 1
            print(e)
            print("err rate: %.2f %%"%(self.__error_pack * 100 / self.__total_pack) )
            pass
