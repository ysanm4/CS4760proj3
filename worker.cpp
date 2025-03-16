//Written by Yosef Alqufidi
//Date 3/18/25
//updated from project 2

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstdio>
#include <cstring>

using namespace std;

//Shared memory clock structure

struct ClockDigi{
	int sysClockS;
	int sysClockNano;
};

//message struct
struct Message{
	long mtype;
	int data;
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

key_t shmKey= 6321;

//access to shared memory

int shmid = shmget(shmKey, sizeof(ClockDigi), 0666);
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

int startSec = clockVal->sysClockS;
int startNano = clockVal->sysClockNano;

//termination

int termSec = startSec + Secval;
int termNano = startNano + Nanoval;

if(termNano >= 1000000000){
	termSec += termNano / 1000000000;
	termNano = termNano % 1000000000;
}

//outputs
//...........................................................................................

key_t msgKey = 6321;
int msgid = msgget(msgKey, 0666);
if(msgid < 0){
	perror("msgget");
	shmdt(clockVal);
	return EXIT_FAILURE;
}

cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << clockVal->sysClockS
         << " SysclockNano: " << clockVal->sysClockNano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
         cout << "JUST STARTING" << "\n";
	
	int iteration = 0;
	Message msg;

//checks and busy wait
  
	 
while (true){

	if(msgrcv(msgid, &msg, sizeof(msg.data), getpid(),0) == -1){
		perror("msgrcv");
		break;
	}

	iteration++;

	int curr_Sec = clockVal->sysClockS;
	int curr_Nano = clockVal->sysClockNano;

    cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << curr_Sec
         << " SysclockNano: " << curr_Nano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
    cout << "--" << iteration << "iteration"
	 << (iteration == 1 ? "has" : "s have")
	 << "passed since starting\n";

   Message reply;
   reply.mtype = getpid(); 
  

    //checks to term or not
   
if(curr_Sec > termSec || (curr_Sec == termSec && curr_Nano >= termNano)){  
    cout << "WORKER PID: " << getpid()
         << " PPID: " << getppid()
         << " SysClockS: " << curr_Sec
         << " SysclockNano: " << curr_Nano
         << " TermTimeS: " << termSec
         << " TermTimeNano: " << termNano << "\n";
    cout << "--Terminating after sending message back to oss" 
	    << iteration << "iterations.\n";
    reply.data = 0;
    if(msgsnd(msgid, &reply, sizeof(reply.data), 0) == -1){
	    perror("msgsnd");
    }
    break;
}else{
	reply.data = 1;
	if(msgsnd(msgid, &reply, sizeof(reply.data), 0) == -1){
		perror("msgsnd");
	}
    }
}
    
    shmdt(clockVal);

    return EXIT_SUCCESS;
}

