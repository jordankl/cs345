#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "kernel.h"
#include "jurassic_park.h"

// Jurassic Park
JPARK myPark;
int parkMutexSemId;						// protect park access
int jurassicDisplaySemId;
int fillSeatSemId[NUM_CARS];			// (signal) seat ready to fill
int seatFilledSemId[NUM_CARS];			// (wait) passenger seated
int rideOverSemId[NUM_CARS];			// (signal) ride over
int moveCarsSemId;
int parkDoneSemId;
int visitors;

/**
 * jurassicTask - start Jurassic Park
 */
int jurassicTask(int argc, char* argv[]) {
	char buffer[32];
	char* newArgv[2];
	NewTask parkInterface;
	tid_t parkInterfaceTid;
	NewTask createDriverCarVisitor;
	tid_t createDriverCarVisitorTid;


	// Get the number of visitors
	if (argc == 2) visitors = atoi(argv[1]);
	else visitors = NUM_VISITORS;

	// The park interface is limited to always have a visitor count
	// that is a multiple of 3!!
	if (visitors == 0 || visitors % 3 != 0) {
		printf("ERROR: visitors must be a multiple of 3\n");
		return 1;
	}

	initializePark(visitors);
	newArgv[0] = buffer;


	sprintf(newArgv[0], "parkInterfaceTask");
	parkInterface.name = "parkInterface";
	parkInterface.task = parkInterfaceTask;
	parkInterface.priority = MED_PRIORITY;
	parkInterface.argc = 1;
	parkInterface.argv = newArgv;
	parkInterface.parentHandlers = TRUE;
	parkInterface.tgidNew = TRUE;
	parkInterfaceTid = createTask(&parkInterface);

	initializeGlobalVariables();

	sprintf(newArgv[0], "createDriverCarVisitorTasks");
	createDriverCarVisitor.name = "createDriverCarVisitorTasks";
	createDriverCarVisitor.task = createDriverCarVisitorTasks;
	createDriverCarVisitor.priority = MED_PRIORITY;
	createDriverCarVisitor.argc = 1;
	createDriverCarVisitor.argv = newArgv;
	createDriverCarVisitor.parentHandlers = TRUE;
	createDriverCarVisitor.tgidNew = TRUE;
	createDriverCarVisitorTid = createTask(&createDriverCarVisitor);

	while (myPark.numExitedPark < visitors) {
		swapTask();
	}
	// wait for display to end
    semWait(parkDoneSemId);

    swapTask();
    // wait for interface to end
    semWait(parkDoneSemId);
    // wait for lost visitor to end
    semWait(parkDoneSemId);
    sigKill(-createDriverCarVisitorTid, KSIGKILL);
    while ((wait(NULL)) > 0);
    parkCleanup();
    shutdownPark();

	return 0;

} // end

