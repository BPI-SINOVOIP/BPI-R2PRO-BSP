import math
import time
import argparse

from rockx import RockX
import cv2

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Finger Demo")
    parser.add_argument('-c', '--camera', help="camera index", type=int, default=0)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    pose_finger_handle = RockX(RockX.ROCKX_MODULE_POSE_FINGER_3, target_device=args.device)

    cap = cv2.VideoCapture(args.camera)
    cap.set(3, 1280)
    cap.set(4, 720)
    last_face_feature = None

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()

        in_img_h, in_img_w = frame.shape[:2]

        ret, result = pose_finger_handle.rockx_pose_finger(frame, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)
        for p in result.points:
            cv2.circle(frame, (p.x, p.y), 3, (0, 255, 0), 3)

        # Display the resulting frame
        cv2.imshow('RockX Finger - ' + str(args.device), frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()

    pose_finger_handle.release()
