//Written by Yosef Alqufidi
//Date 3/3/25
//updated from project 1


#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <csignal>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <ctime>
#include <string>

using namespace std;

//PCB tracking children

#define PROCESS_TABLE 20

struct PCB{
	int occupied;
	pid_t pid;
	int startSeconds;
	int startNano;
};

struct ClockDigi{
	int sysClockS;
	int sysClockNano;
};

PCB processTable[PROCESS_TABLE];

int shmid;
ClockDigi* clockVal = nullptr;


void signal_handler(int sig) {
    // code to send kill signal to all children based on their PIDs in process table
for(int i = 0; i < PROCESS_TABLE; i++){
	if(processTable[i].occupied){
		kill(processTable[i].pid, SIGKILL);
	}
}
    // code to free up shared memory
if(clockVal != nullptr){
	shmdt(clockVal);
}
shmctl(shmid, IPC_RMID, NULL);
    exit(1);
}


//adding command line args for parse
int main( int argc, char *argv[]){
	
	int n_case = 0;
	int s_case = 0;
	int t_case = 0;
	int i_case = 0;
	bool n_var = false, s_var = false, t_var = false, i_var = false;
	int opt;

//setting up the parameters for h,n,s,t,i
	while ((opt = getopt(argc, argv, "hn:s:t:i: ")) != -1) {
		switch (opt){
			case 'h':
			cout<< "This is the help menu\n";
			cout<< "-h: shows help\n";
			cout<< "-n: processes\n";
			cout<< "-s: simultaneous\n";
			cout<< "-t: iterations\n";
			cout<< "To run try ./oss -n 1 -s 1 -t 1 -i 100\n";

	return EXIT_SUCCESS;

			case 'n':
				n_case = atoi(optarg);
				n_var = true;
				break;

			case 's':
				s_case = atoi(optarg);
				s_var = true;
				break;

			case 't':
				t_case = atoi(optarg);
				t_var = true;
				break;

			case 'i':
				i_case = atoi(optarg);
				i_var = true;
				break;	

			default:
				cout<< "invalid";

			return EXIT_FAILURE;
		}
	}

//only allow all three to be used together and not by itself 	
	if(!n_var || !s_var || !t_var || !i_var){
		cout<<"ERROR: You cannot do one alone please do -n, -s, -t, and -i together.\n";
			return EXIT_FAILURE;
	}

//shared memory for clock using key to verify

key_t key = 6321;

shmid = shmget(key, sizeof(ClockDigi), IPC_CREAT | 0666);
	if(shmid < 0){
		perror("shmget");
		return EXIT_FAILURE;
}

clockVal = (ClockDigi*) shmat(shmid, nullptr, 0);
	if(clockVal ==(void*) -1){
		perror("shmat");
		return EXIT_FAILURE;
	}
//lets initialize the clock

clockVal->sysClockS = 0;
clockVal->sysClockNano = 0;

//lets now initialize the process table

for(int i = 0; i < PROCESS_TABLE; i++){
	processTable[i].occupied = 0;
	processTable[i].pid = 0;
	processTable[i].startSeconds = 0;
	processTable[i].startNano = 0;
}

//signal and alarm to terminate after 60 real life seconds

signal(SIGALRM, signal_handler);
signal(SIGINT, signal_handler);
alarm(60);
srand(time(NULL));

//launched
int laun = 0;
//running
int runn = 0;

//tracking final printing 
int Finprsec = clockVal->sysClockS;
int Finprnano = clockVal->sysClockNano;

//trackinh final launched
int FinlaunSec = clockVal->sysClockS;
int FinlaunNano = clockVal->sysClockNano;

//clock simulation -----------------------------------------------------------------------------------------------------------------
int launIntN = i_case * 10000000;

while(laun < n_case || runn > 0){
	clockVal->sysClockNano += 10000000;
	if(clockVal->sysClockNano >= 1000000000){
		clockVal->sysClockS += clockVal->sysClockNano / 1000000000;
		clockVal->sysClockNano %= 1000000000;
	}

	long long lastPrintTotal = (long long)Finprsec * 1000000000LL + Finprnano;
	long long currentTotal = (long long)clockVal->sysClockS * 1000000000LL + clockVal->sysClockNano;
	if(currentTotal - lastPrintTotal >= 500000000){
		cout<<"OSS PID: " << getpid() <<"SysClockS: " << clockVal->sysClockS <<"SysClockNano: " << clockVal->sysClockNano << "\n";


		cout<<"Process Table:------------------------------------------------------------------------------------------\n";
		cout<<"Entry\tOccupied\tPID\tStartS\tStartN\n";
		for(int i = 0; i < PROCESS_TABLE; i++){
		cout<< i << "\t" << processTable[i].occupied
		<< "\t\t" << processTable[i].pid
		<< "\t" << processTable[i].startSeconds
		<< "\t" << processTable[i].startNano << "\n";
		}
		Finprsec = clockVal->sysClockS;
		Finprnano = clockVal->sysClockNano;
	}
//-----------------------------------------------------------------------------------------------------------------------------------
//check if children terminated with a non-blocking wait	
	int status;
	pid_t finished;
	while((finished = waitpid(-1, &status, WNOHANG)) > 0){
		for(int i = 0; i < PROCESS_TABLE; i++){
			if(processTable[i].occupied && processTable[i].pid == finished){
				processTable[i].occupied = 0;
				processTable[i].pid = 0;
				processTable[i].startSeconds = 0;
				processTable[i].startNano = 0;
				runn--;
				break;
			}
		}
	}

	long long lastLaunchTotal = 
		(long long)FinlaunSec * 1000000000LL + FinlaunNano;
	if((currentTotal - lastLaunchTotal >= launIntN) &&
			(laun < n_case) && (runn < s_case)){
		int childOffsetSec = (rand() % t_case) + 1;
		int childOffsetNano = rand() % 1000000000;

//fork and exec the worker		
		pid_t pid = fork();
		if(pid < 0){
			cout<<"fork failed\n";
			signal_handler(SIGALRM);
		}else if 
			(pid == 0){
			string secStr = to_string(childOffsetSec);
			string nanoStr = to_string(childOffsetNano);
			execlp("./worker", "worker", secStr.c_str(), nanoStr.c_str(), (char*)NULL);
			cout<<"exec failed\n";
			exit(EXIT_FAILURE);
		}else{
	
//update the process table			
			for(int i = 0; i < PROCESS_TABLE; i++){
				if(!processTable[i].occupied){
					processTable[i].occupied = 1;
					processTable[i].pid = pid;
					processTable[i].startSeconds = clockVal->sysClockS;
					processTable[i].startNano = clockVal->sysClockNano;
					break;
				}
			}
			laun++;
			runn++;
			FinlaunSec = clockVal->sysClockS;
			FinlaunNano = clockVal->sysClockNano;

		}
	}
}

shmdt(clockVal);
shmctl(shmid, IPC_RMID, NULL);

return EXIT_SUCCESS;

}

