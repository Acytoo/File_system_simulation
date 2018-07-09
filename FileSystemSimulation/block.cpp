#include "const.h"
#include "superblk.h"
#include <stdio.h>
#include <iostream>

extern SuperBlk	super_blk;//文件系统的超级块
/*申请未被使用的磁盘块*/
int get_blk()
{
    int i;
    super_blk.blk_used++;
    for (i = 0; i<BlkNum; ++i) {//找到未被使用的块
        if (!super_blk.blk_map[i]) {
            super_blk.blk_map[i] = 1;
            return i;
        }
    }

    return -1;//没有多余的磁盘块
}


/*释放磁盘块*/
int free_blk(int blk_pos)
{
    super_blk.blk_used--;
    super_blk.blk_map[blk_pos] = 0;
    return 0;
}


std::string get_disk_info()
{
    return "Disk:\nTotal Blocks: " + std::to_string(BlkNum) +
            "\nUsed: " + std::to_string(super_blk.blk_used) +
            "\nAvailable: " + std::to_string(BlkNum - super_blk.blk_used);

}
std::string get_inode_info()
{
    return "Inode:\nTotal Inodes: " + std::to_string(InodeNum) +
            "\nUsed: " + std::to_string(super_blk.inode_used) +
            "\nAvailable: " + std::to_string(InodeNum - super_blk.inode_used);
}

int get_disk_percentage(){
    return super_blk.blk_used * 100 / BlkNum;
}
int get_inode_percentage(){
    return super_blk.inode_used * 100 / InodeNum;
}


