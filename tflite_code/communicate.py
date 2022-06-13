import serial
import time
import sys
import traceback
from cameratest import *

status = "finding"

# timeout time in seconds
timeout = 0.05

def wait_cmd(ser, cmd_target):
    global status
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
                if (status == "finding"):
                    status = "go home"
                elif (status == "go home"):
                    status = "finding"
                print("status = ", status)
                break
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

                if (line in target):
                    print("Target set as", line)
                    print("======== wait_target finish ========")
                    return line

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
     

if __name__ == "__main__":
    
    # waiting the Arduino Mega to be prepared
    try:
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout = timeout)
        ser.reset_input_buffer()
    except:
        print("ACM failed.")
        try:
            ser = serial.Serial('/dev/tty/USB0', 9600, timeout = timeout)
            ser.reset_input_buffer()
        except:
            print("USB failed.")
            try:
                ser = serial.Serial('/dev/tty/AMA0', 9600, timeout = timeout)
                ser.reset_input_buffer()
            except:
                print("Failed")
                sys.exit(1)

    print("Serial connected.")
    print()
    
    try:
        wait_cmd(ser, "Setup finish")
        while (True):
            target = wait_target(ser) 
            while(status == "finding"):
                # waiting receiving "ready"
                wait_cmd(ser, "ready")             
                if (status == "go home"):
                    break

                # sending what picamera received
                msg = find_target(target)
                if (msg == "fail"):
                    print("Can't find the target. Mission failed.")
                    write_cmd(ser, "fail")
                    status = "go home"
        
                else:
                    write_cmd(ser, msg)
                
                if status == "go home":
                    break
                # waiting receiving "received"
                else:
                    wait_cmd(ser, "received")

            print("============================== GO HOME ==============================")
            while(status == "go home"):
                wait_cmd(ser, "ready")

                if (status == "finding"):
                    break

                #sending what picamera received
                msg = find_target("black_tea")
                if (msg == "fail"):
                    input("Can't find the target. Move me back to home and press any key to continue")
                    write_cmd(ser, "fail")
                    status = "finding"

                else:
                    write_cmd(ser, msg)

                if status == "finding":
                    break
                else:
                    wait_cmd(ser, "received")
    
    except Exception:
        ser.close()
        traceback.print_exc()
        
