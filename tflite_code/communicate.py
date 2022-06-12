import serial
import time

# timeout time in seconds
timeout = 0.05

if __name__ == "__main__":
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout = timeout)
    ser.reset_input_buffer()

    while True:
        line = ser.readline().decode('utf-8').rstrip()
        if len(line) > 0:
            print(line)
        time.sleep(timeout)
