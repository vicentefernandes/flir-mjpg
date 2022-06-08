
#include <memory>
#include <cassert>
///TODO #include "capture/SpinnakerHandler.h"
#include "stream/MJPEGStreamer.h"
#include "capture/WebcamHandler.h"
#include "capture/SpinnakerHandler.h"

int main()
{
    auto port = 7777;
    auto fps = 25;
    //Setup Capture
//    std::shared_ptr<Handler> spin = std::make_shared<WebcamHandler>();
    std::shared_ptr<Handler> spin = std::make_shared<SpinnakerHandler>();

    spin->init();
    spin->prepareAcquisition();

    //Streaming
    MJPEGStreamer mjpeg(spin, port,fps);
    mjpeg.run();
    assert(false);
    spin->deinit();


}
