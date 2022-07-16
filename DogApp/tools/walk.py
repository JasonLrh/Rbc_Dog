#!/usr/bin/env python3
import serial
from serial.threaded import LineReader, ReaderThread


import sys
import json

import pygame
from pygame.locals import *


class PrintLines(LineReader):
    TERMINATOR = b'\n'
    def connection_made(self, transport):
        super(PrintLines, self).connection_made(transport)

    def handle_line(self, data):
        print(data, flush=True)

    def connection_lost(self, exc):
        if exc:
            sys.stderr.write(str(exc))
        sys.stdout.write('port closed\n')
    


class MyReaderThread(ReaderThread):
    def __init__(self, serial_instance, protocol_factory):
        super(MyReaderThread, self).__init__(serial_instance, protocol_factory)


if __name__ == '__main__':
    cfgFileName = sys.argv[1]
    if cfgFileName.endswith('.json') != True:
        print("please select config json file")
        exit(0)
    with open(cfgFileName, 'r') as f:
        dic = json.loads(f.read())
        lex = dic['host']
    
    lex = 'socket://' + lex + ':3334'
    com = serial.serial_for_url(lex)
    # while True:
    #     print(com.read().decode())



    ser = MyReaderThread(com, PrintLines)
    ser.start()

    pygame.init()
    screen = pygame.display.set_mode((640,480))

    while True:
        for event in pygame.event.get():
            if event.type == QUIT:
                exit(0)
            if event.type == KEYDOWN:
                if event.key == pygame.K_LEFT:
                    ser.write('SL'.encode())
                elif event.key == pygame.K_RIGHT:
                    ser.write('SR'.encode())
                elif event.key == pygame.K_s:
                    ser.write('SSW'.encode())
                elif event.key == pygame.K_d:
                    ser.write('SSD'.encode())
                elif event.key == pygame.K_SPACE:
                    ser.write('SSU10,1'.encode())
                else:
                    ser.write('SSQ'.encode())


    # while True:
    #     try:
    #         k = input()
    #         # print("[input]", k)
    #         com.write(k.encode())
    #     except KeyboardInterrupt as e:
    #         print("EXIT KEY TRIGGER")
    #         ser.stop()
    #         com.close()
    #         break