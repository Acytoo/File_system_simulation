#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "functions.h"
#include "filewindow.h"

#include <iostream>
#include <string>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    //init ui finished, let's init our file system
    FILE* virtualDisk = fopen("VirtualDisk.yt", "r");
    if (virtualDisk == NULL) {
        //printf("Virtual Disk not exist, initial disk now!");
        initDisk();
    }
    if (!mount()) {
        QMessageBox::about(this,"warning!", "模拟文件系统的文件未能正确初始化，请关闭重试！");
    }


    fileManager = new content();
    connect(this, SIGNAL(sendUserName(QString)), fileManager, SLOT(receiveUserName(QString)));

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
        emit sendUserName(userName);


        fileManager->show();
        this->close();





//        filewindow myfilewindow;
//        myfilewindow.setModal(true);
        //this->close();

        //connect(myfilewindow, SIGNAL(sendUserName(userName)))
//        myfilewindow.exec();



    }
    else {
        ui->txt_err_message->setText("登录失败，请重试！");
    }

}


void MainWindow::on_input_password_returnPressed()
{
    on_btn_login_clicked();
}
