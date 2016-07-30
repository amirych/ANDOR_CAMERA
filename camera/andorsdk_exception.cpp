#include "andorsdk_exception.h"
#include "atcore.h"

AndorSDK_Exception::AndorSDK_Exception(int err_code, const QString &context):
    exception(), msg(context), errCode(err_code)
{

}

AndorSDK_Exception::AndorSDK_Exception(int err_code, const char *context):
    AndorSDK_Exception(err_code,QString(context))
{

}


void andor_sdk_assert(int err, const char *context)
{
    if ( err != AT_SUCCESS ) {
        throw AndorSDK_Exception(err, context);
    }
}


void andor_sdk_assert(int err, const QString &context)
{
    andor_sdk_assert(err,context.toLatin1().data());
}
