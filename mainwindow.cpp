#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <utility>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include "login.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	AccountInfo acc = getSavedAccount();
	ui->lineEdit_user->setText(acc.user.c_str());
	ui->lineEdit_pwd->setText(acc.pwd.c_str());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void showMsg(const QString& text, QWidget *parent = Q_NULLPTR) {
	QMessageBox msgbox(parent);
	msgbox.setWindowTitle(u8"信息框");
	msgbox.setText(text);
	msgbox.exec();
}

#include <iostream>
void MainWindow::on_pushButton_login_clicked()
{
    using namespace std;
    QString user = ui->lineEdit_user->text();
    QString pwd = ui->lineEdit_pwd->text();
    bool rem = ui->checkBox_rem->isChecked();

    if (user.length() < 1 || pwd.length() < 1){
		showMsg(u8"请输入合法信息！");
        return; 
    }
    qDebug() << "user:" << user << endl;
    qDebug() << "pwd:" << pwd << endl;

	try {
		bool success = login(AccountInfo{
			user.toStdString(),
			pwd.toStdString()
			},
			rem
		);
		if (success)
		{
			showMsg(u8"登录成功！");
			std::exit(0);
		}
		else {
			showMsg(u8"登录失败！");
		}
	}
	catch (LoginFailedException& e) {
		string msg(u8"登录失败！原因是：");
		msg += e.what();
		showMsg(msg.c_str());
	}

}