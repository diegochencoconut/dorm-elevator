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

object_detection_from_picture()
print("Object detection done.")
end_time = time.time()
print("time spent:", end_time-start_time)
