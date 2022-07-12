#!/usr/bin/env python3
import serial
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
        time.sleep(0.4)
        self.__send_cmd('EB')
        time.sleep(0.2)
        self.__com.read()
    
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

    bootWriter = cmd_com(server_name)

    if len(sys.argv) > 2:
        if sys.argv[2] == 'E':
            bootWriter.exit_boot()
            exit()

    if bin_file_name.endswith('.bin'):
        # write flash start
        bootWriter.enter_boot()
        ser_com = serial_com(server_name)
        with open(bin_file_name, 'rb') as f:
            bin_to_write = f.read()
            bin_len = len(bin_to_write)
            print("file size :" ,bin_len)
            pack_num = 0xFFFF

            header = bin_len.to_bytes(4, 'little') + pack_num.to_bytes(2, 'little') + 'H'.encode()
            ser_com.tx_raw(header)

            rx_str = b''
            isStart = 0
            bank_size = 0

            retry = 0

            print("Header :", end=' ', flush=True)
            while (1): # start header
                rxb = ser_com.read()
                rx_str += rxb
                print("%02X"%int.from_bytes(rxb, 'little'), end=' ', flush=True)
                
                if len(rx_str) == 7:
                    if rxb.decode() == 'S':
                        print("start (head ok)")
                        isStart = 1
                    elif rxb.decode() == 'E':
                        print("err head")
                        isStart = -1
                    elif retry < 3:
                        retry += 1
                        print("\nHeader :", end=' ', flush=True)
                        ser_com.tx_raw(header)
                        rx_str = b''
                        continue
                    else:
                        isStart = -1
                    bank_size = int.from_bytes(rx_str[0:2], 'little', signed=False)
                    print("ech bank has size : (%d)"%bank_size)
                    break
            # if False:
            if isStart > 0: # send pack to flash
                total_pack_to_send = bin_len // bank_size + 1
                for pack_num in range(total_pack_to_send):
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
                                progress = pack_num / total_pack_to_send
                                print("[\033[1;33mflashing\033[0m] [%-18s] %2.1f %%"%('=' * int(progress * 18) + '>', 100 * progress), end=' ', flush=True)
                                print("\r", end='', flush=False)
                                if ret != 'POK\n':
                                    print("\nunexpect recieve: ", ret[:-1])
                                # else :
                                #     print()
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
                print("[\033[1;32mflash down\033[0m] %d pack                     "%(total_pack_to_send))
            else:
                exit(1)