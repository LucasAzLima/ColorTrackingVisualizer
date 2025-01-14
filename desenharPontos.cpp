#include "OpenGLHandler.h"
#include "OpenCVHandler.h"
#include <thread>

int main(int argc, char **argv) {
    std::thread opencvThread(processOpenCV); // Inicia OpenCV em outra thread

    initOpenGL(argc, argv); // Inicia OpenGL (loop bloqueante)

    opencvThread.join(); // Espera a thread do OpenCV finalizar

    return 0;
}
