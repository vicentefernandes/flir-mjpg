//
// Created by vicente on 11/18/21.
//


#include <thread>
#include "MJPEGStreamer.h"
#include "MJPEGWriter.h"

void MJPEGStreamer::run() {
    Streamer::MJPEGWriter test(_port, _fps); //TODO remane

    auto frame = _handler->acquireImage();
    //cv::Mat frame(img.height, img.width, CV_8UC3, img.rawData.data(), img.width * 3);
    //cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);

    test.write(frame);
    frame.release();
    test.start();
    assert(_fps);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/_fps));
        auto frame = _handler->acquireImage();
        if (frame.total() < 32 * 32 * 3) {
            continue;
        }

        static long cnt = 0;
///    cv::Mat framew(djiImg.height, djiImg.width, CV_8UC3, djiImg.rawData.data(), djiImg.width * 3);
///    cv::cvtColor(framew, framew, cv::COLOR_RGB2BGR);

        /// test.write(framew);
        test.write(frame);
        frame.release();
    }
    test.stop();
    exit(0);
}
