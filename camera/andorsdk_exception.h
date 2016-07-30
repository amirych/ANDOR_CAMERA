#ifndef ANDORSDK_EXCEPTION_H
#define ANDORSDK_EXCEPTION_H

#include <QString>
#include <exception>

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


inline void andor_sdk_assert(int err, const char* context = nullptr);
inline void andor_sdk_assert(int err, const QString& context = QString::null);


#endif // ANDORSDK_EXCEPTION_H
