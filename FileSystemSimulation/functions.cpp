// OS_fs.cpp : Defines the entry point for the console application.
//	main()

/**
*	Copyright 2018, Alec Chen
*	All Rights Reserved.
*	20154479 1506
*	acytoo@gmail.com
*	2018年7月3日15点00分
*/


/*
* 	int fseek ( FILE * stream, long int offset, int origin );
*	SEEK_SET	Beginning of file
*	SEEK_CUR	Current position of the file pointer
*	SEEK_END	End of file *

*	size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );

*/



#define _CRT_SECURE_NO_WARNINGS
//#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <conio.h>

# include "md5.h"
//#define LIBDLL extern "C" __declspec(dllexport)

using namespace std;


namespace acytoo {
    string md5(string strPlain) {
        MD5_CTX mdContext;
        int bytes;
        unsigned char data[1024];

        MD5Init(&mdContext);
        MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
        MD5Final(&mdContext);

        string md5;
        char buf[3];
        for (int i = 0; i < 16; i++)
        {
            sprintf(buf, "%02x", mdContext.digest[i]);
            md5.append(buf);
        }
        return md5;
    }
}





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
const unsigned int USER_NUM = 8;			/*eight users user1..., I prefer acytoo, root...*/
const unsigned int DIRECTORY_NUM = 16;		/*the number of the dir*/
const unsigned short FILE_NAME_LENGTH = 14;	/*file name length, DIR_SIZE, defined in the ppt, plus two inode*/
const unsigned short USER_NAME_LENGTH = 14;	/*length of user name, since I DO NOT WANT to use that stupid name: user2 */
const unsigned short USER_PASSWORD_LENGTH = 32;/*the length of *MODIFIED* md5, since the password is stored as (original-1)md5 in my file system*/
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

struct userInfo {
    unsigned short userID[USER_NUM];
    char userName[USER_NUM][USER_NAME_LENGTH];
    char password[USER_NUM][USER_PASSWORD_LENGTH];
    unsigned short groupID[USER_NUM];
};


FILE* virtualDisk = NULL;
filsys superBlock;
unsigned short inode_bitmap[INODE_NUM];	/*1: used; 0: unused*/
userInfo users;							/*user infomation*/
unsigned short userID = USER_NUM;	/*curent logged in user*/

char userNameInChar[USER_NAME_LENGTH + 6];	/*6: the length of your 'pc' name*/
string userName;
directory currentDirectory;
char currentPath[100][14];					/*current folder stack, initial all this space to 0 at the beginning*/
unsigned short dir_pointer;

void findFreeBlock(unsigned int &inode_number) {
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);		/*SEEK_SET move the pointer to the beginning of a file*/
    fread(&superBlock, sizeof(filsys), 1, virtualDisk);/*read 1 from file, length is sizeof, to superBlock*/
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
        fseek(virtualDisk, DATA_START + (superBlock.special_stack[0] - 50) * BLOCK_SIZE, SEEK_SET);
        fwrite(stack, sizeof(stack), 1, virtualDisk);

        fseek(virtualDisk, DATA_START + superBlock.special_stack[0] * BLOCK_SIZE, SEEK_SET);
        fread(stack, sizeof(stack), 1, virtualDisk);
        for (int i = 0; i < 50; i++) {
            superBlock.special_stack[i] = stack[i];
        }
        superBlock.special_free = stack[50];
    }
    inode_number = superBlock.special_stack[superBlock.special_free];
    superBlock.special_free--;
    superBlock.s_num_fblock--;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);
}

