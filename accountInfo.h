#pragma once
#include <QString>

struct AccountInfo {
    QString user;
    QString pwd;
};

extern QDataStream& operator<<(QDataStream& os, const AccountInfo& account);
extern QDataStream& operator>>(QDataStream& os, AccountInfo& account);

template<class ...streamArgs>
decltype(auto) operator<<(std::basic_ofstream<streamArgs...>& os, const AccountInfo& account)
{
    return os << account.user << endl << account.pwd;
}
template<class ...streamArgs>
decltype(auto) operator>>(std::basic_ifstream<streamArgs...>& is, AccountInfo& account)
{
    return is >> account.user >> account.pwd;
}
