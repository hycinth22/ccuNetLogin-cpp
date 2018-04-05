#include "stdafx.h"
#include <QDir>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "accountInfo.h"
#include "login.h"

static QNetworkAccessManager qtNetwork;

static QDir workDir = QDir::current();
static QFile accountFile(workDir.absoluteFilePath("account.dat"));

bool removeSavedAccount()
{
    return accountFile.remove();
}

void saveAccount(AccountInfo account)
{
    //qDebug() << workDir.absoluteFilePath;
    accountFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    accountFile.reset();
    QDataStream stream(&accountFile);
    stream << account;
}

AccountInfo getSavedAccount() {
    accountFile.open(QIODevice::ReadOnly);
    QDataStream stream(&accountFile);
    AccountInfo account;
    stream >> account;
    accountFile.close();
	return std::move(account);
}

struct LoginResp {
	bool Success;
	QString Msg;
    QString Action;
	int Pop;
    QString Username;
    QString Location;
};

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
    operator QString() {
        return this->toString();
    }
	operator std::string() {
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
void parseField(const QJsonObject& obj, const QString& name, const QJsonValue::Type& type, destType& dest) {
	if (obj.contains(name)) {
		QJsonValueWrapper val = obj.value(name);
		if (val.type() == type)
		{
			dest = val;
		}
		else {
			throw LoginFailedException("服务器返回消息含有不可处理的类型");
		}
    }
    else {
        throw LoginFailedException(QString("尝试解析不存在的字段") + name);
    }
}
// 返回的JSON不标准会导致解析错误，解析之前在这里做修正
QByteArray FixJSONHack(const QByteArray& binary) {
    QByteArray result(binary);
    result.replace("'", "\"");
    return result;
}
LoginResp parseResp(const QByteArray& rawBinary) {
    QByteArray binary = FixJSONHack(rawBinary);
    QJsonParseError *error = new QJsonParseError;  
	auto doc = QJsonDocument::fromJson(binary, error);
	if (error->error == QJsonParseError::NoError && !doc.isNull()) {
		if (doc.isObject()) {
			QJsonObject obj = doc.object();
			LoginResp result;
			parseField(obj, "success", QJsonValue::Type::Bool, result.Success);
			parseField(obj, "msg", QJsonValue::Type::String, result.Msg);
			parseField(obj, "action", QJsonValue::Type::String, result.Action);
			parseField(obj, "pop", QJsonValue::Type::Double, result.Pop);
			parseField(obj, "userName", QJsonValue::Type::String, result.Username);
			parseField(obj, "location", QJsonValue::Type::String, result.Location);
			return result;
		}
	}
    qDebug() << error->errorString();  
	throw LoginFailedException(QString("服务器返回消息的格式不正确") + error->errorString() );
}

bool requestLogin(AccountInfo account)
{
	QNetworkAccessManager network;
	qDebug() << "login begin! " << account.user << "||" << account.pwd << endl;


	// throw LoginFailedException("Not Implemented");

	const QUrl LOGIN_URL("http://1.1.1.2/ac_portal/login.php");
	QNetworkRequest request;
	request.setUrl(LOGIN_URL);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply* reply = network.post(request, 
        ( QString() + "opr=pwdLogin"
            + "&userName=" + account.user
            + "&pwd=" + account.pwd
            + "&rememberPwd=0" ).toUtf8()
	);

	QEventLoop eventLoop;
	QTimer::singleShot(10 * 1000, &eventLoop, &QEventLoop::quit);
	QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	if (!reply->isFinished()) {
		throw LoginFailedException("网络请求超时");
	}
    
	QNetworkReply::NetworkError error = reply->error();
	if (error != QNetworkReply::NetworkError::NoError) {
		throw LoginFailedException(QString("网络请求发生错误，错误码：") + error);
	}

	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (statusCode != 200) {
		throw LoginFailedException(QString("服务器返回错误的状态码：") + statusCode);
	}

	QByteArray replyData = reply->readAll();
 
	LoginResp resp = parseResp(replyData);

	if (!resp.Success)
	{
		throw LoginFailedException(QString("服务器返回消息：") + resp.Msg);
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

