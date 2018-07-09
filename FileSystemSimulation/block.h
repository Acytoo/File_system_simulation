#ifndef BLOCK_H
#define BLOCK_H
#include <iostream>

int free_blk(int);//释放相应的磁盘块
int get_blk(void);//获取磁盘块
//void show_disk_usage();
//void show_inode_usage();

std::string get_disk_info();    //显示磁盘使用情况
std::string get_inode_info();   //显示inode节点使用情况
int get_disk_percentage();
int get_inode_percentage();


#endif
