#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QLineEdit>

#include "content.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    // send user name from login ui to main file ui
    // I use c++ string here, not QString...might cause problem  -> QString is more convinent...
    void sendUserName(QString userName);

private slots:

    void on_btn_login_clicked();

    void on_input_password_returnPressed();

private:
    Ui::MainWindow *ui;
    content* fileManager;
    //QLineEdit input_password;
};


#endif // MAINWINDOW_H