int createDriverCarVisitorTasks(int argc, char* argv[]) {
	int i;
	char* carArgv[2];
	char nameID[32], carID[32], name[32];
	char* driverArgv[2];
	char driverNameID[32], driverID[32], driverName[32];
	char* visitorArgv[2];
	char vistorNameID[32], visitorID[32], visitorName[32];

	// Create driver tasks represented by NUM_DRIVERS
	driverArgv[0] = driverName;
	driverArgv[1] = driverID;
	sprintf(driverArgv[0],"driverTask");
	for (i = 0; i < NUM_DRIVERS; i++) {
		NewTask driverTaskInfo;
		sprintf(driverArgv[1],"%d",i);
		sprintf(driverNameID,"driverTask%d",i);

		driverTaskInfo.name = driverNameID;
		driverTaskInfo.task = driverTask;
		driverTaskInfo.priority = MED_PRIORITY;
		driverTaskInfo.argc = 2;
		driverTaskInfo.argv = driverArgv;
		driverTaskInfo.parentHandlers = TRUE;
		driverTaskInfo.tgidNew = FALSE;
		createTask(&driverTaskInfo);
	}

	// Create car tasks represented by NUM_CARS
	carArgv[0] = name;
	carArgv[1] = carID;
	sprintf(carArgv[0],"carTask");
	for (i = 0; i < NUM_CARS; i++) {
		sprintf(carArgv[1],"%d",i);
		sprintf(nameID,"carTask%d",i);

		NewTask carTaskInfo;
		carTaskInfo.name = nameID;
		carTaskInfo.task = carTask;
		carTaskInfo.priority = MED_PRIORITY;
		carTaskInfo.argc = 2;
		carTaskInfo.argv = carArgv;
		carTaskInfo.parentHandlers = TRUE;
		carTaskInfo.tgidNew = FALSE;
		createTask(&carTaskInfo);
	}

	// Create car tasks represented by NUM_VISITORS of argv[2]
	visitorArgv[0] = visitorName;
	visitorArgv[1] = visitorID;
	sprintf(visitorArgv[0],"visitorTask");
	for (i = 0; i < visitors; i++) {
		sprintf(visitorArgv[1],"%d",i);
		sprintf(vistorNameID,"visitorTask%d",i);

		NewTask visitorTaskInfo;
		visitorTaskInfo.name = vistorNameID;
		visitorTaskInfo.task = visitorTask;
		visitorTaskInfo.priority = MED_PRIORITY;
		visitorTaskInfo.argc = 2;
		visitorTaskInfo.argv = visitorArgv;
		visitorTaskInfo.parentHandlers = TRUE;
		visitorTaskInfo.tgidNew = FALSE;
		createTask(&visitorTaskInfo);
	}

	while(TRUE) {
		swapTask();
	}

	// Shouldn't get here
	return 0;
}

void initializePark(int num_visitors) {
	int i;
	char buf[32];

	// Start park
	printf("Start Jurassic Park...\n");

	visitors = num_visitors;

	// OK, let's get the park going...
	parkMutexSemId = createSemaphore("parkMutex", BINARY, 1);

	// initial car locations
	for (i=0; i < NUM_CARS; i++) {
		myPark.cars[i].location = 33 - i;
		myPark.cars[i].passengers = 0;
	}

	// drivers are all asleep
	for (i=0; i < NUM_DRIVERS; i++) {
		myPark.drivers[i] = 0;
	}

	// initialize other park variables
	myPark.numOutsidePark = 0;								/* # trying to enter park */
	myPark.numTicketsAvailable = MAX_TICKETS;				/* T=# */
	myPark.numInPark = 0;									/* P=# */
	myPark.numRidesTaken = 0;								/* S=# */

	myPark.numInTicketLine = 0;								/* Ticket line */
	myPark.numInMuseumLine = 0;								/* Museum line */
	myPark.numInMuseum = 0;									/* # in museum */
	myPark.numInCarLine = 0;								/* Ride line */
	myPark.numInCars = 0;									/* # in cars */
	myPark.numInGiftLine = 0;								/* Gift shop line */
	myPark.numInGiftShop = 0;								/* # in gift shop */
	myPark.numExitedPark = 0;								/* # exited park */

	// create move care signal semaphore
	moveCarsSemId = createSemaphore("moveCar", BINARY, 0);
	// Create semaphore used for display task to sleep a second
	jurassicDisplaySemId = createSemaphore("jurassicDisplayPark - dc 1 sec", BINARY, 0);

	parkDoneSemId = createSemaphore("parkDone", COUNTING, 0);


	// initialize communication semaphores
	for (i=0; i < NUM_CARS; i++) {
		sprintf(buf, "fillSeat%d", i);
		fillSeatSemId[i] = createSemaphore(buf, BINARY, 0);
		sprintf(buf, "seatFilled%d", i);
		seatFilledSemId[i] = createSemaphore(buf, BINARY, 0);
		sprintf(buf, "rideOver%d", i);
		rideOverSemId[i] = createSemaphore(buf, BINARY, 0);
	}
}

