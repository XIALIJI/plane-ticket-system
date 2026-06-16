#include "server.h"
#include "../common/config.h"
#include "../common/protocol.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CRITICAL_SECTION g_dataLock;

/*
 * 函数名称：ensureDataDir
 * 函数功能：确保 data 数据目录存在。
 * 参数说明：无。
 * 返回值：无。
 * 实现说明：写入航班文件前先创建目录，目录已存在时 _mkdir 会安全失败，不影响程序继续执行。
 */
static void ensureDataDir(void)
{
    (void)_mkdir(DATA_DIR);
}

/*
 * 函数名称：parseFlightLine
 * 函数功能：把文件中的一行文本解析为 FlightRecord 航班结构。
 * 参数说明：line 为文件原始行；flight 为解析后的航班保存位置。
 * 返回值：解析成功返回 1，失败返回 0。
 * 实现说明：优先解析新版“|”分隔格式；若失败，再兼容旧项目空格分隔格式。
 */
static int parseFlightLine(const char *line, FlightRecord *flight)
{
    int ret;

    if (line == 0 || flight == 0)
    {
        return 0;
    }

    memset(flight, 0, sizeof(FlightRecord));
    ret = sscanf_s(line, "%19[^|]|%49[^|]|%49[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%f|%19[^|\n]",
        flight->flightNo, (unsigned)_countof(flight->flightNo),
        flight->startCity, (unsigned)_countof(flight->startCity),
        flight->endCity, (unsigned)_countof(flight->endCity),
        flight->date, (unsigned)_countof(flight->date),
        flight->startTime, (unsigned)_countof(flight->startTime),
        flight->arriveTime, (unsigned)_countof(flight->arriveTime),
        &flight->totalSeat,
        &flight->remainSeat,
        &flight->price,
        flight->status, (unsigned)_countof(flight->status));

    if (ret == 10)
    {
        return 1;
    }

    ret = sscanf_s(line, "%19s %49s %49s %19s %19s %19s %d %d %f",
        flight->flightNo, (unsigned)_countof(flight->flightNo),
        flight->startCity, (unsigned)_countof(flight->startCity),
        flight->endCity, (unsigned)_countof(flight->endCity),
        flight->date, (unsigned)_countof(flight->date),
        flight->startTime, (unsigned)_countof(flight->startTime),
        flight->arriveTime, (unsigned)_countof(flight->arriveTime),
        &flight->totalSeat,
        &flight->remainSeat,
        &flight->price);
    if (ret >= 8)
    {
        if (ret == 8)
        {
            flight->price = 0.0f;
        }
        copyText(flight->status, (int)sizeof(flight->status), "正常");
        return 1;
    }

    return 0;
}

/*
 * 函数名称：loadFlightsData
 * 函数功能：从航班数据文件读取航班列表。
 * 参数说明：flights 为航班数组；maxCount 为最多读取数量。
 * 返回值：实际读取到的航班数量。
 * 实现说明：逐行读取文件，只有解析成功的记录才会加入数组。
 */
int loadFlightsData(FlightRecord flights[], int maxCount)
{
    FILE *fp;
    char line[512];
    int count = 0;

    ensureDataDir();
    if (fopen_s(&fp, FLIGHTS_FILE, "r") != 0 || fp == 0)
    {
        return 0;
    }

    while (count < maxCount && fgets(line, sizeof(line), fp) != 0)
    {
        FlightRecord flight;
        if (parseFlightLine(line, &flight))
        {
            flights[count++] = flight;
        }
    }

    fclose(fp);
    return count;
}

/*
 * 函数名称：saveFlightsData
 * 函数功能：将航班数组保存到航班数据文件。
 * 参数说明：flights 为航班数组；count 为需要保存的航班数量。
 * 返回值：保存成功返回 1，失败返回 0。
 * 实现说明：统一使用“|”分隔格式写入，便于中文字段和多字段数据维护。
 */
