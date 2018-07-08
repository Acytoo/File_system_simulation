#ifndef SUPERBLK_H
#define SUPERBLK_H
#include "const.h"

/*
    super block
    1: used
    0: free
*/
typedef struct {
    int inode_map[InodeNum];//i节点位图
    int blk_map[BlkNum];//磁盘块位图
    int inode_used;//已被使用的i节点数目
    int blk_used;//已被使用的磁盘块数目
}SuperBlk;

#endif
