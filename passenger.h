#ifndef PASSENGER_H
#define PASSENGER_H

#define MAX_PASSENGERS 3000

typedef struct
{
    char name[100];
    char phone[30];
    char flightNo[20];

    int ticketNum;

} Passenger;

void showPassengers(void);
void bookTicket(void);
void refundTicket(void);

#endif