void shutdownPark() {
    // Get rid of non-student-made semaphores
    int i;
    for (i = 0; i < NUM_CARS; i++) {
        deleteSemaphore(fillSeatSemId[i]);
        deleteSemaphore(seatFilledSemId[i]);
        deleteSemaphore(rideOverSemId[i]);
    }
    deleteSemaphore(parkMutexSemId);
    deleteSemaphore(moveCarsSemId);
    deleteSemaphore(jurassicDisplaySemId);
    deleteSemaphore(parkDoneSemId);
	printf("\nJurassic Park is shutting down for the evening!!");
	printf("\nThank you for visiting Jurassic Park!!\n");
}

/**
 * parkInterfaceTask - start Jurassic Park
 */
int parkInterfaceTask(int argc, char* argv[])
{
	int i, j, k;
	int c[NUM_CARS];
	tid_t lostVisitorTid;

	char *newArgv[2];
	char buffer[32];

	// start display park task
	NewTask displayParkInfo;

	// start lost visitor task
	NewTask lostVisitorInfo;

	newArgv[0] = buffer;

	sprintf(newArgv[0], "displayParkTask");
	displayParkInfo.name = "displayPark";
	displayParkInfo.task = jurassicDisplayTask;
	displayParkInfo.priority = MED_PRIORITY;
	displayParkInfo.argc = 1;
	displayParkInfo.argv = newArgv;
	displayParkInfo.parentHandlers = TRUE;
	displayParkInfo.tgidNew = FALSE;
	createTask(&displayParkInfo);

	sprintf(newArgv[0], "lostVisitorTask");
	lostVisitorInfo.name = "lostVisitor";
	lostVisitorInfo.task = lostVisitorTask;
	lostVisitorInfo.priority = MED_PRIORITY;
	lostVisitorInfo.argc = 1;
	lostVisitorInfo.argv = newArgv;
	lostVisitorInfo.parentHandlers = TRUE;
	lostVisitorInfo.tgidNew = FALSE;
	lostVisitorTid = createTask(&lostVisitorInfo);

	// wait to move cars
	do {
		// wait for signal to move cars
		if (PARKDEBUG) printf("jurassicTask - semWait(moveCars)\n");
		semWait(moveCarsSemId);

		// order car locations
		for (i=0; i<NUM_CARS; i++) {
			c[i] = i;
		}

		// sort car positions
		for (i=0; i<(NUM_CARS-1); i++) {
			for (j=i+1; j<NUM_CARS; j++) {
				if (myPark.cars[c[i]].location < myPark.cars[c[j]].location) {
					k = c[i];
					c[i] = c[j];
					c[j] = k;
				}
			}
		}

		// move each car if possible (or load or unload)
		for (i=0; i < NUM_CARS; i++) {
			// move car (-1=location occupied, 0=moved, 1=no move)
			if (makeMove(c[i])) {
				// check if car is loading
				if ((myPark.cars[c[i]].location == 33) &&
					(myPark.cars[c[i]].passengers < NUM_SEATS) &&
				    (myPark.numInCarLine) ) {
					// need a passenger
					if (PARKDEBUG) printf("jurassicTask - semSignal(fillSeat[%d])", c[i]);
					semSignal(fillSeatSemId[c[i]]);						/* seat open */

					if (PARKDEBUG) printf("jurassicTask - semWait(seatFilled[%d])", c[i]);
					semWait(seatFilledSemId[c[i]]);						/* passenger in seat */
					myPark.cars[c[i]].passengers++;
				}

				// check if car is unloading
				if ((myPark.cars[c[i]].location == 30) &&
					(myPark.cars[c[i]].passengers > 0)) {
					// empty each seat until car is empty
					myPark.numRidesTaken++;

					// if car empty, signal ride over
					if (--myPark.cars[c[i]].passengers == 0) {
						if (PARKDEBUG) printf("jurassicTask - semSignal(rideOver[%d])", c[i]);
						semSignal(rideOverSemId[c[i]]);
					}
				}

				// sanity check
				if (	(myPark.cars[0].location == myPark.cars[1].location) ||
						(myPark.cars[0].location == myPark.cars[2].location) ||
						(myPark.cars[0].location == myPark.cars[3].location) ||
						(myPark.cars[1].location == myPark.cars[2].location) ||
						(myPark.cars[1].location == myPark.cars[3].location) ||
						(myPark.cars[2].location == myPark.cars[3].location) ) {
					printf("\nProblem:");
					for (k=0; k<NUM_CARS; k++) printf(" %d", myPark.cars[k].location);
					k=getchar();
				}
			}
		}
	} while (myPark.numExitedPark < visitors);

    semSignal(parkDoneSemId);
	//while(TRUE) {
	//	swapTask();
	//}
	return 0;

} // end

