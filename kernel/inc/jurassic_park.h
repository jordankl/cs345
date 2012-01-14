#ifndef JURASSIC_PARK_H
#define JURASSIC_PARK_H

#define NUM_CARS		4
#define NUM_DRIVERS		4
#define NUM_SEATS		3
#define NUM_VISITORS	(NUM_SEATS*15)
#define MAX_IN_PARK		20
#define MAX_TICKETS		(NUM_CARS*NUM_SEATS)
#define MAX_IN_MUSEUM	5
#define MAX_IN_GIFTSHOP	2

#define PARKDEBUG 0
/**
 * struct CAR
 * @location: the location of the car in the park
 * @passengers: the number of passengers in the car
 */
typedef struct car
{
	int location;
	int passengers;
} CAR;

/**
 * struct JPARK
 * @numOutsidePark: the number outside of the park
 * @numInPark: the number inside the park (P)
 * @numTicketsAvailable: the tickets available for sale (T)
 * @numRidesTaken: the number of passengers that have taken rides
 * @numExitedPark: the number of visitors that have exited
 * @numInTicketLine: the number of visitors waiting for tickets
 * @numInMuseumLine: the number of visitors waiting in museum line
 * @numInMuseum: the number of visitors in the museum
 * @numInCarLine: the number of visitors waiting in the car line
 * @numInCars: the number of visitors riding in cars
 * @numInGiftLine: the number of visitors waiting in the gift shop line
 * @numInGiftShop: the number of visitors in the gift shop
 * @drivers: the array of drivers' states (-1=T, 0=z, 1=A, 2=B, etc.)
 * @cars: the array of cars in the park
 */
typedef struct
{
	int numOutsidePark;
	int numInPark;
	int numTicketsAvailable;
	int numRidesTaken;
	int numExitedPark;
	int numInTicketLine;
	int numInMuseumLine;
	int numInMuseum;
	int numInCarLine;
	int numInCars;
	int numInGiftLine;
	int numInGiftShop;
	int drivers[NUM_DRIVERS];
	CAR cars[NUM_CARS];
} JPARK;

// Students implement
void initializePark(int num_visitors);
int driverTask(int argc, char* argv[]);
int visitorTask(int argc, char* argv[]);
int carTask(int argc, char* argv[]);
void parkCleanup();
// end

void shutdownPark();
int jurassicTask(int argc, char* argv[]);
void initializeGlobalVariables();
int parkInterfaceTask(int argc, char* argv[]);
int createDriverCarVisitorTasks(int argc, char* argv[]);
int makeMove(int car);
int jurassicDisplayTask(int argc, char* argv[]);
void drawPark(JPARK *park);
char* myTime(char*);
int lostVisitorTask(int argc, char* argv[]);

#endif
