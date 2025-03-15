//Written by Yosef Alqufidi
//Date 3/18/25
//updated from project 2


#include <iostream>
#include <fstream>
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
#include <sys/msg.h>

using namespace std;

//PCB tracking children

#define PROCESS_TABLE 20

struct PCB{
	int occupied;
	pid_t pid;
	int startSeconds;
	int startNano;
	int messagesSent;
};

// clock struct
struct ClockDigi{
	int sysClockS;
	int sysClockNano;
};

// message struct
struct Message{
	long mtype;
	int data;
};

PCB processTable[PROCESS_TABLE];

int shmid;
ClockDigi* clockVal = nullptr;
int msgid;

ofstream logFile;

void printProcessTable(){
	cout<<"Process Table:\n";
	cout<<"Entry\tOccupied\tPID\tStartS\tStartN\tMessagesSent\n";
	logFile<<"Process Table:\n";
	logFile << "Entry\tOccupied\tPID\tStartS\tStartN\tMessagesSent\n";
    for (int i = 0; i < PROCESS_TABLE; i++) {
        cout << i << "\t" << processTable[i].occupied << "\t\t"
             << processTable[i].pid << "\t"
             << processTable[i].startSeconds << "\t"
             << processTable[i].startNano << "\t"
             << processTable[i].messagesSent << "\n";
        logFile << i << "\t" << processTable[i].occupied << "\t\t"
                << processTable[i].pid << "\t"
                << processTable[i].startSeconds << "\t"
                << processTable[i].startNano << "\t"
                << processTable[i].messagesSent << "\n";
    }
    cout.flush();
    logFile.flush();
}

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
msgctl(msgid, IPC_RMID, NULL);
if(logFile.is_open()){
	logFile.close();
}
    exit(1);
}


//adding command line args for parse
int main( int argc, char *argv[]){
	
	int n_case = 0;
	int s_case = 0;
	int t_case = 0;
	int i_case = 0;
	string logFileName;
	bool n_var = false, s_var = false, t_var = false, i_var = false, f_var = false;
	int opt;

//setting up the parameters for h,n,s,t,i
	while ((opt = getopt(argc, argv, "hn:s:t:i:f: ")) != -1) {
		switch (opt){
			case 'h':
			cout<< "This is the help menu\n";
			cout<< "-h: shows help\n";
			cout<< "-n: processes\n";
			cout<< "-s: simultaneous\n";
			cout<< "-t: iterations\n";
			cout<< "-f: logfile\n";
			cout<< "To run try ./oss -n 1 -s 1 -t 1 -i 100 -f\n";

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
			case 'f':
                		logFileName = optarg;
                		f_var = true;
                		break;

			default:
				cout<< "invalid";

			return EXIT_FAILURE;
		}
	}

//only allow all three to be used together and not by itself 	
	if(!n_var || !s_var || !t_var || !i_var || !f_var){
		cout<<"ERROR: You cannot do one alone please do -n, -s, -t,-i and -f together.\n";
			return EXIT_FAILURE;
	}

	logFile.open(logFileName.c_str());
	if(!logFile){
		cout<<"error could not open" << logFileName << "\n";
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
	processTable[i].messagesSent = 0;
}

//message queue V system

key_t msgKey = 6321;

msgid = msgget(msgKey, IPC_CREAT | 0666);
	if(msgid < 0){
		perror("msgget");
		return EXIT_FAILURE;
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
//meassages sent
int totalMessagesSent = 0;
//tracking final printing 
long long lastPrintTime = 0;
long long lastLaunchedTime = 0;

//clock simulation -----------------------------------------------------------------------------------------------------------------
long long launInterval = (long long)i_case * 10000000LL;

int next MsgIndex = 0;

while(laun < n_case || runn > 0){
	long long increment;
	if(runn > 0){
		increment = 250000000LL / runn;
	}else{
		increment = 250000000LL;
	}
	long long totalNano = (long long)clockVal->sysClockS * 1000000000LL +
		clockVal->sysClockNano + increment;
	clockVal->sysClockS = totalNano / 1000000000LL;
	clockVal->sysClockNano = totalNano % 1000000000LL;

	long long currentTime = (long long)clockVal->sysClockS * 1000000000 +
		clockVal->sysClockNano;
	if(currentTime - lastPrintTime >= 500000000LL){
		cout<<"OSS PID:" << getpid() <<"SysClock:"
			<< clockVal->sysClockS <<":"<< clockVal->sysClockNano <<"\n";
		logFile <<"OSS PID:" <<getpid() <<"SysClock:"
			<< clockVal->sysClock <<":"<< clockVal->sysClockNano << "\n";
		printProcessTable();
		lastPrintTime = currentTime;
	}

	int startIndex = nextMsgIndex;
	bool found = false;
	int activeIndex = -1;
	for(int i = 0; i < PROCESS_TABLE; i++){
		int idx = (startIndex + i) % PROCESS_TABLE;
		if (processTable[idx].occupied){
			activeIndex = idx;
			found = true;
			break;
		}
	}
	if(found){
		Message msg;
		msg.mtype = processTable[activeIndex].pid;
		msg.data = 1;
		if(msgsnd(msgid, &msg, sizeof(msg.data), 0) == -1){
			perror("msgsnd");
			processTable[activeIndex].messagesSent++;
			totalMessagesSent++
		cout<< "OSS: sending msg to worker" << activeIndex
		    << "PID" << processTable[activeIndex].pid
		    << " at time" << clockVal->sysClockS << ":" << clockVal-sysClockNano << "\n";
    logFilse << "OSS: Sending messages to worker" << activeIndex
             << " PID " << processTable[activeIndex].pid
 	     << " at time " << clockVal->sysClockS << ":" << clockVal->sysClockNano << "\n";
		}

	Message reply;////////////////////////H//////////H

	//clockVal->sysClockNano += 10000000;
	//if(clockVal->sysClockNano >= 1000000000){
	//	clockVal->sysClockS += clockVal->sysClockNano / 1000000000;
	//	clockVal->sysClockNano %= 1000000000;
	//}

	//long long lastPrintTotal = (long long)Finprsec * 1000000000LL + Finprnano;
	//long long currentTotal = (long long)clockVal->sysClockS * 1000000000LL + clockVal->sysClockNano;
	//if(currentTotal - lastPrintTotal >= 500000000){
	//	cout<<"OSS PID: " << getpid() <<"SysClockS: " << clockVal->sysClockS <<"SysClockNano: " << clockVal->sysClockNano << "\n";


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

