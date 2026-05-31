#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "flight.h"
#include "passenger.h"

#define FLIGHT_FILE "flight.txt"
#define PASSENGER_FILE "passenger.txt"
#define WAITLIST_FILE "waitlist.txt"
#define USER_FILE "user.txt"

void safeCopy(char *dest, size_t destSize, const char *src);
void readString(const char *prompt, char *buffer, size_t bufferSize);
int readInt(const char *prompt);
float readFloat(const char *prompt);
void waitEnter(void);

int loadFlights(Flight flights[], int maxCount);
int saveFlights(const Flight flights[], int count);

int loadPassengers(Passenger passengers[], int maxCount);
int savePassengers(const Passenger passengers[], int count);
int appendPassenger(const Passenger *passenger);

#endif