int saveFlightsData(const FlightRecord flights[], int count)
{
    FILE *fp;
    int i;

    ensureDataDir();
    if (fopen_s(&fp, FLIGHTS_FILE, "w") != 0 || fp == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%d|%d|%.2f|%s\n",
            flights[i].flightNo,
            flights[i].startCity,
            flights[i].endCity,
            flights[i].date,
            flights[i].startTime,
            flights[i].arriveTime,
            flights[i].totalSeat,
            flights[i].remainSeat,
            flights[i].price,
            flights[i].status);
    }

    fclose(fp);
    return 1;
}

/*
 * 函数名称：findFlightIndex
 * 函数功能：根据航班号查找航班在数组中的下标。
 * 参数说明：flights 为航班数组；count 为航班数量；flightNo 为要查找的航班号。
 * 返回值：找到返回下标，未找到返回 -1。
 * 实现说明：使用 strcmp 精确匹配航班号，保证新增、修改、订票时定位同一条记录。
 */
int findFlightIndex(const FlightRecord flights[], int count, const char *flightNo)
{
    int i;

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
 * 函数名称：appendFlightLine
 * 函数功能：把一条航班信息格式化后追加到响应文本中。
 * 参数说明：buffer 为响应缓冲区；bufferSize 为缓冲区大小；flight 为航班记录。
 * 返回值：无。
 * 实现说明：用于查询航班时拼接多行显示内容，同时由 appendText 控制长度安全。
 */
static void appendFlightLine(char *buffer, int bufferSize, const FlightRecord *flight)
{
    char line[180];

    sprintf_s(line, sizeof(line), "%s %s->%s %s %s-%s 余票:%d/%d 票价:%.2f 状态:%s\n",
        flight->flightNo,
        flight->startCity,
        flight->endCity,
        flight->date,
        flight->startTime,
        flight->arriveTime,
        flight->remainSeat,
        flight->totalSeat,
        flight->price,
        flight->status);
    appendText(buffer, bufferSize, line);
}

/*
 * 函数名称：handleQueryFlight
 * 函数功能：处理客户端查询航班请求。
 * 参数说明：request 为请求包，data 中保存查询关键字；response 为响应包。
 * 返回值：查询到结果返回 1，否则返回 0。
 * 实现说明：关键字为空表示查询全部；否则匹配航班号、出发地或目的地。
 */
int handleQueryFlight(const Packet *request, Packet *response)
{
    FlightRecord flights[MAX_FLIGHTS_COUNT];
    int count;
    int i;
    int found = 0;

    count = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    copyText(response->data, MAX_DATA, "");

    for (i = 0; i < count; i++)
    {
        if (request->data[0] == '\0' ||
            strstr(flights[i].flightNo, request->data) != 0 ||
            strstr(flights[i].endCity, request->data) != 0 ||
            strstr(flights[i].startCity, request->data) != 0)
        {
            appendFlightLine(response->data, MAX_DATA, &flights[i]);
            found = 1;
        }
    }

    if (!found)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "未查询到航班");
        return 0;
    }

    return 1;
}

/*
 * 函数名称：handleAddFlight
 * 函数功能：处理管理员新增航班请求。
 * 参数说明：session 为当前会话；request 为新增航班数据；response 为处理结果。
 * 返回值：新增成功返回 1，失败返回 0。
 * 实现说明：先校验管理员权限和数据合法性，再进入临界区读取、追加并保存文件。
 */
int handleAddFlight(Session *session, const Packet *request, Packet *response)
{
    FlightRecord flights[MAX_FLIGHTS_COUNT];
    FlightRecord flight;
    int count;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    memset(&flight, 0, sizeof(flight));
    if (sscanf_s(request->data, "%19[^|]|%49[^|]|%49[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%f|%19[^|\n]",
        flight.flightNo, (unsigned)_countof(flight.flightNo),
        flight.startCity, (unsigned)_countof(flight.startCity),
        flight.endCity, (unsigned)_countof(flight.endCity),
        flight.date, (unsigned)_countof(flight.date),
        flight.startTime, (unsigned)_countof(flight.startTime),
        flight.arriveTime, (unsigned)_countof(flight.arriveTime),
        &flight.totalSeat, &flight.remainSeat, &flight.price,
        flight.status, (unsigned)_countof(flight.status)) != 10)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班数据格式错误");
        return 0;
    }

    if (flight.totalSeat <= 0 || flight.remainSeat < 0 || flight.remainSeat > flight.totalSeat || flight.price < 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班座位或票价不合法");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    /* 进入临界区后再读取和保存，避免多个管理员同时修改航班文件。 */
    count = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    if (count >= MAX_FLIGHTS_COUNT || findFlightIndex(flights, count, flight.flightNo) >= 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, count >= MAX_FLIGHTS_COUNT ? "航班数量已达上限" : "航班号已存在");
        LeaveCriticalSection(&g_dataLock);
        return 0;
    }

    flights[count++] = flight;
    response->result = saveFlightsData(flights, count) ? RESULT_OK : RESULT_ERROR;
    copyText(response->data, MAX_DATA, response->result == RESULT_OK ? "新增航班成功" : "保存航班失败");
    LeaveCriticalSection(&g_dataLock);
    return response->result == RESULT_OK;
}

