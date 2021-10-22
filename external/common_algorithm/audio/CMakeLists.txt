#1.cmake version
cmake_minimum_required(VERSION 3.2)

if (AEC_ANR_AGC_ENABLE OR ANR_ENABLE)
	add_subdirectory(rkap_common)
endif()

if (AEC_ANR_AGC_ENABLE)
	add_subdirectory(rkap_3a)
endif()

if (ANR_ENABLE)
	add_subdirectory(rkap_anr)
endif()

if (HDAEC)
	add_subdirectory(HDAEC)
endif()

if (TWO_MIC_BEAMFORM_ENABLE)
	add_subdirectory(Two_mic_beamform)
endif()
