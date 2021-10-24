
set(RockX_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

if (CMAKE_SYSTEM_NAME MATCHES "Android")
    set(RockX_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/${CMAKE_ANDROID_ARCH_ABI}/librknn_api.so
        ${CMAKE_CURRENT_LIST_DIR}/${CMAKE_ANDROID_ARCH_ABI}/librockx.so
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(RockX_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/lib64/librknn_api.dylib
        ${CMAKE_CURRENT_LIST_DIR}/lib64/librockx.dylib
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(RockX_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/lib64/librknn_api.dll
        ${CMAKE_CURRENT_LIST_DIR}/lib64/librockx.dll
    )
else ()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (TARGET_LIB_ARCH lib64)
    else()
        set (TARGET_LIB_ARCH lib)
    endif()
    set(RockX_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/librknn_api.so
        ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/librockx.so
    )
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/npu_transfer_proxy")
    file(GLOB NPU_TRANSFER_PROXY_FILES "${CMAKE_CURRENT_LIST_DIR}/lib/npu_transfer_proxy/*")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109")
    if(${WITH_ROCKX_CARPLATE_RELATIVE})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/carplate_align.data")
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/carplate_detection.data")
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/carplate_recognition.data")
    endif()
    if(${WITH_ROCKX_FACE_DETECTION})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_detection_v3.data")
    endif()
    if(${WITH_ROCKX_FACE_RECOGNITION})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_recognition.data")
    endif()
    if(${WITH_ROCKX_FACE_LANDMARK})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_landmarks68.data")
    endif()
    if(${WITH_ROCKX_FACE_ATTRIBUTE})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_attribute.data")
    endif()
    if(${WITH_ROCKX_HEAD_DETECTION})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/head_detection.data")
    endif()
    if(${WITH_ROCKX_OBJECT_DETECTION})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/object_detection.data")
    endif()
    if(${WITH_ROCKX_POSE_BODY})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/pose_body_v2.data")
    endif()
    if(${WITH_ROCKX_POSE_FINGER})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/pose_finger.data")
    endif()
    if(${WITH_ROCKX_POSE_HAND})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/pose_hand.data")
    endif()
    if(${WITH_ROCKX_PERSON_DETECTION})
        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/person_detection_v2.data")
    endif()
endif()

set(RockX_BINS ${NPU_TRANSFER_PROXY_FILES})
set(RockX_DATA ${ROCKX_DATA_FILES})
