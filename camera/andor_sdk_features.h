#ifndef ANDOR_SDK_FEATURES_H
#define ANDOR_SDK_FEATURES_H

#include <QHash>

                    /**************************************
                    *    ANDOR SDK FEATURES DEFINITIONS   *
                    ***************************************/

namespace ANDOR_SDK_DEFS {

const int ANDOR_SDK_ENUM_FEATURE_STRLEN = 30;

enum AndorFeatureType {BoolType, IntType, FloatType, StringType, EnumType, UnknownType};
typedef QHash<QString,AndorFeatureType> AndorFeaturesHash;


AndorFeaturesHash ANDOR_SDK_FEATURES = {
    {"AccumulateCount", BoolType},
    {"AlternatingReadoutDirection", BoolType},
    {"AOIBinning", EnumType},
    {"AOIHBin", IntType},
    {"AOIHeight", IntType},
    {"AOILayout", EnumType},
    {"AOILeft", IntType},
    {"AOIStride", IntType},
    {"AOITop", IntType},
    {"AOIVBin", IntType},
    {"AOIWidth", IntType},
    {"AuxiliaryOutSource", EnumType},
    {"AuxOutSourceTwo", EnumType},
    {"BackoffTemperatureOffset", FloatType},
    {"Baseline", IntType},
    {"BitDepth", EnumType},
    {"BufferOverflowEvent", IntType},
    {"BytesPerPixel", FloatType},
    {"CameraAcquiring", BoolType},
    {"CameraFamily", StringType},
    {"CameraMemory", IntType},
    {"CameraModel", StringType},
    {"CameraName", StringType},
    {"CameraPresent", BoolType},
    {"ColourFilter", EnumType},
    {"ControllerID", StringType},
    {"CoolerPower", FloatType},
    {"CycleMode", EnumType},
    {"DDR2Type", StringType},
    {"DeviceCount", IntType},
    {"DeviceVideoIndex", IntType},
    {"DisableShutter", BoolType},
    {"DriverVersion", StringType},
    {"ElectronicShutteringMode", EnumType},
    {"EventEnable", BoolType},
    {"EventsMissedEvent", IntType},
    {"EventSelector", EnumType},
    {"ExposedPixelHeight", IntType},
    {"ExposureTime", FloatType},
    {"ExposureEndEvent", IntType},
    {"ExposureStartEvent", IntType},
    {"ExternalIOReadout", BoolType},
    {"ExternalTriggerDelay", FloatType},
    {"FanSpeed", EnumType},
    {"FastAOIFrameRateEnable", BoolType},
    {"FirmwareVersion", StringType},
    {"ForceShutterOpen", BoolType},
    {"FrameCount", IntType},
    {"FrameInterval", FloatType},
    {"FrameIntervalTiming", BoolType},
    {"FrameRate", FloatType},
    {"FullAOIControl", BoolType},
    {"HeatSinkTemperature", FloatType},
    {"ImageSizeBytes", IntType},
    {"InputVoltage", FloatType},
    {"InterfaceType", StringType},
    {"IOControl", EnumType},
    {"IODirection", EnumType},
    {"IOState", BoolType},
    {"IOInvert", BoolType},
    {"IOSelector", EnumType},
    {"IRPreFlashEnable", BoolType},
    {"KeepCleanEnable", BoolType},
    {"KeepCleanPostExposureEnable", BoolType},
    {"LineScanSpeed", FloatType},
    {"MaxInterfaceTransferRate", FloatType},
    {"MetadataEnable", BoolType},
    {"MetadataFrame", BoolType},
    {"MetadataTimestamp", BoolType},
    {"MicrocodeVersion", StringType},
    {"MultitrackBinned", BoolType},
    {"MultitrackCount", IntType},
    {"MultitrackEnd", IntType},
    {"MultitrackSelector", IntType},
    {"MultitrackStart", IntType},
    {"Overlap", BoolType},
    {"PixelEncoding", EnumType},
    {"PixelHeight", FloatType},
    {"PixelReadoutRate", EnumType},
    {"PixelWidth", FloatType},
    {"PortSelector", IntType},
#ifdef ANDOR_CAMERA_DEPRECATED_ENABLED
    {"PreAmpGain", EnumType},
    {"PreAmpGainChannel", EnumType},
    {"PreAmpGainControl", EnumType},
    {"PreAmpGainSelector", EnumType},
#endif
    {"PreAmpGainValue", IntType},
    {"PreAmpOffsetValue", IntType},
    {"ReadoutTime", FloatType},
    {"RollingShutterGlobalClear", BoolType},
    {"RowNExposureEndEvent", IntType},
    {"RowNExposureStartEvent", IntType},
    {"RowReadTime", FloatType},
    {"ScanSpeedControlEnable", BoolType},
    {"SensorCooling", BoolType},
    {"SensorHeight", IntType},
    {"SensorModel", StringType},
    {"SensorReadoutMode", EnumType},
    {"SensorType", EnumType},
    {"SensorTemperature", FloatType},
    {"SensorWidth", IntType},
    {"SerialNumber", StringType},
    {"ShutterAmpControl", BoolType},
    {"ShutterMode", EnumType},
    {"ShutterOutputMode", EnumType},
    {"ShutterState", BoolType},
    {"ShutterStrobePeriod", FloatType},
    {"ShutterStrobePosition", FloatType},
    {"ShutterTransferTime", FloatType},
    {"SimplePreAmpGainControl", EnumType},
    {"SoftwareVersion", StringType},
    {"SpuriousNoiseFilter", BoolType},
    {"StaticBlemishCorrection", BoolType},
#ifdef ANDOR_CAMERA_DEPRECATED_ENABLED
    {"TargetSensorTemperature", FloatType},
#endif
    {"TemperatureControl", EnumType},
    {"TemperatureStatus", EnumType},
    {"TimestampClock", IntType},
    {"TimestampClockFrequency", IntType},
    {"TransmitFrames", BoolType},
    {"TriggerMode", EnumType},
    {"UsbProductId", IntType},
    {"UsbDeviceId", IntType},
    {"VerticallyCentreAOI", BoolType}
};

}
#endif // ANDOR_SDK_FEATURES_H