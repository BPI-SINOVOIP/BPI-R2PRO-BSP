import argparse
from rockx import RockX
import cv2

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Object Detection Demo")
    parser.add_argument('-c', '--camera', help="camera index", type=int, default=0)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    object_det_handle = RockX(RockX.ROCKX_MODULE_OBJECT_DETECTION, target_device=args.device)

    cap = cv2.VideoCapture(args.camera)
    cap.set(3, 1280)
    cap.set(4, 720)
    last_face_feature = None

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()

        in_img_h, in_img_w = frame.shape[:2]

        ret, results = object_det_handle.rockx_face_detect(frame, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)

        for result in results:
            cv2.rectangle(frame,
                          (result.box.left, result.box.top),
                          (result.box.right, result.box.bottom),
                          (0, 255, 0), 2)
            cv2.putText(frame, "%s" % RockX.ROCKX_OBJECT_DETECTION_LABELS_91[result.cls_idx],
                        (result.box.left, result.box.top - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))

        # Display the resulting frame
        cv2.imshow('RockX Object Detection - ' + str(args.device), frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()

    object_det_handle.release()
