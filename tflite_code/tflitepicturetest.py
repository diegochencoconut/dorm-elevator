# Imports
from tflite_support.task import vision
from tflite_support.task import core
from tflite_support.task import processor

# Initialization
model_path = "efficientdet_lite0.tflite"
base_options = core.BaseOptions(file_name=model_path)
detection_options = processor.DetectionOptions(score_threshold=0.25)
options = vision.ObjectDetectorOptions(base_options=base_options, detection_options=detection_options)
detector = vision.ObjectDetector.create_from_options(options)

# Alternatively, you can create an object detector in the following manner:
# detector = vision.ObjectDetector.create_from_file(model_path)

# Run inference
def object_detection_from_picture():
    image_path = "img.jpg"
    image = vision.TensorImage.create_from_file(image_path)
    detection_result = detector.detect(image)
    for the_object in detection_result.detections:
        print(the_object.classes[0].class_name)
        print(the_object.classes[0].score)
        print(the_object.bounding_box.origin_x)
        print(the_object.bounding_box.origin_y)
        print(the_object.bounding_box.width)
        print(the_object.bounding_box.height)
        print("============")
    
if __name__ == "__main__":
    object_detection_from_picture()
    
'''
For reference:
    origin_x: The X coordinate of the top-left corner, in pixels.
    origin_y: The Y coordinate of the top-left corner, in pixels.
    width: The width of the bounding box, in pixels.
    height: The height of the bounding box, in pixels.
    
Image Size: 3280x2464 (as default)
'''
