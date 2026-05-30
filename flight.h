#ifndef FLIGHT_H
#define FLIGHT_H

#define MAX_FLIGHTS 1000

typedef struct
{
    char flightNo[20];
    char startCity[50];
    char endCity[50];
    char date[20];
    char startTime[20];
    char arriveTime[20];

    int totalSeat;
    int remainSeat;

} Flight;

void showAllFlights(void);
void searchFlightByNo(void);
void searchFlightByDestination(void);
void addFlight(void);
void deleteFlight(void);
void modifyFlight(void);

int findFlightIndexByNo(Flight flights[], int count, const char *flightNo);
void printFlightHeader(void);
void printFlight(const Flight *flight);

#endif
