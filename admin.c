#include "admin.h"
#include "file.h"
#include "flight.h"
#include "passenger.h"
#include "queue.h"

#include <conio.h>//鐢ㄤ簬_getch()鍑芥暟璇诲彇瀵嗙爜杈撳叆
#include <stdio.h>//鐢ㄤ簬杈撳叆杈撳嚭鍑芥暟
#include <string.h>//鐢ㄤ簬瀛楃涓插鐞嗗嚱鏁?
/*
 * 函数名称：readPassword
 * 函数功能：读取管理员密码并用星号显示。
 * 参数说明：prompt 为提示文本；password 为密码保存数组；passwordSize 为数组大小。
 * 返回值：无。
 * 实现说明：使用 _getch 逐字符读取，支持回车结束和退格删除，避免密码明文显示。
 */
static void readPassword(const char* prompt, char* password, int passwordSize)//璇诲彇瀵嗙爜杈撳叆锛屾樉绀轰负鏄熷彿
{
    int index = 0;
    int ch;

    if (password == NULL || passwordSize <= 0)
    {
        return;
    }

    printf("%s", prompt);
    while (1)
    {
		ch = _getch();//璇诲彇涓€涓瓧绗︼紝涓嶆樉绀哄湪鎺у埗鍙?        // 鎸変笅鍥炶溅锛氱粨鏉熻緭鍏ワ紝缁欏瘑鐮佸姞缁撴潫绗?        if (ch == '\r' || ch == '\n')
        {
            password[index] = '\0';
            printf("\n");
            return;
        }
        // 鎸変笅閫€鏍奸敭锛氬垹闄や笂涓€涓瓧绗?        else if (ch == '\b')
        {
            if (index > 0) // 鏈夊瓧绗︽墠鑳藉垹
            {
                index--;
                printf("\b \b");
            }
        }
        else if (index < passwordSize - 1)
        {
            password[index++] = (char)ch;
            printf("*");
        }
    }
}

/*
 * 函数名称：adminLogin
 * 函数功能：验证管理员登录信息。
 * 参数说明：无。
 * 返回值：登录成功返回 1，失败返回 0。
 * 实现说明：当前版本使用固定管理员账号和密码，适合课程设计演示。
 */
int adminLogin(void)//绠＄悊鍛樼櫥褰曢獙璇侊紝璐﹀彿涓篴dmin锛屽瘑鐮佷负123456
{
	char username[50];//瀛樺偍绠＄悊鍛樿处鍙?	char password[50];//瀛樺偍绠＄悊鍛樺瘑鐮?
    readString("璇疯緭鍏ョ鐞嗗憳璐﹀彿锛?, username, sizeof(username));
    readPassword("璇疯緭鍏ョ鐞嗗憳瀵嗙爜锛?, password, (int)sizeof(password));

    if (strcmp(username, "admin") == 0 && strcmp(password, "123456") == 0)//strcmp(a, b)锛氬瓧绗︿覆姣旇緝鍑芥暟鐩哥瓑杩斿洖 0
    {
        printf("绠＄悊鍛樼櫥褰曟垚鍔熴€俓n");
        return 1;
    }

    printf("璐﹀彿鎴栧瘑鐮侀敊璇€俓n");
    return 0;
}

/*
 * 函数名称：showStatistics
 * 函数功能：统计并显示航班、乘客、销售额、候补和上座率信息。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：读取航班、乘客和候补数据，按票价和票数汇总销售额。
 */
void showStatistics(void)//鏄剧ず缁熻淇℃伅   
{
    Flight flights[MAX_FLIGHTS];
    Passenger passengers[MAX_PASSENGERS];
    int flightCount;
    int passengerCount;
    int waitCount;
    int totalSeats = 0;
    int soldSeats = 0;
    int totalTickets = 0;
    float totalSales = 0.0f;
    float avgLoadRate = 0.0f;
    int i;

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    passengerCount = loadPassengers(passengers, MAX_PASSENGERS);
    waitCount = getWaitQueueCount();

	for (i = 0; i < flightCount; i++)//缁熻鎬诲骇浣嶆暟鍜屽凡鍞骇浣嶆暟
    {
        totalSeats += flights[i].totalSeat;
        soldSeats += flights[i].totalSeat - flights[i].remainSeat;
    }

	for (i = 0; i < passengerCount; i++)//缁熻鍑虹エ鏁板拰閿€鍞
    {
        int flightIndex = findFlightIndexByNo(flights, flightCount, passengers[i].flightNo);
        totalTickets += passengers[i].ticketNum;
        if (flightIndex >= 0)
        {
            totalSales += flights[flightIndex].price * passengers[i].ticketNum;
        }
    }

	if (totalSeats > 0)//璁＄畻骞冲潎涓婂骇鐜?    {
        avgLoadRate = (float)soldSeats * 100.0f / (float)totalSeats;
    }

    printf("\n========================\n");
    printf("缁熻淇℃伅\n");
    printf("========================\n");
    printf("鎬昏埅鐝暟锛?d\n", flightCount);
    printf("鎬讳箻瀹㈡暟锛?d\n", passengerCount);
    printf("鎬诲嚭绁ㄦ暟锛?d\n", totalTickets);
    printf("鎬诲€欒ˉ浜烘暟锛?d\n", waitCount);
    printf("鎬婚攢鍞锛?.2f鍏僜n", totalSales);
    printf("骞冲潎涓婂骇鐜囷細%.1f%%\n", avgLoadRate);
}

/*
 * 函数名称：adminMenu
 * 函数功能：显示并处理管理员功能菜单。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：循环读取管理员选择，并分发到航班增删改、统计和排序功能。
 */
void adminMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n========================\n");
        printf("绠＄悊鍛樺姛鑳絓n");
        printf("========================\n");
        printf("1.鏂板鑸彮\n");
        printf("2.鍒犻櫎鑸彮\n");
        printf("3.淇敼鑸彮\n");
        printf("4.缁熻淇℃伅\n");
        printf("5.鑸彮鎺掑簭\n");
        printf("0.杩斿洖\n");

        choice = readInt("璇烽€夋嫨锛?);
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
            showStatistics();
            break;
        case 5:
            sortFlightMenu();
            break;
        case 0:
            return;
        default:
            printf("鏃犳晥閫夋嫨锛岃閲嶆柊杈撳叆銆俓n");
            break;
        }
    }
}
