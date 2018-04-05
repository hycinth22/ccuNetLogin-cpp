#ifndef LOGIN_H
#define LOGIN_H

#include "accountInfo.h"
#include <QException>
#include <QString>

bool login(AccountInfo acc, bool rem = false);
void saveAccount(AccountInfo account);
AccountInfo getSavedAccount();

class LoginFailedException : public QException
{
public:
    explicit LoginFailedException(const QString& reason)
        : QException(), reason(reason)
    {}
    LoginFailedException *LoginFailedException::clone() const { 
        return new LoginFailedException(*this);
    }
    QString reason;
};

#endif // LOGIN_H
