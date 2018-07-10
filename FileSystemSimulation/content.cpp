#include "content.h"
#include "ui_content.h"

#include <QToolBar>
#include <QListView>
#include <QMessageBox>
#include <QListWidget>
#include <QContextMenuEvent>
#include <QInputDialog>

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
#include "path.h"

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

char    home_path[10];
int     cut_copy = 0;
char    paste_name[NameLength] = {'\0'};
char    destination_path[50] = {'\0'};
int     paste_ori_inode = 0, paste_dest_inode = 0;
QString comp_name;

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
    btn_goto->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    toolBar->addWidget(btn_goto);

    // add search to tool bar, place holder won't possess any room
    lineEdit_search = new QLineEdit("" ,this);
    lineEdit_search->setPlaceholderText("serach...");
    lineEdit_search->setFixedWidth(100);
    toolBar->addWidget(lineEdit_search);

    // set info pannel
    ui->textEdit_disk_info->setReadOnly(true);
    ui->textEdit_disk_info->setTextColor(QColor(237, 137, 237, 255));        //r,g,b,t
    ui->textEdit_inode_info->setReadOnly(true);
    ui->textEdit_inode_info->setTextColor(QColor(237, 137, 237, 255));



    menu_content = new QMenu;
    action_add_file = new QAction("New file", this);
    action_add_folder = new QAction("New folder", this);
    action_del = new QAction("Delete", this);
    action_detail = new QAction("Properties", this);
    action_cut = new QAction("Cut", this);
    action_copy = new QAction("Copy", this);
    action_paste = new QAction("Past", this);
    action_rename = new QAction("Rename", this);
    action_zip = new QAction("Zip", this);
    action_unzip = new QAction("Unzip", this);
    action_access_change = new QAction("Change permission", this);
    connect(action_add_file, SIGNAL(triggered(bool)), this, SLOT(add_file()));
    connect(action_add_folder, SIGNAL(triggered(bool)), this, SLOT(add_folder()));
    connect(action_del, SIGNAL(triggered(bool)), this, SLOT(del_adjust()));
    connect(action_detail, SIGNAL(triggered(bool)), this, SLOT(pop_detail()));
    connect(action_cut, SIGNAL(triggered(bool)), this, SLOT(cut()));
    connect(action_copy, SIGNAL(triggered(bool)), this, SLOT(copy()));
    connect(action_paste, SIGNAL(triggered(bool)), this, SLOT(paste()));
    connect(action_rename, SIGNAL(triggered(bool)), this, SLOT(rename()));
    connect(action_access_change, SIGNAL(triggered(bool)), this, SLOT(access_change()));
    connect(action_zip, SIGNAL(triggered(bool)), this, SLOT(ui_zip()));
    connect(action_unzip, SIGNAL(triggered(bool)), this, SLOT(ui_unzip()));


    refresh();

    refresh_bulletin();
    //cd function
    connect(btn_goto, SIGNAL(clicked(bool)), this, SLOT(fun()));
    connect(btn_level_up, SIGNAL(clicked(bool)), this, SLOT(cd_parent()));
    connect(lineEdit_path, SIGNAL(returnPressed()), this, SLOT(cd_fun()));

}

content::~content() {
    delete ui;
    fclose(Disk);
}


void content::contextMenuEvent(QContextMenuEvent *event)
{
    //把鼠标位置转化成相对于QWidget的位置，然后判断是否在QListWidget内.
    QPoint i = ui->list_content->mapFromGlobal(event->globalPos());
    QRect rect(QPoint(0,0), ui->list_content->size());
    if (rect.contains(i))
    {
//        m_contextMenu->exec(event->globalPos());

        if(ui->list_content->itemAt(i) != NULL) //如果有item则添加"修改"菜单 [1]*
        {
            menu_content->clear();
            menu_content->addAction(action_rename);
            menu_content->addAction(action_del);
            menu_content->addAction(action_copy);
            menu_content->addAction(action_cut);
            menu_content->addAction(action_zip);
            menu_content->addAction(action_unzip);
            if (comp_name == "root"){
                menu_content->addAction(action_access_change);
            }
            menu_content->addAction(action_detail);
        } else {
            menu_content->clear();
            menu_content->addAction(action_add_file);
            menu_content->addAction(action_add_folder);
            menu_content->addAction(action_paste);
        }
        menu_content->exec(QCursor::pos());

//        QMenu* popMenu = new QMenu(this);
//        popMenu->addAction(new QAction("新建文件", this));
//        popMenu->addAction(new QAction("新建文件夹", this));
//        if(ui->list_content->itemAt(i) != NULL) //如果有item则添加"修改"菜单 [1]*
//        {
//            popMenu->addAction(new QAction("删除", this));
//        }
//        popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
    }

}

