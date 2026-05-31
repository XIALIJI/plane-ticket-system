#include "file.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int inputEnded = 0;

void safeCopy(char *dest, size_t destSize, const char *src)
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

static void trimNewline(char *text)
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

void readString(const char *prompt, char *buffer, size_t bufferSize)
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

        printf("输入不能为空，请重新输入。\n");
    }
}

int readInt(const char *prompt)
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

        printf("请输入合法整数。\n");
    }
}

float readFloat(const char *prompt)
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

        printf("请输入合法数字。\n");
    }
}

void waitEnter(void)
{
    char buffer[8];
    printf("按回车键继续...");
    (void)fgets(buffer, sizeof(buffer), stdin);
}

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

int loadFlights(Flight flights[], int maxCount)
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
        printf("提示：未找到 %s，将按空航班列表处理。\n", FLIGHT_FILE);
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

int saveFlights(const Flight flights[], int count)
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
        printf("错误：无法打开 %s 写入。\n", FLIGHT_FILE);
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

int loadPassengers(Passenger passengers[], int maxCount)
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
        printf("提示：未找到 %s，将按空乘客列表处理。\n", PASSENGER_FILE);
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

int savePassengers(const Passenger passengers[], int count)
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
        printf("错误：无法打开 %s 写入。\n", PASSENGER_FILE);
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
        printf("错误：无法打开 %s 追加写入。\n", PASSENGER_FILE);
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
