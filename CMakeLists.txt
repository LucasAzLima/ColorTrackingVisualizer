cmake_minimum_required(VERSION 3.0)
project( desenharPontos )

find_package( OpenCV REQUIRED )
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

add_executable( desenharPontos desenharPontos.cpp OpenCVHandler.cpp OpenGLHandler.cpp ConfigurationHandler.cpp )

target_link_libraries( desenharPontos ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})