#include "flight.h"
#include "file.h"

#include <stdio.h>
#include <string.h>

/*
 * 函数名称：findFlightIndexByNo
 * 函数功能：根据航班号查找航班下标。
 * 参数说明：flights 为航班数组；count 为航班数量；flightNo 为目标航班号。
 * 返回值：找到返回数组下标，未找到返回 -1。
 * 实现说明：使用 strcmp 精确匹配航班号，供查询、修改、订票等功能复用。
 */
int findFlightIndexByNo(Flight flights[], int count, const char *flightNo)
{
    int i;

    if (flights == NULL || flightNo == NULL)
    {
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(flights[i].flightNo, flightNo) == 0)
        {
            return i;
        }
    }

    return -1;
}

/*
 * 函数名称：printFlightHeader
 * 函数功能：打印航班信息表头。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：与 printFlight 配合使用，使航班列表显示格式统一。
 */
void printFlightHeader(void)// 鎵撳嵃鑸彮淇℃伅琛ㄥご
{
    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8s %-8s %-10s\n",
           "鑸彮鍙?, "鍑哄彂鍦?, "鐩殑鍦?, "鏃ユ湡", "璧烽", "鍒拌揪", "鎬荤エ鏁?, "浣欑エ", "绁ㄤ环");
    printf("------------------------------------------------------------------------------------------\n");
}

/*
 * 函数名称：printFlight
 * 函数功能：打印单条航班信息。
 * 参数说明：flight 为需要显示的航班指针。
 * 返回值：无。
 * 实现说明：函数内部先判断空指针，避免访问无效内存。
 */
void printFlight(const Flight* flight)// 鎵撳嵃鍗曟潯鑸彮淇℃伅
{
    if (flight == NULL)
    {
        return;
    }

    printf("%-12s %-12s %-12s %-14s %-10s %-10s %-8d %-8d %-10.2f\n",
           flight->flightNo,
           flight->startCity,
           flight->endCity,
           flight->date,
           flight->startTime,
           flight->arriveTime,
           flight->totalSeat,
           flight->remainSeat,
           flight->price);
}

/*
 * 函数名称：showFlightArray
 * 函数功能：显示航班数组中的所有航班信息。
 * 参数说明：flights 为航班数组；count 为航班数量。
 * 返回值：无。
 * 实现说明：无数据时输出提示，有数据时先打印表头再逐条打印。
 */
static void showFlightArray(Flight flights[], int count)// 鏄剧ず鑸彮淇℃伅鍒楄〃
{
    int i;

    if (count == 0)
    {
        printf("鏆傛棤鑸彮淇℃伅銆俓n");
        return;
    }

    printFlightHeader();
    for (i = 0; i < count; i++)
    {
        printFlight(&flights[i]);
    }
}

/*
 * 函数名称：showAllFlights
 * 函数功能：显示全部航班信息。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：先从文件加载航班数组，再调用 showFlightArray 统一显示。
 */
void showAllFlights(void)// 鏄剧ず鍏ㄩ儴鑸彮淇℃伅
{
    Flight flights[MAX_FLIGHTS];
    int count = loadFlights(flights, MAX_FLIGHTS);
    showFlightArray(flights, count);
}

/*
 * 函数名称：searchFlightByNo
 * 函数功能：按航班号查询航班。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：读取用户输入的航班号，在航班数组中精确查找并显示结果。
 */
void searchFlightByNo(void)// 鏄剧ず鑸彮鍙锋煡璇㈢粨鏋?{
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;

    readString("璇疯緭鍏ヨ埅鐝彿锛?, flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("鏈壘鍒拌埅鐝彿涓?%s 鐨勮埅鐝€俓n", flightNo);
        return;
    }

    printFlightHeader();
    printFlight(&flights[index]);
}

/*
 * 函数名称：searchFlightByDestination
 * 函数功能：按目的地查询航班。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：遍历所有航班，目的地完全匹配时输出该航班信息。
 */
