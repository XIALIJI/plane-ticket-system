#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "admin.h"
#include "file.h"
#include "flight.h"
#include "passenger.h"
#include "queue.h"

static void initConsole(void)
{
#ifdef _WIN32
    system("chcp 65001 > nul");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF-8");
#else
    setlocale(LC_ALL, "");
#endif
}

static void showMainMenu(void)
{
    printf("\n========================\n");
    printf("航空票务管理系统\n");
    printf("========================\n");
    printf("1.显示全部航班\n");
    printf("2.按航班号查询\n");
    printf("3.按目的地查询\n");
    printf("4.浏览乘客信息\n");
    printf("5.办理订票\n");
    printf("6.办理退票\n");
    printf("7.查看候补名单\n");
    printf("8.管理员功能\n");
    printf("0.退出系统\n");
}

int main(void)
{
    int choice;

    initConsole();

    initQueue();

    while (1)
    {
        showMainMenu();
        choice = readInt("请选择：");

        switch (choice)
        {
        case 1:
            showAllFlights();
            break;
        case 2:
            searchFlightByNo();
            break;
        case 3:
            searchFlightByDestination();
            break;
        case 4:
            showPassengers();
            break;
        case 5:
            bookTicket();
            break;
        case 6:
            refundTicket();
            break;
        case 7:
            showWaitQueue();
            break;
        case 8:
            adminMenu();
            break;
        case 0:
            clearQueue();
            printf("感谢使用，再见！\n");
            return 0;
        default:
            printf("无效选择，请重新输入。\n");
            break;
        }
    }
}
