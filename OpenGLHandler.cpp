#include "ConfigurationHandler.h"
#include "OpenGLHandler.h"
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;
using namespace StepsSettings;

float cubeX = 0.0f, cubeY = 0.0f, cubeZ = 0.0f, cubeSize = 0.2f;
float step = cubeSize;
std::vector<std::tuple<float, float, float>> visitedPositions;
std::mutex positionMutex, cubeMutex;
float rotationX = 0.0, rotationY = 0.0; // Ângulos de rotação para o cubo
int lastMouseX = -1, lastMouseY = -1;   // Posição anterior do mouse

void initOpenGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Caixa cúbica");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glutDisplayFunc(display);
    glutIdleFunc(updateCubePosition);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMainLoop();
}

void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        lastMouseX = x;
        lastMouseY = y;
    }
}

// Função chamada quando o mouse é movido com o botão pressionado
void mouseMotion(int x, int y)
{
    if (lastMouseX >= 0 && lastMouseY >= 0)
    {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        // Ajusta o ângulo de rotação com base no movimento do mouse
        rotationX += dy * 0.1;
        rotationY += dx * 0.1;

        lastMouseX = x;
        lastMouseY = y;
    }
    glutPostRedisplay();
}

bool positionVisited(float x, float y, float z)
{
    for (const auto &pos : visitedPositions)
    {
        if (abs(get<0>(pos) - x) < cubeSize &&
            abs(get<1>(pos) - y) < cubeSize &&
            abs(get<2>(pos) - z) < cubeSize)
        {
            return true;
        }
    }
    return false;
}

void drawVisitedCubes()
{
    if (configStep != CONFIGEXIT)
        return;
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    lock_guard<mutex> lock(positionMutex);

    for (const auto &pos : visitedPositions)
    {
        glPushMatrix();
        glTranslatef(get<0>(pos), get<1>(pos), get<2>(pos));
        glutSolidCube(cubeSize);
        glPopMatrix();
    }
}

// Função de exibição do OpenGL
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef(rotationX, 1.0, 0.0, 0.0); // Rotação no eixo X
    glRotatef(rotationY, 0.0, 1.0, 0.0); // Rotação no eixo Y

    drawGrid();         // Desenhar a grade
    drawVisitedCubes(); // Desenha os cubos visitados
    drawMovingCube();   // Desenhar o cubo em movimento

    glFlush();
}

// Função que desenha a grade 3D
void drawGrid()
{
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);

    for (float i = -1; i <= 1; i += step)
    {
        // Grade frontal
        glVertex3f(i, -1, -1);
        glVertex3f(i, 1, -1);
        glVertex3f(-1, i, -1);
        glVertex3f(1, i, -1);

        // Grade lateral direita
        glVertex3f(1, i, -1);
        glVertex3f(1, i, 1);
        glVertex3f(1, -1, i);
        glVertex3f(1, 1, i);

        // Grade inferior
        glVertex3f(i, -1, -1);
        glVertex3f(i, -1, 1);
        glVertex3f(-1, -1, i);
        glVertex3f(1, -1, i);

        // Grade superior
        glVertex3f(i, 1, -1);
        glVertex3f(i, 1, 1);
        glVertex3f(-1, 1, i);
        glVertex3f(1, 1, i);

        // Grade lateral esquerda
        glVertex3f(-1, i, -1);
        glVertex3f(-1, i, 1);
        glVertex3f(-1, -1, i);
        glVertex3f(-1, 1, i);
    }

    glEnd();
}

// Função que desenha o cubo em movimento
void drawMovingCube()
{
    if (configStep != CONFIGEXIT && drawing)
        return;
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glPushMatrix();

    lock_guard<mutex> lock(cubeMutex);
    glTranslatef(cubeX + cubeSize / 2, cubeY + cubeSize / 2, cubeZ + cubeSize / 2);
    glutSolidCube(0.2);

    glPopMatrix();
}

double escalarValor(double x, double xMin, double xMax, double step = 0.2)
{
    // Normaliza x para um valor entre 0 e 1
    double normalizedX = (x - xMin) / (xMax - xMin);

    // Transforma o valor normalizado para o intervalo [-1, 1]
    double scaledValue = -1 + normalizedX * 2;

    // Ajusta para o passo mais próximo
    scaledValue = round(scaledValue / step) * step;

    // Garante que o valor esteja dentro do intervalo [-1, 1]
    if (scaledValue < -1)
        scaledValue = -1;
    if (scaledValue > 1)
        scaledValue = 1;

    return scaledValue;
}

// Função de atualização de posição do cubo com OpenCV
void updateCubePosition()
{
    if (configStep != CONFIGEXIT)
        return;
    lock_guard<mutex> lock(cubeMutex);

    float newX = escalarValor(matX, 0, NY, cubeSize);
    float newY = -escalarValor(matY, 0, NX, cubeSize);
    float newZ = escalarValor(matZ, zMin, zMax, cubeSize);
    // cout << "scaled: " << newX << ", " << newY << ", " << newZ << endl;
    // cout << "original: " << matX << ", " << matY << ", " << matZ << endl;

    // Atualiza apenas se o cubo mudou de posição
    if (!positionVisited(newX, newY, newZ))
    {
        cubeX = newX;
        cubeY = newY;
        cubeZ = newZ;

        // Adiciona a nova posição à lista de posições visitadas
        lock_guard<mutex> posLock(positionMutex);
        if (drawing)
            visitedPositions.push_back(make_tuple(cubeX, cubeY, cubeZ));
    }

    glutPostRedisplay();
}
