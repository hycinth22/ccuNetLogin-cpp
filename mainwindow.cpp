#include "stdafx.h"
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "login.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    ui.setupUi(this);

    AccountInfo acc = getSavedAccount();
    ui.lineEdit_user->setText(acc.user);
    ui.lineEdit_pwd->setText(acc.pwd);
}

void MainWindow::on_pushButton_login_clicked()
{
    QString user = ui.lineEdit_user->text();
    QString pwd = ui.lineEdit_pwd->text();
    bool rem = ui.checkBox_rem->isChecked();

    if (user.length() < 1 || pwd.length() < 1) {
        QMessageBox::warning(this, "错误输入", "请输入合法信息！");
        return; 
    }
    qDebug() << "user:" << user << endl;
    qDebug() << "pwd:" << pwd << endl;

	try {
		bool success = login(AccountInfo{user, pwd}, rem);
		if (success)
		{
            QMessageBox::information(this, "登录", "登录成功！");
			std::exit(0);
		}
		else {
            QMessageBox::critical(this, "登录", "登录失败！");
		}
	}
	catch (LoginFailedException& e) {
        QMessageBox::critical(this, "登录", QString("登录失败！原因是：\n") + e.reason);
	}

}
