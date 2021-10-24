import argparse
from rockx import RockX
import cv2

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Object Detection Demo")
    parser.add_argument('-i', '--image', help="input image", type=str)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    object_det_handle = RockX(RockX.ROCKX_MODULE_OBJECT_DETECTION, target_device=args.device)

    in_img = cv2.imread(args.image)
    in_img_h, in_img_w = in_img.shape[:2]

    ret, results = object_det_handle.rockx_face_detect(in_img, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)

    for result in results:
        label = RockX.ROCKX_OBJECT_DETECTION_LABELS_91[result.cls_idx]
        print('%s (%d %d %d %d) %f' % (label, result.box.left, result.box.top, result.box.right, result.box.bottom, result.score))
        cv2.rectangle(in_img,
                        (result.box.left, result.box.top),
                        (result.box.right, result.box.bottom),
                        (0, 255, 0), 2)
        cv2.putText(in_img, "%s" % label,
                    (result.box.left, result.box.top - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))

    cv2.imwrite('out.jpg', in_img)

    object_det_handle.release()
