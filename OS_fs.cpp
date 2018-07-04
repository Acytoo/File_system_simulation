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


/*
* 	int fseek ( FILE * stream, long int offset, int origin );
*	SEEK_SET	Beginning of file
*	SEEK_CUR	Current position of the file pointer
*	SEEK_END	End of file *

*	size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );

*/




#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

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


FILE* file = NULL;
filsys superBlock;
unsigned short inode_bitmap[INODE_NUM];	/*1: used; 0: unused*/
userPsw users;							/*user infomation*/
unsigned short userID = ACCOUNT_NUM;	/*curent logged in user*/
char userName[USER_NAME_LENGTH + 6];	/*6: the length of your 'pc' name*/
directory currentDirectory;
char whole_disk_empty[100][14];					/*current folder stack, initial all this space to 0 at the beginning*/
unsigned short dir_pointer;

void find_free_block(unsigned int &inode_number) {
	fseek(file, BLOCK_SIZE, SEEK_SET);		/*SEEK_SET move the pointer to the beginning of a file*/
	fread(&superBlock, sizeof(filsys), 1, file);/*read 1 from file, length is sizeof, to superBlock*/
	if (superBlock.special_free == 0) {
		if (superBlock.special_stack[0] == 0) {
			printf("no enough space!\n");
			return;
		}
		unsigned int stack[51];

		for (int i = 0; i < 50; i++) {
			stack[i] = superBlock.special_stack[i];
		}
		stack[50] = superBlock.special_free;
		fseek(file, DATA_START + (superBlock.special_stack[0] - 50) * BLOCK_SIZE, SEEK_SET);
		fwrite(stack, sizeof(stack), 1, file);

		fseek(file, DATA_START + superBlock.special_stack[0] * BLOCK_SIZE, SEEK_SET);
		fread(stack, sizeof(stack), 1, file);
		for (int i = 0; i < 50; i++) {
			superBlock.special_stack[i] = stack[i];
		}
		superBlock.special_free = stack[50];
	}
	inode_number = superBlock.special_stack[superBlock.special_free];
	superBlock.special_free--;
	superBlock.s_num_fblock--;
	fseek(file, BLOCK_SIZE, SEEK_SET);
	fwrite(&superBlock, sizeof(filsys), 1, file);
}

