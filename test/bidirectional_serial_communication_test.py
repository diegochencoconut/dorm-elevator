import serial
import time

if __name__ == "__main__":
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout = 1)
    ser.reset_input_buffer()
    
    while True:
        message = str(input("Message: ")) + '\n'
        message = bytes(message, encoding = "utf-8")
        print(message)
        ser.write(message)
        line = ser.readline().decode('utf-8').rstrip()
        print(line)
        time.sleep(1)
