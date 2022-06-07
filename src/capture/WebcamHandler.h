//
// Created by vicente on 6/7/22.
//

#ifndef FLIRMJPEG_WEBCAMHANDLER_H
#define FLIRMJPEG_WEBCAMHANDLER_H

#include <opencv2/opencv.hpp>
#include "Handler.h"

class WebcamHandler : public Handler {

public:
    bool init();
    bool prepareAcquisition();
    cv::Mat acquireImage();
    bool deinit();

private:
    cv::VideoCapture cap;
};


#endif //FLIRMJPEG_WEBCAMHANDLER_H
