#include "file.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int inputEnded = 0;

/*
 * 函数名称：safeCopy
 * 函数功能：安全复制字符串，避免目标数组越界。
 * 参数说明：dest 为目标缓冲区；destSize 为缓冲区大小；src 为源字符串。
 * 返回值：无。
 * 实现说明：最多复制 destSize-1 个字符，并手动补充字符串结束符。
 */
void safeCopy(char* dest, size_t destSize, const char* src)//瀹夊叏澶嶅埗瀛楃涓诧紝纭繚鐩爣缂撳啿鍖轰笉婧㈠嚭锛屽苟涓斾互绌哄瓧绗︾粨灏?{
{
    if (dest == NULL || destSize == 0)
    {
        return;
    }

    if (src == NULL)
    {
        dest[0] = '\0';
        return;
    }

    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

/*
 * 函数名称：trimNewline
 * 函数功能：删除字符串末尾的换行符。
 * 参数说明：text 为需要处理的字符串。
 * 返回值：无。
 * 实现说明：兼容 \n 和 \r，适用于 fgets 读入的控制台输入。
 */
static void trimNewline(char* text)//鍘婚櫎瀛楃涓叉湯灏剧殑鎹㈣绗?{
{
    size_t len;

    if (text == NULL)
    {
        return;
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r'))
    {
        text[len - 1] = '\0';
        len--;
    }
}

/*
 * 函数名称：readString
 * 函数功能：读取非空字符串输入。
 * 参数说明：prompt 为提示语；buffer 为输入缓冲区；bufferSize 为缓冲区大小。
 * 返回值：无。
 * 实现说明：循环读取直到用户输入非空内容，同时处理输入结束情况。
 */
void readString(const char* prompt, char* buffer, size_t bufferSize)//璇诲彇瀛楃涓茶緭鍏ワ紝鏄剧ず鎻愮ず淇℃伅锛岀‘淇濊緭鍏ヤ笉涓虹┖锛屽苟涓斿鐞嗚緭鍏ョ粨鏉熺殑鎯呭喌
{
    if (buffer == NULL || bufferSize == 0)
    {
        return;
    }

    while (1)
    {
        if (prompt != NULL)
        {
            printf("%s", prompt);
        }

        if (fgets(buffer, (int)bufferSize, stdin) == NULL)
        {
            if (feof(stdin))
            {
                inputEnded = 1;
                buffer[0] = '\0';
                return;
            }

            clearerr(stdin);
            buffer[0] = '\0';
            continue;
        }

        trimNewline(buffer);
        if (buffer[0] != '\0')
        {
            return;
        }

        printf("杈撳叆涓嶈兘涓虹┖锛岃閲嶆柊杈撳叆銆俓n");
    }
}

/*
 * 函数名称：readInt
 * 函数功能：读取并校验整数输入。
 * 参数说明：prompt 为提示语。
 * 返回值：返回合法整数，输入结束时返回 0。
 * 实现说明：使用 strtol 判断输入是否完全为整数，非法则提示重新输入。
 */
int readInt(const char* prompt)//璇诲彇鏁存暟杈撳叆锛屾樉绀烘彁绀轰俊鎭紝纭繚杈撳叆鏄悎娉曠殑鏁存暟锛屽苟涓斿鐞嗚緭鍏ョ粨鏉熺殑鎯呭喌
{
    char buffer[64];
    char *endPtr;
    long value;

    while (1)
    {
        readString(prompt, buffer, sizeof(buffer));
        if (inputEnded)
        {
            return 0;
        }

        value = strtol(buffer, &endPtr, 10);
        while (*endPtr != '\0' && isspace((unsigned char)*endPtr))
        {
            endPtr++;
        }

        if (*endPtr == '\0')
        {
            return (int)value;
        }

        printf("璇疯緭鍏ュ悎娉曟暣鏁般€俓n");
    }
}

/*
 * 函数名称：readFloat
 * 函数功能：读取并校验浮点数输入。
 * 参数说明：prompt 为提示语。
 * 返回值：返回合法浮点数，输入结束时返回 0.0。
 * 实现说明：使用 strtod 解析，确保输入内容没有多余非法字符。
 */
float readFloat(const char* prompt)//璇诲彇娴偣鏁拌緭鍏ワ紝鏄剧ず鎻愮ず淇℃伅锛岀‘淇濊緭鍏ユ槸鍚堟硶鐨勬诞鐐规暟锛屽苟涓斿鐞嗚緭鍏ョ粨鏉熺殑鎯呭喌
{
    char buffer[64];
    char *endPtr;
    float value;

    while (1)
    {
        readString(prompt, buffer, sizeof(buffer));
        if (inputEnded)
        {
            return 0.0f;
        }

        value = (float)strtod(buffer, &endPtr);
        while (*endPtr != '\0' && isspace((unsigned char)*endPtr))
        {
            endPtr++;
        }

        if (*endPtr == '\0')
        {
            return value;
        }

        printf("璇疯緭鍏ュ悎娉曟暟瀛椼€俓n");
    }
}

/*
 * 函数名称：waitEnter
 * 函数功能：暂停程序等待用户按回车。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：用于菜单操作后停留画面，便于用户查看输出结果。
 */
void waitEnter(void)//绛夊緟鐢ㄦ埛鎸変笅鍥炶溅閿户缁?{
{
    char buffer[8];
    printf("鎸夊洖杞﹂敭缁х画...");
    (void)fgets(buffer, sizeof(buffer), stdin);
}

/*
 * 函数名称：parseFlightLine
 * 函数功能：解析航班文件中的一行数据。
 * 参数说明：line 为文件行文本；flight 为解析后的航班结构体。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：兼容旧数据中没有票价字段的情况，缺失票价时默认设为 0。
 */
static int parseFlightLine(const char *line, Flight *flight)
{
    int result;

    if (line == NULL || flight == NULL)
    {
        return 0;
    }

    flight->price = 0.0f;

#ifdef _MSC_VER
    result = sscanf_s(line, "%19s %49s %49s %19s %19s %19s %d %d %f",
                      flight->flightNo, (unsigned)_countof(flight->flightNo),
                      flight->startCity, (unsigned)_countof(flight->startCity),
                      flight->endCity, (unsigned)_countof(flight->endCity),
                      flight->date, (unsigned)_countof(flight->date),
                      flight->startTime, (unsigned)_countof(flight->startTime),
                      flight->arriveTime, (unsigned)_countof(flight->arriveTime),
                      &flight->totalSeat, &flight->remainSeat, &flight->price);
#else
    result = sscanf(line, "%19s %49s %49s %19s %19s %19s %d %d %f",
                    flight->flightNo, flight->startCity, flight->endCity,
                    flight->date, flight->startTime, flight->arriveTime,
                    &flight->totalSeat, &flight->remainSeat, &flight->price);
#endif

    if (result == 8)
    {
        flight->price = 0.0f;
        return 1;
    }

    return result == 9;
}

/*
 * 函数名称：parsePassengerLine
 * 函数功能：解析乘客订单文件中的一行数据。
 * 参数说明：line 为文件行文本；passenger 为解析后的乘客结构体。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：兼容新旧两种乘客记录格式，旧格式会补默认订单号。
 */
static int parsePassengerLine(const char *line, Passenger *passenger)
{
    int result;

    if (line == NULL || passenger == NULL)
    {
        return 0;
    }

#ifdef _MSC_VER
    result = sscanf_s(line, "%29s %99s %29s %19s %d",
                      passenger->orderId, (unsigned)_countof(passenger->orderId),
                      passenger->name, (unsigned)_countof(passenger->name),
                      passenger->phone, (unsigned)_countof(passenger->phone),
                      passenger->flightNo, (unsigned)_countof(passenger->flightNo),
                      &passenger->ticketNum);
#else
    result = sscanf(line, "%29s %99s %29s %19s %d",
                    passenger->orderId, passenger->name, passenger->phone,
                    passenger->flightNo, &passenger->ticketNum);
#endif
    if (result == 5)
    {
        return 1;
    }

#ifdef _MSC_VER
    result = sscanf_s(line, "%99s %29s %19s %d",
                      passenger->name, (unsigned)_countof(passenger->name),
                      passenger->phone, (unsigned)_countof(passenger->phone),
                      passenger->flightNo, (unsigned)_countof(passenger->flightNo),
                      &passenger->ticketNum);
#else
    result = sscanf(line, "%99s %29s %19s %d",
                    passenger->name, passenger->phone, passenger->flightNo,
                    &passenger->ticketNum);
#endif

    if (result == 4)
    {
        safeCopy(passenger->orderId, sizeof(passenger->orderId), "OLDORDER");
        return 1;
    }

    return 0;
}

/*
 * 函数名称：loadFlights
 * 函数功能：从文件加载航班数据。
 * 参数说明：flights 为航班数组；maxCount 为最大读取数量。
 * 返回值：返回实际加载的航班数量。
 * 实现说明：逐行读取 flight.txt，解析成功的记录才加入数组。
 */
int loadFlights(Flight flights[], int maxCount)//浠庢枃浠跺姞杞借埅鐝俊鎭埌鏁扮粍锛岃繑鍥炲姞杞界殑璁板綍鏁?{
{
    FILE *fp;
    char line[512];
    int count = 0;

    if (flights == NULL || maxCount <= 0)
    {
        return 0;
    }

    fp = fopen(FLIGHT_FILE, "r");
    if (fp == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌 %s锛屽皢鎸夌┖鑸彮鍒楄〃澶勭悊銆俓n", FLIGHT_FILE);
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != NULL)
    {
        Flight flight;
        if (parseFlightLine(line, &flight))
        {
            flights[count++] = flight;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：saveFlights
 * 函数功能：保存航班数组到文件。
 * 参数说明：flights 为航班数组；count 为航班数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：采用覆盖写方式保存完整航班列表。
 */
int saveFlights(const Flight flights[], int count)//淇濆瓨鑸彮淇℃伅鍒版枃浠讹紝杩斿洖淇濆瓨鎴愬姛鐨勮褰曟暟
{
    FILE *fp;
    int i;

    if (flights == NULL || count < 0)
    {
        return 0;
    }

    fp = fopen(FLIGHT_FILE, "w");
    if (fp == NULL)
    {
        printf("閿欒锛氭棤娉曟墦寮€ %s 鍐欏叆銆俓n", FLIGHT_FILE);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s %s %s %s %s %s %d %d %.2f\n",
                flights[i].flightNo,
                flights[i].startCity,
                flights[i].endCity,
                flights[i].date,
                flights[i].startTime,
                flights[i].arriveTime,
                flights[i].totalSeat,
                flights[i].remainSeat,
                flights[i].price);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：loadPassengers
 * 函数功能：从文件加载乘客订单数据。
 * 参数说明：passengers 为乘客数组；maxCount 为最大读取数量。
 * 返回值：返回实际加载的乘客数量。
 * 实现说明：逐行解析 passenger.txt，跳过格式不正确的记录。
 */
int loadPassengers(Passenger passengers[], int maxCount)//浠庢枃浠跺姞杞戒箻瀹俊鎭埌鏁扮粍锛岃繑鍥炲姞杞界殑璁板綍鏁?{
{
    FILE *fp;
    char line[512];
    int count = 0;

    if (passengers == NULL || maxCount <= 0)
    {
        return 0;
    }

    fp = fopen(PASSENGER_FILE, "r");
    if (fp == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌 %s锛屽皢鎸夌┖涔樺鍒楄〃澶勭悊銆俓n", PASSENGER_FILE);
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != NULL)
    {
        Passenger passenger;
        if (parsePassengerLine(line, &passenger))
        {
            passengers[count++] = passenger;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：savePassengers
 * 函数功能：保存乘客订单数组到文件。
 * 参数说明：passengers 为乘客数组；count 为乘客数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：退票删除订单后会调用本函数重写乘客文件。
 */
int savePassengers(const Passenger passengers[], int count)//淇濆瓨涔樺淇℃伅鍒版枃浠讹紝杩斿洖淇濆瓨鎴愬姛鐨勮褰曟暟
{
    FILE *fp;
    int i;

    if (passengers == NULL || count < 0)
    {
        return 0;
    }

    fp = fopen(PASSENGER_FILE, "w");
    if (fp == NULL)
    {
        printf("閿欒锛氭棤娉曟墦寮€ %s 鍐欏叆銆俓n", PASSENGER_FILE);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s %s %s %s %d\n",
                passengers[i].orderId,
                passengers[i].name,
                passengers[i].phone,
                passengers[i].flightNo,
                passengers[i].ticketNum);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：appendPassenger
 * 函数功能：追加保存一条乘客订单记录。
 * 参数说明：passenger 为待保存的乘客订单。
 * 返回值：追加成功返回 1，失败返回 0。
 * 实现说明：订票成功后使用追加写入，避免每次重写整个订单文件。
 */
int appendPassenger(const Passenger *passenger)
{
    FILE *fp;

    if (passenger == NULL)
    {
        return 0;
    }

    fp = fopen(PASSENGER_FILE, "a");
    if (fp == NULL)
    {
        printf("閿欒锛氭棤娉曟墦寮€ %s 杩藉姞鍐欏叆銆俓n", PASSENGER_FILE);
        return 0;
    }

    fprintf(fp, "%s %s %s %s %d\n",
            passenger->orderId,
            passenger->name,
            passenger->phone,
            passenger->flightNo,
            passenger->ticketNum);

    fclose(fp);
    return 1;
}
