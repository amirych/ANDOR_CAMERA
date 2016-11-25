#include "andor_camera.h"
#include "andorsdk_exception.h"


ANDOR_Camera::WaitBufferThread::WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout):
    cameraPtr(camera), waitBufferTimeout(timeout)
{

}


void ANDOR_Camera::WaitBufferThread::run()
{
    QString log_msg;

    if ( cameraPtr == nullptr ) {
        log_msg = "Andor camera object is nullptr in WaitBufferThread!";
        cameraPtr->printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, log_msg);
        return;
    }


    try {
        QString buff_addr, isize_addr;
        AT_U8* buff;
        int buff_size;



        for ( long currentExposureCounter = 0; currentExposureCounter < cameraPtr->imageBufferListSize; ++currentExposureCounter) {
            buff_addr.sprintf("%p",&buff);
            isize_addr.sprintf("%p",&buff_size);
            log_msg = QString("AT_WaitBuffer(%1,%2,%3,%4)").arg(cameraPtr->cameraHndl).arg(buff_addr).
                    arg(isize_addr).arg(waitBufferTimeout);
            andor_sdk_assert(AT_WaitBuffer(cameraPtr->cameraHndl,&buff,&buff_size,waitBufferTimeout),log_msg);
        }

    } catch (AndorSDK_Exception &ex) {
        cameraPtr->logToFile(ex);
    }
}
