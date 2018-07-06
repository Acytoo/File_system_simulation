#include "content.h"
#include "ui_content.h"
#include "functions.h"

#include <iostream>
#include <QToolBar>

content::content(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::content) {
    ui->setupUi(this);
    // add tool bar
    QToolBar *toolBar = addToolBar("");
    //set logged in user name
    user_name = new QLabel(this, nullptr);
    user_name->setFixedWidth(100);
    toolBar->addWidget(user_name);
    // add path to tool bar
    path = "/";
    lineEdit_path = new QLineEdit(path,this);
    toolBar->addWidget(lineEdit_path);
    // add search to tool bar, place holder won't possess any room
    lineEdit_search = new QLineEdit("" ,this);
    lineEdit_search->setPlaceholderText("serach...");
    lineEdit_search->setFixedWidth(100);
    toolBar->addWidget(lineEdit_search);




    //cd function
    connect(lineEdit_path, SIGNAL(returnPressed()), this, SLOT(cd_fun()));
}

content::~content() {
    delete ui;
}

void content::receiveUserName(QString userName) {
    user_name->setText("hello: " + userName);
}

void content::cd_fun() {
    QString qstrPath = lineEdit_path->text();
    std::string strPath = qstrPath.toStdString();
    //std::cout << strPath[2] << std::endl;
    bool flag = cd(strPath);

}




