//
// Created by vicente on 6/7/22.
//

#include "WebcamHandler.h"

bool WebcamHandler::init() {
    return true;
}

bool WebcamHandler::prepareAcquisition() {
    return cap.open(0);
}

cv::Mat WebcamHandler::acquireImage() {
    cv::Mat frame;
    cap >> frame;
    return std::move(frame);
}

bool WebcamHandler::deinit() {
    cap.release();
    return true;
}
