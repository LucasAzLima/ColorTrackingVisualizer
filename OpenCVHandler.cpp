#include "OpenCVHandler.h"
#include "ConfigurationHandler.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;
using namespace cv;
using namespace StepsSettings;

enum Steps configStep = CONFIGCOLOR1;
Mat image, image2, maskColor1, maskColor2, currentMask, frame, gammaImage;
bool drawing = false;
double start = 0.0;
Scalar average1, average2, stdev1, stdev2;
int matX = 0, matY = 0, matZ = 0, zMin = 0, zMax = 0;

pair<Mat, Mat> configureColors(VideoCapture &cap, Mat &image, double &start, double &frequency, Steps &configStep);
Mat applyGammaCorrection(const Mat &inputImage);

void processOpenCV()
{
  VideoCapture cap(0);
  if (!cap.isOpened())
  {
    cerr << "Erro na captura" << endl;
    return;
  }

  cap.set(CAP_PROP_FRAME_WIDTH, NY);
  cap.set(CAP_PROP_FRAME_HEIGHT, NX);
  cap.set(CAP_PROP_AUTO_EXPOSURE, 0);
  cap.set(CAP_PROP_EXPOSURE, 100);
  cap.read(frame);
  frame = applyGammaCorrection(frame);

  namedWindow("Mask", WINDOW_AUTOSIZE);
  namedWindow("Gamma", WINDOW_AUTOSIZE);
  namedWindow("Contours", WINDOW_AUTOSIZE);
  namedWindow("Camera", WINDOW_AUTOSIZE);
  double start = getTickCount();
  double tickFrequency = getTickFrequency();

  while (true)
  {
    cap.read(image);
    flip(image, image, 1);

    char c = (char)waitKey(10);
    if (c == 27)
      break;

    image.copyTo(image2);
    auto result = configureColors(cap, image, start, tickFrequency, configStep);
    if (configStep == CONFIGCOLOR1 && record)
    {

      meanStdDev(result.first, average1, stdev1, result.second);
    }

    if (configStep == CONFIGCOLOR2)
    {

      meanStdDev(result.first, average2, stdev2, result.second);
    }

    if (configStep == CONFIGZMIN)
    {

      zMin = matZ;
    }
    if (configStep == CONFIGZMAX)
    {

      zMax = matZ;
    }
    if (configStep == CONFIGDONE)
    {
      cout << "Configs:" << endl;
      cout << average1 << ", " << stdev1 << endl;
      cout << average2 << ", " << stdev2 << endl;
      cout << zMin << ", " << zMax << endl;
      configStep = CONFIGEXIT;
    }

    imshow("Camera", image);

    gammaImage = applyGammaCorrection(image2);

    inRange(gammaImage, average1 - stdev1, average1 + stdev1, maskColor1);
    inRange(gammaImage, average2 - stdev2, average2 + stdev2, maskColor2);
    // cout << countNonZero(maskColor1) << " | " << countNonZero(maskColor2) << endl;

    if (countNonZero(maskColor1) > 40)
    {
      currentMask = maskColor1;
      drawing = true;
    }
    if (countNonZero(maskColor2) > 40)
    {
      currentMask = maskColor2;
      drawing = false;
    }

    vector<vector<Point>> contours;
    findContours(currentMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> hull(contours.size());
    for (size_t i = 0; i < contours.size(); i++)
    {
      convexHull(contours[i], hull[i]);
    }

    int largestContourIndex = -1;
    double largestContourArea = 0.0;
    for (int i = 0; i < contours.size(); i++)
    {
      double contourArea = cv::contourArea(hull[i]);
      if (contourArea > largestContourArea)
      {
        largestContourArea = contourArea;
        largestContourIndex = i;
      }
    }

    Mat contoursImage = Mat::zeros(frame.size(), CV_8UC3);

    if (largestContourIndex != -1 && largestContourArea > 40.0)
    {
      vector<Point> largestContour = contours[largestContourIndex];
      Point2f center;
      float radius;
      minEnclosingCircle(largestContour, center, radius);

      if (center.x < NY && center.y < NX && radius < NZ)
      {
        // cout << center.x << ", " << center.y << ", " << radius << endl;

        matX = (int)center.x;
        matY = (int)center.y;
        matZ = (int)radius;
      }
      circle(contoursImage, center, radius, Scalar(0, 255, 0), 2);
    }
    imshow("Contours", contoursImage);
    imshow("Mask", currentMask);
    imshow("Gamma", gammaImage);
  }
  cap.release();
  destroyAllWindows();
}
