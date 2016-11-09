#include "andor_camera.h"
#include "andor_sdk_features.h"
#include "andorsdk_exception.h"


#include <QDateTime>

#define ANDOR_CAMERA_LOG_LEVEL_TAB 3 // indentation length in symbols for log levels

#define CTIME_STAMP QDateTime::currentDateTime().toString(" dd-MM-yyyy hh:mm:ss: ").toUtf8().data()
#define QTIME_STAMP QDateTime::currentDateTime().toString(" dd-MM-yyyy hh:mm:ss: ")

#define ANDOR_CAMERA_LOG_CAMERA_IDENT "[CAMERA]"
#define ANDOR_CAMERA_LOG_ANDOR_SDK_IDENT "[ANDOR SDK]"


            /* CameraPresent feature callback function  */

static int AT_EXP_CONV disconnection_callback(AT_H hndl, AT_WC* name, void* context)
{
    if ( context == nullptr ) return AT_ERR_INVALIDHANDLE; // just check (should not be NULL!!!)

    ANDOR_Camera *camera = (ANDOR_Camera*)context;
    if ( camera == nullptr ) return AT_ERR_INVALIDHANDLE; // just check

    camera->disconnectFromCamera();
}


                    /*  ANDOR camera info structure implementation */

ANDOR_CameraInfo::ANDOR_CameraInfo():
    cameraModel("Unknown"), cameraName("Unknown"), serialNumber("Unknown"),
    controllerID("Unknown"), interfaceType("Unknown"), device_index(-1)
{
}


                        /***************************************
                        *                                      *
                        *    ANDOR_Camera class implementation *
                        *                                      *
                        ***************************************/


                    /*   STATIC MEMBERS INITIALIZATIONS   */

QList<ANDOR_CameraInfo> ANDOR_Camera::foundCameras = QList<ANDOR_CameraInfo>();
QList<int> ANDOR_Camera::openedCameras = QList<int>();
size_t ANDOR_Camera::numberOfCreatedObjects = 0;

ANDOR_Camera::ANDOR_Feature ANDOR_Camera::DeviceCount(AT_HANDLE_SYSTEM,"DeviceCount");
ANDOR_Camera::ANDOR_Feature ANDOR_Camera::SoftwareVersion(AT_HANDLE_SYSTEM,"SoftwareVersion");



                        /*  CONSTRUCTORS AND DESTRUCTOR  */

ANDOR_Camera::ANDOR_Camera(QObject *parent) : QObject(parent),
    lastError(AT_SUCCESS), cameraLog(nullptr), cameraHndl(AT_HANDLE_SYSTEM),
    CameraPresent("CameraPresent"), cameraFeature(), currentFitsFilename("")
{
    if ( !numberOfCreatedObjects ) {
        AT_InitialiseLibrary();
        scanConnectedCameras();
    }

    ++numberOfCreatedObjects;
}


ANDOR_Camera::~ANDOR_Camera()
{
    if ( isCameraConnected() ) disconnectFromCamera();

    --numberOfCreatedObjects;

    if ( !numberOfCreatedObjects ) {
        AT_FinaliseLibrary();
    }
}


                    /*  PUBLIC METHODS  */


AT_H ANDOR_Camera::getCameraHndl() const
{
    return cameraHndl;
}


int ANDOR_Camera::getLastCameraError() const
{
    return lastError;
}


bool ANDOR_Camera::isCameraConnected()
{
    return CameraPresent;
}


int ANDOR_Camera::scanConnectedCameras()
{
    int N = ANDOR_Camera::DeviceCount;

    AT_H hndl;

    if ( foundCameras.size() ) foundCameras.clear();

    // suppose device indices are continuous sequence from 0 to (DeviceCount-1)
    for (int i = 0; i < N-1; ++i ) {
        int err = AT_Open(i,&hndl);
        if ( err == AT_SUCCESS ) {
            ANDOR_CameraInfo info;
            ANDOR_Feature feature(hndl,"CameraModel");

            info.cameraModel = (QString)feature;

            feature.init("CameraName");
            info.cameraName = (QString)feature;

            feature.init("SerialNumber");
            info.serialNumber = (QString)feature;

            feature.init("ControllerID");
            info.controllerID = (QString)feature;

            feature.init("SensorWidth");
            info.sensorWidth = feature;

            feature.init("SensorHeight");
            info.sensorHeight = feature;

            feature.init("PixelWidth");
            info.pixelWidth = feature;

            feature.init("PixelHeight");
            info.pixelHeight = feature;

            feature.init("InterfaceType");
            info.interfaceType = (QString)feature;

            info.device_index = i;

            foundCameras << info;
        }
        AT_Close(hndl);
    }

    return N;
}


