# Imports
from tflite_support.task import vision
from tflite_support.task import core
from tflite_support.task import processor

def object_detection(target):

    print()
    print("Object detection start.")
    print()
    print("Target:", target)
    print()
    print("Model start initializing......")
    # Initialization
    model_path = "recog.tflite"
    base_options = core.BaseOptions(file_name=model_path)
    detection_options = processor.DetectionOptions(score_threshold=0.25)
    options = vision.ObjectDetectorOptions(base_options=base_options, detection_options=detection_options)
    detector = vision.ObjectDetector.create_from_options(options)
    print("Model finished initialized")
    print()
    # Alternatively, you can create an object detector in the following manner:
    # detector = vision.ObjectDetector.create_from_file(model_path)
    
    # Run inference
    print("Object detecting......")
    image_path = "img.jpg"
    image = vision.TensorImage.create_from_file(image_path)
    detection_result = detector.detect(image)
    for the_object in detection_result.detections:
        if (the_object.classes[0].class_name == target):
            print("Object found! Returning......")
            print()
            print("Object detection end.")
            print()
            return(the_object.classes[0].class_name, the_object.bounding_box.origin_x, the_object.bounding_box.origin_y, the_object.bounding_box.width, the_object.bounding_box.height)
    print("Object not found! Returning......")
    print()
    print("+++++++++++++++++ Object detection FAILED!!!!!!!! +++++++++++++++++++++++")
    print()
    return("fail", -1, -1, -1, -1)

if __name__ == "__main__":
    object_detection("soy")
    
'''
For reference:
    origin_x: The X coordinate of the top-left corner, in pixels.
    origin_y: The Y coordinate of the top-left corner, in pixels.
    width: The width of the bounding box, in pixels.
    height: The height of the bounding box, in pixels.
    
Image Size:  1280x720 
'''
