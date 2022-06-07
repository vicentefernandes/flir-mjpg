//
// Created by vince on 13/08/20.
//

#include <iostream>
#include "SpinnakerHandler.h"

#include <iostream>
#include <sstream>
#include <memory>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;


bool SpinnakerHandler::init() {
    _system = System::GetInstance();
    /// Print out current library version
    const LibraryVersion spinnakerLibraryVersion = _system->GetLibraryVersion();
    cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
         << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl
         << endl;
    /// Retrieve list of cameras from the system
    _camList = _system->GetCameras();
    const unsigned int numCameras = _camList.GetSize();
    cout << "Number of cameras detected: " << numCameras << endl << endl;
    // Finish if there are no cameras
    if (numCameras == 0)
    {
        /// Clear camera list before releasing system
        _camList.Clear();
        /// Release system
        _system->ReleaseInstance();
        cout << "Not enough cameras!" << endl;
        return false;
    }

    _pCam = nullptr;
    _pCam = _camList.GetByIndex(0); ///using first cam only.
    _nodeMapTLDevice = &_pCam->GetTLDeviceNodeMap();
    _pCam->Init();
    _nodeMap = &_pCam->GetNodeMap();

}



bool SpinnakerHandler::prepareAcquisition() {
    try {
        CEnumerationPtr ptrAcquisitionMode = _nodeMap->GetNode("AcquisitionMode");
        if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode)) {
            cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << endl << endl;
            return false;
        }
        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous)) {
            cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << endl << endl;
            return false;
        }
        const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
        cout << "Acquisition mode set to continuous..." << endl;

        _pCam->BeginAcquisition();

    }
    catch (Spinnaker::Exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    return true;
}



ImagePtr SpinnakerHandler::acquireImage() {
    static int count=0;
    count++;
    ImagePtr convertedImage;
    try{
        ImagePtr pResultImage = _pCam->GetNextImage((int) 2000);
        if (pResultImage->IsIncomplete())
        {
            // Retrieve and print the image status description
            cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
                 << "..." << endl
                 << endl;
        }
        else
        {

            const size_t width = pResultImage->GetWidth();
            const size_t height = pResultImage->GetHeight();
            cout << "Grabbed image " << count << " width = " << width << ", height = " << height
                 << " Format: " << pResultImage->GetPixelFormatName() << endl;
            //
            // Convert image to mono 8
            //
            // *** NOTES ***
            // Images can be converted between pixel formats by using
            // the appropriate enumeration value. Unlike the original
            // image, the converted one does not need to be released as
            // it does not affect the camera buffer.
            //
            // When converting images, color processing algorithm is an
            // optional parameter.
            //
            convertedImage = pResultImage->Convert(PixelFormat_BGR8, HQ_LINEAR);
            /// cout << "img converted to BGR8" << endl;
        }
        pResultImage->Release();
    }


    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        this->deinit();
    }
    return convertedImage;

}

bool SpinnakerHandler::deinit() {
    _pCam->EndAcquisition();
    _pCam = nullptr;
    _camList.Clear();
    _system->ReleaseInstance();
}