QList<ANDOR_CameraInfo> ANDOR_Camera::getConnectedCameras()
{
    return foundCameras;
}


ANDOR_Camera::ANDOR_Feature& ANDOR_Camera::operator [](const QString &feature_name)
{
    if ( !ANDOR_SDK_DEFS::ANDOR_SDK_FEATURES.contains(feature_name) ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Unknown ANDOR SDK feature!");
    }

    cameraFeature.init(feature_name);

    return cameraFeature;
}


ANDOR_Camera::ANDOR_Feature& ANDOR_Camera::operator [](const char* feature_name)
{
    return operator [](QString(feature_name));
}


bool ANDOR_Camera::connectToCamera(int device_index, std::ostream *log_file)
{
    disconnectFromCamera();

    cameraLog = log_file;

    // print header of log
    QString log_str = "   ";
    for (int i = 0; i < 5; ++i ) printLog("",log_str);
    log_str.fill('*',80);
    printLog("",log_str);
    printLog("",QTIME_STAMP);
    printLog("",log_str);

    printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,"Try to open connection to Andor camera ... ");

    bool ok;

    try {
//        AT_64 dev_num = (*this)["DeviceCount"];
        AT_64 dev_num = ANDOR_Camera::DeviceCount;
        log_str = QString("Number of found cameras: %1").arg(dev_num);
        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,log_str,1);

        if ( dev_num == 0 ) {
            printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,"No one camera was detected!!! Can not perform connection!!!");
            lastError = AT_ERR_CONNECTION;
            emit lastCameraError(lastError);

            return false;
        }

        log_str = QString(" initiate connection to device with index %1 ...").arg(device_index);
        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,log_str,1);

        andor_sdk_assert( AT_Open(device_index,&cameraHndl), QString("AT_Open(%1,&hndl)").arg(device_index) );
        CameraPresent.setCameraHndl(cameraHndl);
        ok = CameraPresent;
        if ( !ok ) { // it is very strange!!!
            throw AndorSDK_Exception(AT_ERR_CONNECTION,"Connection lost!");
        }

        log_str = QString("Connection established! Camera handler is %1 ").arg(cameraHndl);
        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,log_str);

        cameraIndex = device_index;

        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, "Try to register 'CameraPresent'-feature callback function ...");

        wchar_t name[] = L"CameraPresent";
        andor_sdk_assert( AT_RegisterFeatureCallback(cameraHndl,name,(FeatureCallback)disconnection_callback,(void*)this),
                          QString("AT_RegisterFeatureCallback(%1,%2)").arg(cameraHndl).arg(QString::fromWCharArray(name)));

        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, "The function was registered successfully!");

        cameraFeature.setCameraHndl(cameraHndl);

        emit cameraIsOpened();

        lastError = AT_SUCCESS;
        ok = true;

    } catch ( AndorSDK_Exception &ex ) {
        lastError = ex.getError();
        emit lastCameraError(lastError);
        printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,ex.what());
        ok = false;
    }

    emit lastCameraError(lastError);

    return ok;
}


bool ANDOR_Camera::connectToCamera(QString &identifier, ANDOR_CameraInfo::ANDOR_CameraInfoType type, std::ostream *log_file)
{
    QString str = identifier.trimmed();
    if ( str.isNull() || str.isEmpty() ) return false;

    QString msg = "Ask for connection to camera by ";
    switch ( type ) {
        case ANDOR_CameraInfo::CameraModel: {
           msg += "'CameraModel'";
           break;
        }
        case ANDOR_CameraInfo::CameraName: {
           msg += "'CameraName'";
           break;
        }
        case ANDOR_CameraInfo::SerialNumber: {
           msg += "'SerialNumber'";
           break;
        }
        case ANDOR_CameraInfo::ControllerID: {
           msg += "'ControllerID'";
           break;
        }
    default: {
           msg += "'unknown identifier type!!! Cannot open a camera!!!'";
           printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,msg);
           return false; // unknown type!
        }
    }
    msg += " identificator ...";
    printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,msg);


    int ok;
    for ( int i = 0; i < foundCameras.size(); ++i ) {
        switch ( type ) {
            case ANDOR_CameraInfo::CameraModel: {
               ok = QString::compare(str,foundCameras[i].cameraModel);
               break;
            }
            case ANDOR_CameraInfo::CameraName: {
               ok = QString::compare(str,foundCameras[i].cameraName);
               break;
            }
            case ANDOR_CameraInfo::SerialNumber: {
               ok = QString::compare(str,foundCameras[i].serialNumber);
               break;
            }
            case ANDOR_CameraInfo::ControllerID: {
               ok = QString::compare(str,foundCameras[i].controllerID);
               break;
            }
            default: return false; // unknown type!
        }

        if ( !ok ) { // coincidence was found
            msg = QString("The identificator was found! The camera device index is %1").arg(foundCameras[i].device_index);
            printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT,msg);
            return connectToCamera(foundCameras[i].device_index, log_file);
        }
    }

    // coincidence was not found
    return false;
}



