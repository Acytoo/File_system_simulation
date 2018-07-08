#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "user.h"

extern Dir 	dir_table[MaxDirNum];//将当前目录文件的内容都载入内存
extern int 	dir_num;//相应编号的目录项数
extern int	 	inode_num;//当前目录的inode编号
extern Inode 	curr_inode;//当前目录的inode结构
extern SuperBlk	super_blk;//文件系统的超级块
extern FILE*	Disk;
extern char	path[40];
extern UserTab user_tab;

/*
init the super block
*/
int init_fs(void)
{
    fseek(Disk, UserTabBeg, SEEK_SET);
    fread(&user_tab, sizeof(user_tab), 1, Disk);//读用户信息

    fseek(Disk, SuperBeg, SEEK_SET);
    fread(&super_blk, sizeof(SuperBlk), 1, Disk);//读取超级块

    inode_num = 0;//当前根目录的inode为0

    if (!open_dir(inode_num)) {
            printf("CANT'T OPEN ROOT DIRECTORY\n");
            return 0;
    }

    return 1;

}

int close_fs(void)
{
        fseek(Disk, SuperBeg, SEEK_SET);
        fwrite(&super_blk, sizeof(SuperBlk), 1, Disk);

        close_dir(inode_num);
        return 1;
}
/*  format, two functions
    如果不能正常初始化，则尝试新建文件并初始化磁盘块
*/
int format_new(void)
{
    Disk = fopen(DISK, "wb+");
    if (!Disk)
    {
        printf("Create disk failed, Please check permissions\n (0x0001)\n");
        system("pause");
        exit(-2);
    }
    format_fs();
    return 1;
}

int format_fs(void)
{
    /*格式化用户*/

    memset(&user_tab, 0, sizeof(user_tab));

    strcpy(user_tab.user_name[0], "root");
    strcpy(user_tab.user_password[0], "7b24afc8bc80e548d66c4e7ff72171c");      //toor  md5 - last(1) -> 32位， 最后一位 '\0'
    strcpy(user_tab.user_name[1], "acytoo");
    strcpy(user_tab.user_password[1], "90340c1085e1e22d8b0d80193d12e50");         //KingJoffrey
    strcpy(user_tab.user_name[2], "user1");
    strcpy(user_tab.user_password[2], "24c9e15e52afc47c225b757e7bee1f9");          //user1
    strcpy(user_tab.user_name[3], "user2");
    strcpy(user_tab.user_password[3], "7e58d63b60197ceb55a1c487989a372");
    strcpy(user_tab.user_name[4], "user3");
    strcpy(user_tab.user_password[4], "92877af70a45fd6a2ed7fe81e1236b7");            //...
    strcpy(user_tab.user_name[5], "user4");
    strcpy(user_tab.user_password[5], "3f02ebe3d7929b091e3d8ccfde2f3bc");
    strcpy(user_tab.user_name[6], "user5");
    strcpy(user_tab.user_password[6], "0a791842f52a0acfbb3a783378c066b");
    strcpy(user_tab.user_name[7], "user6");
    strcpy(user_tab.user_password[7], "affec3b64cf90492377a8114c86fc09");
    strcpy(user_tab.user_name[8], "user7");
    strcpy(user_tab.user_password[8], "3e0469fb134991f8f75a2760e409c6e");
    strcpy(user_tab.user_name[9], "user8");
    strcpy(user_tab.user_password[9], "7668f673d5669995175ef91b5d17194");

    fseek(Disk, UserTabBeg, SEEK_SET);
    fwrite(&user_tab, sizeof(user_tab), 1, Disk);

    /*格式化inode_map,保留根目录*/
    memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
    super_blk.inode_map[0] = 1;
    super_blk.inode_used = 1;

    /*格式化blk_map,保留第一个磁盘块给根目录*/
    memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
    super_blk.blk_map[0] = 1;
    super_blk.blk_used = 1;

    inode_num = 0;//将当前目录改为根目录

                              /*读取根目录的i节点*/
    fseek(Disk, InodeBeg, SEEK_SET);
    fread(&curr_inode, sizeof(Inode), 1, Disk);
    //	printf("%d\n",curr_inode.file_size/sizeof(Dir));

    curr_inode.file_size = 2 * sizeof(Dir);
    curr_inode.blk_num = 1;
    curr_inode.blk_identifier[0] = 0;//第零块磁盘一定是根目录的
    curr_inode.type = Directory;
    // if there is multi-user, the access can be xxx xxx xxx xxx ...
    // each of them define the permission of that user "in sequence"
    curr_inode.access[0] = 1;//可读
    curr_inode.access[1] = 1;//可写
    curr_inode.access[2] = 1;//可执行
    curr_inode.i_atime = time(NULL);
    curr_inode.i_ctime = time(NULL);
    curr_inode.i_mtime = time(NULL);

    fseek(Disk, InodeBeg, SEEK_SET);
    fwrite(&curr_inode, sizeof(Inode), 1, Disk);

    dir_num = 2;//仅.和..目录项有效

    strcpy(dir_table[0].name, ".");
    dir_table[0].inode_num = 0;
    strcpy(dir_table[1].name, "..");
    dir_table[1].inode_num = 0;

    //might change this function, we can receive a parameter to change the path.
    //const char* temp = "acytoo@acytii:";
    //strcpy(path, temp);

    //strcpy(path, "acytoo@acytii:");

    return 1;
}


void set_name(){
    strcpy(dir_table[0].name, ".");
    dir_table[0].inode_num = 0;
    strcpy(dir_table[1].name, "..");
    dir_table[1].inode_num = 0;
}
