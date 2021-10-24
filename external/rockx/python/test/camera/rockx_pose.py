import math
import time
import argparse

from rockx import RockX
import cv2

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Pose Demo")
    parser.add_argument('-c', '--camera', help="camera index", type=int, default=0)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    pose_body_handle = RockX(RockX.ROCKX_MODULE_POSE_BODY, target_device=args.device)

    cap = cv2.VideoCapture(args.camera)
    cap.set(3, 1280)
    cap.set(4, 720)
    last_face_feature = None

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()

        in_img_h, in_img_w = frame.shape[:2]

        start = time.time()
        ret, results = pose_body_handle.rockx_pose_body(frame, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)
        end = time.time()
        print('pose body use: ', end - start)

        index = 0
        for result in results:
            for p in result.points:
                cv2.circle(frame, (p.x, p.y), 3, (0, 255, 0), 3)
            for pairs in RockX.ROCKX_POSE_BODY_KEYPOINTS_PAIRS:
                pt1 = result.points[pairs[0]]
                pt2 = result.points[pairs[1]]
                if pt1.x <= 0 or pt1.y <= 0 or pt2.x <= 0 or pt2.y <= 0:
                    continue
                cv2.line(frame, (pt1.x, pt1.y), (pt2.x, pt2.y), (255, 0, 0), 2)
            index += 1

        # Display the resulting frame
        cv2.imshow('RockX Pose - ' + str(args.device), frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()

    pose_body_handle.release()
