#ifndef OPENGLHANDLER_H
#define OPENGLHANDLER_H

#include "ConfigurationHandler.h"
#include <GL/glut.h>
#include <vector>
#include <mutex>
#include <tuple>

extern float cubeX, cubeY, cubeZ, cubeSize, step;
extern bool drawing;
extern float rotationX, rotationY;
extern std::vector<std::tuple<float, float, float>> visitedPositions;
extern std::mutex positionMutex, cubeMutex;
extern int matX, matY, matZ, zMin, zMax;
extern enum StepsSettings::Steps configStep;

void initOpenGL(int argc, char **argv);
void drawGrid();
void drawVisitedCubes();
void drawMovingCube();
void display();
void mouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);
void updateCubePosition();

#endif
