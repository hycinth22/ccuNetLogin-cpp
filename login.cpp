#include "login.h"
#include <iostream>
#include <fstream>
#include <filesystem>

// hack on STL experimental libraries
namespace std {
	using namespace ::std;
	using namespace ::std::experimental;
}

using namespace std;
namespace fs = filesystem;

const fs::path workDir = fs::current_path();
const fs::path accountFile = workDir / "account.dat";


void removeSavedAccount()
{
    remove(accountFile);
}

void saveAccount(AccountInfo account)
{
	clog << workDir;
	ofstream os(accountFile, ios::binary|ios::trunc);
	os << account;
	os.close();
}

AccountInfo getSavedAccount() {
	AccountInfo account;
	ifstream is(accountFile, ios::binary);
	is >> account;
	is.close();
	return move(account);
}


struct LoginResp {
	bool Success;
	string Msg;
	string Action;
	int Pop;
	string Username;
	string Location;
};
#include <map>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
class QJsonValueWrapper : public QJsonValue {
	using QJsonValue::QJsonValue;
public:
	QJsonValueWrapper() = default;
	QJsonValueWrapper(const QJsonValue& rd) 
		: QJsonValue(rd)
	{};
	QJsonValueWrapper(QJsonValue&& rd)
		: QJsonValue(rd)
	{};
	operator bool() {
		return this->toBool();
	}
	operator int() {
		return this->toDouble();
	}
	operator double() {
		return this->toDouble();
	}
	operator string() {
		return this->toString().toStdString();
	}
	operator QJsonArray() {
		return this->toArray();
	}
	operator QJsonObject() {
		return this->toObject();
	}
};
template <typename destType>
bool parseField(const QJsonObject& obj, const QString& name, const QJsonValue::Type& type, destType& dest) {
	if (obj.contains(name)) {
		QJsonValueWrapper val = obj.value(name);
		if (val.type() == type)
		{
			dest = val;
			return true;
		}
		else {
			throw LoginFailedException(string(u8"服务器返回消息含有不可处理的类型"));
		}
	}
	return false;
}
LoginResp parseResp(const QByteArray& binary) {
	auto doc = QJsonDocument::fromBinaryData(binary);
	if (!doc.isNull()) {
		if (doc.isObject()) {
			QJsonObject obj = doc.object();
			LoginResp result;
			parseField(obj, "Success", QJsonValue::Type::Bool, result.Success);
			parseField(obj, "Msg", QJsonValue::Type::String, result.Msg);
			parseField(obj, "Action", QJsonValue::Type::String, result.Action);
			parseField(obj, "Pop", QJsonValue::Type::Double, result.Pop);
			parseField(obj, "UserName", QJsonValue::Type::String, result.Username);
			parseField(obj, "Location", QJsonValue::Type::String, result.Location);
			return move(result);
		}
	}
	throw LoginFailedException(string(u8"服务器返回消息的格式不正确"));
}
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
static QNetworkAccessManager qNetwork;
bool requestLogin(AccountInfo account)
{
	QNetworkAccessManager network;
	qDebug() << "login begin! " << account.user.c_str() << "||" << account.pwd.c_str() << endl;


	// throw LoginFailedException("Not Implemented");

	const QUrl LOGIN_URL = "http://1.1.1.2/ac_portal/login.php";
	QNetworkRequest request;
	request.setUrl(LOGIN_URL);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply* reply = network.post(request, QByteArray::fromStdString(
		string() + 
		"opr=pwdLogin"
		+ "&userName=" + account.user
		+ "&pwd=" + account.pwd
		+ "&rememberPwd=0"

	));

	QEventLoop eventLoop;
	QTimer::singleShot(10 * 1000, &eventLoop, &QEventLoop::quit);
	QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	if (!reply->isFinished()) {
		throw LoginFailedException(string(u8"网络请求超时"));
	}

	QNetworkReply::NetworkError error = reply->error();
	if (error != QNetworkReply::NetworkError::NoError) {
		throw LoginFailedException(string(u8"网络请求发生错误，错误码：") + to_string(error));
	}

	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (statusCode != 200) {
		throw LoginFailedException(string(u8"服务器返回错误的状态码：") + to_string(statusCode));
	}

	QByteArray replyData = reply->readAll();

	LoginResp resp = parseResp(replyData);

	if (!resp.Success)
	{
		throw LoginFailedException(string(u8"服务器返回消息：") + resp.Msg);
	}

	return resp.Success;
}


bool login(AccountInfo acc, bool rem)
{
	if (rem) {
		saveAccount(acc);
	}
	else {
		removeSavedAccount();
	}

	bool success = requestLogin(acc);
	return success;
}

