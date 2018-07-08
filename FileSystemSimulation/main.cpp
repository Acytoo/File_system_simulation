#include "mainwindow.h"
#include "content.h"
#include<content.h>

#include <QApplication>


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

UserTab  user_tab;       // 10user
Dir 	dir_table[MaxDirNum];//将当前目录文件的内容都载入内存
int 	dir_num;//相应编号的目录项数
int	 	inode_num;//当前目录的inode编号
FILE*	Disk;
Inode 	curr_inode;//当前目录的inode结构
SuperBlk	super_blk;//文件系统的超级块
FILETIME BuffModifyTimeBeforeEdit;
FILETIME BuffModifyTimeAfterEdit;

//char*	command[] = { "mkfs","q","mkdir","rmdir","cd","ls","touch","rm","vi",
//                      "cp","mv", "stat", "chmod", "zip", "unzip", "man", "df", "ps"};
char	path[40] = "acytoo@acytii:";



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    content c;
    QObject::connect(&w,SIGNAL(showmain()),&c,SLOT(re()));
    w.setStyleSheet("QMainWindow {background: 'white';}");
    w.show();
    //c.show();



    Disk = fopen(DISK, "rb+");
    if (!Disk) {
        printf("open fail \n(0x0000)\n try to format a new disk\n");
        if (!format_new())
            exit(-1);
        printf("disk format success! you can operate your new fisk!!!\n");
    }
    if (init_fs()){
        cout << "init fs success" << endl;
    }
    //fclose(Disk);


    return a.exec();
}