void ANDOR_Camera::disconnectFromCamera()
{
//    if ( !CameraPresent ) return;

    QString str = QString("Unregistering 'CameraPresent'-feature callback function for camera with handler of %1").arg(cameraHndl);
    printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, str);

    wchar_t name[] = L"CameraPresent";
    AT_UnregisterFeatureCallback(cameraHndl,name,(FeatureCallback)disconnection_callback,nullptr);

    str = QString("Disconnection from camera with handler of %1").arg(cameraHndl);
    printLog(ANDOR_CAMERA_LOG_CAMERA_IDENT, str);
    AT_Close(cameraHndl);

    emit cameraIsClosed();
}


                    /*  PUBLIC SLOTS  */

void ANDOR_Camera::acquisitioStart()
{
    andor_sdk_assert(AT_Command(cameraHndl,L"AcquisionStart"),);
}


void ANDOR_Camera::acquisitioStop()
{
    andor_sdk_assert(AT_Command(cameraHndl,L"AcquisionStop"));
}


void ANDOR_Camera::setFitsFilename(const QString &filename)
{
    currentFitsFilename = filename.trimmed();
}


                    /*  PROTECTED METHODS  */

void ANDOR_Camera::printLog(const QString ident, const QString log_str, int log_level)
{
    if ( cameraLog ) {
        QString str = QTIME_STAMP;
        str.fill(' ',log_level*ANDOR_CAMERA_LOG_LEVEL_TAB);
        str.append(log_str);
        if ( ident.length() ) {
            *cameraLog << ident.toLatin1().data() << " ";
        }
        *cameraLog << log_str.toLatin1().data() << std::endl << std::flush;
    }

}


                            /****************************************
                            *                                       *
                            *    ANDOR_Feature class implementation *
                            *                                       *
                            ****************************************/


ANDOR_Camera::ANDOR_Feature::ANDOR_Feature(): cameraHndl(AT_HANDLE_SYSTEM),
    type(ANDOR_SDK_DEFS::UnknownType), featureName(nullptr), featureNameQString(),
    str_val()
{

}


ANDOR_Camera::ANDOR_Feature::ANDOR_Feature(const AT_H hndl, const QString &name):ANDOR_Feature()
{
    cameraHndl = hndl;
    init(name);
}


ANDOR_Camera::ANDOR_Feature::ANDOR_Feature(const AT_H hndl, const char *name):ANDOR_Feature(hndl,QString(name))
{

}


ANDOR_Camera::ANDOR_Feature::ANDOR_Feature(const QString &name):ANDOR_Feature()
{
    init(name);
}


ANDOR_Camera::ANDOR_Feature::ANDOR_Feature(const char *name):ANDOR_Feature(QString(name))
{

}


ANDOR_Camera::ANDOR_Feature::~ANDOR_Feature()
{
    delete[] featureName;
}


void ANDOR_Camera::ANDOR_Feature::setCameraHndl(const AT_H hndl)
{
    cameraHndl = hndl;
}


//  feature_name must be valid name!!!
void ANDOR_Camera::ANDOR_Feature::init(const QString &feature_name)
{
    featureNameQString = feature_name.trimmed();

    if ( featureNameQString.isNull() || featureNameQString.isEmpty() ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED, "Invalid feature name!");
    }

    type = ANDOR_SDK_DEFS::ANDOR_SDK_FEATURES[featureNameQString];


    delete[] featureName;
    featureName = new AT_WC[featureNameQString.length()+1];
    featureNameQString.toWCharArray(featureName);
    featureName[featureNameQString.length()] = '\0';

    float_val = 0.0;
    str_val.clear();
}


                        /*  Type conversional operators to get feature value  */