void searchFlightByDestination(void)// 鏄剧ず鐩殑鍦版煡璇㈢粨鏋?{
{
    Flight flights[MAX_FLIGHTS];
    char destination[50];
    int count;
    int i;
    int found = 0;

    readString("璇疯緭鍏ョ洰鐨勫湴锛?, destination, sizeof(destination));
    count = loadFlights(flights, MAX_FLIGHTS);

    for (i = 0; i < count; i++)
    {
        if (strcmp(flights[i].endCity, destination) == 0)
        {
            if (!found)
            {
                printFlightHeader();
            }
            printFlight(&flights[i]);
            found = 1;
        }
    }

    if (!found)
    {
        printf("鏈壘鍒扮洰鐨勫湴涓?%s 鐨勮埅鐝€俓n", destination);
    }
}

/*
 * 函数名称：addFlight
 * 函数功能：新增航班信息。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：先检查航班数量上限和航班号唯一性，再保存新增航班到文件。
 */
void addFlight(void)//鏂板鑸彮淇℃伅
{
    Flight flights[MAX_FLIGHTS];
    Flight flight;
    int count;

    count = loadFlights(flights, MAX_FLIGHTS);
    if (count >= MAX_FLIGHTS)
    {
        printf("鑸彮鏁伴噺宸茶揪涓婇檺锛屾棤娉曟柊澧炪€俓n");
        return;
    }

    readString("璇疯緭鍏ヨ埅鐝彿锛?, flight.flightNo, sizeof(flight.flightNo));
    if (findFlightIndexByNo(flights, count, flight.flightNo) >= 0)
    {
        printf("鑸彮鍙峰凡瀛樺湪锛屾柊澧炲け璐ャ€俓n");
        return;
    }

    readString("璇疯緭鍏ュ嚭鍙戝湴锛?, flight.startCity, sizeof(flight.startCity));
    readString("璇疯緭鍏ョ洰鐨勫湴锛?, flight.endCity, sizeof(flight.endCity));
    readString("璇疯緭鍏ユ棩鏈?YYYY-MM-DD)锛?, flight.date, sizeof(flight.date));
    readString("璇疯緭鍏ヨ捣椋炴椂闂?HH:MM)锛?, flight.startTime, sizeof(flight.startTime));
    readString("璇疯緭鍏ュ埌杈炬椂闂?HH:MM)锛?, flight.arriveTime, sizeof(flight.arriveTime));
    flight.totalSeat = readInt("璇疯緭鍏ユ€诲骇浣嶆暟锛?);
    flight.remainSeat = readInt("璇疯緭鍏ヤ綑绁ㄦ暟锛?);
    flight.price = readFloat("璇疯緭鍏ョエ浠凤細");

    if (flight.totalSeat < 0 || flight.remainSeat < 0 || flight.remainSeat > flight.totalSeat || flight.price < 0)
    {
        printf("鑸彮鏁版嵁涓嶅悎娉曪紝鏂板澶辫触銆俓n");
        return;
    }

    flights[count++] = flight;
    if (saveFlights(flights, count))
    {
        printf("鏂板鑸彮鎴愬姛銆俓n");
    }
}

/*
 * 函数名称：deleteFlight
 * 函数功能：删除指定航班。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：找到目标航班后，通过数组元素前移覆盖实现删除。
 */
void deleteFlight(void)//鍒犻櫎鑸彮淇℃伅
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;
    int i;

    readString("璇疯緭鍏ヨ鍒犻櫎鐨勮埅鐝彿锛?, flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("鑸彮涓嶅瓨鍦紝鍒犻櫎澶辫触銆俓n");
        return;
    }

    for (i = index; i < count - 1; i++)
    {
        flights[i] = flights[i + 1];
    }
    count--;

    if (saveFlights(flights, count))
    {
        printf("鍒犻櫎鑸彮鎴愬姛銆俓n");
    }
}

/*
 * 函数名称：modifyFlight
 * 函数功能：修改指定航班信息。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：按航班号定位原记录，重新读取各字段并覆盖保存。
 */
void modifyFlight(void)//淇敼鑸彮淇℃伅
{
    Flight flights[MAX_FLIGHTS];
    char flightNo[20];
    int count;
    int index;
    Flight *flight;

    readString("璇疯緭鍏ヨ淇敼鐨勮埅鐝彿锛?, flightNo, sizeof(flightNo));
    count = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, count, flightNo);

    if (index < 0)
    {
        printf("鑸彮涓嶅瓨鍦紝淇敼澶辫触銆俓n");
        return;
    }

    flight = &flights[index];
    printf("褰撳墠鑸彮淇℃伅锛歕n");
    printFlightHeader();
    printFlight(flight);

    readString("璇疯緭鍏ユ柊鐨勫嚭鍙戝湴锛?, flight->startCity, sizeof(flight->startCity));
    readString("璇疯緭鍏ユ柊鐨勭洰鐨勫湴锛?, flight->endCity, sizeof(flight->endCity));
    readString("璇疯緭鍏ユ柊鐨勬棩鏈?YYYY-MM-DD)锛?, flight->date, sizeof(flight->date));
    readString("璇疯緭鍏ユ柊鐨勮捣椋炴椂闂?HH:MM)锛?, flight->startTime, sizeof(flight->startTime));
    readString("璇疯緭鍏ユ柊鐨勫埌杈炬椂闂?HH:MM)锛?, flight->arriveTime, sizeof(flight->arriveTime));
    flight->totalSeat = readInt("璇疯緭鍏ユ柊鐨勬€诲骇浣嶆暟锛?);
    flight->remainSeat = readInt("璇疯緭鍏ユ柊鐨勪綑绁ㄦ暟锛?);
    flight->price = readFloat("璇疯緭鍏ユ柊鐨勭エ浠凤細");

    if (flight->totalSeat < 0 || flight->remainSeat < 0 || flight->remainSeat > flight->totalSeat || flight->price < 0)
    {
        printf("鑸彮鏁版嵁涓嶅悎娉曪紝淇敼澶辫触銆俓n");
        return;
    }

    if (saveFlights(flights, count))
    {
        printf("淇敼鑸彮鎴愬姛銆俓n");
    }
}

/*
 * 函数名称：bubbleSort
 * 函数功能：按照指定字段对航班数组进行冒泡排序。
 * 参数说明：flights 为航班数组；count 为航班数量；sortType 为排序类型。
 * 返回值：无。
 * 实现说明：sortType 控制按航班号、余票、票价或起飞时间比较并交换。
 */
static void bubbleSort(Flight flights[], int count, int sortType)//鍐掓场鎺掑簭瀹炵幇锛屾牴鎹畇ortType閫夋嫨鎺掑簭渚濇嵁
{
    int i;
    int j;

    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - 1 - i; j++)
        {
            int needSwap = 0;

            if (sortType == 1 && strcmp(flights[j].flightNo, flights[j + 1].flightNo) > 0)
            {
                needSwap = 1;
            }
            else if (sortType == 2 && flights[j].remainSeat > flights[j + 1].remainSeat)
            {
                needSwap = 1;
            }
            else if (sortType == 3 && flights[j].price > flights[j + 1].price)
            {
                needSwap = 1;
            }
            else if (sortType == 4 && strcmp(flights[j].startTime, flights[j + 1].startTime) > 0)
            {
                needSwap = 1;
            }

            if (needSwap)
            {
                Flight temp = flights[j];
                flights[j] = flights[j + 1];
                flights[j + 1] = temp;
            }
        }
    }
}

/*
 * 函数名称：sortAndShow
 * 函数功能：加载航班、排序并显示结果。
 * 参数说明：sortType 为排序类型。
 * 返回值：无。
 * 实现说明：排序只影响当前显示顺序，不修改文件中的原始航班顺序。
 */
static void sortAndShow(int sortType)//鎺掑簭骞舵樉绀鸿埅鐝俊鎭?{
{
	Flight flights[MAX_FLIGHTS];//鏍规嵁sortType閫夋嫨鎺掑簭渚濇嵁锛?-鑸彮鍙凤紝2-浣欑エ鏁帮紝3-绁ㄤ环锛?-璧烽鏃堕棿
    int count = loadFlights(flights, MAX_FLIGHTS);

    bubbleSort(flights, count, sortType);
    showFlightArray(flights, count);
}

/*
 * 函数名称：sortFlightByNo
 * 函数功能：按航班号排序并显示。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：调用 sortAndShow 并传入航班号排序类型。
 */
void sortFlightByNo(void)   //鎸夎埅鐝彿鎺掑簭
{
    sortAndShow(1);
}

/*
 * 函数名称：sortFlightByRemainSeat
 * 函数功能：按余票数量排序并显示。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：调用 sortAndShow 并传入余票排序类型。
 */
void sortFlightByRemainSeat(void)      //鎸変綑绁ㄦ暟鎺掑簭
{
    sortAndShow(2);
}

/*
 * 函数名称：sortFlightByPrice
 * 函数功能：按票价排序并显示。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：调用 sortAndShow 并传入票价排序类型。
 */
void sortFlightByPrice(void)//鎸夌エ浠锋帓搴?{
{
    sortAndShow(3);
}

/*
 * 函数名称：sortFlightByStartTime
 * 函数功能：按起飞时间排序并显示。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：调用 sortAndShow 并传入起飞时间排序类型。
 */
void sortFlightByStartTime(void)//鎸夎捣椋炴椂闂存帓搴?{
{
    sortAndShow(4);
}

/*
 * 函数名称：sortFlightMenu
 * 函数功能：显示并处理航班排序菜单。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：根据用户选择调用不同排序函数，输入 0 返回管理员菜单。
 */
void sortFlightMenu(void)//鏄剧ず鑸彮鎺掑簭鑿滃崟骞跺鐞嗙敤鎴烽€夋嫨
{
    int choice;

    while (1)
    {
        printf("\n========================\n");
        printf("鑸彮鎺掑簭\n");
        printf("========================\n");
        printf("1.鎸夎埅鐝彿鎺掑簭\n");
        printf("2.鎸変綑绁ㄦ帓搴廫n");
        printf("3.鎸夌エ浠锋帓搴廫n");
        printf("4.鎸夎捣椋炴椂闂存帓搴廫n");
        printf("0.杩斿洖\n");

        choice = readInt("璇烽€夋嫨锛?);
        switch (choice)
        {
        case 1:
            sortFlightByNo();
            break;
        case 2:
            sortFlightByRemainSeat();
            break;
        case 3:
            sortFlightByPrice();
            break;
        case 4:
            sortFlightByStartTime();
            break;
        case 0:
            return;
        default:
            printf("鏃犳晥閫夋嫨锛岃閲嶆柊杈撳叆銆俓n");
            break;
        }
    }
}
