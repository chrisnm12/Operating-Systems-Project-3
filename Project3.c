#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Lock
pthread_mutex_t cpu_mutex;
pthread_mutex_t memory_mutex;
pthread_mutex_t scheduling_mutex;
pthread_mutex_t interrupts_mutex;

// the global variables and registers.
int PC = 0; //Program Counter
int ACC = 0; //Accumulator
int IR = 0; //Instruction register
int memory[256]; //memory (instructions + data)
// the opcode and operand
int opcode;
int operand;

// instruction types
#define ADD_operation 1
#define SUB_operation 2
#define LOAD_operation 3
#define STORE_operation 4
#define MUL_operation 5
#define DIV_operation 6
#define AND_operation 7
#define OR_operation 8
#define JMP_operation 9
#define JZ_operation 10

// flags
int interruptFlag = 0; // flag to signal the interrupt
int carryFlag;
int zeroFlag;
int overFlowFlag;

// Memory System
#define RAM_SIZE 1024 // Main memory (RAM)
#define L1_SIZE 64 // L1 Cache
#define L2_SIZE 128 // L2 Cache
int RAM[RAM_SIZE];
int L1Cache[L1_SIZE];
int L2Cache[L2_SIZE];
int L1Tags[L1_SIZE];
int L2Tags[L2_SIZE];
int MEMORY_TABLE_SIZE = RAM_SIZE / 100;

// Process Variables and Structs (M3)
#define MAX_PROCESSES 3
#define TIME_SLICE 5
#define MAX_MESSAGES 10
int currentProcess = 0;

struct MessageQueue {
	int messages[MAX_MESSAGES];
	int data[MAX_MESSAGES];
	int size;
};

struct PCB {
	int pid;
	int pc;
	int acc;
	int state; // Process state (0 = ready, 1 = running, 3 = terminated)
	int time; // Process time
	int priority; // Process Priority Level (Higher number = higher priority)
	struct MessageQueue messagesQueue; // Message Queue from IPC
};

struct PCB processTable[MAX_PROCESSES];

// Messages List
#define START 1
#define NEW_PC 2
#define NEW_ACC 3

// Interrupts (M4)
void (*IVT[3])(); // Interrupt Vector Table

bool complete_processes() {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processTable[i].state != 3) {
			return false;
		}
	}
	return true;
}

void initProcesses() {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		processTable[i].pid = 100 + (i * 100);
		processTable[i].pc = 0;
		processTable[i].acc = 0;
		processTable[i].state = 0;
		processTable[i].time = (rand() % 24) + 1;
		processTable[i].priority = (rand() % 5) + 1;
		processTable[i].messagesQueue.size = 0;
	}
}

void contextSwitch(int currProcess, int nextProcess) {
	processTable[currProcess].pc = PC;
	processTable[currProcess].acc = ACC;
	if (processTable[currProcess].state != 3) {
		processTable[currProcess].state = 0;
	}
	PC = processTable[nextProcess].pc;
	ACC = processTable[nextProcess].acc;
	processTable[nextProcess].state = 1;

	printf("Context switch complete! Process %d to Process %d\n", processTable[currProcess].pid, processTable[nextProcess].pid);
}

void round_robin() {
	int nextProcess = (currentProcess + 1) % MAX_PROCESSES;
	while (processTable[nextProcess].state == 3) {
		nextProcess = (nextProcess + 1) % MAX_PROCESSES;
		printf("Next Process ID: %d, State: %d, Time: %d\n", processTable[nextProcess].pid, processTable[nextProcess].state, processTable[nextProcess].time);
		if (nextProcess == currentProcess) {
			if (processTable[nextProcess].state != 3) {
				contextSwitch(currentProcess, nextProcess);
				currentProcess = nextProcess;
			} else {
				printf("All processes complete.\n");
				break;
			}
		}
	}
	if (processTable[nextProcess].state == 0) {
		contextSwitch(currentProcess, nextProcess);
		currentProcess = nextProcess;
	}
	return;
}

void priorityScheduler() {
	int highestIndex = -1;
	int highestPriority = -1;

	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processTable[i].state == 0) {
			if (processTable[i].priority > highestPriority) {
				highestPriority = processTable[i].priority;
				highestIndex = i;
			}
		}
	}
	if (highestIndex != -1) {
		contextSwitch(currentProcess, highestIndex);
		currentProcess = highestIndex;
	} else {
		printf("No ready processes found.\n");
	}
}
// int int int -> void
// Purpose: Takes a processID (receiver), a message, and data for that message and sends the message and data to the receiver.
void sendMessage(int process, int message, int dataSent) {
	struct MessageQueue *queue = &processTable[process].messagesQueue;

	if (queue->size < MAX_MESSAGES) {
		int index = queue->size;
		queue->messages[index] = message;
		queue->data[index] = dataSent;
		queue->size++;
		printf("Message sent from Process %d to Process %d\n", currentProcess, process);
	} else {
		printf("Message Queue is full for Process %d!\n", process);
	}
}

void recieveMessage(int process) {
	struct MessageQueue *queue = &processTable[process].messagesQueue;
	if (queue->size == 0) {
		printf("No messages found for Process: %d\n", process);
	}
	int message = queue->messages[0];
	int data = queue->data[0];

	switch (message) {
		case START:
			printf("Process %d received a START message.\n", process);
			processTable[process].state = 1;
			break;
		case NEW_PC:
			printf("Process %d received a NEW_PC message.\n", process);
			processTable[process].pc = data;
			break;
		case NEW_ACC:
			printf("Process %d received a NEW_ACC message.\n", process);
			processTable[process].acc = data;
			break;
		default:
			printf("Process %d received an INVALID message: %d.\n", process, message);
	}
	for (int i = 0; i < queue->size; i++) {
		queue->messages[i] = queue->messages[i + 1];
		queue->data[i] = queue->data[i + 1];
	}
	queue->size--;
}

void* schedulingTask(void* arg) {
	while (1) {
		pthread_mutex_lock(&scheduling_mutex);
		round_robin();
		pthread_mutex_unlock(&scheduling_mutex);
		if (complete_processes()) {
			printf("Completion detected in schedulingTask!");
			break;
		}
		sleep(TIME_SLICE);
	}
	return NULL;
}

int main() {
	initProcesses();
	pthread_mutex_init(&scheduling_mutex, NULL);
	pthread_t scheduler_thread;
	pthread_create(&scheduler_thread, NULL, schedulingTask, NULL);
	pthread_join(scheduler_thread, NULL);
	pthread_mutex_destroy(&scheduling_mutex);
	return 0;
}
