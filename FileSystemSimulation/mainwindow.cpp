#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "zip.h"
#include "fs.h"
#include "file.h"
#include "block.h"
#include "md5.h"
#include "user.h"
#include "login.h"

#include <iostream>
#include <string>
#include <QMessageBox>

extern content *fileManager;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);



}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_btn_login_clicked() {
    QString userName = ui->input_username->text();
    QString password = ui->input_password->text();
    std::string user = userName.toStdString();
    std::string pass= password.toStdString();

    bool flag = login(user, pass);
    if (flag){
        std::cout << "login successfully" << std::endl;
        ui->txt_err_message->setText("欢迎回来： " + userName);
        //emit sendUserName(userName);
        this->close();
        emit showmain(userName);
    }
    else {
        ui->txt_err_message->setText("登录失败，请重试！");
    }

}


void MainWindow::on_input_password_returnPressed()
{
    on_btn_login_clicked();
}
