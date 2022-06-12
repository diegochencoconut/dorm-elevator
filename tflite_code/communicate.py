import serial
import time
from cameratest import *

# timeout time in seconds
timeout = 0.05

def wait_cmd(ser, cmd_target):
    print()
    print("========== wait_cmd running ==========")
    print()
    print("Waiting target command", cmd_target, "......")
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            if len(line) > 0:
                print("Received:", line)
            if (line == cmd_target):
                print("Target cmd received.")
                break
            if (line == "Gotcha"):
                go_home(ser)
            time.sleep(timeout)
    print()
    print("========== wait_cmd complete =========")
    print()
   

def wait_target(ser):
    print()
    print("========== wait_target running ========")
    print()
    print("Target cmd waiting......")
    target = ["black_tea", "green_tea", "soy", "chocolate"]
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            if len(line) > 0:
                print("Received:", line)

def write_cmd(ser, cmd):
    print()
    print("========== write_cmd running ===========")
    print()
    print("Command content:")
    print(cmd)
    cmd = bytes(cmd, encoding = "utf-8")
    ser.write(cmd)
    print()
    print("========== write_cmd complete ==========")
    print()
    ser.reset_output_buffer()

def go_home(ser):
    print()
    print("========= GO HOME :) =============")
    ser.close()

if __name__ == "__main__":
    
    # waiting the Arduino Mega to be prepared
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout = timeout)
    ser.reset_input_buffer()
    print("Serial connected.")
    print()

    for i in range(30):
        # waiting receiving "ready"
        wait_cmd(ser, "ready")
        
    
        # sending what picamera received
        msg = find_target("soy")
        if (msg == "fail"):
            print("Can't find the target. Mission failed.")
            write_cmd(ser, "fail")
            go_home(ser)
            break

        else:
            write_cmd(ser, msg)
    
        # waiting receiving "received"
        wait_cmd(ser, "received")

    go_home(ser)
