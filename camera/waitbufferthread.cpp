#include "andor_camera.h"
#include "andorsdk_exception.h"


ANDOR_Camera::WaitBufferThread::WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout):
    cameraPtr(camera), waitBufferTimeout(timeout)
{

}


void ANDOR_Camera::WaitBufferThread::run()
{
    if ( cameraPtr == nullptr ) {
        QString log_msg = "Andor camera object is nullptr in WaitBufferThread!";
        cameraPtr->printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, log_msg);
        return;
    }


}