void content::ui_zip()
{
    char zip_name[NameLength] = {'\0'};
    char ori_name[NameLength] = {'\0'};
    bool ok;
    QString temp_name = ui->list_content->currentItem()->text();
    QString qstr_zip_name = QInputDialog::getText(this,tr("压缩"),tr("请输入压缩文件名"),QLineEdit::Normal,temp_name,&ok);
    if (ok)
    {
        strcpy(zip_name, qstr_zip_name.toStdString().c_str());
        strcpy(ori_name, temp_name.toStdString().c_str());
        zip(zip_name, ori_name);
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }
}

void content::ui_unzip()
{
    char unzip_name[NameLength] = {'\0'};
    char ori_name[NameLength] = {'\0'};
    bool ok;
    QString temp_name = ui->list_content->currentItem()->text();
    QString qstr_zip_name = QInputDialog::getText(this,tr("解压"),tr("请输入解压文件名"),QLineEdit::Normal,"un_zip_file",&ok);
    if (ok)
    {
        strcpy(unzip_name, qstr_zip_name.toStdString().c_str());
        strcpy(ori_name, temp_name.toStdString().c_str());
        unzip(ori_name, unzip_name);
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }
}




void content::access_change()
{
    bool ok;
    char rule[5] = {'\0'};
    char temp_name[NameLength] = {'\0'};
    cout << "permission change" << endl;
    QString access_rule = QInputDialog::getText(this,tr("更改权限"),tr("请输入权限规则"),QLineEdit::Normal,tr("-r"),&ok);
    if (ok)
    {
        strcpy(rule, access_rule.toStdString().c_str());
        strcpy(temp_name, ui->list_content->currentItem()->text().toStdString().c_str());
        change_mode(rule, temp_name);
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }

}




/**
 * @brief content::cut
 *
 * cut_copy = 1
 *
 */
void content::cut()
{
    cut_copy = 1;
    paste_ori_inode = inode_num;
    cout << "in start cut inode is " << paste_ori_inode << endl;
    strcpy(paste_name, ui->list_content->currentItem()->text().toStdString().c_str());
}

/**
 * @brief content::copy
 * cut_copy = 2;
 */

void content::copy()
{
    cut_copy = 2;
    paste_ori_inode = inode_num;
    cout << "in start copy inode is " << paste_ori_inode << endl;
    strcpy(paste_name, ui->list_content->currentItem()->text().toStdString().c_str());

}
/**
 * @brief content::paste
 *
 * do sth, then cut_copy = 0
 *
 */


void content::paste()
{
    strcpy(destination_path, lineEdit_path->text().toStdString().c_str());
    int temp_inode = inode_num;
    inode_num = paste_ori_inode;
    if (cut_copy == 1)
    {
        //cut
        if (file_move(paste_name, destination_path) != 0){
            QMessageBox::about(this,"Error", "File move error!");
        }

    }
    else
    {
        //copy
        if (file_copy(paste_name, destination_path) != 0){
            QMessageBox::about(this,"Error", "File copy error!");
        }
    }
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
    inode_num = temp_inode;
    paste_ori_inode = 0;
    refresh();
    refresh_bulletin();
    cut_copy = 0;
}

