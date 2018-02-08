#ifndef LOGIN_H
#define LOGIN_H

#include <string>
#include <istream>
#include <ostream>
#include <exception>

struct AccountInfo {
    std::string user;
    std::string pwd;
};
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

bool login(AccountInfo acc, bool rem = false);
void saveAccount(AccountInfo account);
AccountInfo getSavedAccount();

class LoginFailedException : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};


class QNetworkAccessManager;
extern QNetworkAccessManager qNetwork;

#endif // LOGIN_H
