//
// Created by vicente on 11/18/21.
//

#ifndef MJPEGSTREAMER_H
#define MJPEGSTREAMER_H

#include "../capture/Handler.h"

class MJPEGStreamer {
  public:
    MJPEGStreamer() = delete;
    MJPEGStreamer(std::shared_ptr<Handler> handler, int port, int fps)
        : _port(port), _fps(fps), _handler(std::move(handler)){};
    void run();

  private:
    int _port;
    int _fps;
    std::shared_ptr<Handler> _handler;
};

#endif // MJPEGSTREAMER_H
