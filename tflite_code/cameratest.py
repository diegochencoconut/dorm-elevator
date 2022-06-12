from picamera import PiCamera
import time
from tflitepicturetest import *

camera = PiCamera()
time.sleep(2)

camera.capture("img.jpg")
print("Done.")

object_detection_from_picture()