cmake_minimum_required(VERSION 3.8.2)

project(rockface)

install(DIRECTORY sdk/rockface-${COMPILE_PLATFORM}/include/ DESTINATION include/rockface)
install(DIRECTORY sdk/rockface-${COMPILE_PLATFORM}/${COMPILE_LIB}/ DESTINATION lib)

if(${COMPILE_PLATFORM} STREQUAL "rv1109-Linux")
set(ROCKFACE_DATA_DIR sdk/rockface-data-rv1109)
else()
set(ROCKFACE_DATA_DIR sdk/rockface-data)
endif()

if(${COMPILE_PLATFORM} STREQUAL "rv1109-Linux")

if(${WITH_FACE_DETECTION})
    install(FILES ${ROCKFACE_DATA_DIR}/face_detection.data DESTINATION lib)
endif()

if(${WITH_FACE_DETECTION_V2})
    install(FILES ${ROCKFACE_DATA_DIR}/face_detection_v2.data DESTINATION lib)
endif()

if(${WITH_FACE_DETECTION_V3})
    install(FILES ${ROCKFACE_DATA_DIR}/face_detection_v3.data DESTINATION lib)
endif()

if(${WITH_FACE_DETECTION_V3_FAST})
    install(FILES ${ROCKFACE_DATA_DIR}/face_detection_v3_fast.data DESTINATION lib)
endif()

if(${WITH_FACE_DETECTION_V3_LARGE})
    install(FILES ${ROCKFACE_DATA_DIR}/face_detection_v3_large.data DESTINATION lib)
endif()

if(${WITH_FACE_RECOGNITION})
    install(FILES ${ROCKFACE_DATA_DIR}/face_recognition.data DESTINATION lib)
endif()

if(${WITH_FACE_LANDMARK})
    install(FILES ${ROCKFACE_DATA_DIR}/face_landmark5.data DESTINATION lib)
    install(FILES ${ROCKFACE_DATA_DIR}/face_landmarks106.data DESTINATION lib)
endif()

if(${WITH_FACE_ATTRIBUTE})
    install(FILES ${ROCKFACE_DATA_DIR}/face_attribute.data DESTINATION lib)
endif()

if(${WITH_LIVING_DETECTION})
    install(FILES ${ROCKFACE_DATA_DIR}/face_liveness_2d.data DESTINATION lib)
endif()

if(${WITH_OBJECT_DETECTION})
    install(FILES ${ROCKFACE_DATA_DIR}/object_detection.data DESTINATION lib)
endif()

if(${WITH_MASK_CLASSIFY})
    install(FILES ${ROCKFACE_DATA_DIR}/face_mask_classify.data DESTINATION lib)
endif()

if(${WITH_MASK_LANDMARKS})
    install(FILES ${ROCKFACE_DATA_DIR}/face_mask_landmarks.data DESTINATION lib)
endif()

if(${WITH_MASK_RECOGNITION})
    install(FILES ${ROCKFACE_DATA_DIR}/face_mask_recognition.data DESTINATION lib)
endif()

if(${WITH_MASKS_DETECTION})
    install(FILES ${ROCKFACE_DATA_DIR}/face_masks_detection.data DESTINATION lib)
endif()

else()
    install(DIRECTORY ${ROCKFACE_DATA_DIR}/ DESTINATION lib)
endif()