ANDOR_Camera::ANDOR_Feature::operator AT_64()
{
    if ( type != ANDOR_SDK_DEFS::IntType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    QString err_msg = QString("AT_GetInt(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert( AT_GetInt(cameraHndl,featureName,&at64_val), err_msg);

    return at64_val;
}


ANDOR_Camera::ANDOR_Feature::operator double()
{
    if ( type != ANDOR_SDK_DEFS::FloatType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    QString err_msg = QString("AT_GetFloat(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert( AT_GetFloat(cameraHndl,featureName,&float_val), err_msg);

    return float_val;
}


ANDOR_Camera::ANDOR_Feature::operator bool()
{
    AT_BOOL flag;

    if ( type != ANDOR_SDK_DEFS::BoolType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    QString err_msg = QString("AT_GetBool(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert( AT_GetBool(cameraHndl,featureName,&flag), err_msg);

    return flag == AT_TRUE ? true : false;
}


ANDOR_Camera::ANDOR_Feature::operator QString()
{
    QString err_msg;

    switch ( type ) {
        case ANDOR_SDK_DEFS::StringType: {
            int len;
            err_msg = QString("AT_GetStringMaxLength(%1,%2)").arg(cameraHndl).arg(featureNameQString);
            andor_sdk_assert( AT_GetStringMaxLength(cameraHndl,featureName,&len), err_msg);
            if ( len ) {
                AT_WC val[len];
                err_msg = QString("AT_GetString(%1,%2,str_val,%3)").arg(cameraHndl).arg(featureNameQString).arg(len);
                andor_sdk_assert( AT_GetString(cameraHndl,featureName,val,len), err_msg);
                str_val = QString::fromWCharArray(val);
            } else {
                throw AndorSDK_Exception(AT_ERR_NULL_MAXSTRINGLENGTH,"Length of string feature value is 0!");
            }
            break;
        }
        case ANDOR_SDK_DEFS::EnumType: {
            AT_WC val[ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN];
            int index;

            err_msg = QString("AT_GetEnumIndex(%1,%2)").arg(cameraHndl).arg(featureNameQString);
            andor_sdk_assert( AT_GetEnumIndex(cameraHndl,featureName,&index), err_msg);

            err_msg = QString("AT_GetEnumStringByIndex(%1,%2,%3,str_val,%4)").arg(cameraHndl)
                                                                  .arg(featureNameQString).arg(index)
                                                                  .arg(ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN);
            andor_sdk_assert(
                        AT_GetEnumStringByIndex(cameraHndl,featureName,index,
                                                val,ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN),
                        err_msg);
            str_val = QString::fromWCharArray(val);
            break;
        }
        default: { // invalid type
            throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
        }
    }

    return str_val;
}


ANDOR_Camera::ANDOR_Feature::operator andor_enum_index_t()
{
    if ( type != ANDOR_SDK_DEFS::EnumType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    int index;

    QString err_msg = QString("AT_GetEnumIndex(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert( AT_GetEnumIndex(cameraHndl,featureName,&index), err_msg);

    index_val = (andor_enum_index_t)index;

    return index_val;
}



                /*  Type conversion operators to get range of feature value  */


ANDOR_Camera::ANDOR_Feature::operator std::pair<AT_64, AT_64>()
{
    if ( type != ANDOR_SDK_DEFS::IntType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    AT_64 val;

    std::pair<AT_64,AT_64> range;

    QString err_msg = QString("AT_GetIntMin(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert(AT_GetIntMin(cameraHndl,featureName,&val), err_msg);
    range.first = val;

    err_msg = QString("AT_GetIntMax(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert(AT_GetIntMax(cameraHndl,featureName,&val), err_msg);
    range.second = val;

    return range;
}


ANDOR_Camera::ANDOR_Feature::operator std::pair<double, double>()
{
    if ( type != ANDOR_SDK_DEFS::FloatType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    double val;

    std::pair<double, double> range;

    QString err_msg = QString("AT_GetFloatMin(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert(AT_GetFloatMin(cameraHndl,featureName,&val), err_msg);
    range.first = val;

    err_msg = QString("AT_GetFloatMax(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert(AT_GetFloatMax(cameraHndl,featureName,&val), err_msg);
    range.second = val;

    return range;
}


ANDOR_Camera::ANDOR_Feature::operator QStringList()
{
    if ( type != ANDOR_SDK_DEFS::EnumType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    AT_WC val[ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN];

    int n;

    QStringList range;

    QString err_msg = QString("AT_GetEnumCount(%1,%2)").arg(cameraHndl).arg(featureNameQString);
    andor_sdk_assert( AT_GetEnumCount(cameraHndl, featureName, &n), err_msg);

    for ( int index = 0; index < n; ++index ) {
        AT_BOOL ok;
        err_msg = QString("AT_IsEnumIndexAvailable(%1,%2,%3)").arg(cameraHndl)
                .arg(featureNameQString).arg(index);
        andor_sdk_assert( AT_IsEnumIndexAvailable(cameraHndl,featureName,index,&ok), err_msg);
        if ( ok == AT_FALSE ) continue;

        err_msg = QString("AT_IsEnumIndexImplemented(%1,%2,%3)").arg(cameraHndl)
                .arg(featureNameQString).arg(index);
        andor_sdk_assert( AT_IsEnumIndexImplemented(cameraHndl,featureName,index,&ok), err_msg);
        if ( ok == AT_FALSE ) continue;

        err_msg = QString("AT_GetEnumStringByIndex(%1,%2,%3,str_val,%4)").arg(cameraHndl)
                                                              .arg(featureNameQString).arg(index)
                                                              .arg(ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN);
        andor_sdk_assert(
                    AT_GetEnumStringByIndex(cameraHndl,featureName,index,
                                            val,ANDOR_SDK_DEFS::ANDOR_SDK_ENUM_FEATURE_STRLEN),
                    err_msg);
        range.append(QString::fromWCharArray(val));
    }

    return range;
}


            /*  Assignment operators to set feature value  */


AT_64 & ANDOR_Camera::ANDOR_Feature::operator = (const AT_64 & value)
{
    if ( type != ANDOR_SDK_DEFS::IntType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    at64_val = value;
    QString err_msg = QString("AT_SetInt(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(at64_val);
    andor_sdk_assert( AT_SetInt(cameraHndl,featureName, at64_val), err_msg);

    return at64_val;
}


double & ANDOR_Camera::ANDOR_Feature::operator = (const double & value)
{
    if ( type != ANDOR_SDK_DEFS::FloatType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    float_val = value;
    QString err_msg = QString("AT_SetFloat(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(float_val);
    andor_sdk_assert( AT_SetFloat(cameraHndl,featureName, float_val), err_msg);

    return float_val;
}


bool & ANDOR_Camera::ANDOR_Feature::operator = (const bool & value)
{
    if ( type != ANDOR_SDK_DEFS::BoolType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    bool_val = value;
    AT_BOOL flag = bool_val == true ? AT_TRUE : AT_FALSE;

    QString err_msg = QString("AT_SetBool(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(flag);
    andor_sdk_assert( AT_SetBool(cameraHndl,featureName, flag), err_msg);

    return bool_val;
}


QString & ANDOR_Camera::ANDOR_Feature::operator = (const QString & value)
{
    switch ( type ) {
    case ANDOR_SDK_DEFS::StringType: {
        str_val = value;
        QString err_msg = QString("AT_SetString(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(str_val);
        AT_WC str[str_val.length()+1];
        str_val.toWCharArray(str);
        str[str_val.length()] = '\0';
        andor_sdk_assert( AT_SetString(cameraHndl,featureName, str), err_msg);
        break;
    }
    case ANDOR_SDK_DEFS::EnumType: {
        str_val = value;
        QString err_msg = QString("AT_SetEnumString(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(str_val);
        AT_WC str[str_val.length()+1];
        str_val.toWCharArray(str);
        str[str_val.length()] = '\0';
        andor_sdk_assert( AT_SetEnumString(cameraHndl,featureName, str), err_msg);
        break;
    }
    default: {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }
    }
    return str_val;
}


andor_enum_index_t & ANDOR_Camera::ANDOR_Feature::operator = (const andor_enum_index_t & value)
{
    if ( type != ANDOR_SDK_DEFS::EnumType ) {
        throw AndorSDK_Exception(AT_ERR_NOTIMPLEMENTED,"Feature type missmatch!");
    }

    index_val = value;
    int index = (int)index_val;

    QString err_msg = QString("AT_SetEnumIndex(%1,%2,%3)").arg(cameraHndl).arg(featureNameQString).arg(index_val);
    andor_sdk_assert( AT_SetEnumIndex(cameraHndl,featureName, index), err_msg);

    return index_val;
}
