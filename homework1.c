#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

#define MAX_NUM_PROCESSES 5
#define MAX_PROCESS_NAME 25

#define NAME_INPUT "processes.in"
#define NAME_OUTPUT "processes.out"

typedef enum {
algorithmNull = 0, algorithmFCFS, algorithmSJF, algorithmRR
} algorithm_type;

const char* ALGORITHM_STRINGS[] = { "Debug Test", "First Come First Served", "Shortest Job First (Pre)", "Round Robin" };

typedef struct {
	int id;
	char name[MAX_PROCESS_NAME];
	int arrival;
	int burst;
	int wait;
	int turnaround;
	int isFinished;
	int totalburst;
} process;

typedef struct {
    process *data;
    struct node *last;
    struct node *next;
} node;

typedef struct {
	node *head;
}nodeQueue;

process processList [MAX_NUM_PROCESSES];
//process arrivedProcessList [MAX_NUM_PROCESSES];
nodeQueue queueFirstCome;

int totalProcesses = 0, totalRunTime = 0, currentAlgorithm = 0, currentTimeQuantum = 0;
int currentRunTimeLocation = 0;
int currentIDLocation = 0;

char checkerString [1024];

int algorithmDetection(char* inputstring){
	if(strcmp(inputstring, "fcfs") == 0){
		return 1;
	} else if(strcmp(inputstring, "sjf")==0){
		return 2;
	} else if(strcmp(inputstring, "rr")==0){
		return 3;
	} else {
		printf("Algorithm Detection failed!\n");
		return 0;
	}
}

// Printed Process Actions within Timeline
void processArrives(FILE* fout, int processid){
	if(processList[processid].burst > 0){
		processList[processid].isFinished = 0;
		if(DEBUG==1){printf("Time %d: %s arrived\n", currentRunTimeLocation, processList[processid].name);}
		fprintf(fout, "Time %d: %s arrived\n", currentRunTimeLocation, processList[processid].name);
		processList[processid].turnaround++;
	} else if(processList[processid].burst <= 0){
		processList[processid].isFinished = 1;
		processList[processid].turnaround=0;
		if(DEBUG==1){printf("Time %d: %s died on arrival\n", currentRunTimeLocation, processList[processid].name);}
		fprintf(fout, "Time %d: %s died on arrival\n", currentRunTimeLocation, processList[processid].name);
	}
}
void processFinishes(FILE* fout, int processid){
	if(DEBUG==1){printf("Time %d: %s finished\n", currentRunTimeLocation, processList[processid].name);}
	fprintf(fout, "Time %d: %s finished\n", currentRunTimeLocation, processList[processid].name);
}
void processSelected(FILE* fout, int processid){
	if(DEBUG==1){printf("Time %d: %s selected (burst %d)\n", currentRunTimeLocation, processList[processid].name, processList[processid].burst);}
	fprintf(fout, "Time %d: %s selected (burst %d)\n", currentRunTimeLocation, processList[processid].name, processList[processid].burst);
	//processList[processid].burst--;
}

// Printed Situations in Timeline that are distinct from Processes 
void systemIdle(FILE* fout){
	if(DEBUG==1){printf("Time %d: IDLE\n", currentRunTimeLocation);}
	fprintf(fout, "Time %d: IDLE\n", currentRunTimeLocation);
}
void systemEnd(FILE* fout){
	int i = 0;
	
	if(DEBUG == 1){printf("Finished at time %d\n\n", currentRunTimeLocation);}
	fprintf(fout, "Finished at time %d\n\n", currentRunTimeLocation);

	for(i = 0; i<totalProcesses; i++){
		if(DEBUG==1){printf ("%s wait %d turnaround %d\n", processList[i].name, processList[i].wait, processList[i].turnaround);}
		fprintf (fout, "%s wait %d turnaround %d\n", processList[i].name, processList[i].wait, processList[i].turnaround);
	}
}

// General Timeline Checks
int isArrived(int processid){
	if(processList[processid].arrival == currentRunTimeLocation){
		return 1;
	}
	return 0;
}
int isFinished(int processid){
	if(processList[processid].burst == 0){
		return 1;
	}
	return 0;
}
int isReady(int processid){
	if(processList[processid].arrival <= currentRunTimeLocation){
		if(isFinished(processid)==0){
			return 1;
		}
	}
	return 0;
}

