#ifndef ANDOR_CAMERA_H
#define ANDOR_CAMERA_H

#include "../export_dec.h"
#include "andor_sdk_features.h"
#include "atcore.h"


#include <QObject>
#include <QStringList>
#include <QThread>
#include <memory>
#include <utility>


#define ANDOR_CAMERA_LOG_CAMERA_IDENT "[CAMERA]"
#define ANDOR_CAMERA_LOG_ANDOR_SDK_IDENT "[ANDOR SDK]"
#define ANDOR_CAMERA_LOG_ERROR "[AN ERROR OCCURED! ERROR MESSAGE]"

#define ANDOR_CAMERA_DEFAULT_MAX_BUFFERS_NUMBER 50 // default maximum buffers number to be allocated for reading data



                /*  ANDOR camera info structure  */

struct ANDOR_CAMERA_EXPORT_DECL ANDOR_CameraInfo
{
    ANDOR_CameraInfo();

    QString cameraModel;
    QString cameraName;
    QString serialNumber;
    QString controllerID;
    AT_64 sensorWidth;
    AT_64 sensorHeight;
    double pixelWidth; // in micrometers
    double pixelHeight;
    QString interfaceType;
    int device_index;

    enum ANDOR_CameraInfoType {CameraModel, CameraName, SerialNumber, ControllerID};
};



                 /**********************************************
                 *                                             *
                 * QT WRAPPER CLASS DEFINITION FOR ANDOR SDK   *
                 *                                             *
                 **********************************************/

typedef int andor_enum_index_t; // type for ANDOR SDK enumerated feature index

class ANDOR_CAMERA_EXPORT_DECL ANDOR_Camera : public QObject
{
    Q_OBJECT

protected:

                /*  a proxy class to access ANDOR SDK features  */

    class ANDOR_Feature
    {
    public:
        ANDOR_Feature();

        ANDOR_Feature(const AT_H hndl, const QString &name);
        ANDOR_Feature(const AT_H hndl, const char* name);

        ANDOR_Feature(const QString &name);
        ANDOR_Feature(const char* name);

        ~ANDOR_Feature();

        void init(const QString &feature_name);
        void setCameraHndl(const AT_H hndl);

        // get feature value
        operator AT_64();
        operator double();
        operator bool();
        operator QString(); // for string and enumerated features
        operator andor_enum_index_t(); // for enumerated features

        // get range of feature value
        operator std::pair<AT_64,AT_64>();
        operator std::pair<double,double>();
        operator QStringList();

        AT_64 & operator = (const AT_64 & value);
        double & operator = (const double & value);
        bool & operator = (const bool & value);
        QString & operator = (const QString & value);
        andor_enum_index_t & operator = (const andor_enum_index_t & value);


    private:
        AT_H cameraHndl;
        ANDOR_SDK_DEFS::AndorFeatureType type;
        AT_WC* featureName;
        QString featureNameQString;
        union {
            AT_64 at64_val;
            double float_val;
            bool bool_val;
            andor_enum_index_t index_val;
        };
        QString str_val; // for string and enumerated type features
    };

                /*   ANDOR SDK AT_WaitBuffer thread class   */

    class WaitBufferThread : public QThread
    {
    public:
        WaitBufferThread(ANDOR_Camera *camera, unsigned int timeout = AT_INFINITE);

        void run();

    private:
        ANDOR_Camera *cameraPtr;
        unsigned int waitBufferTimeout;
    };
    friend class WaitBufferThread;

public:

    explicit ANDOR_Camera(QObject *parent = 0);

    virtual ~ANDOR_Camera();

    bool connectToCamera(int device_index = 0, std::ostream *log_file = nullptr);
    bool connectToCamera(QString &identifier, ANDOR_CameraInfo::ANDOR_CameraInfoType type = ANDOR_CameraInfo::SerialNumber,
                         std::ostream *log_file = nullptr);

    void disconnectFromCamera();

    AT_H getCameraHndl() const;

    int getLastCameraError() const;

    bool isCameraConnected();

    static QList<ANDOR_CameraInfo> getConnectedCameras();

    void setMaxBuffersNumber(const qint64 num);
    qint64 getMaxBuffersNumber() const;

                /* operator[] for accessing Andor SDK features (const and non-const versions) */

//    const ANDOR_Feature& operator[](QString &feature_name) const;
    ANDOR_Feature& operator[](const QString &feature_name);
    ANDOR_Feature& operator[](const char* feature_name);


                /* Andor SDK global features */

    static ANDOR_Feature DeviceCount;
    static ANDOR_Feature SoftwareVersion;


                /* "CameraPresent" and "CameraAcquiring" feature declarations */

    ANDOR_Feature CameraPresent;
    ANDOR_Feature CameraAcquiring;

signals:
    void lastCameraError(int err);
    void cameraIsOpened();
    void cameraIsClosed();


public slots:
    void acquisitionStart();
    void acquisitionStop();

    void setFitsFilename(const QString &filename, const QString &userFitsHdrFilename = QString::null);

protected:

    static QList<ANDOR_CameraInfo> foundCameras;
    static QList<int> openedCameras;
    static size_t numberOfCreatedObjects;

    AT_H cameraHndl;
    int cameraIndex;

    int lastError;

    std::ostream *cameraLog;

    ANDOR_Feature cameraFeature;

    QString currentFitsFilename;
    QString currentUserFitsHeaderFilename;

    qint64 maxBuffersNumber;

    std::unique_ptr<WaitBufferThread> waitBufferThreadUniquePtr;
    WaitBufferThread *waitBufferThread;

    std::unique_ptr<unsigned char*> imageBufferList;
    size_t imageBufferListSize;

    void deleteImageBuffers();

    void printLog(const QString ident, const QString log_str, int log_level = 0);
    void printError(const QString ident, const QString err_str, int log_level = 0);

    static int scanConnectedCameras(); // scan connected cameras when the first object will be created
};



#endif // ANDOR_CAMERA_H
