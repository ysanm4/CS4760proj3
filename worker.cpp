//Written by Yosef Alqufidi
//Date 3/3/25
//updated from project 1

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>

using namespace std;

//Shared memory clock structure

struct ClockDigi{
	int sysClockS;
	int sysClockNano;
};

//logic for shared memory

int main(int argc, char** argv){
	if(argc !=3){
		cout<<"Error please use two arguments for:"<< argv[0] <<"\n";
		return EXIT_FAILURE;
	}


//start parseing time

int Secval = atoi(argv[1]);
int Nanoval = atoi(argv[2]);

//shared memory key

key_t key = 6321;

//access to shared memory

int shmid = shmget(key, sizeof(ClockDigi), 0666);
if(shmid < 0){
	perror("shmget");
	return EXIT_FAILURE;
}

ClockDigi* clockVal = (ClockDigi*) shmat(shmid, nullptr, 0);
if (clockVal == (void*) -1){
	perror("shmat");
	return EXIT_FAILURE;
}

//start reading from simulated clock 

int stRdSec = clockVal->sysClockS;
int stRdNano = clockVal->sysClockNano;

//termination

int termSec = stRdSec + Secval;
int termNano = stRdNano + Nanoval;

if(termNano >= 1000000000){
	termSec += termNano / 1000000000;
	termNano = termNano % 1000000000;
}

//outputs
//...........................................................................................
cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << clockVal->sysClockS
         << " SysclockNano: " << clockVal->sysClockNano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
         cout << "JUST STARTING" << "\n";

//checks and busy wait
  
	 int lastPrintedSec = stRdSec;
while (true){
	int curr_Sec = clockVal->sysClockS;
	int curr_Nano = clockVal->sysClockNano;


if(curr_Sec > lastPrintedSec && curr_Sec < termSec){
	int contnu = curr_Sec - stRdSec;
    cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << clockVal->sysClockS
         << " SysclockNano: " << clockVal->sysClockNano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
    cout << contnu << " seconds have passed since starting" << "\n";
    lastPrintedSec = curr_Sec;
}

    //checks to term or not
   
if(curr_Sec > termSec || (curr_Sec == termSec && curr_Nano >= termNano)){  
    cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << clockVal->sysClockS
         << " SysclockNano: " << clockVal->sysClockNano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
    cout << "TERMINATING" << "\n";
    break;
	}
}
    
    shmdt(clockVal);

    return EXIT_SUCCESS;
}