// The Main Algorithm Work
void executeFirstComeFirstServedAlgorithm(FILE* fout){
	
	int i = 0;
	int stagnate = 0;
	int selection = 0, previousSelection = -1;
	int isNoneReady = 1;
	int isAllFinished = 1;

	for(currentRunTimeLocation=0; currentRunTimeLocation < totalRunTime; currentRunTimeLocation++){	
	
		if(previousSelection != -1){
			processList[previousSelection].burst--;
		}
		
		for(i=0; i < totalProcesses; i++){
			
			if(processList[i].arrival == currentRunTimeLocation){
				
				stagnate = 0;
				processArrives(fout, i);
			}
			
			if(processList[i].isFinished == 0){
				
				if(isFinished(i) == 1){
					
					processList[i].isFinished = 1;
					stagnate = 0;
					processFinishes(fout, i);
				}
			}
			
			if(isReady(i) == 1){
				
				stagnate = 0;
			}
		}
		
		selection = isNextArrive();
		
		if(selection != -1){
			
			if(selection == previousSelection){
				
			} 
			
			else {
				processSelected(fout, selection);
				previousSelection = selection;
			}
		}	
		
		isNoneReady = 1;
		isAllFinished = 1;
		
		for(i=0; i < totalProcesses; i++){
			
			if(i != selection && isReady(i)){
				
				processList[i].wait++;
			}
			
			if(isReady(i) == 1){
				
				isNoneReady = 0;
				isAllFinished = 0;
				
				if(processList[i].arrival < currentRunTimeLocation){
					
					processList[i].turnaround++;
				}
			}
			
			if(processList[i].isFinished==0){
				
				isAllFinished = 0;
			}
		}
		if(stagnate == 0 && isNoneReady==1){
			
			systemIdle(fout);
			stagnate = 1;
			
			if(isAllFinished == 1 || currentRunTimeLocation+1 == totalRunTime){
				continue;
			}
		}
	}
	
	systemEnd(fout);

}

int isNextArrive(){
	int i, output = 0;
	
	for(i = 0; i < MAX_NUM_PROCESSES;i++){
		
		if(isReady(i) == 1){
			if(isFinished(output) == 1){
				do{
					output++;
				}while(isFinished(output) == 1);
			}
			if((processList[i].arrival <= processList[output].arrival) && isFinished(output) != 1){
				output = processList[i].id;
			}
		}
	}
	
	if(processList[output].burst <= 0 || isReady(output) == 0){
		output = -1;
	}
	
	return output;
	
}


void executeShortestJobFirstAlgorithm(FILE* fout){
	int i=0;
	int stagnate = 0;
	int selection = 0, previousSelection = -1;
	int isNoneReady = 1;
	int isAllFinished = 1;
	for(currentRunTimeLocation=0; currentRunTimeLocation < totalRunTime; currentRunTimeLocation++){
		if(previousSelection != -1){
			processList[previousSelection].burst--;
		}
		for(i=0; i < totalProcesses; i++){
			if(processList[i].arrival == currentRunTimeLocation){
				stagnate = 0;
				processArrives(fout, i);
			}
			if(processList[i].isFinished == 0){
				if(isFinished(i)==1){
					processList[i].isFinished = 1;
					stagnate = 0;
					processFinishes(fout, i);
				}
			}
			if(isReady(i)==1){
				stagnate = 0;
			}
		}
		selection = isShortestJob();
		if(selection != -1){
			if(selection == previousSelection){
			} else {
				processSelected(fout, selection);
				previousSelection = selection;
			}
		}	
		
		isNoneReady = 1;
		isAllFinished = 1;
		for(i=0; i < totalProcesses; i++){
			if(i != selection && isReady(i)){
				processList[i].wait++;
			}
			if(isReady(i)==1){
				isNoneReady = 0;
				isAllFinished = 0;
				if(processList[i].arrival < currentRunTimeLocation){
					processList[i].turnaround++;
				}
			}
			if(processList[i].isFinished==0){
				isAllFinished = 0;
			}
		}
		if(stagnate == 0 && isNoneReady==1){
			systemIdle(fout);
			stagnate = 1;
			if(isAllFinished == 1 || currentRunTimeLocation+1 == totalRunTime){
				//systemEnd(fout);
				continue;
			}
		}
	}
	systemEnd(fout);
}