void recycleBlock(unsigned int &inode_number) {
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fread(&superBlock, sizeof(filsys), 1, virtualDisk);/*read 1 from file, length is sizeof, to superBlock*/
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
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
        fwrite(stack, sizeof(stack), 1, virtualDisk);
        block_num -= 50;
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
        fread(stack, sizeof(stack), 1, virtualDisk);
        for (int i = 0; i < 50; i++) {
            superBlock.special_stack[i] = stack[i];
        }
        superBlock.special_free = stack[50];
    }
    superBlock.special_free++;
    superBlock.s_num_fblock++;
    superBlock.special_stack[superBlock.special_free] = inode_number;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);
}
/**
*	format the disk, erase all the data stored in our disk
*	if the program run on a brand new machine, call this function
*/
bool initDisk() {
    /*
    *	1. Create a empty file to emulate the file system.
    */
    FILE* virtualDisk = fopen("./VirtualDisk.yt", "wb+");
    if (virtualDisk == NULL) {
        printf("Fail to initialize the virtualDisk system!\n");
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
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fread(&superBlock, sizeof(filsys), 1, virtualDisk);

    /*
    *	3. Initialize inode and block bitmaps.
    */
    unsigned short inode_bitmap[INODE_NUM];
    //Root directory and accounting file will use some inodes and blocks.
    memset(inode_bitmap, 0, INODE_NUM);
    inode_bitmap[0] = 1;
    inode_bitmap[1] = 1;
    //Write bitmaps into file.
    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);

    //成组链接
    unsigned int stack[51];
    for (int i = 0; i < BLOCK_NUM / 50; i++) {
        memset(stack, 0, sizeof(stack));
        for (unsigned int j = 0; j < 50; j++) {
            stack[j] = (49 + i * 50) - j;
        }
        stack[0] = 49 + (i + 1) * 50;
        stack[50] = 49;
        fseek(virtualDisk, DATA_START + (49 + i * 50)*BLOCK_SIZE, SEEK_SET);
        fwrite(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
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
    fseek(virtualDisk, DATA_START + 511 * BLOCK_SIZE, SEEK_SET);
    fwrite(stack, sizeof(unsigned int) * 51, 1, virtualDisk);

    fseek(virtualDisk, DATA_START + 49 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 99 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 149 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 199 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 249 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 299 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 349 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 399 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 449 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 499 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);
    fseek(virtualDisk, DATA_START + 511 * BLOCK_SIZE, SEEK_SET);
    fread(stack, sizeof(unsigned int) * 51, 1, virtualDisk);



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
    fseek(virtualDisk, INODE_START, SEEK_SET);
    fwrite(&iroot_tmp, sizeof(inode), 1, virtualDisk);

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
    fseek(virtualDisk, DATA_START, SEEK_SET);
    fwrite(&droot_tmp, sizeof(directory), 1, virtualDisk);

    /*
    *	5. Create user information.
    */
    //Create inode
    inode iaccouting_tmp;
    iaccouting_tmp.i_ino = 1;					//Identification
    iaccouting_tmp.di_number = 1;				//Associations
    iaccouting_tmp.di_mode = 1;					//1 stands for file
    iaccouting_tmp.di_size = sizeof(userInfo);	//File size
    memset(iaccouting_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
    iaccouting_tmp.di_addr[0] = 1;				//Root directory is stored on 1st block.
    iaccouting_tmp.di_uid = 0;					//Root user id.
    iaccouting_tmp.di_grp = 1;
    iaccouting_tmp.permission = 320;
    iaccouting_tmp.icount = 0;
    t = time(0);
    strftime(iaccouting_tmp.time, sizeof(iaccouting_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
    iaccouting_tmp.time[64] = 0;
    fseek(virtualDisk, INODE_START + INODE_SIZE, SEEK_SET);
    fwrite(&iaccouting_tmp, sizeof(inode), 1, virtualDisk);

    //Create accouting file.
    userInfo paccouting_tmp;
    memset(paccouting_tmp.userName, 0, sizeof(char) * USER_NAME_LENGTH * USER_NUM);
    memset(paccouting_tmp.password, 0, sizeof(char) * USER_PASSWORD_LENGTH * USER_NUM);
    //Only default user 'admin' is registered. Default password is 'admin'.
    strcpy(paccouting_tmp.userName[0], "root");
    strcpy(paccouting_tmp.userName[1], "acytoo");
    strcpy(paccouting_tmp.password[0], "7b24afc8bc80e548d66c4e7ff72171c\0"); //toor 5
    strcpy(paccouting_tmp.password[1], "90340c1085e1e22d8b0d80193d12e50\0"); //KingJoffrey 2
    //0 stands for root user. Other IDs are only used to identify users.
    for (unsigned short i = 0; i < USER_NUM; i++) {
        paccouting_tmp.userID[i] = i;
    }
    paccouting_tmp.groupID[0] = 1;
    paccouting_tmp.groupID[1] = 2;
    //Write
    fseek(virtualDisk, DATA_START + BLOCK_SIZE, SEEK_SET);
    fwrite(&paccouting_tmp, sizeof(userInfo), 1, virtualDisk);

    //Close file.
    fclose(virtualDisk);

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
        printf("mount disk failed in mount function\n");
        return false;
    }
    /*first read the superBlock, bitmaps, userinfo, current directory*/
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fread(&superBlock, sizeof(superBlock), 1, virtualDisk);

    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fread(inode_bitmap, sizeof(unsigned int) * INODE_NUM, 1, virtualDisk);

    fseek(virtualDisk, DATA_START, SEEK_SET);
    fread(&currentDirectory, sizeof(directory), 1, virtualDisk);

    fseek(virtualDisk, DATA_START + BLOCK_SIZE, SEEK_SET);
    fread(&users, sizeof(userInfo), 1, virtualDisk);

    return true;
}

/**
*	every time we boot up or log out, we need to login to operate out file system
*   the maxmun of logged-in-user is ten, so every time a user logged in, increase
*/
//bool login(const char* user, const char* passwd) { return false; }
bool login(string userName, string password) {
    char tmp_userName[USER_NAME_LENGTH];
    char tmp_userPassword[USER_PASSWORD_LENGTH * 5];
    memset(tmp_userName, 0, USER_NAME_LENGTH);
    memset(tmp_userPassword, 0, USER_PASSWORD_LENGTH * 5);
    cout << "{" << userName << "}" << "{" << password << "}" << endl;


    if (userName.empty() || password.empty()) {
        cout << "user name and password can't be empty!" << endl;
        return false;
    }
    //  我就不判断口令是否是32位以下了，有些无聊
    if (userID != USER_NUM) {
        cout << "Login failed: User has been logged in. Please log out first.\n";
        return false;
    }

    string md5Passwd = acytoo::md5(password);
    md5Passwd = md5Passwd.substr(0, 31);
    cout << "{" << userName << "}" << "{" << md5Passwd << "}" << endl;
    for (int i = 0; i < USER_NUM; i++) {
        cout << " in that for loop: " << userName << " users.userName[1] {" << users.userName[i] << "}" << endl; ;
        if (users.userName[i] == userName) {
            //find the user and check password
            //cout << "user name match "  << "password is " << md5Passwd<< endl;
            //printf("{%s} {%s}\n", users.password[i], md5Passwd);
            //cout << md5Passwd << endl;
            if (users.password[i] == md5Passwd) {
                //Login successfully
                //printf("Login successfully.\n");
                userID = users.userID[i];
                //make user's name, root user is special
                memset(userNameInChar, 0, USER_NAME_LENGTH + 6);
                if (userID == 0) {
                    //can i use string add here? ?? string +
                    strcat(userNameInChar, users.userName[i]);
                    strcat(userNameInChar, "@Acytxx: ");
                    strcat(userNameInChar, "#");    //root user: #
                }
                else {
                    strcat(userNameInChar, users.userName[i]);
                    strcat(userNameInChar, "@Acytxx: ");
                    strcat(userNameInChar, "$");    //none root user: $
                }

                return true;
            }
            else {
                //Password wrong
                cout << "Login failed: Wrong password.\n";
                return false;
            }
        }
    }
    cout << "Login failed: User not found\n";
    return false;
}

bool login() {
    string password;
    string userName;
    cout << "\tUser : ";
    cin >> userName;
    cout << "\tPassword : ";
    cin >> password;
    printf("\n");
    //string md5Passwd = acytoo::md5(password);
    //cout << "username: " << userName << " password : " << password << endl;
    return login(userName, password);
}

/*
*	log out to switch user
*/
void logout() {
    //remove user's states
    userID = USER_NUM;
    memset(&users, 0, sizeof(users));
    memset(userNameInChar, 0, 6 + USER_NAME_LENGTH);
    //re-mount the file system
    mount();
}

/**
*       touch a file, to create an empty file
*/
bool touch(const char* fileName) {
    //parameter check
    if (fileName == NULL || strlen(fileName) > FILE_NAME_LENGTH) {
        printf("Error: Illegal file name.\n");
        return false;
    }

    /*
    *	1. Check whether free inodes and blocks are used up.
    */
    if (superBlock.s_num_fblock <= 0 || superBlock.s_num_finode <= 0) {
        printf("File creation error: No valid spaces.\n");
        return false;
    }
    //Find new inode number and new block address
    int new_ino = 0;
    unsigned int new_block_addr = -1;
    for (; new_ino < INODE_NUM; new_ino++) {
        if (inode_bitmap[new_ino] == 0) {
            break;
        }
    }

    /*
    *	2. Check whether file name has been used in current directory.
    */
    for (int i = 0; i < DIRECTORY_NUM; i++) {
        if (strcmp(currentDirectory.fileName[i], fileName) == 0) {
            inode* tmp_file_inode = new inode;
            int tmp_file_ino = currentDirectory.inodeID[i];
            fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
            fread(tmp_file_inode, sizeof(inode), 1, virtualDisk);
            if (tmp_file_inode->di_mode == 0) continue;
            else {
                printf("File creation error: File name '%s' has been used.\n", currentDirectory.fileName[i]);
                return false;
            }
        }
    }

    /*
    *	3. Check whether current directory contains too many items already.
    */
    int itemCounter = 0;
    for (int i = 0; i < DIRECTORY_NUM; i++) {
        if (strlen(currentDirectory.fileName[i]) > 0) {
            itemCounter++;
        }
    }
    if (itemCounter >= DIRECTORY_NUM) {
        printf("File creation error: Too many files or directories in current path.\n");
        return false;
    }

    /*
    *	4. Create new inode.
    */
    //Create inode
    inode ifile_tmp;
    ifile_tmp.i_ino = new_ino;				//Identification
    ifile_tmp.di_number = 1;				//Associations
    ifile_tmp.di_mode = 1;					//1 stands for virtualDisk
    ifile_tmp.di_size = 0;					//New virtualDisk is empty
    memset(ifile_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
    ifile_tmp.di_uid = userID;				//Current user id.
    ifile_tmp.di_grp = users.groupID[userID];//Current user group id
    ifile_tmp.permission = MAX_PERMISSION;
    ifile_tmp.icount = 0;
    time_t t = time(0);
    strftime(ifile_tmp.time, sizeof(ifile_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
    ifile_tmp.time[64];
    fseek(virtualDisk, INODE_START + new_ino * INODE_SIZE, SEEK_SET);
    fwrite(&ifile_tmp, sizeof(inode), 1, virtualDisk);

    /*
    *	5.  Update bitmaps.
    */
    //Update bitmaps
    inode_bitmap[new_ino] = 1;
    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);

    /*
    *	6. Update directory.
    */
    //Fetch current directory's inode
    //Inode position of current directory
    int pos_directory_inode = 0;
    pos_directory_inode = currentDirectory.inodeID[0]; //"."
    inode tmp_directory_inode;
    fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
    fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

    //Add to current directory item
    for (int i = 2; i < DIRECTORY_NUM; i++) {
        if (strlen(currentDirectory.fileName[i]) == 0) {
            strcat(currentDirectory.fileName[i], fileName);
            currentDirectory.inodeID[i] = new_ino;
            break;
        }
    }
    //write
    fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

    //Update associations
    directory tmp_directory = currentDirectory;
    int tmp_pos_directory_inode = pos_directory_inode;
    while (true) {
        //Update association
        tmp_directory_inode.di_number++;
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        //If reach the root directory, finish updating.
        if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
            break;
        }
        //Fetch father directory
        tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
    }

    /*
    *	7. Update super block.
    */
    //superBlock.s_num_fblock--;
    superBlock.s_num_finode--;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);

    return true;
}

/**
*	delete a file or folder
*/
bool rm(const char* fileName) {
    //parameter check
    if (fileName == NULL || strlen(fileName) > FILE_NAME_LENGTH) {
        printf("Error: Illegal file name.\n");
        return false;
    }

    /*
    *	1. Check whether the file exists in current directory.
    */
    int pos_in_directory = -1, tmp_file_ino;
    inode tmp_file_inode;
    do {
        pos_in_directory++;
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++) {
            if (strcmp(currentDirectory.fileName[pos_in_directory], fileName) == 0) {
                break;
            }
        }
        if (pos_in_directory == DIRECTORY_NUM) {
            printf("Delete error: File not found.\n");
            return false;
        }

        /*
        *	2. Fetch inode and check whether it's a directory.
        */
        //Fetch inode
        tmp_file_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
        fread(&tmp_file_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_file_inode.di_mode == 0);	//is a directory, roll back and continue to search the virtualDisk

                                            //Access check

    if (userID == tmp_file_inode.di_uid) {
        if (!(tmp_file_inode.permission & OWN_E)) {
            printf("Delete error: Access deny.\n");
            return -1;
        }
    }
    else if (users.groupID[userID] == tmp_file_inode.di_grp) {
        if (!(tmp_file_inode.permission & GRP_E)) {
            printf("Delete error: Access deny.\n");
            return -1;
        }
    }
    else {
        if (!(tmp_file_inode.permission & ELSE_E)) {
            printf("Delete error: Access deny.\n");
            return -1;
        }
    }
    /*
    *	3. Start deleting. Fill the inode's original space with 0.
    */
    if (tmp_file_inode.icount > 0) {
        tmp_file_inode.icount--;
        fseek(virtualDisk, INODE_START + tmp_file_inode.i_ino * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_file_inode, sizeof(inode), 1, virtualDisk);
        /*
        *	Update directories
        */
        //Fetch current directory inode
        int pos_directory_inode = currentDirectory.inodeID[0];	//"."
        inode tmp_directory_inode;
        fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

        //Update current directory item
        memset(currentDirectory.fileName[pos_in_directory], 0, FILE_NAME_LENGTH);
        currentDirectory.inodeID[pos_in_directory] = -1;
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

        //Update associations
        directory tmp_directory = currentDirectory;
        int tmp_pos_directory_inode = pos_directory_inode;
        while (true) {
            //Update association
            tmp_directory_inode.di_number--;
            fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
            fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
            //If reach the root directory, finish updating.
            if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
                break;
            }
            //Fetch father directory
            tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
            fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
            fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
            fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
            fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
        }
        return true;
    }
    //Fill original space
    int tmp_fill[sizeof(inode)];
    memset(tmp_fill, 0, sizeof(inode));
    fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
    fwrite(&tmp_fill, sizeof(inode), 1, virtualDisk);

    /*
    *	4. Update bitmaps
    */
    //inode bitmap
    inode_bitmap[tmp_file_ino] = 0;
    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fwrite(&inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);
    //block bitmap

    for (int i = 0; i < NADDR - 2; i++) {
        if (tmp_file_inode.di_addr[i] != -1)
            recycleBlock(tmp_file_inode.di_addr[i]);
        else break;
    }
    if (tmp_file_inode.di_addr[NADDR - 2] != -1) {
        unsigned int f1[128];
        fseek(virtualDisk, DATA_START + tmp_file_inode.di_addr[NADDR - 2] * BLOCK_SIZE, SEEK_SET);
        fread(f1, sizeof(f1), 1, virtualDisk);
        for (int k = 0; k < 128; k++) {
            recycleBlock(f1[k]);
        }
        recycleBlock(tmp_file_inode.di_addr[NADDR - 2]);
    }

    /*
    *	5. Update directories
    */
    //Fetch current directory inode
    int pos_directory_inode = currentDirectory.inodeID[0];	//"."
    inode tmp_directory_inode;
    fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
    fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

    //Update current directory item
    memset(currentDirectory.fileName[pos_in_directory], 0, FILE_NAME_LENGTH);
    currentDirectory.inodeID[pos_in_directory] = -1;
    fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

    //Update associations
    directory tmp_directory = currentDirectory;
    int tmp_pos_directory_inode = pos_directory_inode;
    while (true) {
        //Update association
        tmp_directory_inode.di_number--;
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        //If reach the root directory, finish updating.
        if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
            break;
        }
        //Fetch father directory
        tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
    }

    /*
    *	6. Update super block
    */
    //superBlock.s_num_fblock += tmp_file_inode.di_size / BLOCK_SIZE + 1;
    superBlock.s_num_finode++;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);

    return true;
}


/*
*	move a file or folder, or rename it
*/
void mv(char* fileName) {
    printf("File or Dir?(0 stands for file, 1 for dir):");
    int tt;
    scanf("%d", &tt);
    //parameter check
    if (fileName == NULL || strlen(fileName) > FILE_NAME_LENGTH)
    {
        printf("Error: Illegal file name.\n");
        return;
    }

    /*
    *	1. Check whether the virtualDisk exists in current directory.
    */
    int pos_in_directory = -1;
    inode* tmp_file_inode = new inode;
    do {
        pos_in_directory++;
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++)
        {
            if (strcmp(currentDirectory.fileName[pos_in_directory], fileName) == 0)
            {
                break;
            }
        }
        if (pos_in_directory == DIRECTORY_NUM)
        {
            printf("Not found.\n");
            return;
        }

        /*
        *	2. Fetch inode and check whether it's a directory.
        */
        //Fetch inode
        int tmp_file_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
        fread(tmp_file_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_file_inode->di_mode == tt);


    printf("Please input new file name:");
    char str[14];
    scanf("%s", str);
    str[14] = 0;
    for (int i = 0; i < DIRECTORY_NUM; i++)
    {
        if (currentDirectory.inodeID[i] == tmp_file_inode->i_ino)
        {
            strcpy(currentDirectory.fileName[i], str);
            break;
        }
    }
    //write
    fseek(virtualDisk, DATA_START + tmp_file_inode->di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);
    return;
}

/*
*	switch work space
*/
bool cd(string strDirName) {
    const char* dirName = strDirName.c_str();
    //parameter check
    if (dirName == NULL || strlen(dirName) > FILE_NAME_LENGTH) {
        printf("Error: Illegal directory name.\n");
        return false;
    }
    /*
    *	1. Check whether the directory exists in current directory.
    */
    int pos_in_directory = 0;
    inode tmp_dir_inode;
    int tmp_dir_ino;
    do {
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++) {
            if (strcmp(currentDirectory.fileName[pos_in_directory], dirName) == 0) {
                break;
            }
        } {
            printf("Delete error: Directory not found.\n");
            return false;
        }

        /*
        *	2. Fetch inode and check whether it's a file.
        */
        //Fetch inode
        tmp_dir_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_dir_ino * INODE_SIZE, SEEK_SET);
        fread(&tmp_dir_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_dir_inode.di_mode == 1);
    //ACCESS CHECK
    if (userID == tmp_dir_inode.di_uid) {
        if (tmp_dir_inode.permission & OWN_E != OWN_E) {
            printf("Open dir error: Access deny.\n");
            return NULL;
        }
    }
    else if (users.groupID[userID] == tmp_dir_inode.di_grp) {
        if (tmp_dir_inode.permission & GRP_E != GRP_E) {
            printf("Open dir error: Access deny.\n");
            return NULL;
        }
    }
    else {
        if (tmp_dir_inode.permission & ELSE_E != ELSE_E) {
            printf("Open dir error: Access deny.\n");
            return NULL;
        }
    }
    /*
    *	3. Update current directory.
    */
    directory new_current_dir;
    fseek(virtualDisk, DATA_START + tmp_dir_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fread(&new_current_dir, sizeof(directory), 1, virtualDisk);
    currentDirectory = new_current_dir;
    if (dirName[0] == '.' && dirName[1] == 0) {
        dir_pointer;
    }
    else if (dirName[0] == '.' && dirName[1] == '.' && dirName[2] == 0) {
        if (dir_pointer != 0) dir_pointer--;
    }
    else {
        for (int i = 0; i < 14; i++) {
            currentPath[dir_pointer][i] = dirName[i];
        }
        dir_pointer++;
    }
    return true;
}

/*
*	show current work space
*/
void pwd() {

    for (int i = 0; i < dir_pointer; i++)
        printf("%s/", currentPath[i]), cout << currentPath[i] << endl;
    printf("\n");
}

/**
*	change permission
*/
bool chmod() { return false; }

/*
*	list all the file info
*/
void ls() {
    printf("\n     name\tuser\tgroup\tinodeID\tIcount\tsize\tpermission\ttime\n");
    for (int i = 0; i < DIRECTORY_NUM; i++) {
        if (strlen(currentDirectory.fileName[i]) > 0) {
            inode tmp_inode;
            fseek(virtualDisk, INODE_START + currentDirectory.inodeID[i] * INODE_SIZE, SEEK_SET);
            fread(&tmp_inode, sizeof(inode), 1, virtualDisk);

            const char* tmp_type = tmp_inode.di_mode == 0 ? "d" : "-";
            const char* tmp_user = users.userName[tmp_inode.di_uid];
            const int tmp_grpID = tmp_inode.di_grp;

            printf("%10s\t%s\t%d\t%d\t%d\t%u\t%s", currentDirectory.fileName[i], tmp_user, tmp_grpID, tmp_inode.i_ino, tmp_inode.icount, tmp_inode.di_size, tmp_type);
            for (int x = 8; x > 0; x--) {
                if (tmp_inode.permission & (1 << x)) {
                    if ((x + 1) % 3 == 0) printf("r");
                    else if ((x + 1) % 3 == 2) printf("w");
                    else printf("x");
                }
                else printf("-");
            }
            if (tmp_inode.permission & 1) printf("x\t");
            else printf("-\t");
            printf("%s\n", tmp_inode.time);
        }
    }

    printf("\n\n");

}

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
bool vim(inode& ifile, const char* content) {
    //parameter check
    if (content == NULL){
        printf("Error: Illegal virtualDisk name.\n");
        return -1;
    }
    //Access check
    if (userID == ifile.di_uid)
    {
        if (!(ifile.permission & OWN_W)) {
            printf("Write error: Access deny.\n");
            return -1;
        }
    }
    else if (users.groupID[userID] == ifile.di_grp) {
        if (!(ifile.permission & GRP_W)) {
            printf("Write error: Access deny.\n");
            return -1;
        }
    }
    else {
        if (!(ifile.permission & ELSE_W)) {
            printf("Write error: Access deny.\n");
            return -1;
        }
    }

    /*
    *	1. Check whether the expected virtualDisk will be out of length.
    */
    int len_content = strlen(content);
    unsigned int new_file_length = len_content + ifile.di_size;
    if (new_file_length >= FILE_SIZE_MAX)
    {
        printf("Write error: File over length.\n");
        return -1;
    }

    /*
    *	2. Get the number of needed blocks and check is there any enough free spaces.
    */
    //Get the number of needed blocks
    unsigned int block_num;
    if (ifile.di_addr[0] == -1)block_num = -1;
    else
    {
        for (int i = 0; i < NADDR - 2; i++)
        {
            if (ifile.di_addr[i] != -1)
                block_num = ifile.di_addr[i];
            else break;
        }
        int f1[128];
        fseek(virtualDisk, DATA_START + ifile.di_addr[NADDR - 2] * BLOCK_SIZE, SEEK_SET);
        int num;
        if (ifile.di_size%BLOCK_SIZE == 0)
            num = ifile.di_size / BLOCK_SIZE;
        else num = ifile.di_size / BLOCK_SIZE + 1;
        if (num > 4 && num <= 132)
        {
            fseek(virtualDisk, DATA_START + ifile.di_addr[NADDR - 2] * BLOCK_SIZE, SEEK_SET);
            fread(f1, sizeof(f1), 1, virtualDisk);
            block_num = f1[num - 4];
        }

    }
    int free_space_firstBlock = BLOCK_SIZE - ifile.di_size % BLOCK_SIZE;
    unsigned int num_block_needed;
    if (len_content - free_space_firstBlock > 0)
    {
        num_block_needed = (len_content - free_space_firstBlock) / BLOCK_SIZE + 1;
    }
    else
    {
        num_block_needed = 0;
    }
    //Check is there any enough free spaces
    if (num_block_needed > superBlock.s_num_fblock)
    {
        printf("Write error: No enough space available.\n");
        return -1;
    }

    /*
    *	3. Write first block.
    */
    if (ifile.di_addr[0] == -1)
    {
        findFreeBlock(block_num);
        ifile.di_addr[0] = block_num;
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
    }
    else
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE + ifile.di_size % BLOCK_SIZE, SEEK_SET);
    char data[BLOCK_SIZE];
    if (num_block_needed == 0)
    {
        fwrite(content, len_content, 1, virtualDisk);
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
        fread(data, sizeof(data), 1, virtualDisk);
        ifile.di_size += len_content;
    }
    else
    {
        fwrite(content, free_space_firstBlock, 1, virtualDisk);
        fseek(virtualDisk, DATA_START + block_num * BLOCK_SIZE, SEEK_SET);
        fread(data, sizeof(data), 1, virtualDisk);
        ifile.di_size += free_space_firstBlock;
    }

    /*
    *	4. Write the other blocks. Update virtualDisk information in inode and block bitmap in the meanwhile.
    */
    char write_buf[BLOCK_SIZE];
    unsigned int new_block_addr = -1;
    unsigned int content_write_pos = free_space_firstBlock;
    //Loop and write each blocks
    if ((len_content + ifile.di_size) / BLOCK_SIZE + ((len_content + ifile.di_size) % BLOCK_SIZE == 0 ? 0 : 1) <= NADDR - 2) {
        //direct addressing
        for (int i = 0; i < num_block_needed; i++)
        {
            findFreeBlock(new_block_addr);
            if (new_block_addr == -1)return -1;
            for (int j = 0; j < NADDR - 2; j++)
            {
                if (ifile.di_addr[j] == -1)
                {
                    ifile.di_addr[j] = new_block_addr;
                    break;
                }
            }
            memset(write_buf, 0, BLOCK_SIZE);
            //Copy from content to write buffer
            unsigned int tmp_counter = 0;
            for (; tmp_counter < BLOCK_SIZE; tmp_counter++)
            {
                if (content[content_write_pos + tmp_counter] == '\0')
                    break;
                write_buf[tmp_counter] = content[content_write_pos + tmp_counter];
            }
            content_write_pos += tmp_counter;
            //Write
            fseek(virtualDisk, DATA_START + new_block_addr * BLOCK_SIZE, SEEK_SET);
            fwrite(write_buf, tmp_counter, 1, virtualDisk);
            fseek(virtualDisk, DATA_START + new_block_addr * BLOCK_SIZE, SEEK_SET);
            fread(data, sizeof(data), 1, virtualDisk);
            //Update inode information: blocks address and virtualDisk size
            ifile.di_size += tmp_counter;
        }
    }
    else if ((len_content + ifile.di_size) / BLOCK_SIZE + ((len_content + ifile.di_size) % BLOCK_SIZE == 0 ? 0 : 1)> NADDR - 2) {
        //direct addressing
        for (int i = 0; i < NADDR - 2; i++)
        {
            if (ifile.di_addr[i] != -1)continue;

            memset(write_buf, 0, BLOCK_SIZE);
            new_block_addr = -1;

            findFreeBlock(new_block_addr);
            if (new_block_addr == -1)return -1;
            ifile.di_addr[i] = new_block_addr;
            //Copy from content to write buffer
            unsigned int tmp_counter = 0;
            for (; tmp_counter < BLOCK_SIZE; tmp_counter++)
            {
                if (content[content_write_pos + tmp_counter] == '\0') {
                    break;
                }
                write_buf[tmp_counter] = content[content_write_pos + tmp_counter];
            }
            content_write_pos += tmp_counter;
            //Write
            fseek(virtualDisk, DATA_START + new_block_addr * BLOCK_SIZE, SEEK_SET);
            fwrite(write_buf, tmp_counter, 1, virtualDisk);

            //Update inode information: blocks address and virtualDisk size
            ifile.di_size += tmp_counter;
        }
        //first indirect addressing
        int cnt = 0;
        unsigned int f1[BLOCK_SIZE / sizeof(unsigned int)] = { 0 };

        new_block_addr = -1;
        findFreeBlock(new_block_addr);
        if (new_block_addr == -1)return -1;
        ifile.di_addr[NADDR - 2] = new_block_addr;
        for (int i = 0; i < BLOCK_SIZE / sizeof(unsigned int); i++)
        {
            new_block_addr = -1;
            findFreeBlock(new_block_addr);
            if (new_block_addr == -1)return -1;
            else
                f1[i] = new_block_addr;
        }
        fseek(virtualDisk, DATA_START + ifile.di_addr[4] * BLOCK_SIZE, SEEK_SET);
        fwrite(f1, sizeof(f1), 1, virtualDisk);
        bool flag = 0;
        for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++) {
            fseek(virtualDisk, DATA_START + f1[j] * BLOCK_SIZE, SEEK_SET);
            //Copy from content to write buffer
            unsigned int tmp_counter = 0;
            for (; tmp_counter < BLOCK_SIZE; tmp_counter++)
            {
                if (content[content_write_pos + tmp_counter] == '\0') {
                    //tmp_counter--;
                    flag = 1;
                    break;
                }
                write_buf[tmp_counter] = content[content_write_pos + tmp_counter];
            }
            content_write_pos += tmp_counter;
            fwrite(write_buf, tmp_counter, 1, virtualDisk);
            ifile.di_size += tmp_counter;
            if (flag == 1) break;
        }
    }
    time_t t = time(0);
    strftime(ifile.time, sizeof(ifile.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
    ifile.time[64] = 0;
    //Write inode
    fseek(virtualDisk, INODE_START + ifile.i_ino * INODE_SIZE, SEEK_SET);
    fwrite(&ifile, sizeof(inode), 1, virtualDisk);

    /*
    *	5. Update super block.
    */
    //superBlock.s_num_fblock -= num_block_needed;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(superBlock), 1, virtualDisk);

    return len_content;

}

/**
*   view the file in the same terminal
*/
void cat(inode& ifile) {
    //Access check
    if (userID == ifile.di_uid) {
        if (!(ifile.permission & OWN_R)) {
            printf("Read error: Access deny.\n");
            return;
        }
    }
    else if (users.groupID[userID] == ifile.di_grp) {
        if (!(ifile.permission & GRP_R)) {
            printf("Read error: Access deny.\n");
            return;
        }
    }
    else {
        if (!(ifile.permission & ELSE_R)) {
            printf("Read error: Access deny.\n");
            return;
        }
    }
    int block_num = ifile.di_size / BLOCK_SIZE + 1;
    int print_line_num = 0;		//16 bytes per line.
                                //Read virtualDisk from data blocks
    char stack[BLOCK_SIZE];
    if (block_num <= NADDR - 2) {
        for (int i = 0; i < block_num; i++) {
            fseek(virtualDisk, DATA_START + ifile.di_addr[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (stack[j] == '\0')break;
                if (j % 16 == 0) {
                    printf("\n");
                    printf("%d\t", ++print_line_num);
                }
                printf("%c", stack[j]);
            }
        }
        //int i = 0;
    }
    else if (block_num > NADDR - 2) {
        //direct addressing
        for (int i = 0; i < NADDR - 2; i++) {
            fseek(virtualDisk, DATA_START + ifile.di_addr[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (stack[j] == '\0')break;
                if (j % 16 == 0) {
                    printf("\n");
                    printf("%d\t", ++print_line_num);
                }
                printf("%c", stack[j]);
            }
        }

        //first indirect addressing
        unsigned int f1[BLOCK_SIZE / sizeof(unsigned int)] = { 0 };
        fseek(virtualDisk, DATA_START + ifile.di_addr[NADDR - 2] * BLOCK_SIZE, SEEK_SET);
        fread(f1, sizeof(f1), 1, virtualDisk);
        for (int i = 0; i < block_num - (NADDR - 2); i++) {
            fseek(virtualDisk, DATA_START + f1[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (stack[j] == '\0')break;
                if (j % 16 == 0) {
                    printf("\n");
                    printf("%d\t", ++print_line_num);
                }
                printf("%c", stack[j]);
            }
        }
    }
    printf("\n\n\n");
}

/*
*	list all the file(folders) with detail infomation
*/
bool lla() { return false; }

/*
*	exit su, exit program
*/

bool exit() { return false; }

/**
*   make directory
*/
bool mymkdir(const char* dirName) {
    //parameter check
    if (dirName == NULL || strlen(dirName) > FILE_NAME_LENGTH) {
        printf("Error: Illegal directory name.\n");
        return false;
    }

    /*
    *	1. Check whether free inodes and blocks are used up.
    */
    if (superBlock.s_num_fblock <= 0 || superBlock.s_num_finode <= 0) {
        printf("File creation error: No valid spaces.\n");
        return false;
    }
    //Find new inode number and new block address
    int new_ino = 0;
    unsigned int new_block_addr = 0;
    for (; new_ino < INODE_NUM; new_ino++) {
        if (inode_bitmap[new_ino] == 0) {
            break;
        }
    }
    findFreeBlock(new_block_addr);
    if (new_block_addr == -1) return false;
    if (new_ino == INODE_NUM || new_block_addr == BLOCK_NUM) {
        printf("File creation error: No valid spaces.\n");
        return false;
    }

    /*
    *	2. Check whether directory name has been used in current directory.
    */
    for (int i = 0; i < DIRECTORY_NUM; i++) {
        if (strcmp(currentDirectory.fileName[i], dirName) == 0) {
            inode* tmp_file_inode = new inode;
            int tmp_file_ino = currentDirectory.inodeID[i];
            fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
            fread(tmp_file_inode, sizeof(inode), 1, virtualDisk);
            if (tmp_file_inode->di_mode == 1) continue;
            else {
                printf("File creation error: Directory name '%s' has been used.\n", currentDirectory.fileName[i]);
                return false;
            }
        }
    }

    /*
    *	3. Check whether current directory contains too many items already.
    */
    int itemCounter = 0;
    for (int i = 0; i < DIRECTORY_NUM; i++) {
        if (strlen(currentDirectory.fileName[i]) > 0) {
            itemCounter++;
        }
    }
    if (itemCounter >= DIRECTORY_NUM) {
        printf("File creation error: Too many files or directories in current path.\n");
        return false;
    }

    /*
    *	4. Create new inode.
    */
    //Create inode
    inode idir_tmp;
    idir_tmp.i_ino = new_ino;				//Identification
    idir_tmp.di_number = 1;					//Associations
    idir_tmp.di_mode = 0;					//0 stands for directory
    idir_tmp.di_size = sizeof(directory);	//"For directories, the value is 0."
    memset(idir_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
    idir_tmp.di_addr[0] = new_block_addr;
    idir_tmp.di_uid = userID;				//Current user id.
    idir_tmp.di_grp = users.groupID[userID];
    time_t t = time(0);
    strftime(idir_tmp.time, sizeof(idir_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
    idir_tmp.time[64] = 0;
    idir_tmp.icount = 0;
    idir_tmp.permission = MAX_PERMISSION;
    fseek(virtualDisk, INODE_START + new_ino * INODE_SIZE, SEEK_SET);
    fwrite(&idir_tmp, sizeof(inode), 1, virtualDisk);

    /*
    *	5. Create directory virtualDisk.
    */
    directory tmp_dir;
    memset(tmp_dir.fileName, 0, sizeof(char) * DIRECTORY_NUM * FILE_NAME_LENGTH);
    memset(tmp_dir.inodeID, -1, sizeof(unsigned int) * DIRECTORY_NUM);
    strcpy(tmp_dir.fileName[0], ".");
    tmp_dir.inodeID[0] = new_ino;
    strcpy(tmp_dir.fileName[1], "..");
    tmp_dir.inodeID[1] = currentDirectory.inodeID[0];
    fseek(virtualDisk, DATA_START + new_block_addr * BLOCK_SIZE, SEEK_SET);
    fwrite(&tmp_dir, sizeof(directory), 1, virtualDisk);

    /*
    *	6.  Update bitmaps.
    */
    //Update bitmaps
    inode_bitmap[new_ino] = 1;
    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);

    /*
    *	7. Update directory.
    */
    //Fetch current directory's inode
    //Inode position of current directory
    int pos_directory_inode = 0;
    pos_directory_inode = currentDirectory.inodeID[0]; //"."
    inode tmp_directory_inode;
    fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
    fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

    //Add to current directory item
    for (int i = 2; i < DIRECTORY_NUM; i++) {
        if (strlen(currentDirectory.fileName[i]) == 0) {
            strcat(currentDirectory.fileName[i], dirName);
            currentDirectory.inodeID[i] = new_ino;
            break;
        }
    }
    //write
    fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

    //Update associations
    directory tmp_directory = currentDirectory;
    int tmp_pos_directory_inode = pos_directory_inode;
    while (true) {
        //Update association
        tmp_directory_inode.di_number++;
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        //If reach the root directory, finish updating.
        if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
            break;
        }
        //Fetch father directory
        tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
    }

    /*
    *	8. Update super block.
    */
    //superBlock.s_num_fblock--;
    superBlock.s_num_finode--;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);

    return true;
}


bool rm_r(const char* dirName) {

    //parameter check
    if (dirName == NULL || strlen(dirName) > FILE_NAME_LENGTH) {
        printf("Error: Illegal directory name.\n");
        return false;
    }

    /*
    *	1. Check whether the directory exists in current directory.
    */
    int pos_in_directory = 0;
    int tmp_dir_ino;
    inode tmp_dir_inode;
    do {
        pos_in_directory++;
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++) {
            if (strcmp(currentDirectory.fileName[pos_in_directory], dirName) == 0) {
                break;
            }
        }
        if (pos_in_directory == DIRECTORY_NUM) {
            printf("Delete error: Directory not found.\n");
            return false;
        }

        /*
        *	2. Fetch inode and check whether it's a virtualDisk.
        */
        //Fetch inode
        tmp_dir_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_dir_ino * INODE_SIZE, SEEK_SET);
        fread(&tmp_dir_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_dir_inode.di_mode == 1);

    /*
    *	3. Access check.
    */
    if (userID == tmp_dir_inode.di_uid) {
        if (!(tmp_dir_inode.permission & OWN_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    else if (users.groupID[userID] == tmp_dir_inode.di_grp) {
        if (!(tmp_dir_inode.permission & GRP_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    else {
        if (!(tmp_dir_inode.permission & ELSE_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    /*
    *	4. Start deleting. Delete all sub-directories and files first.
    */
    if (tmp_dir_inode.icount > 0) {
        tmp_dir_inode.icount--;
        fseek(virtualDisk, INODE_START + tmp_dir_inode.i_ino * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_dir_inode, sizeof(inode), 1, virtualDisk);
        /*
        *	Update directories
        */
        //Fetch current directory inode
        int pos_directory_inode = currentDirectory.inodeID[0];	//"."
        inode tmp_directory_inode;
        fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

        //Update current directory item
        memset(currentDirectory.fileName[pos_in_directory], 0, FILE_NAME_LENGTH);
        currentDirectory.inodeID[pos_in_directory] = -1;
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

        //Update associations
        directory tmp_directory = currentDirectory;
        int tmp_pos_directory_inode = pos_directory_inode;
        while (true) {
            //Update association
            tmp_directory_inode.di_number--;
            fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
            fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
            //If reach the root directory, finish updating.
            if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
                break;
            }
            //Fetch father directory
            tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
            fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
            fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
            fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
            fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
        }
        return true;
    }
    directory tmp_dir;
    fseek(virtualDisk, DATA_START + tmp_dir_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fread(&tmp_dir, sizeof(directory), 1, virtualDisk);

    //Search all sub files and directories and delete them.
    inode tmp_sub_inode;
    char tmp_sub_filename[FILE_NAME_LENGTH];
    memset(tmp_sub_filename, 0, FILE_NAME_LENGTH);
    for (int i = 2; i < DIRECTORY_NUM; i++) {
        if (strlen(tmp_dir.fileName[i]) > 0) {
            strcpy(tmp_sub_filename, tmp_dir.fileName[i]);
            //Determine whether it's a virtualDisk or a directory.
            fseek(virtualDisk, INODE_START + tmp_dir.inodeID[i] * INODE_SIZE, SEEK_SET);
            fread(&tmp_sub_inode, sizeof(inode), 1, virtualDisk);
            //Before delete sub files and directories, change current directory first and recover after deleting.
            directory tmp_swp;
            tmp_swp = currentDirectory;
            currentDirectory = tmp_dir;
            tmp_dir = tmp_swp;
            //Is a virtualDisk.
            if (tmp_sub_inode.di_mode == 1) {
                rm(tmp_sub_filename);
            }
            //Is a directory.
            else if (tmp_sub_inode.di_mode == 0) {
                rm_r(tmp_sub_filename);
            }
            tmp_swp = currentDirectory;
            currentDirectory = tmp_dir;
            tmp_dir = tmp_swp;
        }
    }

    /*
    *	5. Start deleting itself. Fill the inode's original space with 0s.
    */
    //Fill original space
    int tmp_fill[sizeof(inode)];
    memset(tmp_fill, 0, sizeof(inode));
    fseek(virtualDisk, INODE_START + tmp_dir_ino * INODE_SIZE, SEEK_SET);
    fwrite(&tmp_fill, sizeof(inode), 1, virtualDisk);

    /*
    *	6. Update bitmaps
    */
    //inode bitmap
    inode_bitmap[tmp_dir_ino] = 0;
    fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
    fwrite(&inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);
    //block bitmap
    for (int i = 0; i < (tmp_dir_inode.di_size / BLOCK_SIZE + 1); i++) {
        recycleBlock(tmp_dir_inode.di_addr[i]);
    }

    /*
    *	7. Update directories
    */
    //Fetch current directory inode
    int pos_directory_inode = currentDirectory.inodeID[0];	//"."
    inode tmp_directory_inode;
    fseek(virtualDisk, INODE_START + pos_directory_inode * INODE_SIZE, SEEK_SET);
    fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);

    //Update current directory item
    memset(currentDirectory.fileName[pos_in_directory], 0, FILE_NAME_LENGTH);
    currentDirectory.inodeID[pos_in_directory] = -1;
    fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * INODE_SIZE, SEEK_SET);
    fwrite(&currentDirectory, sizeof(directory), 1, virtualDisk);

    //Update associations
    directory tmp_directory = currentDirectory;
    int tmp_pos_directory_inode = pos_directory_inode;
    while (true) {
        //Update association
        tmp_directory_inode.di_number--;
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fwrite(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        //If reach the root directory, finish updating.
        if (tmp_directory.inodeID[1] == tmp_directory.inodeID[0]) {
            break;
        }
        //Fetch father directory
        tmp_pos_directory_inode = tmp_directory.inodeID[1];		//".."
        fseek(virtualDisk, INODE_START + tmp_pos_directory_inode * INODE_SIZE, SEEK_SET);
        fread(&tmp_directory_inode, sizeof(inode), 1, virtualDisk);
        fseek(virtualDisk, DATA_START + tmp_directory_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
        fread(&tmp_directory, sizeof(directory), 1, virtualDisk);
    }

    /*
    *	8 Update super block
    */
    //superBlock.s_num_fblock += tmp_dir_inode.di_size / BLOCK_SIZE + 1;
    superBlock.s_num_finode++;
    fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);

    return true;
}


inode* op(const char* fileName) {
    //parameter check
    if (fileName == NULL || strlen(fileName) > FILE_NAME_LENGTH)
    {
        printf("Error: Illegal virtualDisk name.\n");
        return NULL;
    }

    /*
    *	1. Check whether the virtualDisk exists in current directory.
    */
    int pos_in_directory = -1;
    inode* tmp_file_inode = new inode;
    do {
        pos_in_directory++;
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++)
        {
            if (strcmp(currentDirectory.fileName[pos_in_directory], fileName) == 0)
            {
                break;
            }
        }
        if (pos_in_directory == DIRECTORY_NUM)
        {
            printf("Open virtualDisk error: File not found.\n");
            return NULL;
        }

        /*
        *	2. Fetch inode and check whether it's a directory.
        */
        //Fetch inode
        int tmp_file_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
        fread(tmp_file_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_file_inode->di_mode == 0);

    return tmp_file_inode;
}

void simulation_start() {
    mount();
    /**
    * may be a acytoo in the future
    */
    printf("started\n");
}

/**
*   copy a file
*/
bool cp(char* filename, inode*& currentInode) {
    currentInode = op(filename);
    int block_num = currentInode->di_size / BLOCK_SIZE + 1;
    //Read virtualDisk from data blocks
    char stack[BLOCK_SIZE];
    char str[100000];
    int cnt = 0;
    if (block_num <= NADDR - 2)
    {
        for (int i = 0; i < block_num; i++)
        {
            if (currentInode->di_addr[i] == -1) break;
            fseek(virtualDisk, DATA_START + currentInode->di_addr[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                if (stack[j] == '\0') {
                    str[cnt] = 0;
                    break;
                }
                str[cnt++] = stack[j];
            }
        }
        //int i = 0;
    }
    else if (block_num > NADDR - 2) {
        //direct addressing
        for (int i = 0; i < NADDR - 2; i++)
        {
            fseek(virtualDisk, DATA_START + currentInode->di_addr[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                if (stack[j] == '\0') {
                    str[cnt] = 0;
                    break;
                }
                str[cnt++] = stack[j];
            }
        }

        //first indirect addressing
        unsigned int f1[BLOCK_SIZE / sizeof(unsigned int)] = { 0 };
        fseek(virtualDisk, DATA_START + currentInode->di_addr[NADDR - 2] * BLOCK_SIZE, SEEK_SET);
        fread(f1, sizeof(f1), 1, virtualDisk);
        for (int i = 0; i < block_num - (NADDR - 2); i++) {
            fseek(virtualDisk, DATA_START + f1[i] * BLOCK_SIZE, SEEK_SET);
            fread(stack, sizeof(stack), 1, virtualDisk);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                if (stack[j] == '\0') {
                    str[cnt] = 0;
                    break;
                }
                str[cnt++] = stack[j];
            }
        }
    }

    int pos_in_directory = -1;
    inode* tmp_file_inode = new inode;
    do {
        pos_in_directory++;
        for (; pos_in_directory < DIRECTORY_NUM; pos_in_directory++)
        {
            if (strcmp(currentDirectory.fileName[pos_in_directory], filename) == 0)
            {
                break;
            }
        }
        if (pos_in_directory == DIRECTORY_NUM)
        {
            printf("Not found.\n");
            return false;
        }

        /*
        *	2. Fetch inode and check whether it's a directory.
        */
        //Fetch inode
        int tmp_file_ino = currentDirectory.inodeID[pos_in_directory];
        fseek(virtualDisk, INODE_START + tmp_file_ino * INODE_SIZE, SEEK_SET);
        fread(tmp_file_inode, sizeof(inode), 1, virtualDisk);
        //Directory check
    } while (tmp_file_inode->di_mode == 0);

    //Access check

    if (userID == tmp_file_inode->di_uid)
    {
        if (!(tmp_file_inode->permission & OWN_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    else if (users.groupID[userID] == tmp_file_inode->di_grp) {
        if (!(tmp_file_inode->permission & GRP_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    else {
        if (!(tmp_file_inode->permission & ELSE_E)) {
            printf("Delete error: Access deny.\n");
            return false;
        }
    }
    //get absolute path
    char absolute[1024];
    int path_pos = 0;
    printf("Input absolute path(end with '#'):");
    scanf("%s", absolute);
    //get directory inode
    char dirName[14];
    int pos_dir = 0;
    bool root = false;
    inode dir_inode;
    directory cur_dir;
    int i;
    for (i = 0; i < 5; i++)
    {
        dirName[i] = absolute[i];
    }
    dirName[i] = 0;
    if (strcmp("root/", dirName) != 0)
    {
        printf("path error!\n");
        return false;
    }
    fseek(virtualDisk, INODE_START, SEEK_SET);
    fread(&dir_inode, sizeof(inode), 1, virtualDisk);
    for (int i = 5; absolute[i] != '#'; i++)
    {
        if (absolute[i] == '/')
        {
            dirName[pos_dir++] = 0;
            pos_dir = 0;
            fseek(virtualDisk, DATA_START + dir_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
            fread(&cur_dir, sizeof(directory), 1, virtualDisk);
            int i;
            for (i = 0; i < DIRECTORY_NUM; i++)
            {
                if (strcmp(cur_dir.fileName[i], dirName) == 0)
                {
                    fseek(virtualDisk, INODE_START + cur_dir.inodeID[i] * INODE_SIZE, SEEK_SET);
                    fread(&dir_inode, sizeof(inode), 1, virtualDisk);
                    if (dir_inode.di_mode == 0)break;
                }
            }
            if (i == DIRECTORY_NUM)
            {
                printf("path error!\n");
                return false;
            }
        }
        else
            dirName[pos_dir++] = absolute[i];
    }
    //update this directory
    fseek(virtualDisk, DATA_START + dir_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fread(&cur_dir, sizeof(directory), 1, virtualDisk);
    for (i = 0; i < DIRECTORY_NUM; i++)
    {
        if (strlen(cur_dir.fileName[i]) == 0)
        {
            strcat(cur_dir.fileName[i], filename);
            int new_ino = 0;
            for (; new_ino < INODE_NUM; new_ino++)
            {
                if (inode_bitmap[new_ino] == 0)
                {
                    break;
                }
            }
            inode ifile_tmp;
            ifile_tmp.i_ino = new_ino;				//Identification
            ifile_tmp.icount = 0;
            ifile_tmp.di_uid = tmp_file_inode->di_uid;
            ifile_tmp.di_grp = tmp_file_inode->di_grp;
            ifile_tmp.di_mode = tmp_file_inode->di_mode;
            memset(ifile_tmp.di_addr, -1, sizeof(unsigned int) * NADDR);
            ifile_tmp.di_size = 0;
            ifile_tmp.permission = tmp_file_inode->permission;
            time_t t = time(0);
            strftime(ifile_tmp.time, sizeof(ifile_tmp.time), "%Y/%m/%d %X %A %jday %z", localtime(&t));
            cur_dir.inodeID[i] = new_ino;
            vim(ifile_tmp, str);
            //Update bitmaps
            inode_bitmap[new_ino] = 1;
            fseek(virtualDisk, 2 * BLOCK_SIZE, SEEK_SET);
            fwrite(inode_bitmap, sizeof(unsigned short) * INODE_NUM, 1, virtualDisk);
            superBlock.s_num_finode--;
            fseek(virtualDisk, BLOCK_SIZE, SEEK_SET);
            fwrite(&superBlock, sizeof(filsys), 1, virtualDisk);
            break;
        }
    }
    if (i == DIRECTORY_NUM)
    {
        printf("No value iterms!\n");
        return false;
    }
    fseek(virtualDisk, DATA_START + dir_inode.di_addr[0] * BLOCK_SIZE, SEEK_SET);
    fwrite(&cur_dir, sizeof(directory), 1, virtualDisk);
    dir_inode.di_number++;
    fseek(virtualDisk, INODE_START + tmp_file_inode->i_ino*INODE_SIZE, SEEK_SET);
    fwrite(tmp_file_inode, sizeof(inode), 1, virtualDisk);
    return true;
}


void help() {
    cout << " All the commands are the same as linux\n";
}

void commandParser(inode*& currentInode) {
    char para1[11];
    char para2[1024];//maxmum of a insert file content
    bool flag = false;
    //Recieve commands
    while (true) {
        unsigned int f1[BLOCK_SIZE / sizeof(unsigned int)] = { 0 };
        fseek(virtualDisk, DATA_START + 8 * BLOCK_SIZE, SEEK_SET);
        fread(f1, sizeof(f1), 1, virtualDisk);
        memset(para1, 0, 11);
        memset(para2, 0, 1024);

        printf("%s>", userNameInChar);
        scanf("%s", para1);
        para1[10] = 0;		//incase we input two much, since c char[] is fucking terrible

        if (strcmp("ls", para1) == 0) {
            flag = false;
            ls();
        }
        else if (strcmp("cp", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;	//incase we input two much, since c char[] is fucking terrible
            cp(para2, currentInode);
        }
        else if (strcmp("mv", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;	//incase we input two much, since c char[] is fucking terrible
            mv(para2);
        }
        else if (strcmp("pwd", para1) == 0) {
            flag = false;
            pwd();
        }
        else if (strcmp("chmod", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;
            chmod();
        }
        else if (strcmp("fdisk", para1) == 0) {
            printf("System Info:\nTotal Block:%d\nFree Block:%d\nTotal Inode:%d\nFree Inode:%d\n\n", superBlock.s_num_block, superBlock.s_num_fblock, superBlock.s_num_inode, superBlock.s_num_finode);
            for (int i = 0; i < 50; i++) {
                if (i>superBlock.special_free)printf("-1\t");
                else printf("%d\t", superBlock.special_stack[i]);
                if (i % 10 == 9)printf("\n");
            }
            printf("\n\n");
        }
        //Create file
        else if (strcmp("touch", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;

            touch(para2);
        }
        //Delete file
        else if (strcmp("rm", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;

            rm(para2);
        }
        //Write file
        else if (strcmp("write", para1) == 0 && flag) {
            scanf("%s", para2);
            para2[1023] = 0;

            vim(*currentInode, para2);
        }
        //Read file
        else if (strcmp("cat", para1) == 0 && flag) {
             cat(*currentInode);
        }
        //Open a directory
        else if (strcmp("cd", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;

            cd(para2);
        }
        //Create dirctory
        else if (strcmp("mkdir", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;

            mymkdir(para2);
        }
        //Delete directory
        else if (strcmp("rm_r", para1) == 0) {
            flag = false;
            scanf("%s", para2);
            para2[1023] = 0;

            rm_r(para2);
        }
        //Log out
        else if (strcmp("logout", para1) == 0) {
            flag = false;
            logout();
        }
        //Log in
        else if (strcmp("su", para1) == 0) {
            login();
        }
        //Exit
        else if (strcmp("exit", para1) == 0) {
            flag = false;
            break;
        }
        //Error or help
        else {
            flag = false;
            help();
        }
    }
}



int mymain() {

    memset(currentPath, 0, sizeof(currentPath));//initial the work space, set all the bits on our disk to 0
    dir_pointer = 0;
    //read the file, if the file not exist, then we init the disk for the first use
    FILE* virtualDisk = fopen("VirtualDisk.yt", "r");
    if (virtualDisk == NULL) {
        printf("Virtual Disk not exist, initial disk now!");
        initDisk();
    }
    //if the 'disk' exist, then we can start the simulation, mount the disk and display some infomation
    simulation_start();

    //login to operate


    while (!login());
    cout << "logged in successfully" << endl;

    inode* currentInode = new inode;

    commandParser(currentInode);

    return 0;


    //cout << "the md5 of your password is :" << md5res << endl;


    system("pause");

    return 0;
}
