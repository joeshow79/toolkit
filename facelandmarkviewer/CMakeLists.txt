cmake_minimum_required(VERSION 2.8)
project( facelandmarkviewer )
find_package( OpenCV REQUIRED )
add_executable( facelandmarkviewer face_landmark_viewer.cpp )
target_link_libraries( facelandmarkviewer ${OpenCV_LIBS} )