void content::rename()
{
    bool ok;
    char old_name[NameLength] = {'\0'};
    char new_name[NameLength] = {'\0'};
    strcpy(old_name, ui->list_content->currentItem()->text().toStdString().c_str());
    QString file_name = QInputDialog::getText(this,tr("重命名"),tr("请输入新文件名"),QLineEdit::Normal,tr("file_rename"),&ok);
    if (ok)
    {
        strcpy(new_name, file_name.toStdString().c_str());
        file_move(old_name, new_name);
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }
}


void content::add_file(){
    //cout << "add file " << endl;
    bool ok;
    QString file_name = QInputDialog::getText(this,tr("新建文件"),tr("请输入文件名"),QLineEdit::Normal,tr("file"),&ok);
    if (ok) {
        //cout << "create file " << file_name.toStdString() << endl;
        char temp[NameLength] = {'\0'};
        strcpy(temp,file_name.toStdString().c_str());
        if (make_file(inode_num, temp, File) == -1)
        {
            QMessageBox::about(this,"Error", "File already exist!");
        }
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }else {
        cout << "user canceled" << endl;
    }
}
void content::add_folder(){
    //cout << "add folder" << endl;
    bool ok;
    QString folder_name = QInputDialog::getText(this,tr("新建文件夹"),tr("请输入文件夹名"),QLineEdit::Normal,tr("folder"),&ok);
    if (ok) {
        char temp[NameLength] = {'\0'};
        strcpy(temp,folder_name.toStdString().c_str());
        if (make_file(inode_num, temp, Directory) == -1)
        {
            QMessageBox::about(this,"Error", "Folder already exist!");
        }
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();

    }else {
        cout << "user canceled" << endl;
    }
}

/**
 * @brief content::del_adjust
 * Auto delete folder or file
 */
void content::del_adjust(){

    char temp_del_name[NameLength];
    strcpy(temp_del_name, ui->list_content->currentItem()->text().toStdString().c_str());
    remove_file(inode_num, temp_del_name, 0, type_check(temp_del_name));
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
    refresh();
    refresh_bulletin();
}

void content::pop_detail()
{
    char temp_name[NameLength];
    strcpy(temp_name, ui->list_content->currentItem()->text().toStdString().c_str());
    string detail_information = get_detail(temp_name);
    //cout << detail_information << endl;
    QMessageBox::about(this,"Properties",QString::fromStdString(detail_information));
}


/**
 * @brief content::re
 * @param userName
 * receive signal, start this window and receive parameter(user name)
 */
void content::re(QString userName)
{
    strcpy(home_path, userName.toStdString().c_str());
    comp_name = userName;
    user_name->setText("Hello:  " + userName);
    lineEdit_path->setText("/" + userName);
    this->show();
    refresh();
    refresh_bulletin();
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
    refresh_bulletin();
}

/**
 * @brief content::cd_fun
 * 在地址输入栏右键则会进入相应的目录
 */

void content::cd_fun()
{
    //printf("start cd\n");
    QString qstrPath = lineEdit_path->text();
    const char* cpath = qstrPath.toStdString().c_str();
    //printf("the path is %s \n", cpath);   enter_dir(cpath)
    if (enter_dir(cpath) == -1){
        //error
        QMessageBox::about(this,"Warning!","There is no such folder\t\nPlease check again!\t");
        return;
    }
    lineEdit_path->setText(qstrPath);
    //std::cout << "finall " << std::endl;
    refresh();

}

/**
 * @brief content::cd_parent
 * cd upper level
 */
