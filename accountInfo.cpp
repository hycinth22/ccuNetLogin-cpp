#include "stdafx.h"
#include "accountInfo.h"

class QDataStream;
QDataStream& operator<<(QDataStream& os, const AccountInfo& account)
{
    return os << account.user << account.pwd;
}
QDataStream& operator>>(QDataStream& is, AccountInfo& account)
{
    return is >> account.user >> account.pwd;
}