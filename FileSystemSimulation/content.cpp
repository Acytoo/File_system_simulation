#include "content.h"
#include "ui_content.h"

#include <QToolBar>
#include<QListView>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <string>

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

using namespace std;


extern UserTab  user_tab;       // 10user
extern Dir      dir_table[MaxDirNum];//将当前目录文件的内容都载入内存
extern int      dir_num;//相应编号的目录项数
extern int	 	inode_num;//当前目录的inode编号
extern FILE*	Disk;
extern Inode 	curr_inode;//当前目录的inode结构
extern SuperBlk	super_blk;//文件系统的超级块
extern FILETIME BuffModifyTimeBeforeEdit;
extern FILETIME BuffModifyTimeAfterEdit;


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

    refresh();


    //cd function
    connect(btn_goto, SIGNAL(clicked(bool)), this, SLOT(fun()));
    connect(lineEdit_path, SIGNAL(returnPressed()), this, SLOT(cd_fun()));
    printf("conncted\n\n\n");

}

content::~content() {
    delete ui;
    fclose(Disk);
}

void content::re()
{
    this->show();
}

void content::fun(){





//    /*指令集合*/
//    char*	command[] = { "mkfs","q","mkdir","rmdir","cd","ls","touch","rm","vi",
//                          "cp","mv", "stat", "chmod", "zip", "unzip", "man", "df", "ps"};
//    char	path[40] = "acytoo@acytii:";

//    char comm[NameLength], name[NameLength],
//             cp_name[NameLength], mv_name[NameLength],
//             zip_name[NameLength];

//    char parameter[10];
//    int i, quit = 0, choice;

//    //Disk = fopen(DISK, "rb+");
//    if (!Disk) {
//        printf("open fail \n(0x0000)\n try to format a new disk\n");
//        if (!format_new())
//            exit(-1);
//        printf("disk format success! you can operate your new fisk!!!\n");
//    }
//    format_fs();
//    init_fs();

    set_name();
    show_dir(inode_num);
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据









//    Disk = fopen(DISK, "rb+");
//    init_fs();
//    show_dir(0);

    //std::cout << "hello" << std::endl;
    //std::string res = ls_dir(0);
    //std::cout << res << std::endl;
}
void content::receiveUserName(QString userName) {
    user_name->setText("hello: " + userName);
}

void content::cd_fun() {
    //printf("start cd\n");
    QString qstrPath = lineEdit_path->text();
    const char* cpath = qstrPath.toStdString().c_str();
    //printf("the path is %s \n", cpath);   enter_dir(cpath)
    if (1){
        lineEdit_path->setText(cpath);
    }
    else{
        lineEdit_path->setText("no");
    }

    lineEdit_path->setText("set without if");
    std::cout << "finall " << std::endl;

}


void content::refresh()
{
//    for (;;){
        QListWidgetItem *item = new QListWidgetItem;
        item->setText("folder");
        item->setIcon(QIcon(":/folder.png"));

        QListWidgetItem *item1 = new QListWidgetItem;
        item1->setText("file");
        item1->setIcon(QIcon(":/file.png"));


        ui->list_content->addItem(item);
        ui->list_content->addItem(item1);
       // break;
//    }



}



