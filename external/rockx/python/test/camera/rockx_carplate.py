import time
import argparse

import numpy as np
from PIL import ImageDraw, ImageFont, Image

from rockx import RockX
import cv2


def draw_cn_char(img, str, pos, color):
    img_PIL = Image.fromarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))

    font = ImageFont.truetype('/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc', 40)

    if not isinstance(str, np.unicode):
        str = str.decode('utf8')

    draw = ImageDraw.Draw(img_PIL)
    draw.text(pos, str, font=font, fill=color)

    img_OpenCV = cv2.cvtColor(np.asarray(img_PIL), cv2.COLOR_RGB2BGR)
    return img_OpenCV


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Carplate Demo")
    parser.add_argument('-c', '--camera', help="camera index", type=int, default=0)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    carplate_det_handle = RockX(RockX.ROCKX_MODULE_CARPLATE_DETECTION, target_device=args.device)
    carplate_align_handle = RockX(RockX.ROCKX_MODULE_CARPLATE_ALIGN, target_device=args.device)
    carplate_recog_handle = RockX(RockX.ROCKX_MODULE_CARPLATE_RECOG, target_device=args.device)

    cap = cv2.VideoCapture(args.camera)
    cap.set(3, 1280)
    cap.set(4, 720)
    last_face_feature = None

    while True:
        ret, frame = cap.read()

        show_frame = frame.copy()

        in_img_h, in_img_w = frame.shape[:2]
        start = time.time()
        ret, results = carplate_det_handle.rockx_carplate_detect(frame, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)
        end = time.time()
        print('face detect use: ', end - start)

        index = 0
        for result in results:
            cv2.rectangle(show_frame,
                          (result.box.left, result.box.top),
                          (result.box.right, result.box.bottom),
                          (0, 255, 0), 2)

            ret, align_result = carplate_align_handle.rockx_carplate_align(frame, in_img_w, in_img_h,
                                                                           RockX.ROCKX_PIXEL_FORMAT_RGB888, result.box)

            if align_result is not None:
                ret, recog_result = carplate_recog_handle.rockx_carplate_recognize(align_result.aligned_image)

                if recog_result is not None:
                    show_frame = draw_cn_char(show_frame, recog_result, (160 + 10, (40*index)), (255, 0, 0))
                    # cv2.putText(frame, str, (160 + 10, (10+40*index+20)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255))

                show_frame[(10+40*index):(10+40*index+40), 10:(160+10)] = align_result.aligned_image

            index += 1

        cv2.imshow('RockX Carplate - ' + str(args.device), show_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

    carplate_det_handle.release()