//***********************************************************************
// Move car in Jurassic Park (if possible)
//
//	return -1 if location occupied
//			  0 if move ok
//         1 if no move
//
int makeMove(int car)
{
	int i, j;
	int moved;

	switch (j = myPark.cars[car].location) {
			// at cross roads 1
		case 7:
		{
			j = (rand()%2) ? 24 : 8;
			break;
		}

			// at cross roads 2
		case 20:
		{
			// take shorter route if no one in line
			if (myPark.numInCarLine == 0)
			{
				j = 28;
			}
			else
			{
				j = (rand()%3) ? 28 : 21;
			}
			break;
		}

			// bridge loop
		case 23:
		{
			j = 1;
			break;
		}

			// bridge short route
		case 27:
		{
			j = 20;
			break;
		}

			// exit car
		case 30:
		{
			if (myPark.cars[car].passengers == 0)
			{
				j++;
			}
			break;
		}

			// loading car
		case 33:
		{
			// if there is someone in car and noone is in line, proceed
			//if ((myPark.cars[car].passengers == NUM_SEATS) ||
			//	 ((myPark.numInCarLine == 0) && myPark.cars[car].passengers))

			// if car is full, proceed into park
			if (myPark.cars[car].passengers == NUM_SEATS)
			{
				j = 0;
			}
			break;
		}

			// all other moves
		default:
		{
			j++;
			break;
		}
	}
	// check to see if position is taken
	for (i=0; i<NUM_CARS; i++) {
		if (i != car) {
			if (myPark.cars[i].location == j) {
				return -1;
			}
		}
	}

	// return 0 if car moved
	moved = (myPark.cars[car].location == j);

	// make move
	myPark.cars[car].location = j;
	return moved;
}



// ***********************************************************************
// ***********************************************************************
// jurassic task
// ***********************************************************************
int jurassicDisplayTask(int argc, char* argv[])
{
	// wait for park to get initialized...
	//while (!parkMutexSemId) swapTask();
    JPARK currentPark;

	// Insert sem into delta clock and wait
	insertDeltaClock(10,jurassicDisplaySemId,1);

	// display park every second
	do {

		// Wait until time is up
		semWait(jurassicDisplaySemId);

		// take snapshot of park
		semWait(parkMutexSemId);
		currentPark = myPark;
		semSignal(parkMutexSemId);

		// draw current park
		drawPark(&currentPark);

		// signal for cars to move
		semSignal(moveCarsSemId);

	} while (myPark.numExitedPark < visitors);

    // take snapshot of park
    semWait(parkMutexSemId);
    currentPark = myPark;
    semSignal(parkMutexSemId);

    // draw current park
    drawPark(&currentPark);

	semSignal(parkDoneSemId);

	return 0;
} // end



