#ifndef ANDORSDK_EXCEPTION_H
#define ANDORSDK_EXCEPTION_H

#include <QString>
#include <exception>
#include "atcore.h"

class AndorSDK_Exception : public std::exception
{
public:
    AndorSDK_Exception(int err_code, const QString &context = QString::null);
    AndorSDK_Exception(int err_code, const char* context = nullptr);

    int getError() const;

    const char* what() const noexcept;

private:
    QString msg;
    int errCode;
};


inline void andor_sdk_assert(int err, const char* context = nullptr)
{
    if ( err != AT_SUCCESS ) {
        throw AndorSDK_Exception(err, context);
    }
};

inline void andor_sdk_assert(int err, const QString& context = QString::null)
{
    andor_sdk_assert(err,context.toLatin1().data());
};


#endif // ANDORSDK_EXCEPTION_H
