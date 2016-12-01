#ifndef WAITBUFFERTHREAD_H
#define WAITBUFFERTHREAD_H

#include "andor_camera.h"

class WaitBufferThread : public QThread
{
    Q_OBJECT

public:
    WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout = AT_INFINITE);

    void run();

signals:
    void bufferReady(AT_U8 *const buffer);

private:
    ANDOR_Camera *cameraPtr;
    unsigned int waitBufferTimeout;
};

#endif // WAITBUFFERTHREAD_H
