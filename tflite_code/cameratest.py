from picamera import PiCamera
import time
from tflitepicturetest import *

start_time = time.time()
camera = PiCamera()
time.sleep(2)

# camera parameters settings
camera.resolution = (1280, 720)
camera.capture("img.jpg")
print("Photo captured, recognizing......")

(label, x, y, w, h) = object_detection_from_picture()
print("Object detection done.")
print(label, x, y, w, h)
label_str = "label " + label + '\n'
x_str = "x " + str(x) + '\n'
w_str = "w " + str(w) + '\n'
h_str = "h " + str(h) + '\n'
print(label_str + x_str + w_str + h_str)
end_time = time.time()
print("time spent:", end_time-start_time)
