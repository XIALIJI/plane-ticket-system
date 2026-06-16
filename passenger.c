#include "passenger.h"
#include "file.h"
#include "flight.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * 函数名称：printPassengerHeader
 * 函数功能：打印乘客订单信息表头。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：与 printPassenger 配合使用，保证乘客列表格式统一。
 */
static void printPassengerHeader(void)//鎵撳嵃涔樺淇℃伅琛ㄥご
{
    printf("%-18s %-16s %-16s %-12s %-8s\n", "璁㈠崟鍙?, "濮撳悕", "鎵嬫満鍙?, "鑸彮鍙?, "绁ㄦ暟");
    printf("--------------------------------------------------------------------------\n");
}

/*
 * 函数名称：printPassenger
 * 函数功能：打印单条乘客订单信息。
 * 参数说明：passenger 为乘客订单指针。
 * 返回值：无。
 * 实现说明：打印前判断空指针，避免访问无效内存。
 */
static void printPassenger(const Passenger* passenger)//鎵撳嵃涔樺淇℃伅
{
    if (passenger == NULL)
    {
        return;
    }

    printf("%-18s %-16s %-16s %-12s %-8d\n",
           passenger->orderId,
           passenger->name,
           passenger->phone,
           passenger->flightNo,
           passenger->ticketNum);
}

/*
 * 函数名称：getTodayText
 * 函数功能：生成当天日期文本。
 * 参数说明：dateText 为输出缓冲区；dateTextSize 为缓冲区大小。
 * 返回值：无。
 * 实现说明：日期格式为 YYYYMMDD，用于订单号自动生成。
 */
static void getTodayText(char* dateText, int dateTextSize)//鐢熸垚褰撳ぉ鏃ユ湡鏂?{
{
    time_t now;
    struct tm localTime;

    if (dateText == NULL || dateTextSize <= 0)
    {
        return;
    }

    now = time(NULL);
#ifdef _MSC_VER
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif

    snprintf(dateText, (size_t)dateTextSize, "%04d%02d%02d",
             localTime.tm_year + 1900,
             localTime.tm_mon + 1,
             localTime.tm_mday);
}

/*
 * 函数名称：generateOrderId
 * 函数功能：自动生成订单号。
 * 参数说明：orderId 为输出订单号；orderIdSize 为缓冲区大小。
 * 返回值：无。
 * 实现说明：扫描当天已有订单的最大流水号，在其基础上加一生成新订单号。
 */
void generateOrderId(char* orderId, int orderIdSize)//鐢熸垚璁㈠崟鍙?{
{
    Passenger passengers[MAX_PASSENGERS];
    char dateText[16];
    char prefix[20];
    int count;
    int i;
    int maxSerial = 0;

    if (orderId == NULL || orderIdSize <= 0)
    {
        return;
    }

    getTodayText(dateText, sizeof(dateText));
    snprintf(prefix, sizeof(prefix), "OD%s", dateText);

	count = loadPassengers(passengers, MAX_PASSENGERS);//鍔犺浇涔樺淇℃伅锛屾煡鎵惧綋澶╄鍗曠殑鏈€澶у簭鍒楀彿
    for (i = 0; i < count; i++)
    {
        if (strncmp(passengers[i].orderId, prefix, strlen(prefix)) == 0)
        {
            int serial = atoi(passengers[i].orderId + strlen(prefix));
            if (serial > maxSerial)
            {
                maxSerial = serial;
            }
        }
    }

    snprintf(orderId, (size_t)orderIdSize, "%s%04d", prefix, maxSerial + 1);
}

/*
 * 函数名称：showPassengers
 * 函数功能：显示全部乘客订单信息。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：从文件加载乘客订单数组，无数据时输出提示。
 */
void showPassengers(void)//鏄剧ず涔樺淇℃伅
{
    Passenger passengers[MAX_PASSENGERS];
    int count;
    int i;

    count = loadPassengers(passengers, MAX_PASSENGERS);
    if (count == 0)
    {
        printf("鏆傛棤涔樺淇℃伅銆俓n");
        return;
    }

    printPassengerHeader();
    for (i = 0; i < count; i++)
    {
        printPassenger(&passengers[i]);
    }
}

/*
 * 函数名称：bookTicket
 * 函数功能：办理订票业务。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：检查航班是否存在和余票是否充足；余票不足时加入候补，保存订单失败时回滚余票。
 */
void bookTicket(void)//鍔炵悊璁㈢エ
{
    Flight flights[MAX_FLIGHTS];
    Passenger passenger;
    int flightCount;
    int index;
    float totalAmount;

    generateOrderId(passenger.orderId, sizeof(passenger.orderId));
    readString("璇疯緭鍏ュ鍚嶏細", passenger.name, sizeof(passenger.name));
    readString("璇疯緭鍏ユ墜鏈哄彿锛?, passenger.phone, sizeof(passenger.phone));
    readString("璇疯緭鍏ヨ埅鐝彿锛?, passenger.flightNo, sizeof(passenger.flightNo));
    passenger.ticketNum = readInt("璇疯緭鍏ョエ鏁帮細");

    if (passenger.ticketNum <= 0)
    {
        printf("绁ㄦ暟蹇呴』澶т簬 0銆俓n");
        return;
    }

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    index = findFlightIndexByNo(flights, flightCount, passenger.flightNo);
    if (index < 0)
    {
        printf("鑸彮涓嶅瓨鍦紝璁㈢エ澶辫触銆俓n");
        return;
    }

    if (flights[index].remainSeat < passenger.ticketNum)
    {
        printf("浣欑エ涓嶈冻锛屽綋鍓嶄綑绁?%d 寮犮€俓n", flights[index].remainSeat);
        if (enqueueWait(passenger.name, passenger.phone, passenger.flightNo, passenger.ticketNum))
        {
            printf("宸茶嚜鍔ㄥ姞鍏ュ€欒ˉ鍚嶅崟銆俓n");
        }
        return;
    }

    flights[index].remainSeat -= passenger.ticketNum;
    if (!saveFlights(flights, flightCount))
    {
        printf("淇濆瓨鑸彮淇℃伅澶辫触锛岃绁ㄥ彇娑堛€俓n");
        return;
    }

    if (!appendPassenger(&passenger))
    {
        flights[index].remainSeat += passenger.ticketNum;
        (void)saveFlights(flights, flightCount);
        printf("淇濆瓨涔樺璁㈠崟澶辫触锛岃绁ㄥ彇娑堛€俓n");
        return;
    }

    totalAmount = flights[index].price * passenger.ticketNum;
    printf("璁㈢エ鎴愬姛銆俓n");
    printf("璁㈠崟鍙凤細%s\n", passenger.orderId);
    printf("绁ㄤ环锛?.2f\n", flights[index].price);
    printf("璁㈢エ鏁伴噺锛?d\n", passenger.ticketNum);
    printf("鎬婚噾棰濓細%.2f鍏僜n", totalAmount);
}

/*
 * 函数名称：refundTicket
 * 函数功能：办理退票业务。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：按订单号查找订单，恢复对应航班余票，删除订单后触发候补自动出票。
 */
void refundTicket(void)//鍔炵悊閫€绁?{
{
    Passenger passengers[MAX_PASSENGERS];
    Flight flights[MAX_FLIGHTS];
    char orderId[30];
    char refundedFlightNo[20];
    int passengerCount;
    int flightCount;
    int passengerIndex = -1;
    int flightIndex;
    int i;
    int refundNum;

    readString("璇疯緭鍏ラ€€绁ㄨ鍗曞彿锛?, orderId, sizeof(orderId));
    passengerCount = loadPassengers(passengers, MAX_PASSENGERS);

	for (i = 0; i < passengerCount; i++)//鏌ユ壘璁㈠崟鍙峰搴旂殑涔樺淇℃伅
    {
        if (strcmp(passengers[i].orderId, orderId) == 0)
        {
            passengerIndex = i;
            break;
        }
    }

    if (passengerIndex < 0)
    {
        printf("鏈壘鍒拌璁㈠崟鍙峰搴旂殑璁㈠崟銆俓n");
        return;
    }

    safeCopy(refundedFlightNo, sizeof(refundedFlightNo), passengers[passengerIndex].flightNo);
    refundNum = passengers[passengerIndex].ticketNum;

    flightCount = loadFlights(flights, MAX_FLIGHTS);
    flightIndex = findFlightIndexByNo(flights, flightCount, refundedFlightNo);
    if (flightIndex >= 0)
    {
        flights[flightIndex].remainSeat += refundNum;
        if (flights[flightIndex].remainSeat > flights[flightIndex].totalSeat)
        {
            flights[flightIndex].remainSeat = flights[flightIndex].totalSeat;
        }
        (void)saveFlights(flights, flightCount);
    }

    for (i = passengerIndex; i < passengerCount - 1; i++)
    {
        passengers[i] = passengers[i + 1];
    }
    passengerCount--;

    if (!savePassengers(passengers, passengerCount))
    {
        printf("淇濆瓨涔樺淇℃伅澶辫触锛岄€€绁ㄦ湭瀹屾垚銆俓n");
        return;
    }

    printf("閫€绁ㄦ垚鍔燂紝宸叉仮澶嶄綑绁?%d 寮犮€俓n", refundNum);
    autoBookFromQueue();
}
