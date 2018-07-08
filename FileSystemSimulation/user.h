#ifndef USER_H
#define USER_H
#include "const.h"

/*
    currently no user id...
    Alec Chen
    由用户表取代索引块。。。
*/
typedef struct {
    char user_name[UserNum][UserNameLen];
    char user_password[UserNum][UserPasswdLen];
}UserTab;

#endif
