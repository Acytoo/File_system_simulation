// OS_fs.cpp : Defines the entry point for the console application.
//	main()

/**
*	Copyright 2018, Alec Chen
*	All Rights Reserved. 
*	20154479 1506
*	acytoo@gmail.com
*	2018年7月3日15点00分
*/

//#include "stdafx.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

//since these code is written in c++. DO NOT USE DEFINE, use const instead. 

const unsigned short BLOCK_SIZE = 512;		/*size of a block*/
const unsigned short SYS_OPEN_FILE = 40;	/*max number of system-open files*/
const unsigned short DIR_NUM = 128;			/*max number of files(folders) in each node*/
const unsigned short NADDR = 10;			/*maxmum blocks A node can point to*/
const unsigned int FILE_SIZE_MAX = (NADDR - 2) * BLOCK_SIZE + BLOCK_SIZE / sizeof(int) * BLOCK_SIZE;/*max file size*/
const unsigned short BLOCK_NUM = 512;		/*the number of blocks*/
const unsigned short INODE_SIZE = 128;		/*the size of inode*/
const unsigned short INODE_NUM = 256;		/*the number of inode*/
const unsigned int INODE_START = 3 * BLOCK_SIZE;	/*start pointer of the inode*/
const unsigned int DATA_START = INODE_START + INODE_NUM * INODE_SIZE;/*the start pointer of the real-file-content*/
const unsigned int ACCOUNT_NUM = 8;			/*eight users user1..., I prefer acytoo, root...*/
const unsigned int DIRECTORY_NUM = 16;		/*the number of the dir*/
const unsigned short FILE_NAME_LENGTH = 14;	/*file name length, DIR_SIZE, defined in the ppt, plus two inode*/
const unsigned short USER_NAME_LENGTH = 13;	/*length of user name, since I DO NOT WANT to use that stupid name: user2 */
const unsigned short USER_PASSWORD_LENGTH = 32;/*the length of md5, since the password is stored as md5 in my file system*/
const unsigned short MAX_PERMISSION = 511;	/*the max permission of a file, no 777 */
const unsigned short MAX_OWNER_PERMISSION = 448;

/*
permissions, these shift expression not working on ARM machine
*/
const unsigned short ELSE_E = 1;
const unsigned short ELSE_W = 1 << 1;
const unsigned short ELSE_R = 1 << 2;
const unsigned short GRP_E = 1 << 3;
const unsigned short GRP_W = 1 << 4;
const unsigned short GRP_R = 1 << 5;
const unsigned short OWN_E = 1 << 6;
const unsigned short OWN_W = 1 << 7;
const unsigned short OWN_R = 1 << 8;

struct inode {
	unsigned int i_ino;			//Identification of the inode.
	unsigned int di_addr[NADDR];//Number of data blocks where the file stored.
	unsigned short di_number;	//Number of associated files.
	unsigned short di_mode;		//0 stands for a directory, 1 stands for a file.
	unsigned short icount;		//link number
	unsigned short permission;	//file permission
	unsigned short di_uid;		//File's user id.
	unsigned short di_grp;		//File's group id
	unsigned short di_size;		//File size.
	char time[83];
};

struct filsys {
	unsigned short s_num_inode;			//Total number of inodes.
	unsigned short s_num_finode;		//Total number of free inodes.
	unsigned short s_size_inode;		//Size of an inode.

	unsigned short s_num_block;			//Total number of blocks.
	unsigned short s_num_fblock;		//Total number of free blocks.
	unsigned short s_size_block;		//Size of a block.

	unsigned int special_stack[50];
	int special_free;
};

struct directory {
	char fileName[SYS_OPEN_FILE][FILE_NAME_LENGTH];
	unsigned int inodeID[DIRECTORY_NUM];
};

struct userPsw {
	unsigned short userID[ACCOUNT_NUM];
	char userName[ACCOUNT_NUM][USER_NAME_LENGTH];
	char password[ACCOUNT_NUM][USER_PASSWORD_LENGTH];
	unsigned short groupID[ACCOUNT_NUM];
};





















int main() {
    return 0;
}

