#!/usr/bin/env python3

from numpy import byte
import serial
from serial.threaded import ReaderThread
import sys
import time
import json

class cmd_com:
    def __init__(self, s_name:str) -> None:
        self.__com = serial.serial_for_url(s_name + ':3333', timeout=2)
    
    def __send_cmd(self, cmd:str):
        self.__com.write(cmd.encode())
    
    def enter_boot(self):
        self.__send_cmd('EE')
        time.sleep(0.8)
        self.__send_cmd('EB')
        time.sleep(1.8)
    
    def exit_boot(self):
        self.__send_cmd('EE')
    

class serial_com:
    def __init__(self, s_name:str) -> None:
        self.__com = serial.serial_for_url(s_name + ':3334',  timeout=2)
    
    def __send_cmd(self, cmd):

        if type(cmd) == str:
            cmd = cmd.encode()
        elif type(cmd) != bytes:
            print("not valid type: serial_com")
        self.__com.write(cmd)
    
    def tx_raw(self, cmd):
        self.__send_cmd(cmd)
    
    def app_tx(self, cmd:str):
        if cmd[-1] != '\n':
            cmd += '\n'
        self.__send_cmd(cmd)
    
    def read(self) -> bytes:
        return self.__com.read(1)


if __name__ == '__main__':
    with open("config.json", 'r') as f:
        dic = json.loads(f.read())
        server_name = dic['host']
        bin_file_name = dic['firmware']

    if ':' not in server_name:
        server_name = 'socket://' + server_name

    bootWriter = cmd_com(server_name)

    if len(sys.argv) > 1:
        if sys.argv[1] == 'E':
            bootWriter.exit_boot()
            exit()

    if bin_file_name.endswith('.bin'):
        # write flash start
        bootWriter.enter_boot()
        ser_com = serial_com(server_name)
        with open(bin_file_name, 'rb') as f:
            bin_to_write = f.read()
            bin_len = len(bin_to_write)
            print(bin_len)
            pack_num = 0xFFFF

            header = bin_len.to_bytes(4, 'little') + pack_num.to_bytes(2, 'little') + 'H'.encode()
            ser_com.tx_raw(header)

            rx_str = b''
            isStart = 0
            bank_size = 0

            while (1): # start header
                rxb = ser_com.read()
                rx_str += rxb
                print("%02X"%int.from_bytes(rxb, 'little'))
                
                if len(rx_str) == 7:
                    if rxb.decode() == 'S':
                        print("start (head ok)")
                        isStart = 1
                    elif rxb.decode() == 'E':
                        print("err head")
                        isStart = -1
                    
                    bank_size = int.from_bytes(rx_str[0:2], 'little', signed=False)
                    print("ech bank has size : (%d)"%bank_size)
                    break
            # if False:
            if isStart > 0: # send pack to flash
                for pack_num in range(bin_len // bank_size + 1):
                    ret = ''
                    while ret != 'POK\n':
                        bin_len = len(bin_to_write)
                        if (bin_len > bank_size):
                            bin_len = bank_size
                        header = bin_len.to_bytes(4, 'little') + pack_num.to_bytes(2, 'little') + 'D'.encode()
                        ser_com.tx_raw(header + bin_to_write[0:bin_len])

                        rx_str = b''
                        while (1):
                            rxb = ser_com.read()
                            rx_str += rxb

                            if rxb.decode() == '\n':
                                ret = rx_str.decode()
                                print("(page %d)"%(pack_num), end=' ')
                                if ret != 'POK\n':
                                    print("unexpect recieve: ", ret[:-1])
                                else :
                                    print()
                                break
                    bin_to_write = bin_to_write[bin_len:]
                # go to app
                bootWriter.exit_boot()
                bin_len = 0
                pack_num = 0xFFFF
                header = bin_len.to_bytes(4, 'little') + pack_num.to_bytes(2, 'little') + 'E'.encode()
                ser_com.tx_raw(header)
                header = bin_len.to_bytes(4, 'little') + pack_num.to_bytes(2, 'little') + 'R'.encode()
                ser_com.tx_raw(header)


                    

            

    