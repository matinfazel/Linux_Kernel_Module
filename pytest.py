import os
import random
import threading

path = "/dev/iutnode"
fd = os.open(path, os.O_RDWR)

def exchange():
    for i in range(10**6):
        rand = random.randint(1,99)
        msg = f"e,{rand},0,1"
        os.write(fd, msg.encode())

t1 = threading.Thread(target=exchange)
t2 = threading.Thread(target=exchange)

t1.start()
t2.start()

t1.join()
t2.join()
print("Accounts Status:")
data = os.read(fd,1024)
print(data.decode())
os.close(fd)