void executeRoundRobinAlgorithm(FILE* fout){
	int i = 0;
	int j = 0;
	int isNoneReady = 1;
	int isAllFinished = 1;
	int running = 1;
	currentRunTimeLocation = 0;
	
	// Init total burst variable (do this to help calculate wait / tat. Burst is manipulated so its good to keep it a seperate variable)
	for(i = 0; i < totalProcesses; i++)
	{
		processList[i].totalburst = 0;
		processList[i].totalburst = processList[i].burst;
	}

	for(j = 0; j < totalRunTime; j++)
	{
		// Go through all the processes one by one
		for(i = 0; i < totalProcesses; i++)
		{
			// When a process arrives, print it to the file
			if(processList[i].arrival == currentRunTimeLocation)
			{
				processArrives(fout, i);
			}

			// Round-Robin selection done
			if(processList[i].burst > 0 && isReady(i) == 1)
			{
				if(processList[i].burst > currentTimeQuantum)
				{
					processSelected(fout, i);
					currentRunTimeLocation += currentTimeQuantum;
					processList[i].burst -= currentTimeQuantum;
				}
				else
				{
					processSelected(fout, i);
					currentRunTimeLocation += processList[i].burst;
					processList[i].wait = currentRunTimeLocation - (processList[i].arrival + processList[i].totalburst);
					processList[i].burst = 0;
					processList[i].isFinished = 1;
					processFinishes(fout, i);
				}
				
			}
		}

		// Decide whether the program is in idle or not
		isNoneReady = 1;
		isAllFinished = 1;
		for(i=0; i < totalProcesses; i++){
			if(isReady(i)==1)
			{
				isNoneReady = 0;
				isAllFinished = 0;
			}
			if(processList[i].isFinished==0){
				isAllFinished = 0;
			}
		}
		if(isNoneReady == 1){
			systemIdle(fout);
			if(currentRunTimeLocation + 1 == totalRunTime){	
				currentRunTimeLocation++;	
				break;
			}
		}

	}
	
	// Calculate TAT
	for(i = 0; i < totalProcesses; i++)
		processList[i].turnaround = processList[i].wait + processList[i].totalburst;
	
	// Print final values to file
	systemEnd(fout);
}

// First Come First Serve Specific Functions
/*
int addToFirstComeQueue(){
	int output = 0;
	int i = 0;
	for(i = 0; i < MAX_NUM_PROCESSES; i++){
		if(isArrivedNow(i)){
			addToQueue(processList[i]);
		}
	}
	return output;
}
int getFromFirstComeQueue(){
	return queueFirstCome.head;
}
*/

void algorithmExecution(FILE* fout, int algorithm){
	switch(algorithm){
		case 0:
			printf("Algorithm Execution failed! currentAlgorithm variable must not have been set properly!\n");
			exit(1);
			break;
		case 1:
			//printf("First Come First Served Algorithm Detected!\n");
			executeFirstComeFirstServedAlgorithm(fout);
			break;
		case 2:
			//printf("Shortest Job First Algorithm Detected!\n");
			executeShortestJobFirstAlgorithm(fout);
			break;
		case 3:
			//printf("Round Robin Algorithm Detected!\n");
			executeRoundRobinAlgorithm(fout);
			break;
		default:
			printf("Algorithm Execution failed!\n");
			exit(1);
			break;
	}
}

// Shortest Job First Specific Functions
int isShortestJob(){
	int output = 0;
	int i = 0;
	for(i = 0; i < MAX_NUM_PROCESSES; i++){
		if(isReady(i) == 1){
			if((processList[i].burst < processList[output].burst || isReady(output)!=1)){
				if(processList[i].burst > 0){
					output = processList[i].id;
				}
			}
		}
	}
	if(processList[output].burst == 0 || isReady(output) == 0){
		output = -1;
	}
	return output;
}

// Round Robin Specific Functions
int isNextRobin(){
	// Move to Next in Queue
	// Return that Next Process
}

