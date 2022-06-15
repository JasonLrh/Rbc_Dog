#!/usr/bin/env python3
import serial
from serial.threaded import LineReader, ReaderThread

import sys
import json


class PrintLines(LineReader):
    TERMINATOR = b'\n'
    def connection_made(self, transport):
        super(PrintLines, self).connection_made(transport)

    def handle_line(self, data):
        sys.stderr.write(data)

    def connection_lost(self, exc):
        if exc:
            sys.stderr.write(str(exc))
        sys.stdout.write('port closed\n')
    


class MyReaderThread(ReaderThread):
    def __init__(self, serial_instance, protocol_factory):
        super(MyReaderThread, self).__init__(serial_instance, protocol_factory)


if __name__ == '__main__':
    # lex = sys.argv[1]
    with open("config.json", 'r') as f:
        dic = json.loads(f.read())
        lex = dic['host']
    
    lex = 'socket://' + lex + ':3334'
    com = serial.serial_for_url(lex)
    # while True:
    #     print(com.read().decode())



    ser = MyReaderThread(com, PrintLines)
    ser.start()
    while True:
        try:
            k = input()
            # print("[input]", k)
            com.write(k.encode())
        except KeyboardInterrupt as e:
            print("EXIT KEY TRIGGER")
            ser.stop()
            com.close()
            break