//***********************************************************************
// Draw Jurassic Park
//          1         2         3         4         5         6         7
//01234567890123456789012345678901234567890123456789012345678901234567890
//                _______________________________________________________	0
//   Entrance    /            ++++++++++++++++++++++++++++++++++++++++++|	1
//              -             +o0o-o1o-o2o-o3o-o4o-o5o-o6o             +|	2
//          ## -             /+   /                       \            +|	3
//            /             o +  o     ************        o           +|	4
//   |********         ##>>33 + 23     *Cntrl Room*        7           +|	5
//   |*Ticket*              o +  o     * A=# D1=# *        o           +|	6
//   |*Booth *             32 +  |     * B=# D2=# *       / \          +|	7
//   |* T=#  * ##           o +  o     * C=# D3=# *      o   o8o-o9o   +|	8
//   |* P=## *             31 + 22     * D=# D4=# *    24           \  +|	9
//   |* S=#  *              o +  o     ************    o             o +|	10
//   |********         ##<<30 +   \                   /             10 +|	11
//   |                      o +    o21-o20-o27-o26-o25               o +|	12
//   |                       \+       /   \                          | +|	13
//   |                        +o29-o28     o                         o +|	14
//   |                        +++++++++++ 19                        11 +|	15
//   |                                  +  o                         o +|	16
//   |              ##     ##           +   \                       /  +|	17
//   |        ******\ /****\ /********  +    o  O\                 o   +|	18
//   |        *         *            *  +   18    \/|||\___       12   +|	19
//    \       *  Gifts  *   Museum   *  +    o     x   x           o   +|	20
//     -      *    #    *     ##     *  +     \                   /    +|	21
//   ## -     *         *            *  +      o17-o16-o15-o14-o13     +|	22
//       \    ************************  ++++++++++++++++++++++++++++++++|	23
//
#define D1Upper	13
#define D1Left	45
#define D1Lower	18
#define D1Right	53

#define D2Upper	4
#define D2Left	53
#define D2Lower	13
#define D2Right	57

