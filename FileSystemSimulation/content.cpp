#include "content.h"
#include "ui_content.h"

#include <QToolBar>
#include <QListView>
#include <QMessageBox>

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
extern char     dir_content[TempLength][NameLength + 11];   //10 for :/file.png and 1 for '\0'


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

void content::re(QString userName)
{
    user_name->setText("Hello:  " + userName);
    this->show();
    refresh();
}
/**
 * @brief content::fun
 * stupid test function for debug...
 */
void content::fun(){


//    std::cout << "hello" << std::endl;
//    int dir_content_nmu = ls_dir_v2(inode_num);
//    std::cout << dir_content_nmu << std::endl;

//    close_dir(inode_num);
//    close_fs();//每执行完一条指令就保存一次数据

    refresh();
}

/**
 * @brief content::cd_fun
 * 在地址输入栏右键则会进入相应的目录
 */

void content::cd_fun() {
    //printf("start cd\n");
    QString qstrPath = lineEdit_path->text();
    const char* cpath = qstrPath.toStdString().c_str();
    //printf("the path is %s \n", cpath);   enter_dir(cpath)
    if (enter_dir(cpath) == -1){
        //error
        QMessageBox::about(this,"Warning!","There is no such folder\nPlease check again!");
        return;
    }
    lineEdit_path->setText(qstrPath);
    //std::cout << "finall " << std::endl;
    refresh();

}

/**
 * @brief content::refresh
 * 刷新页面文件夹与文件，调用前一定要执行ls_dir_v2(inode_num）;
 * 执行后千万千万要记得执行close
 */

void content::refresh()
{
    ui->list_content->clear();

    int dir_content_num = ls_dir_v2(inode_num);

    for (int i=0; i<dir_content_num; i++){
        QListWidgetItem *item = new QListWidgetItem;
        char temp_icon_name[11];
        strncpy(temp_icon_name, dir_content[i], 10);
        temp_icon_name[10] = '\0';
        item->setIcon(QIcon(temp_icon_name));
        item->setText(dir_content[i] + 10);     //10位开始是文件名开始的地方
        //cout << string(temp_icon_name) << " " << string(dir_content[i]) << endl;
        ui->list_content->addItem(item);
    }
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
}



