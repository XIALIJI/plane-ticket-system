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
#include "user.h"

/*
 * 函数名称：initConsole
 * 函数功能：初始化控制台编码和区域设置。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：Windows 下设置控制台为 UTF-8，尽量保证中文菜单正常显示。
 */
static void initConsole(void)//璁剧疆鎺у埗鍙扮紪鐮佸拰鍖哄煙璁剧疆
{
#ifdef _WIN32
    system("cmd /c chcp 65001 > nul < nul");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF-8");
#else
    setlocale(LC_ALL, "");
#endif
}

/*
 * 函数名称：showStartMenu
 * 函数功能：显示系统主菜单。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：该函数只负责打印菜单，具体选择由 main 函数读取并处理。
 */
static void showStartMenu(void)//鏄剧ず涓昏彍鍗?鍖哄垎鐢ㄦ埛鍜岀鐞嗗憳鐧诲綍閫夐」
{
    printf("\n========================\n");
    printf("鑸┖绁ㄥ姟绠＄悊绯荤粺\n");
    printf("========================\n");
    printf("1.鐢ㄦ埛鐧诲綍\n");
    printf("2.鐢ㄦ埛娉ㄥ唽\n");
    printf("3.绠＄悊鍛樼櫥褰昞n");
    printf("0.閫€鍑篭n");
}

/*
 * 函数名称：showUserMenu
 * 函数功能：显示普通用户功能菜单。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：集中展示用户可用的航班查询、订票、退票和候补查询功能。
 */
static void showUserMenu(void)//鏄剧ず鐢ㄦ埛鑿滃崟
{
    printf("\n========================\n");
    printf("鐢ㄦ埛鍔熻兘鑿滃崟\n");
    printf("========================\n");
    printf("1.鏄剧ず鍏ㄩ儴鑸彮\n");
    printf("2.鎸夎埅鐝彿鏌ヨ\n");
    printf("3.鎸夌洰鐨勫湴鏌ヨ\n");
    printf("4.娴忚涔樺淇℃伅\n");
    printf("5.鍔炵悊璁㈢エ\n");
    printf("6.鍔炵悊閫€绁╘n");
    printf("7.鏌ョ湅鍊欒ˉ鍚嶅崟\n");
    printf("0.杩斿洖涓婄骇鑿滃崟\n");
}

/*
 * 函数名称：userMenu
 * 函数功能：处理普通用户菜单循环。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：根据用户输入的编号调用对应业务函数，输入 0 时返回上级菜单。
 */
static void userMenu(void)//澶勭悊鐢ㄦ埛鑿滃崟鐨勮緭鍏ュ拰鍔熻兘閫夋嫨
{
    int choice;

    while (1)
    {
        showUserMenu();
        choice = readInt("璇烽€夋嫨锛?);

        switch (choice)
        {
        case 1:
			showAllFlights();  //鏄剧ず鍏ㄩ儴鑸彮淇℃伅
            break;
        case 2:
			searchFlightByNo();//鏄剧ず鑸彮鍙锋煡璇㈢粨鏋?            break;
        case 3:
			searchFlightByDestination();//鏄剧ず鐩殑鍦版煡璇㈢粨鏋?            break;
        case 4:
			showPassengers();//鏄剧ず涔樺淇℃伅
            break;
        case 5:
			bookTicket();//鍔炵悊璁㈢エ
            break;
        case 6:
			refundTicket();//鍔炵悊閫€绁?            break;
        case 7:
			showWaitQueue();   //鏄剧ず鍊欒ˉ鍚嶅崟
            break;
        case 0:
            return;
        default:
            printf("鏃犳晥閫夋嫨锛岃閲嶆柊杈撳叆銆俓n");
            break;
        }
    }
}

/*
 * 函数名称：main
 * 函数功能：单机版航空票务系统入口函数。
 * 参数说明：无。
 * 返回值：程序正常退出返回 0。
 * 实现说明：先初始化控制台和候补队列，再进入主菜单循环处理登录、注册和退出。
 */
int main(void)
{
    int choice;

	initConsole();//鍒濆鍖栨帶鍒跺彴缂栫爜鍜屽尯鍩熻缃?	initQueue();//鍒濆鍖栧€欒ˉ鍚嶅崟闃熷垪

    while (1)
    {
        showStartMenu();
        choice = readInt("璇烽€夋嫨锛?);

        switch (choice)
        {
		case 1://鐢ㄦ埛鐧诲綍
            if (userLogin())
            {
                userMenu();
            }
            break;
		case 2://鐢ㄦ埛娉ㄥ唽
            registerUser();
            break;
		case 3://绠＄悊鍛樼櫥褰?            if (adminLogin())
            {
                adminMenu();
            }
            break;
		case 0://閫€鍑虹▼搴?            clearQueue();
            printf("鎰熻阿浣跨敤锛屽啀瑙侊紒\n");
            return 0;
        default:
            printf("鏃犳晥閫夋嫨锛岃閲嶆柊杈撳叆銆俓n");
            break;
        }
    }
}