void drawPark(JPARK *park)
{   //@DISABLE_SWAPS
	static int direction1 = D1Left;
	static int dy1 = D1Lower;

	static int direction2 = D2Left;
	static int dy2 = D1Lower-9;

	int i, j;
	char svtime[64];						// ascii current time
	char driver[] = {'T', 'z', 'A', 'B', 'C', 'D' };
	char buf[32];
	char pk[25][80];
	char cp[34][3] = {	{2, 29, 0}, {2, 33, 0}, {2, 37, 0}, {2, 41, 0},		/* 0-6 */
		{2, 45, 0}, {2, 49, 0}, {2, 53, 0},
		{4, 57, 1},														/* 7 */
		{8, 59, 0}, {8, 63, 0},										/* 8-9 */
		{10, 67, 1}, {14, 67, 1}, {18, 65, 1},					/* 10-12 */
		{22, 61, 0}, {22, 57, 0}, {22, 53, 0},					/* 13-17 */
		{22, 49, 0}, {22, 45, 0},
		{18, 43, 1}, {14, 41, 1},									/* 18-19 */
		{12, 37, 0}, {12, 33, 0}, {8, 31, 1}, {4, 31, 1},	/* 20-23 */
		{8, 55, 3},														/* 24 */
		{12, 49, 0}, {12, 45, 0}, {12, 41, 0},					/* 25-27 */
		{14, 33, 0}, {14, 29, 0},									/* 28-29 */
		{10, 26, 1}, {8, 26, 1}, {6, 26, 1}, {4, 26, 1},	/* 30-33 */
	};

	strcpy(&pk[0][0],  "                ___Jurassic Park_______________________________________");
	strcpy(&pk[1][0],  "   Entrance    /            ++++++++++++++++++++++++++++++++++++++++++|");
	strcpy(&pk[2][0],  "              -             +---------------------------             +|");
	strcpy(&pk[3][0],  "           # -             /+   /                       \\            +|");
	strcpy(&pk[4][0],  "            /             | +  |     ************        |           +|");
	strcpy(&pk[5][0],  "   |********          # >>| +  |     *Cntrl Room*        |           +|");
	strcpy(&pk[6][0],  "   |*Ticket*              | +  |     * A=# D1=# *        |           +|");
	strcpy(&pk[7][0],  "   |*Booth *              | +  |     * B=# D2=# *       / \\          +|");
	strcpy(&pk[8][0],  "   |* T=#  * #            | +  |     * C=# D3=# *      /   -------   +|");
	strcpy(&pk[9][0],  "   |* P=#  *              | +  |     * D=# D4=# *     /           \\  +|");
	strcpy(&pk[10][0], "   |* S=#  *              | +  |     ************    /             | +|");
	strcpy(&pk[11][0], "   |********            <<| +   \\                   /              | +|");
	strcpy(&pk[12][0], "   |                      | +    -------------------               | +|");
	strcpy(&pk[13][0], "   |                       \\+       /   \\                          | +|");
	strcpy(&pk[14][0], "   |                        +-------     |                         | +|");
	strcpy(&pk[15][0], "   |                        +++++++++++  |                         | +|");
	strcpy(&pk[16][0], "   |                                  +  |                         | +|");
	strcpy(&pk[17][0], "   |                #      #          +   \\                       /  +|");
	strcpy(&pk[18][0], "   |        ******\\ /****\\ /********  +    |                     |   +|");
	strcpy(&pk[19][0], "   |        *         *            *  +    |                     |   +|");
	strcpy(&pk[20][0], "    \\       *  Gifts  *   Museum   *  +    |                     |   +|");
	strcpy(&pk[21][0], "     -      *         *            *  +     \\                   /    +|");
	strcpy(&pk[22][0], "    # -     *         *            *  +      -------------------     +|");
	strcpy(&pk[23][0], "       \\    ************************  ++++++++++++++++++++++++++++++++|");
	strcpy(&pk[24][0], "   Exit \\_____________________________________________________________|");
	//@ENABLE_SWAPS

	// output time
	sprintf(buf, "%s", myTime(svtime));
	memcpy(&pk[0][strlen(pk[0]) - strlen(buf)], buf, strlen(buf));

	// out number waiting to get into park
	sprintf(buf, "%d", park->numOutsidePark);
	memcpy(&pk[3][12 - strlen(buf)], buf, strlen(buf));

	// T=#, out number of tickets available
	sprintf(buf, "%d ", park->numTicketsAvailable);
	memcpy(&pk[8][8], buf, strlen(buf));

	// P=#, out number in park (not to exceed OSHA requirements)
	sprintf(buf, "%d ", park->numInPark);
	memcpy(&pk[9][8], buf, strlen(buf));

	// S=#, output guests completing ride
	sprintf(buf, "%d", park->numRidesTaken);
	memcpy(&pk[10][8], buf, strlen(buf));

	// out number in ticket line
	sprintf(buf, "%d ", park->numInTicketLine);
	memcpy(&pk[8][13], buf, strlen(buf));

	// out number in gift shop line
	sprintf(buf, "%d", park->numInGiftLine);
	memcpy(&pk[17][21 - strlen(buf)], buf, strlen(buf));

	// out number in museum line
	sprintf(buf, "%d", park->numInMuseumLine);
	memcpy(&pk[17][28 - strlen(buf)], buf, strlen(buf));

	// out number in car line
	sprintf(buf, "%d", park->numInCarLine);
	memcpy(&pk[5][23 - strlen(buf)], buf, strlen(buf));

	// out number in gift shop
	sprintf(buf, "%d ", park->numInGiftShop);
	memcpy(&pk[21][17], buf, strlen(buf));

	// out number in museum
	sprintf(buf, "%d ", park->numInMuseum);
	memcpy(&pk[21][29], buf, strlen(buf));

	// out number exited park
	sprintf(buf, "%d", park->numExitedPark);
	memcpy(&pk[22][5 - strlen(buf)], buf, strlen(buf));

	// cars
	for (i=0; i<NUM_CARS; i++)
	{
		sprintf(buf, "%d", park->cars[i].passengers);
		memcpy(&pk[6+i][42 - strlen(buf)], buf, strlen(buf));
	}

	// drivers
	for (i=0; i<NUM_DRIVERS; i++)
	{
		pk[6+i][46] = driver[park->drivers[i] + 1];
	}

	// output cars
	for (i=0; i<NUM_CARS; i++)
	{
		// draw car
		j = park->cars[i].location;
		switch (cp[j][2])
		{
				// horizontal
			case 0:
			{
				pk[(int)cp[j][0]][(int)cp[j][1]+0] = 'o';
				pk[(int)cp[j][0]][(int)cp[j][1]+1] = 'A'+i;
				pk[(int)cp[j][0]][(int)cp[j][1]+2] = 'o';
				break;
			}
				// vertical
			case 1:
			{
				pk[(int)cp[j][0]+0][(int)cp[j][1]] = 'o';

				//pk[cp[j][0]+1][cp[j][1]] = 'A'+i;
				//if ((park->cars[i].passengers > 0) && (park->cars[i].passengers < NUM_SEATS))
				if ((park->cars[i].passengers > 0) &&
					((j == 30) || (j == 33)))
				{
					pk[(int)cp[j][0]+1][(int)cp[j][1]] = '0'+park->cars[i].passengers;
				}
				else pk[(int)cp[j][0]+1][(int)cp[j][1]] = 'A'+i;

				pk[(int)cp[j][0]+2][(int)cp[j][1]] = 'o';
				break;
			}
			case 2:
			{
				pk[cp[j][0]+0][cp[j][1]+0] = 'o';
				pk[cp[j][0]+1][cp[j][1]+1] = 'A'+i;
				pk[cp[j][0]+2][cp[j][1]+2] = 'o';
				break;
			}
			case 3:
			{
				pk[cp[j][0]+0][cp[j][1]-0] = 'o';
				pk[cp[j][0]+1][cp[j][1]-1] = 'A'+i;
				pk[cp[j][0]+2][cp[j][1]-2] = 'o';
				break;
			}
		}
	}

	// move dinosaur #1
	dy1 = dy1 + (rand()%3) - 1;
	if (dy1 < D1Upper) dy1 = D1Upper;
	if (dy1 > D1Lower) dy1 = D1Lower;

	if (direction1 > 0)
	{
		memcpy(&pk[dy1+0][direction1+4], "...  /O", 7);
		memcpy(&pk[dy1+1][direction1+0], "___/|||\\/", 9);
		memcpy(&pk[dy1+2][direction1+3], "x   x", 5);
		if (++direction1 > D1Right) direction1 = -direction1;
	}
	else
	{
		if ((rand()%3) == 1)
		{
			memcpy(&pk[dy1+0][-direction1+4], "...", 3);
			memcpy(&pk[dy1+1][-direction1+1], "__/|||\\___", 10);
			memcpy(&pk[dy1+2][-direction1+0], "O  x   x", 8);
		}
		else
		{
			memcpy(&pk[dy1+0][-direction1+0], "O\\  ...", 7);
			memcpy(&pk[dy1+1][-direction1+2], "\\/|||\\___", 9);
			memcpy(&pk[dy1+2][-direction1+3], "x   x", 5);
		}
		if (++direction1 > -D1Left) direction1 = -direction1;
	}

	// move dinosaur #2

	dy2 = dy2 + (rand()%3) - 1;
	if (dy2 < D2Upper) dy2 = D2Upper;
	if (dy2 > D2Lower) dy2 = D2Lower;
	dy2 = (dy2+9) >= dy1 ? dy1-9 : dy2;

	if (direction2 > 0)
	{
		memcpy(&pk[dy2+0][direction2+7], "_", 1);
		memcpy(&pk[dy2+1][direction2+6], "/o\\", 3);
		memcpy(&pk[dy2+2][direction2+4], "</ _<", 5);
		memcpy(&pk[dy2+3][direction2+3], "</ /", 4);
		memcpy(&pk[dy2+4][direction2+2], "</ ==x", 6);
		memcpy(&pk[dy2+5][direction2+3], "/  \\", 4);
		memcpy(&pk[dy2+6][direction2+2], "//)__)", 6);
		memcpy(&pk[dy2+7][direction2+0], "<<< \\_ \\_", 9);
		if (++direction2 > D2Right) direction2 = -direction2;
	}
	else
	{
		memcpy(&pk[dy2+0][-direction2+1], "_", 1);
		memcpy(&pk[dy2+1][-direction2+0], "/o\\", 3);
		memcpy(&pk[dy2+2][-direction2+0], ">_ \\>", 5);
		memcpy(&pk[dy2+3][-direction2+2], "\\ \\>", 4);
		memcpy(&pk[dy2+4][-direction2+1], "x== \\>", 6);
		memcpy(&pk[dy2+5][-direction2+2], "/  \\", 4);
		memcpy(&pk[dy2+6][-direction2+1], "(__(\\\\", 6);
		memcpy(&pk[dy2+7][-direction2+0], "_/ _/ >>>", 9);
		if (++direction2 > -D2Left) direction2 = -direction2;
	}

	// draw park
	//system("cls");
	printf("\n");
	for (i=0; i<25; i++) printf("\n%s", &pk[i][0]);
	printf("\n");

	// driver in only one place at a time
	for (i=0; i<(NUM_DRIVERS-1); i++)
	{
		if (park->drivers[i] != 0)
		{
			for (j=i+1; j<NUM_DRIVERS; j++)
			{
				assert("Driver Error" && (park->drivers[i] != park->drivers[j]));
			}
		}
	}

	return;
} // end drawPark