int main(int argc, char *argv[]) {
	//Parameters: processes.in processes.out
	FILE* fin = fopen(argv[1], "r");
	FILE* fout = fopen(argv[2],"wb");
	int i = 0;	
	if (fin == NULL) {
        printf("Valid input file not found. Place \"processes.in\" by \"homework1.exe\"\n");
        exit(1);
    }
	
	// Scanning Process Total
	fscanf(fin, "%s", checkerString);
	
	if(strcmp(checkerString, "processcount")!=0){
		printf("Input Error: expected \"processcount\" in input\n");
		fclose(fin);
		exit(1);
	}	
	fscanf(fin, "%d", &totalProcesses);
	if(totalProcesses > MAX_NUM_PROCESSES || totalProcesses < 0){
		printf("Input Error: expect either less processes or a non-negative amount of processes\n");
		fclose(fin);
		exit(1);
	}
	// Scanning Run Time Total
	fscanf(fin, "%s", checkerString);
	if(strcmp(checkerString, "#")==0){
		fscanf(fin, "%*[^\n]");
		fscanf(fin, "%s", checkerString);
	}
	if(strcmp(checkerString, "runfor")!=0){
		printf("Input Error: expected \"runfor\" in input\n");
		fclose(fin);
		exit(1);
	}
	fscanf(fin, "%d", &totalRunTime);
		
	// Scanning Expected Algorithm
	fscanf(fin, "%s", checkerString);
	if(strcmp(checkerString, "#")==0){
		fscanf(fin, "%*[^\n]");
		fscanf(fin, "%s", checkerString);
	}
	if(strcmp(checkerString, "use")!= 0){
		printf("Input Error: expected \"use\" in input\n");
		fclose(fin);
		exit(1);
	}
	fscanf(fin, "%s", checkerString);
	currentAlgorithm = algorithmDetection(checkerString);
	if(currentAlgorithm == 0){
		printf("Input Error: expected \"fcfs\",\"sjf\",or \"rr\" after \"use\"\n");
		fclose(fin);
		exit(1);
	}
		
	// Scanning Time Quantum
	fscanf(fin, "%s", checkerString);
	if(strcmp(checkerString, "#")==0){
		fscanf(fin, "%*[^\n]");
		fscanf(fin, "%s", checkerString);
	}
	if(strcmp(checkerString, "#")==0){
		fscanf(fin, "%*[^\n]");
	} else if(strcmp(checkerString, "quantum")==0){
		fscanf(fin, "%d", &currentTimeQuantum);	
		fscanf(fin, "%s", checkerString);
		if(strcmp(checkerString, "#")==0){
			fscanf(fin, "%*[^\n]");
		}
	} else {
		if(strcmp(checkerString, "quantum")!=0){
			printf("Input Error: expected \"quantum\" in input\n");
			fclose(fin);
			exit(1);
		}
	}
	
	// Scanning Processes
	for(i = 0; i < totalProcesses; i++){
		process newProcess;
		
		// Scanning Process Name
		fscanf(fin, "%s", checkerString);
		if(strcmp(checkerString, "process") != 0){
			printf("Input Error: expected \"process\" in input\n");
			fclose(fin);
			exit(1);
		}
		fscanf(fin, "%s", checkerString);
		if(strcmp(checkerString, "name") != 0){
			printf("Input Error: expected \"name\" in input\n");
			fclose(fin);
			exit(1);
		}
		fscanf(fin, "%s", newProcess.name);
		
		// Scanning Process Arrival
		fscanf(fin, "%s", checkerString);
		if(strcmp(checkerString, "arrival") != 0){
			printf("Input Error: expected \"arrival\" in input\n");
			fclose(fin);
			exit(1);
		}
		fscanf(fin, "%d", &newProcess.arrival);
		
		// Scanning Process Burst Time
		fscanf(fin, "%s", checkerString);
		if(strcmp(checkerString, "burst")!=0){
			printf("Input Error: expected \"burst\" in input\n");
			fclose(fin);
			exit(1);
		}
		fscanf(fin, "%d", &newProcess.burst);
		newProcess.id = currentIDLocation;
		currentIDLocation++;
		newProcess.wait = 0;
		newProcess.turnaround = 0;
		newProcess.isFinished = 0;
		processList[i] = newProcess;
	}
	//
	
	// Scanning end marker
	fscanf(fin, "%s", checkerString);
	if(strcmp(checkerString, "end") != 0){
		printf("Input Error: \"end\" expected at end of input file\n");
		fclose(fin);
		//exit(1);
	}
	
	// Debug Logging of Scanned Variables
	if(DEBUG == 1){
		printf("Total Processes: %d\nTotal Runtime: %d\nAlgorithm: %d\nTime Quantum: %d\n", totalProcesses, totalRunTime, currentAlgorithm, currentTimeQuantum);
		for(i = 0; i < totalProcesses; i++){
			printf(" ID: %d Name: %s Arrival: %d Burst: %d\n", processList[i].id, processList[i].name, processList[i].arrival, processList[i].burst);
		}
		printf("\n");
	}
	fclose(fin);
	
	// Beginning Output Creation
	if(DEBUG == 1){printf("%d processes\n", totalProcesses);}
	fprintf(fout, "%d processes\n", totalProcesses);
	if(DEBUG == 1){printf("Using %s\n", ALGORITHM_STRINGS[currentAlgorithm]);}
	fprintf(fout, "Using %s\n", ALGORITHM_STRINGS[currentAlgorithm]);
	if(currentAlgorithm == algorithmRR){
		if(DEBUG == 1){printf("Quantum %d\n\n", currentTimeQuantum);}
		fprintf(fout, "Quantum %d\n\n", currentTimeQuantum);
	} else {
		if(DEBUG == 1){printf("\n");}
		fprintf(fout, "\n");
	}
	algorithmExecution(fout, currentAlgorithm);
	fclose(fout);
	
	if(DEBUG == 1){
		printf("Scheduling algorithm simulation completed!\n");
		system("PAUSE");
	}
    return 0;
}
