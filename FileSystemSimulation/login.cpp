#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>

#include "login.h"
#include "const.h"
#include "md5.h"
#include "user.h"

using namespace std;

extern UserTab user_tab;

namespace acytoo {
    std::string md5(std::string strPlain) {
        MD5_CTX mdContext;
        int bytes;
        unsigned char data[1024];

        MD5Init(&mdContext);
        MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
        MD5Final(&mdContext);

        std::string md5;
        char buf[3];
        for (int i = 0; i < 16; i++)
        {
            sprintf(buf, "%02x", mdContext.digest[i]);
            md5.append(buf);
        }
        return md5;
    }
}

bool login(string strUserName, string password) {
    char tmp_userName[UserNameLen];
    char tmp_userPassword[UserPasswdLen];
    memset(tmp_userName, 0, sizeof(tmp_userName));
    memset(tmp_userPassword, 0, sizeof(tmp_userPassword));


    if (strUserName.empty() || password.empty()) {
        cout << "user name and password can't be empty!" << endl;
        return false;
    }

    string md5Passwd = acytoo::md5(password);
    md5Passwd = md5Passwd.substr(0, 31);

    for (int i = 0; i < UserNum; i++) {
        //cout << " in that for loop: " << userName << endl;
        if (user_tab.user_name[i] == strUserName) {
            //find the user and check password
            //cout << "user name match "  << "password is " << md5Passwd<< endl;
            //printf("{%s} {%s}\n", users.password[i], md5Passwd);
            //cout << md5Passwd << endl;
            if (user_tab.user_password[i] == md5Passwd) {
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