char* myTime(char* svtime)
{
	time_t cTime;							/* current time */
	time(&cTime);							/* read current time */
	strcpy(svtime, asctime(localtime(&cTime)));
	svtime[strlen(svtime)-1] = 0;		/* eliminate nl at end */
	return svtime;
}

// ***********************************************************************
// ***********************************************************************
// lostVisitor task
// ***********************************************************************
int lostVisitorTask(int argc, char* argv[])
{
	int inPark;

	while (myPark.numExitedPark < visitors) {

		if (semWait(parkMutexSemId) == SEMNOTFOUND) {
			break;
		}

		// number in park
		inPark = myPark.numInTicketLine +
		myPark.numInMuseumLine +
		myPark.numInMuseum +
		myPark.numInCarLine +
		myPark.numInCars +
		myPark.numInGiftLine +
		myPark.numInGiftShop;

		if (inPark != myPark.numInPark) {
			printf("\nSomeone is lost!!!");
			printf("\nThere are %d visitors in the park,", myPark.numInPark);
			printf("\nbut I can only find %d of them!\n", inPark);

			printf("\n      numInPark=%d", myPark.numInPark);
			printf("\n         inPark=%d", inPark);
			printf("\nnumInTicketLine=%d", myPark.numInTicketLine);
			printf("\nnumInMuseumLine=%d", myPark.numInMuseumLine);
			printf("\n    numInMuseum=%d", myPark.numInMuseum);
			printf("\n   numInCarLine=%d", myPark.numInCarLine);
			printf("\n      numInCars=%d", myPark.numInCars);
			printf("\n  numInGiftLine=%d", myPark.numInGiftLine);
			printf("\n  numInGiftShop=%d", myPark.numInGiftShop);
			printf("\n");

			assert("Too few in Park!" && (inPark == myPark.numInPark));
		}

		semSignal(parkMutexSemId);
		swapTask();
	}

	semSignal(parkDoneSemId);

	return 0;
} // end lostVisitorTask
