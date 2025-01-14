#ifndef CONFIGURATIONHANDLER_H
#define CONFIGURATIONHANDLER_H

#include <opencv2/opencv.hpp>

#define NX 480.0f
#define NY 640.0f
#define NZ 300.0f

namespace StepsSettings
{
    enum Steps
    {
        CONFIGCOLOR1,
        CONFIGCOLOR2,
        CONFIGZMIN,
        CONFIGZMAX,
        CONFIGDONE,
        CONFIGEXIT
    };
}

#endif