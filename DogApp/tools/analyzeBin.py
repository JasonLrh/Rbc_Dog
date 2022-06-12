import json

if __name__ == '__main__':
    with open("config.json", 'r') as f:
        dic = json.loads(f.read())
        server_name = dic['host']
        bin_file_name = dic['firmware']
    
    with open(bin_file_name, 'rb') as f:
        bin_to_write = f.read()
        bin_len = len(bin_to_write)

        cnt = 0
        while True:
            print("%d : %02X"%(cnt,int.from_bytes(bin_to_write[0:1], 'little')))
            cnt += 1
            if len(bin_to_write) > 256:
                bin_to_write = bin_to_write[256:]
            else:
                break
    