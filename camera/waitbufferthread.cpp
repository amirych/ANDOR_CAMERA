#include "waitbufferthread.h"
#include "andorsdk_exception.h"


WaitBufferThread::WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout):
//    ANDOR_Camera::WaitBufferThread::WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout):
    cameraPtr(camera), waitBufferTimeout(timeout)
{

}


void WaitBufferThread::run()
//void ANDOR_Camera::WaitBufferThread::run()
{
    QString log_msg;
    AT_64 bufferCounter;

    if ( cameraPtr == nullptr ) {
        log_msg = "Andor camera object is nullptr in WaitBufferThread!";
        //cameraPtr->printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, log_msg);
        cameraPtr->logToFile(ANDOR_Camera::CAMERA_ERROR,log_msg);
        return;
    }


    try {
        QString buff_addr, isize_addr;
        AT_U8* buff;
        int buff_size;
        QString addr,ret_addr;
        AT_64 image_size = (*cameraPtr)["ImageSizeBytes"];
        unsigned char** buff_ptr = cameraPtr->imageBufferList.get();


        // main acquisition cycle
        for ( long currentExposureCounter = 0; currentExposureCounter < cameraPtr->requestedFramesNumber; ++currentExposureCounter) {
            bufferCounter = currentExposureCounter % cameraPtr->imageBufferListSize;
            buff_addr.sprintf("%p",&buff);
            isize_addr.sprintf("%p",&buff_size);
            log_msg = QString("AT_WaitBuffer(%1,%2,%3,%4)").arg(cameraPtr->cameraHndl).arg(buff_addr).
                    arg(isize_addr).arg(waitBufferTimeout);
            andor_sdk_assert(AT_WaitBuffer(cameraPtr->cameraHndl,&buff,&buff_size,waitBufferTimeout),log_msg);

            // check returned buffer address equality
            if ( buff != buff_ptr[bufferCounter] ) {
                ret_addr.sprintf("%p",buff);
                addr.sprintf("%p",buff_ptr[bufferCounter]);
                log_msg = QString("AT_waitBuffer routine returned wrong address of image buffer! Returned %1, must be %2")
                          .arg(ret_addr).arg(addr);
                cameraPtr->logToFile(ANDOR_Camera::CAMERA_ERROR,log_msg);
                cameraPtr->lastError = AT_ERR_NULL_WAIT_PTR; // i had no idea what Andor SDK error is better for that
                emit cameraPtr->lastCameraError(cameraPtr->lastError);
                return;
            }

//            emit cameraPtr->imageReady(buff);
            emit bufferReady(buff);

            // if number of requested frames is greater than maximum number of buffers, then
            // re-queue buffers (use of circular buffer list)
            if ( cameraPtr->requestedFramesNumber > cameraPtr->imageBufferListSize ) {
                addr.sprintf("%p",buff_ptr[bufferCounter]);
                log_msg = QString("AT_QueueBuffer(%1,%2,%2)").arg(cameraPtr->cameraHndl).arg(addr).arg(image_size);
                andor_sdk_assert(AT_QueueBuffer(cameraPtr->cameraHndl,buff_ptr[bufferCounter],image_size),log_msg);
            }
        }

    } catch (AndorSDK_Exception &ex) {
        cameraPtr->lastError = ex.getError();
        emit cameraPtr->lastCameraError(cameraPtr->lastError);
        cameraPtr->logToFile(ex);
    }
}
