//
// Created by vicente on 13/08/20.
//

#ifndef SPINNAKERHANDLER_H
#define SPINNAKERHANDLER_H


#include <memory>
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "Handler.h"


using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class SpinnakerHandler : public Handler, public std::enable_shared_from_this<SpinnakerHandler> {
private:
    SystemPtr _system;
    CameraList _camList;
    unsigned int _numCameras;
    CameraPtr _pCam = nullptr;
    INodeMap  *_nodeMapTLDevice;
    INodeMap *_nodeMap;

public:
    bool init();
    bool prepareAcquisition();
    //ImagePtr acquireImage();
    cv::Mat acquireImage();
    bool deinit();

};


#endif //SPINNAKERHANDLER_H
