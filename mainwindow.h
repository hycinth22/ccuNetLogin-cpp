#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void on_pushButton_login_clicked();

private:
    Ui::MainWindow ui;
};

#endif // MAINWINDOW_H
