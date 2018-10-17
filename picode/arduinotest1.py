#!/user/bin/env python

import serial
import sys
port_ext = "ACM0"
if len(sys.argv) > 1:
    port_ext = sys.argv[1]
port = "/dev/tty" + port_ext
rate = 9600

s1 = serial.Serial(port,rate)
s1.flushInput()

print("begin waiting")
s1.read_until()
print("Enter a number to square")
x = input()

s1.write(str(x).encode('utf-8'))
print(s1.read_until().decode("utf-8"))
