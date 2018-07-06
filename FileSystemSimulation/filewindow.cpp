#include "filewindow.h"
#include "ui_filewindow.h"
#include "functions.h"
#include "mainwindow.h"

#include <QMessageBox>
#include <QApplication>

//void functinalInit();
filewindow::filewindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::filewindow) {
    ui->setupUi(this);

    //  always init ui first, then init the filesystem
    FILE* virtualDisk = fopen("VirtualDisk.yt", "r");
    if (virtualDisk == NULL) {
        //printf("Virtual Disk not exist, initial disk now!");
        initDisk();
    }
    if (!mount()) {
        QMessageBox::about(this,"warning!", "模拟文件系统的文件未能正确初始化，请关闭重试！");
    }

}


filewindow::~filewindow() {
    delete ui;
}

// set user name on file ui
void filewindow::receiveUserName(QString userName) {
    ui->txt_user_label->setText(userName);
}


void filewindow::on_btn_logout_clicked() {
    logout();
    this->close();
    //QApplication a();
    //MainWindow w;
    //w.show();
    //std::cout << "after close\n";
    //a.exec();

}
