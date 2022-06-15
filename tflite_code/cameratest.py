from picamera import PiCamera
import time
from tflitepicturetest import *

attempt_times = 5

def find_target(target):
    print()
    print("======================= find_target start =============================")
    print()
    start_time = time.time()
    print()
    
    # camera parameters settings
    print("Initializing camera......")
    camera = PiCamera()
    
    # camera parameters settings
    camera.resolution = (1280, 720)
    camera.capture("img.jpg")
    saved_image_name = "debug/" + str(time.localtime()) + ".jpg"
    camera.capture(saved_image_name)
    print("Camera initialized.")
    print()

    global attempt_times
    for i in range (attempt_times):
        print()
        print("Start capturing photo......")
        camera.capture("img.jpg")
        print("Photo captured, recognizing......")
        print()
        print("Finding object: attempt", i+1, "of", attempt_times)
        print()
        (label, x, y, w, h) = object_detection(target)
        if (label != target):
            print("Object not found.")
        else:
            print("Object found!")
            label_str = "label " + label + '\n'
            x_str = "x " + str(x) + '\n'
            w_str = "w " + str(w) + '\n'
            h_str = "h " + str(h) + '\n'

            end_time = time.time()
            print("time spent:", end_time - start_time)
            result = label_str + x_str + w_str + h_str
            print()
            print("=================== find_target end =============================")
            camera.close()
            return result

    print("Object not found! Returning......")
    print()
    end_time = time.time()
    print("time spent:", end_time - start_time)
    print()
    print("+++++++++++++++++ FIND_TARGET FAIL!!!!!!!!!!!+++++++++++++++++++++++++")
    print()
    
    camera.close()
    return "fail"

if __name__ == "__main__":
    find_target("soy")
