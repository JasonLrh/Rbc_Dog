from PyQt5.QtWidgets   import QApplication, QWidget, QMainWindow, QCheckBox
from PyQt5             import QtWidgets
from PyQt5.QtCore      import Qt, QLineF, QTimer

import sys


class MainWindow(QWidget):

    def __init__(self):
        super().__init__()
        self.cb_motor_en = []
        for i in range(8):
            self.cb_motor_en.append( QCheckBox('&'+str(i+1), self) )
        for i in range(len( self.cb_motor_en )):
            # self.vbl.addWidget(i, alignment=Qt.AlignRight)
            self.cb_motor_en[i].setChecked(False)
            self.cb_motor_en[i].setGeometry(i*32,0,28,20)
            self.cb_motor_en[i].stateChanged.connect(self.generate_function(i))
        self.show()
        # self.setLayout(self.vbl)
        
    
    def generate_function(self, num):
        def g():
            state = self.cb_motor_en[num].isChecked()
            print(num, state)
            
        # g = lambda x:print(num)
        return g


if __name__ == '__main__':
    app = QApplication([sys.argv])
    # app.setStyleSheet("QPushButton { margin: 10ex; }")
    window = MainWindow()
    app.exec_()
