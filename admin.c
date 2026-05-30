#include "admin.h"
#include "file.h"
#include "flight.h"

#include <stdio.h>
#include <string.h>

int adminLogin(void)
{
    char username[50];
    char password[50];

    readString("请输入管理员账号：", username, sizeof(username));
    readString("请输入管理员密码：", password, sizeof(password));

    if (strcmp(username, "admin") == 0 && strcmp(password, "123456") == 0)
    {
        printf("管理员登录成功。\n");
        return 1;
    }

    printf("账号或密码错误。\n");
    return 0;
}

void adminMenu(void)
{
    int choice;

    if (!adminLogin())
    {
        return;
    }

    while (1)
    {
        printf("\n========================\n");
        printf("管理员功能\n");
        printf("========================\n");
        printf("1.新增航班\n");
        printf("2.删除航班\n");
        printf("3.修改航班\n");
        printf("4.返回\n");

        choice = readInt("请选择：");
        switch (choice)
        {
        case 1:
            addFlight();
            break;
        case 2:
            deleteFlight();
            break;
        case 3:
            modifyFlight();
            break;
        case 4:
            return;
        default:
            printf("无效选择，请重新输入。\n");
            break;
        }
    }
}
