cmake_minimum_required(VERSION 2.6)

find_package(OpenCV REQUIRED)
include_directories(/usr/local/include)
include_directories( ${OPENCV_INCLUDE_DIR} )
link_directories(/usr/local/lib)

add_library(FrameCompress  SHARED  H264Compress.cpp)

target_link_libraries(FrameCompress avcodec avdevice avfilter avformat avutil postproc swresample swscale x264)
target_link_libraries(FrameCompress ${OpenCV_LIBS})

install(TARGETS FrameCompress LIBRARY DESTINATION install/lib)
