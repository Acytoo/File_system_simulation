#include "content.h"
#include "ui_content.h"
//#include "functions.h"
#include "dir.h"

#include <iostream>
#include <QToolBar>
#include<QListView>

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

    // add level up button
    btn_level_up = new QPushButton();
    btn_level_up->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    toolBar->addWidget(btn_level_up);

    // add path to tool bar
    path = "/";
    lineEdit_path = new QLineEdit(path,this);
    toolBar->addWidget(lineEdit_path);

    //forward
    btn_goto = new QPushButton();
    btn_goto->setText("->");
    toolBar->addWidget(btn_goto);



    // add search to tool bar, place holder won't possess any room
    lineEdit_search = new QLineEdit("" ,this);
    lineEdit_search->setPlaceholderText("serach...");
    lineEdit_search->setFixedWidth(100);
    toolBar->addWidget(lineEdit_search);


    //cd function
    connect(btn_goto, SIGNAL(clicked(bool)), this, SLOT(fun()));
    connect(lineEdit_path, SIGNAL(returnPressed()), this, SLOT(cd_fun()));
    printf("conncted\n\n\n");

}

content::~content() {
    delete ui;
}

void content::re()
{
    this->show();
}

void content::fun(){
    emit start();
    printf("emitted\n");
}
void content::receiveUserName(QString userName) {
    user_name->setText("hello: " + userName);
}

void content::cd_fun() {
    //printf("start cd\n");
    QString qstrPath = lineEdit_path->text();
    const char* cpath = qstrPath.toStdString().c_str();
    //printf("the path is %s \n", cpath);
    if (enter_dir(cpath)){
        lineEdit_path->setText(cpath);
    }
    else{
        lineEdit_path->setText("no");
    }

    lineEdit_path->setText("set without if");
    std::cout << "finall " << std::endl;

}




