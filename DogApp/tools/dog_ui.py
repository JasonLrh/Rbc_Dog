from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QSlider
from PyQt5.QtCore import Qt
import sys
import json
from grpc import server
import serial
from serial.threaded import ReaderThread,LineReader



class MainWindow(QWidget):

    def __init__(self):
        super().__init__()

        cfgFileName = sys.argv[1]
        if cfgFileName.endswith('.json') != True:
            print("please select config json file")
            exit(0)
        with open(cfgFileName, 'r') as f:
            dic = json.loads(f.read())
            server_name = dic['host']
            bin_file_name = dic['firmware']
        if ':' not in server_name:
            server_name = 'socket://' + server_name
        server_name += ':3334'

        com = serial.serial_for_url(server_name)
        self.serial = ReaderThread(com, self.PrintLines)

        self.button_name = [
            ['stand up','SSU'],
            ['sit down','SSD'],
            ['walk','SSW'],
            ['stop','SSS']
        ]

        self.slider_name = ['k_p', 'k_v']

        self.initUI()

    def initUI(self):
        self.setGeometry(300, 300, 305, 100)
        self.setWindowTitle('Dog Control Pannel')
        self.btns = {}
        for i in range(len(self.button_name)):
            self.btns[self.button_name[i][0]] = QPushButton(self.button_name[i][0], self)
            self.btns[self.button_name[i][0]].move(5, 5 + 22 * (len(self.btns.keys()) - 1 ))
            self.btns[self.button_name[i][0]].clicked.connect(self.onBtnClick(self.button_name[i][1]))
            self.btns[self.button_name[i][0]].show()
        self.sliders = {}
        self.sliders_value_label = {}

        for i in range(len(self.slider_name)):
            label = QLabel(self.slider_name[i] + ":", self)
            self.sliders[self.slider_name[i]] = QSlider(Qt.Horizontal, self)
            
            if self.slider_name[i] == 'k_p':
                self.sliders[self.slider_name[i]].setRange(0, 600)
                self.sliders[self.slider_name[i]].setValue( int (9 * 10) )
            else:
                self.sliders[self.slider_name[i]].setRange(0, 100)
                self.sliders[self.slider_name[i]].setValue( int (1.2 * 10) )
            self.sliders_value_label[self.slider_name[i]] = QLabel("%02.3f"%(self.sliders[self.slider_name[i]].value() / 10), self)

            label.move(90, 5 + 22 * (len(self.sliders.keys()) - 1 ))
            self.sliders_value_label[self.slider_name[i]].move(115, 5 + 22 * (len(self.sliders.keys()) - 1 ))

            self.sliders[self.slider_name[i]].setGeometry(150, 5 + 22 * (len(self.sliders.keys()) - 1), 150, 20)

            self.sliders[self.slider_name[i]].valueChanged.connect(self.onSliderChange)

        self.show()
    
    def onSliderChange(self):
        for i in self.sliders.keys():
            self.sliders_value_label[i].setText("%02.3f"%(self.sliders[i].value() / 10))
    
    def onBtnClick(self, cmd):
        if cmd == 'SSU':
            def y():
                st = "SSU%.2f,%.2f\n"%(self.sliders['k_p'].value() / 10, self.sliders['k_v'].value() / 10)
                print(st[:-1])
                self.serial.write(st.encode())
            return y
        else:
            def y():
                if cmd == 'SSD':
                    self.sliders[self.slider_name[0]].setValue( int (9 * 10) )
                    self.sliders[self.slider_name[1]].setValue( int (1.2 * 10) )
                print(cmd)
                st = cmd + '\n'
                self.serial.write(st.encode())
            return y
    
    @classmethod
    def process_line(self, data):
        print(data)

    class PrintLines(LineReader):
        TERMINATOR = b'\n'
        
        def connection_made(self, transport):
            super(MainWindow.PrintLines, self).connection_made(transport)

        def handle_line(self, data):
            print("!!")
            MainWindow.process_line(data)

        def connection_lost(self, exc):
            sys.stdout.write('port closed\n')

if __name__ == '__main__':
    app = QApplication([sys.argv])
    # app.setStyleSheet("QPushButton { margin: 10ex; }")
    window = MainWindow()
    app.exec_()