void content::cd_parent()
{
    enter_dir("..");
    refresh();
    lineEdit_path->setText(lineEdit_path->text().section('/', 0, -2));

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

    //. 和 ..有不同的地位和图标

    for (int j=0; j < 2; j++){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/ano.png"));
        item->setText(dir_content[j] + 10);     //10位开始是文件名开始的地方
        item->setTextColor("#eee888000");
        ui->list_content->addItem(item);
    }

    for (int i=2; i<dir_content_num; i++){
        QListWidgetItem *item = new QListWidgetItem;
        char temp_icon_name[11];
        strncpy(temp_icon_name, dir_content[i], 10);
        temp_icon_name[10] = '\0';
        if (!strcmp(temp_icon_name, ":/fold.png"))
        {
            item->setTextColor("#eee888000");
        }
        item->setIcon(QIcon(temp_icon_name));
        item->setText(dir_content[i] + 10);     //10位开始是文件名开始的地方

        //cout << string(temp_icon_name) << " " << string(dir_content[i]) << endl;
        ui->list_content->addItem(item);
    }
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
    if (inode_num == 0 && dir_content_num == 2)
    {
        make_file(inode_num, home_path, Directory);
        refresh();
    }

    //ui->list_content->setViewMode(QListView::IconMode);
    //ui->list_content->sortOrder();
    //手动排序， 我能行。。。
    ui->list_content->sortItems(Qt::AscendingOrder);//Qt::DescendingOrder

}


/**
 * @brief content::on_list_content_itemDoubleClicked
 * @param item
 *
 * 双击事件，首先判断文件类型，然后：如果是文件夹则cd， 文件则vi
 * 2018年7月9日12点18分
 */

void content::on_list_content_itemDoubleClicked(QListWidgetItem *item)
{
    char temp_clicked_name[NameLength];
    //int original_inode = inode_num, target_inode;
    QString original_qstr_path = item->text();

    strcpy(temp_clicked_name, original_qstr_path.toStdString().c_str());
    if (type_check(temp_clicked_name) == File)
    {   //file
        if (file_edit(temp_clicked_name) != 0)
        {
            QMessageBox::about(this,"Error!","Failed to edit file： Permission denied");
        }
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh_bulletin();
        return;
    }
    else //因为通过双击文件名来进行操作，所以这个函数不可能遇到不存在的文件，无需考虑第三种情况
    {   //folder
        if (enter_dir(temp_clicked_name) == -1){
            //error
            QMessageBox::about(this,"Warning!","There is no such folder\t\nPlease check again!\t");
            return;
        }
    }
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
    refresh();
    //设置目录名

    char original_c_path_full[130] = {'\0'};      //每一级最大的目录名是30， 目录可以有很多级，所以这个目录要很长很长。。130可能不够。。
    strcpy(original_c_path_full, lineEdit_path->text().toStdString().c_str());

    if (!(strcmp(temp_clicked_name, "..")))
    {
//        cout << "level up " << endl;
//        char *result = strrchr(original_c_path_full, '/');
//        int target_pos = strlen(original_c_path_full) - strlen(result-);
//        char res[30];
//        strncat(res, original_c_path_full, target_pos);
//        cout << string(res) << endl;

        //太蠢了， QString有split函数，我却自己通过C语言写。。。真实太蠢了
        lineEdit_path->setText(lineEdit_path->text().section('/', 0, -2));
        //cout << original_qstr_path.section('/', -1,-1).toStdString() << endl;

    }
    else if ((strcmp(temp_clicked_name, ".")))  //如果不是.和..
    {
        lineEdit_path->setText(lineEdit_path->text() + "/" + original_qstr_path);
    }


}

void content::refresh_bulletin()
{
    ui->textEdit_disk_info->setText(QString::fromStdString(get_disk_info()));
    ui->textEdit_inode_info->setText(QString::fromStdString(get_inode_info()));
    ui->progressBar_disk->setValue(get_disk_percentage());
    cout << "disk " << to_string(get_disk_percentage()) << " inode " << to_string(get_inode_percentage()) << endl;
    ui->progressBar_inode->setValue(get_inode_percentage());
    close_dir(inode_num);
    close_fs();//每执行完一条指令就保存一次数据
}



void content::on_btn_format_clicked()
{


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Are you sure ?"),
                                        "This will erase all your data on this disk!",
                                        QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
        format_fs();
        lineEdit_path->setText("/");
        close_dir(inode_num);
        close_fs();//每执行完一条指令就保存一次数据
        refresh();
        refresh_bulletin();
    }

}

void content::on_btn_log_out_clicked()
{
    fclose(Disk);
    this->close();
    emit log_out();
}