/*
 * 函数名称：handleDeleteFlight
 * 函数功能：处理管理员删除航班请求。
 * 参数说明：session 为当前会话；request->data 为航班号；response 为处理结果。
 * 返回值：删除成功返回 1，失败返回 0。
 * 实现说明：找到目标航班后，通过数组前移覆盖的方式删除记录。
 */
int handleDeleteFlight(Session *session, const Packet *request, Packet *response)
{
    FlightRecord flights[MAX_FLIGHTS_COUNT];
    int count;
    int index;
    int i;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    count = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    index = findFlightIndex(flights, count, request->data);
    if (index < 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班不存在");
        LeaveCriticalSection(&g_dataLock);
        return 0;
    }

    for (i = index; i < count - 1; i++)
    {
        flights[i] = flights[i + 1];
    }
    count--;

    response->result = saveFlightsData(flights, count) ? RESULT_OK : RESULT_ERROR;
    copyText(response->data, MAX_DATA, response->result == RESULT_OK ? "删除航班成功" : "保存航班失败");
    LeaveCriticalSection(&g_dataLock);
    return response->result == RESULT_OK;
}

/*
 * 函数名称：handleUpdateFlight
 * 函数功能：处理管理员修改航班请求。
 * 参数说明：session 为当前会话；request 为修改后的完整航班数据；response 为处理结果。
 * 返回值：修改成功返回 1，失败返回 0。
 * 实现说明：按航班号定位原记录，再用新记录整体覆盖，最后保存到文件。
 */
int handleUpdateFlight(Session *session, const Packet *request, Packet *response)
{
    FlightRecord flights[MAX_FLIGHTS_COUNT];
    FlightRecord flight;
    int count;
    int index;

    if (!session->isAdmin)
    {
        response->result = RESULT_ADMIN_REQUIRED;
        copyText(response->data, MAX_DATA, "需要管理员权限");
        return 0;
    }

    memset(&flight, 0, sizeof(flight));
    if (sscanf_s(request->data, "%19[^|]|%49[^|]|%49[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%f|%19[^|\n]",
        flight.flightNo, (unsigned)_countof(flight.flightNo),
        flight.startCity, (unsigned)_countof(flight.startCity),
        flight.endCity, (unsigned)_countof(flight.endCity),
        flight.date, (unsigned)_countof(flight.date),
        flight.startTime, (unsigned)_countof(flight.startTime),
        flight.arriveTime, (unsigned)_countof(flight.arriveTime),
        &flight.totalSeat, &flight.remainSeat, &flight.price,
        flight.status, (unsigned)_countof(flight.status)) != 10)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班数据格式错误");
        return 0;
    }

    EnterCriticalSection(&g_dataLock);
    count = loadFlightsData(flights, MAX_FLIGHTS_COUNT);
    index = findFlightIndex(flights, count, flight.flightNo);
    if (index < 0)
    {
        response->result = RESULT_ERROR;
        copyText(response->data, MAX_DATA, "航班不存在");
        LeaveCriticalSection(&g_dataLock);
        return 0;
    }

    flights[index] = flight;
    response->result = saveFlightsData(flights, count) ? RESULT_OK : RESULT_ERROR;
    copyText(response->data, MAX_DATA, response->result == RESULT_OK ? "修改航班成功" : "保存航班失败");
    LeaveCriticalSection(&g_dataLock);
    return response->result == RESULT_OK;
}
