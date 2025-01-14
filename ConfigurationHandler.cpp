#include "ConfigurationHandler.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace cv;
using namespace StepsSettings;

float timerCount = 5.0;
double gammaValue = 100.0, currentTime = 0;

pair<Mat, Mat> captureAverageColor(Mat &image, Rect region)
{
    Mat diff, mask, imgGray;

    Mat roi(image, region);
    cvtColor(roi, imgGray, COLOR_BGR2GRAY);
    threshold(imgGray, mask, 150, 255, THRESH_BINARY);
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Mat contourMask = Mat::zeros(imgGray.size(), CV_8UC1);
    drawContours(contourMask, contours, -1, Scalar(255, 255, 255), FILLED);

    return make_pair(roi, contourMask);
}

Mat applyGammaCorrection(const Mat &inputImage)
{
    Mat lookUpTable(1, 256, CV_8U);
    uchar *p = lookUpTable.ptr();
    for (int i = 0; i < 256; ++i)
    {
        p[i] = saturate_cast<uchar>(pow(i / 255.0, gammaValue) * 255.0);
    }

    Mat result;
    LUT(inputImage, lookUpTable, result);
    return result;
}

pair<Mat, Mat> configureColors(VideoCapture &cap, Mat &image, double &start, double &frequency, Steps &configStep)
{
    int width = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);

    Rect topLeftRegion(0, 0, 100, 100);            // Região no canto superior esquerdo
    Rect topRightRegion(width - 100, 0, 100, 100); // Região no canto superior direito
    currentTime = (getTickCount() - start) / frequency;
    if (currentTime >= 3.0)
    {
        start = getTickCount();

        switch (configStep)
        {
        case CONFIGCOLOR1:
            configStep = CONFIGCOLOR2;
            break;
        case CONFIGCOLOR2:
            configStep = CONFIGZMIN;
            break;
        case CONFIGZMIN:
            configStep = CONFIGZMAX;
            break;
        case CONFIGZMAX:
            configStep = CONFIGDONE;
            break;
        default:
            break;
        }
    }
    Mat gammaImage = applyGammaCorrection(image);
        switch (configStep)
        {
        case CONFIGCOLOR1:
            rectangle(image, topLeftRegion, Scalar(255, 0, 0), 2);
            putText(image, "Defina a cor 1", Point(170, 40), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            putText(image, to_string(static_cast<int>(4.0 - currentTime)), Point(45, 55), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            return captureAverageColor(gammaImage, topLeftRegion);
            break;
        case CONFIGCOLOR2:
            rectangle(image, topRightRegion, Scalar(255, 0, 0), 2);
            putText(image, "Defina a cor 2", Point(170, 40), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            putText(image, to_string(static_cast<int>(4.0 - currentTime)), Point(width - 55, 55), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            return captureAverageColor(gammaImage, topRightRegion);
            break;
        case CONFIGZMIN:
            putText(image, "Se aproxime da tela", Point(170, 40), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            putText(image, to_string(static_cast<int>(4.0 - currentTime)), Point(width / 2, height / 2), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            break;
        case CONFIGZMAX:
            putText(image, "Se afaste da tela", Point(170, 40), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            putText(image, to_string(static_cast<int>(4.0 - currentTime)), Point(width / 2, height / 2), FONT_HERSHEY_DUPLEX, 1.0, Scalar(255, 0, 0), 2);
            break;
        default:
            break;
        }
}