//
// Created by vicente on 6/7/22.
//

#ifndef FLIRMJPEG_HANDLER_H
#define FLIRMJPEG_HANDLER_H

#include <opencv2/opencv.hpp>

class Handler {
public:
    virtual bool init() = 0;
    virtual bool prepareAcquisition() = 0;
    //// virtual ImagePtr acquireImage() = 0;
        //cv::Mat frame(img.height, img.width, CV_8UC3, img.rawData.data(), img.width * 3);
        //cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
    virtual cv::Mat acquireImage() = 0;
    virtual bool deinit() = 0;

};


#endif //FLIRMJPEG_HANDLER_H
