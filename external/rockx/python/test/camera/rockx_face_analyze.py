import math
import time
import argparse

from rockx import RockX
import cv2

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="RockX Face Analyze Demo")
    parser.add_argument('-c', '--camera', help="camera index", type=int, default=0)
    parser.add_argument('-d', '--device', help="target device id", type=str)
    args = parser.parse_args()

    face_det_handle = RockX(RockX.ROCKX_MODULE_FACE_DETECTION, target_device=args.device)
    face_landmark68_handle = RockX(RockX.ROCKX_MODULE_FACE_LANDMARK_68, target_device=args.device)
    face_landmark5_handle = RockX(RockX.ROCKX_MODULE_FACE_LANDMARK_5, target_device=args.device)
    face_attr_handle = RockX(RockX.ROCKX_MODULE_FACE_ANALYZE, target_device=args.device)

    cap = cv2.VideoCapture(args.camera)
    cap.set(3, 1280)
    cap.set(4, 720)
    last_face_feature = None

    while True:
        ret, frame = cap.read()

        show_frame = frame.copy()

        in_img_h, in_img_w = frame.shape[:2]
        start = time.time()
        ret, results = face_det_handle.rockx_face_detect(frame, in_img_w, in_img_h, RockX.ROCKX_PIXEL_FORMAT_BGR888)
        end = time.time()
        print('face detect use: ', end - start)

        index = 0
        for result in results:
            # face landmark
            start = time.time()
            ret, landmark = face_landmark68_handle.rockx_face_landmark(frame, in_img_w, in_img_h,
                                                                       RockX.ROCKX_PIXEL_FORMAT_BGR888,
                                                                       result.box)
            end = time.time()
            print('face landmark use: ', end - start)
            # face align
            ret, align_img = face_landmark5_handle.rockx_face_align(frame, in_img_w, in_img_h,
                                                                     RockX.ROCKX_PIXEL_FORMAT_BGR888,
                                                                     result.box, None)
            # face attribute
            ret, face_attribute = face_attr_handle.rockx_face_attribute(align_img)
            # face pose
            ret, face_angle = face_landmark68_handle.rockx_face_pose(landmark)

            # draw
            cv2.rectangle(show_frame,
                          (result.box.left, result.box.top),
                          (result.box.right, result.box.bottom),
                          (0, 255, 0), 2)
            if align_img is not None and index < 3:
                show_align_img = cv2.cvtColor(align_img, cv2.COLOR_RGB2BGR)
                show_frame[10+112*index:10+112*index+112, 10:112+10] = show_align_img
            if face_attribute is not None:
                cv2.putText(show_frame, "g:%d a:%d" % (face_attribute.gender, face_attribute.age), (result.box.left, result.box.top-10),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))
            if face_angle is not None and landmark.landmarks_count > 0:
                print('face angle: %f %f %f' % (face_angle.pitch, face_angle.yaw, face_angle.roll))
                cv2.putText(show_frame, "p=%.0f y=%.0f r=%.0f" % (face_angle.pitch, face_angle.yaw, face_angle.roll), (result.box.left, result.box.bottom+30),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))
            for p in landmark.landmarks:
                cv2.circle(show_frame, (p.x, p.y), 1, (0, 255, 0), 2)
            index += 1

        # Display the resulting frame
        cv2.imshow('RockX Face Analyze - ' + str(args.device), show_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()

    face_det_handle.release()
    face_landmark5_handle.release()
    face_landmark68_handle.release()
    face_attr_handle.release()
