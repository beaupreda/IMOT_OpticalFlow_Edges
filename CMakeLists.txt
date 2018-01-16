cmake_minimum_required(VERSION 2.8)

project(IMOT_OpticalFlow_Edges)

find_package(OpenCV REQUIRED)

add_subdirectory(of_dis)

include_directories(./of_dis ${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/main.cpp src/support/Place.cpp src/support/Line.cpp src/support/Argument.cpp
    src/support/FileOperations.cpp src/frame_processing/FrameProcessor.cpp src/flow_processing/FlowProcessor.cpp
    src/edge_processing/EdgeProcessor.cpp src/detection/Detector.cpp)

add_executable(IMOT_OpticalFlow_Edges ${SOURCE_FILES})

target_link_libraries(IMOT_OpticalFlow_Edges ${OpenCV_LIBS})