import numpy as np
import cv2
from rknn.api import RKNN

from yolov3_utils import yolov3_post_process, draw, download_yolov3_weight

GRID0 = 13
GRID1 = 26
GRID2 = 52
LISTSIZE = 85
SPAN = 3

if __name__ == '__main__':

    MODEL_PATH = './yolov3.cfg'
    WEIGHT_PATH = './yolov3.weights'
    RKNN_MODEL_PATH = './yolov3_416.rknn'
    im_file = './dog_bike_car_416x416.jpg'
    DATASET = './dataset.txt'

    # Download yolov3.weight
    download_yolov3_weight(WEIGHT_PATH)

    # Create RKNN object
    rknn = RKNN()
    
    # pre-process config
    print('--> config model')
    rknn.config(mean_values=[0, 0, 0], std_values=[255, 255, 255])
    print('done')

    # Load tensorflow model
    print('--> Loading model')
    ret = rknn.load_darknet(model=MODEL_PATH,
                            weight=WEIGHT_PATH,
                            )
    if ret != 0:
        print('Load yolov3 failed! Ret = {}'.format(ret))
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset=DATASET)
    if ret != 0:
        print('Build yolov3 failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export RKNN model')
    ret = rknn.export_rknn(RKNN_MODEL_PATH)
    if ret != 0:
        print('Export yolov3.rknn failed!')
        exit(ret)
    print('done')

    # Set inputs
    img = cv2.imread(im_file)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    print('--> Init runtime environment')
    ret = rknn.init_runtime()
    if ret != 0:
        print('Init runtime environment failed')
        exit(ret)
    print('done')

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[img])
    print('done')


    input0_data = outputs[0]
    input1_data = outputs[1]
    input2_data = outputs[2]

    input0_data = input0_data.reshape(SPAN, LISTSIZE, GRID0, GRID0)
    input1_data = input1_data.reshape(SPAN, LISTSIZE, GRID1, GRID1)
    input2_data = input2_data.reshape(SPAN, LISTSIZE, GRID2, GRID2)

    input_data = []
    input_data.append(np.transpose(input0_data, (2, 3, 0, 1)))
    input_data.append(np.transpose(input1_data, (2, 3, 0, 1)))
    input_data.append(np.transpose(input2_data, (2, 3, 0, 1)))

    boxes, classes, scores = yolov3_post_process(input_data)

    image = cv2.imread(im_file)
    if boxes is not None:
        draw(image, boxes, scores, classes)

    print('Save results to results.jpg!')
    cv2.imwrite('results.jpg', image)

    rknn.release()

