#ifndef WAITBUFFERTHREAD_H
#define WAITBUFFERTHREAD_H

#include <QThread>

#include "atcore.h"

    /* just forward declaration */
class ANDOR_Camera;

class WaitBufferThread : public QThread
{
public:
    WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout = AT_INFINITE);

    void run();
};

#endif // WAITBUFFERTHREAD_H