void recycle_block(unsigned int &inode_number) {
	fseek(file, BLOCK_SIZE, SEEK_SET);
	fread(&superBlock, sizeof(filsys), 1, file);/*read 1 from file, length is sizeof, to superBlock*/
	if (superBlock.special_free == 49) {
		unsigned int block_num;
		unsigned int stack[51];
		if (superBlock.special_stack[0] == 0)
			block_num = 499;
		else
			block_num = superBlock.special_stack[0] - 50;
		for (int i = 0; i < 50; i++) {
			stack[i] = superBlock.special_stack[i];
		}
		stack[50] = superBlock.special_free;
		fseek(file, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
		fwrite(stack, sizeof(stack), 1, file);
		block_num -= 50;
		fseek(file, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
		fread(stack, sizeof(stack), 1, file);
		for (int i = 0; i < 50; i++) {
			superBlock.special_stack[i] = stack[i];
		}
		superBlock.special_free = stack[50];
	}
	superBlock.special_free++;
	superBlock.s_num_fblock++;
	superBlock.special_stack[superBlock.special_free] = inode_number;
	fseek(file, BLOCK_SIZE, SEEK_SET);
	fwrite(&superBlock, sizeof(filsys), 1, file);
}
/**
*	format the disk, erase all the data stored in our disk
*	if the program run on a brand new machine, call this function
*/
bool initDisk() {
	/*
	*	1. Create a empty file to emulate the file system.
	*/
	FILE* file = fopen("./VirtualDisk.yt", "wb+");
	if (file == NULL) {
		printf("Fail to initialize the file system!\n");
		return false;
	}

	/*
	*	2. Initialize super block.
	*/
	filsys superBlock;
	superBlock.s_num_inode = INODE_NUM;
	superBlock.s_num_block = BLOCK_NUM + 3 + 64; //3代表0空闲块、1超级块、2Inode位示图表,64块存inode
	superBlock.s_size_inode = INODE_SIZE;
	superBlock.s_size_block = BLOCK_SIZE;
	//Root directory and accounting file will use some inodes and blocks.
	superBlock.s_num_fblock = BLOCK_NUM - 2;
	superBlock.s_num_finode = INODE_NUM - 2;
	superBlock.special_stack[0] = 99;
	for (int i = 1; i < 50; i++) {
		superBlock.special_stack[i] = 49 - i;
	}
	superBlock.special_free = 47;
	//Write super block into file.
	fseek(file, BLOCK_SIZE, SEEK_SET);
	fwrite(&superBlock, sizeof(filsys), 1, file);
	fseek(file, BLOCK_SIZE, SEEK_SET);
	fread(&superBlock, sizeof(filsys), 1, file);

	/*
	*	3. Initialize inode and block bitmaps.
	*/
	unsigned short inode_bitmap[INODE_NUM];
	//Root directory and accounting file will use some inodes and blocks.
	memset(inode_bitmap, 0, INODE_NUM);
	inode_bitmap[0] = 1;
	inode_bitmap[1] = 1;
	//Write bitmaps into file.
	fseek(file, 2 * BLOCK_SIZE, SEEK_SET);
	fwrite(inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, file);

	//成组链接
	unsigned int stack[51];
	for (int i = 0; i < BLOCK_NUM / 50; i++) {
		memset(stack, 0, sizeof(stack));
		for (unsigned int j = 0; j < 50; j++) {
			stack[j] = (49 + i * 50) - j;
		}
		stack[0] = 49 + (i + 1) * 50;
		stack[50] = 49;
		fseek(file, DATA_START + (49 + i * 50)*BLOCK_SIZE, SEEK_SET);
		fwrite(stack, sizeof(unsigned int) * 51, 1, file);
	}
	memset(stack, 0, sizeof(stack));
	for (int i = 0; i < 12; i++) {
		stack[i] = 511 - i;
	}
	/*
	*	write node info to disk
	*/
	stack[0] = 0;
	stack[50] = 11;
	fseek(file, DATA_START + 511 * BLOCK_SIZE, SEEK_SET);
	fwrite(stack, sizeof(unsigned int) * 51, 1, file);

	fseek(file, DATA_START + 49 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 99 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 149 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 199 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 249 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 299 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 349 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 399 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 449 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 499 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);
	fseek(file, DATA_START + 511 * BLOCK_SIZE, SEEK_SET);
	fread(stack, sizeof(unsigned int) * 51, 1, file);



	/*
	*	4. Create root directory.
	*/
	//Create inode
	//Now root directory contain 1 accounting file.
	inode iroot_tmp;
	iroot_tmp.i_ino = 0;					//Identification
	iroot_tmp.di_number = 2;				//Associations: itself and accouting file
	iroot_tmp.di_mode = 0;					//0 stands for directory
	iroot_tmp.di_size = 0;					//"For directories, the value is 0."
	memset(iroot_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
	iroot_tmp.di_addr[0] = 0;				//Root directory is stored on 1st block. FFFFFF means empty.
	iroot_tmp.permission = MAX_OWNER_PERMISSION;
	iroot_tmp.di_grp = 1;
	iroot_tmp.di_uid = 0;					//Root user id.
	iroot_tmp.icount = 0;
	time_t t = time(0);
	strftime(iroot_tmp.time, sizeof(iroot_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
	iroot_tmp.time[64] = 0;
	fseek(file, INODE_START, SEEK_SET);
	fwrite(&iroot_tmp, sizeof(inode), 1, file);

	//Create directory file.
	directory droot_tmp;
	memset(droot_tmp.fileName, 0, sizeof(char) * DIRECTORY_NUM * FILE_NAME_LENGTH);
	memset(droot_tmp.inodeID, -1, sizeof(unsigned int) * DIRECTORY_NUM);
	strcpy(droot_tmp.fileName[0], ".");
	droot_tmp.inodeID[0] = 0;
	strcpy(droot_tmp.fileName[1], "..");
	droot_tmp.inodeID[1] = 0;
	//A sub directory for accounting files
	strcpy(droot_tmp.fileName[2], "pw");
	droot_tmp.inodeID[2] = 1;

	//Write
	fseek(file, DATA_START, SEEK_SET);
	fwrite(&droot_tmp, sizeof(directory), 1, file);

	/*
	*	5. Create accouting file.
	*/
	//Create inode
	inode iaccouting_tmp;
	iaccouting_tmp.i_ino = 1;					//Identification
	iaccouting_tmp.di_number = 1;				//Associations
	iaccouting_tmp.di_mode = 1;					//1 stands for file
	iaccouting_tmp.di_size = sizeof(userPsw);	//File size
	memset(iaccouting_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
	iaccouting_tmp.di_addr[0] = 1;				//Root directory is stored on 1st block.
	iaccouting_tmp.di_uid = 0;					//Root user id.
	iaccouting_tmp.di_grp = 1;
	iaccouting_tmp.permission = 320;
	iaccouting_tmp.icount = 0;
	t = time(0);
	strftime(iaccouting_tmp.time, sizeof(iaccouting_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
	iaccouting_tmp.time[64] = 0;
	fseek(file, INODE_START + INODE_SIZE, SEEK_SET);
	fwrite(&iaccouting_tmp, sizeof(inode), 1, file);

	//Create accouting file.
	userPsw paccouting_tmp;
	memset(paccouting_tmp.userName, 0, sizeof(char) * USER_NAME_LENGTH * ACCOUNT_NUM);
	memset(paccouting_tmp.password, 0, sizeof(char) * USER_PASSWORD_LENGTH * ACCOUNT_NUM);
	//Only default user 'admin' is registered. Default password is 'admin'.
	strcpy(paccouting_tmp.userName[0], "root");
	strcpy(paccouting_tmp.userName[1], "acytoo");
	strcpy(paccouting_tmp.password[0], "toor");
	strcpy(paccouting_tmp.password[1], "KingJoffrey");
	//0 stands for super user. Other IDs are only used to identify users.
	for (unsigned short i = 0; i < ACCOUNT_NUM; i++) {
		paccouting_tmp.userID[i] = i;
	}
	paccouting_tmp.groupID[0] = 1;
	paccouting_tmp.groupID[1] = 2;
	//Write
	fseek(file, DATA_START + BLOCK_SIZE, SEEK_SET);
	fwrite(&paccouting_tmp, sizeof(userPsw), 1, file);

	//Close file.
	fclose(file);

	return true;
};

/*
*	mount the virtual disk when we boot up
*	if everything is right, return true
*/
bool mount() {
	virtualDisk = fopen("./VirtualDisk.yt", "rb+");
	if (virtualDisk == NULL) {
		/*stupid error, I don't think this kind if situation would happen, but I'll write it*/
		printf("mount disk failed in mount functionn\n");
		return false;
	}
	/*first read the superBlock, bitmaps, userinfo, current directory*/
	fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
	fread(&superBlock, sizeof(superBlock), 1, virtualDisk);

	fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
	fread(inode_bitmap, sizeof(unsigned int) * INODE_NUM, 1, virtualDisk);

	fseek(virtualDisk, DATA_START, SEEK_SET);
	fread(&currentDirectory, sizeof(directory), 1, virtualDisk);

	fseek(file, DATA_START + BLOCK_SIZE, SEEK_SET);
	fread(&users, sizeof(userPsw), 1, virtualDisk);

	return true;
}

/**
*	every time we boot up or log out, we need to login to operate out file system
*/
bool login(const char* user, const char* passwd) { return false; }

/*
*	log out to switch user
*/
bool logout(const char* user) { return false; }

/**
*       touch a file, to create an empty file
*/
bool touch() { return false; }

/**
*	delete a file or folder
*/
bool rm() { return false; }


/*
*	move a file or folder, or rename it
*
*/
bool mv() { return false; }

/*
*	switch work space
*/
bool cd() { return false; }

/*
*	show current work space
*/
bool pwd() { return false; }

/**
*	change permission
*/
bool chmod() { return false; }

/*
*	list all the file info
*/
bool ls() { return false; }

/*
*	save content
*/
bool save() { return false; }

/**
*	load file
*/
bool load() { return false; }

/*
*	view file content
*/
bool vim() { return false; }

/*
*	list all the file(folders) with detail infomation
*/
bool lla() { return false; }

/*
*	exit su, exit program
*/

bool exit() { return false; }

















void simulation_start() {
	mount();
	/**
	* may be a acytoo in the future
	*/
	printf("started\n");
}





int main() {
	memset(whole_disk_empty, 0, sizeof(whole_disk_empty));/*initial the work space, set all the bits on our disk to 0*/
	dir_pointer = 0;
	/*read the file, if the file not exist, then we init the disk for the first use*/
	FILE* virtualDisk = open("VirtualDisk.yt", "r");
	if (virtualDisk == NULL) {
		printf("Virtual Disk not exist, initial disk now!");
		initDisk();
	}
	/*if the 'disk' exist, then we can start the simulation, mount the disk and display some infomation*/
	simulation_start();

	/*login to operate*/
 	String username;
	String password;
	cin >> username;
	cin >> password;



















    return 0;
}
