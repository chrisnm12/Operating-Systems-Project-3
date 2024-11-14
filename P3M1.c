//this is a starter file

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
//Status register flags
struct StatusRegister {
 int ZF;
 int CF;
} Status;

//cpu core registration system.
struct CpuCore{
    int PC;
    int ACC;
    int IR;
    int startMemAdd;
    int endMemAdd;
    int interruptFlag; // flag to signal the interrupt
    int carryFlag;
    int zeroFlag;
    int overFlowFlag;
    int errorFlag;
};
struct CpuCore cores[2];

void initCpuCores(){
    //core 1 initializes here
    cores[0].PC = 0;
    cores[0].ACC = 0;
    cores[0].IR = 0;
    cores[0].startMemAdd = 0;
    cores[0].endMemAdd = 19;
    cores[0].interruptFlag = 0;
    cores[0].carryFlag = 0;
    cores[0].zeroFlag = 0;
    cores[0].overFlowFlag = 0;
    cores[0].errorFlag = 0;
    
    //core 2 initializes here
    cores[1].PC = 20;
    cores[1].ACC = 0;
    cores[1].IR = 0;
    cores[1].startMemAdd = 20;
    cores[1].endMemAdd = 37;
    cores[1].interruptFlag = 0;
    cores[1].carryFlag = 0;
    cores[1].zeroFlag = 0;
    cores[1].overFlowFlag = 0;
    cores[1].errorFlag = 0;
} 
// flags
int interruptFlag = 0; // flag to signal the interrupt
int carryFlag;
int zeroFlag;
int overFlowFlag;
int errorFlag = 0;

int memory[256];

//Mutex for safe resource access
pthread_mutex_t memory_mutex;
//Define constants for instruction set
// instruction types
#define ADD 1
#define SUB 2
#define LOAD 3
#define STORE 4
#define MUL 5
#define DIV 6
#define AND 7
#define OR 8
#define JMP 9
#define JZ 10

// the opcode and operand
int opcode;
int operand;

//add more based on the rquirements
//Function to initialize memory with sample instructions
void loadProgram() {
 //Complete: Load sample instructions into memory
 //Eg:
 //memory[0] = LOAD; memory[1] = 10; //LOAD 10 into ACC
 //Add more instructions as needed
    // Core 1 instructions
    memory[0] = LOAD;  memory[1] = 10;    // Load 10 into ACC (for Core 0)
    memory[2] = ADD;   memory[3] = 5;     // Add 5 to ACC (Core 0 ACC = 15)
    memory[4] = STORE; memory[5] = 3;    // Store ACC value (15) to memory address 20
    memory[6] = LOAD;  memory[7] = 12;    // Load 12 into ACC
    memory[8] = SUB;   memory[9] = 3;     // Subtract 3 from ACC (Core 0 ACC = 9)
    memory[10] = JZ;   memory[11] = 16;   // Jump to address 16 if zeroFlag is set
    memory[12] = JMP;  memory[13] = 16;   // Unconditional jump to address 16
    memory[14] = MUL;  memory[15] = 2;    // Multiply ACC by 2 (Should be skipped if JZ succeeds)
    memory[16] = LOAD; memory[17] = 2;    //loads 2 onto ACC
    memory[18] = DIV;  memory[19] = 2;    // Divide ACC by 2 = 1

    // Core 2 instructions
    memory[20] = LOAD;  memory[21] = 3;     // Core 1 loads 3 into ACC
    memory[22] = ADD;   memory[23] = 10;    // Adds 10 to ACC (Core 1 ACC = 13)
    memory[24] = AND;  memory[25] = 4;    // Perform AND with 4 (binary operation on ACC)
    memory[26] = OR;   memory[27] = 8;    // Perform OR with 8 (Core 1 ACC modified)
    memory[28] = STORE; memory[29] = 25;  // Store ACC to memory address 25
    memory[30] = LOAD;  memory[31] = 7;   // Load 7 into ACC
    memory[32] = JMP;   memory[33] = 36;  // Jump to address 22
    memory[34] = DIV;   memory[35] = 2;   // Divide ACC by 2 (should be skipped if JMP succeeds)
    memory[36] = SUB;   memory[37] = 1;   // Subtract 1 from ACC (Core 1 ACC = 6 after all operations)
}
//Function to simulate instruction fetching
void fetch(int coreNum) {
    cores[coreNum].IR = memory[cores[coreNum].PC]; //Fetch the next instruction
}

void execute(int coreNum){
    //Decode the instruction in IR
    //Implement logic to extract the opcode and operands   
    int opcode = cores[coreNum].IR;
    int operand = memory[cores[coreNum].PC + 1];
    int core = coreNum + 1;
    int result;
    int prev;

    switch (opcode) {
        case ADD: 
            prev = cores[coreNum].ACC;
            result = cores[coreNum].ACC + operand;
            cores[coreNum].ACC = result;
            printf("Core %d Operation Adding: %d + %d = %d\n", core, prev, operand, cores[coreNum].ACC);
            break;
        case SUB:
            prev = cores[coreNum].ACC;
            result = cores[coreNum].ACC - operand;
            cores[coreNum].ACC = result;
            printf("Core %d Operation: Subtracting: %d - %d = %d\n", core, prev, operand, cores[coreNum].ACC);
            break;
        case MUL:
            prev = cores[coreNum].ACC;
            result = cores[coreNum].ACC * operand;
            cores[coreNum].ACC = result;
            printf("Core %d Operation Multiplying: %d * %d = %d\n", core, prev, operand, cores[coreNum].ACC);
            break;
        case DIV:
            if(operand != 0){
                prev = cores[coreNum].ACC;
                result = cores[coreNum].ACC / operand;
                cores[coreNum].ACC = result;
                printf("Core %d Operation: Dividing: %d / %d = %d\n", core, prev, operand, cores[coreNum].ACC);
                break;
            }
            else{
                printf("Core %d Operation: Division Error: Division by zero\n", core);
                cores[coreNum].errorFlag = 1;
                break;
            }
        case LOAD:
            prev = cores[coreNum].ACC;
            cores[coreNum].ACC = operand;
			printf("Core %d Operation: Loading data in ACC: %d -> %d\n",core , prev, cores[coreNum].ACC);
			break;
        case STORE:
            memory[operand] = cores[coreNum].ACC;
            printf("Core %d Operation: Storing data into memory from ACC to Adress: %d -> %d\n", core, cores[coreNum].ACC, operand);
			break;
        case AND:
            prev = cores[coreNum].ACC;
            result = cores[coreNum].ACC & operand;
            cores[coreNum].ACC = result;
			printf("Core %d Operation: AND operation: %d & %d = %d \n", core,  prev, operand, cores[coreNum].ACC);
            break;
        case OR:
            prev = cores[coreNum].ACC;
            result = cores[coreNum].ACC | operand;
            cores[coreNum].ACC = result;
            printf("Core %d Operation: OR operation: %d | %d = %d \n", core , prev, operand, cores[coreNum].ACC);
            break;
        case JMP:
            if (operand >= cores[coreNum].startMemAdd && operand < cores[coreNum].endMemAdd){
                prev = cores[coreNum].PC;
                cores[coreNum].PC = operand - 2;
                printf("Core %d Operation: Jump: PC has been changed from %d to %d\n",core ,  prev, cores[coreNum].PC);
                break;
            }
            else{
                printf("Core %d Operation: Error: Invalid Jump attempt. Out of Bounds Error\n", core);
                cores[coreNum].errorFlag = 1;
                break;
            }
        case JZ:
            if(cores[coreNum].zeroFlag == 1){
                if(operand >= cores[coreNum].startMemAdd && operand < cores[coreNum].endMemAdd){
                    prev = cores[coreNum].PC;
                    cores[coreNum].PC = operand - 2;
                    printf("Core %d Operation: Jump Zero: PC has been changed from %d to %d\n", core, prev, cores[coreNum].PC);
                    break;
                    }
                    else{
                        printf("Core %d Operation: Error: Invalid Jump attempt. Out of Bounds Error\n",core);
                        cores[coreNum].errorFlag = 1;
                        break;
                    }
                
            }
            else{
                printf("Core %d Operation: Jump Zero: Unsuccessful jump, no zero flag present.\n", core);
                break;
            }
		default:
			// Handle undefined/invalid opcodes
			printf("Core %d Operation: Invalid opcode given. Please try again.\n", core);
            cores[coreNum].errorFlag = 1;
			break;
        }

    cores[coreNum].PC += 2; //Move to the next instruction
}

//Function to simulate the instruction cycle with concurrency
void* cpuCore(void* arg) {
int coreNum = *(int *)arg;
    while (1) {
        if(cores[coreNum].errorFlag == 1){
        break;
        }
        //Lock memory for exclusive access
        pthread_mutex_lock(&memory_mutex);
        fetch(coreNum); //fetch instruction
        execute(coreNum); //decode and execute instruction
        pthread_mutex_unlock(&memory_mutex); //unlock memory
        //Stop condition (example: check PC bounds)
        if (cores[coreNum].PC >= cores[coreNum].endMemAdd || cores[coreNum].PC < cores[coreNum].startMemAdd) break;
    }
    if(cores[coreNum].errorFlag == 1){
    printf("Core %d Error Exit\n", coreNum +1);
    }
    else{
    printf("Core %d Execution Complete\n", coreNum + 1);
    }
        return NULL;
}

void startCores(){
 initCpuCores();
 int *core1 = malloc(sizeof(int));
 *core1 = 0;

 int *core2 = malloc(sizeof(int));
 *core2 = 1;

 //create a thread for concurrent processing
 pthread_t cpu_thread;
 pthread_t cpu_thread2;
 
 pthread_create(&cpu_thread, NULL, cpuCore, core1);
 pthread_create(&cpu_thread2,NULL, cpuCore, core2);
 
 //wait for the thread to complete
 pthread_join(cpu_thread, NULL);
 pthread_join(cpu_thread2, NULL);

 free(core1);
 free(core2);
}

int main() {
 loadProgram(); //initialize memory with instructions
 //initialize mutex for memory access
 pthread_mutex_init(&memory_mutex, NULL);
 startCores();
 //clean up mutex
 pthread_mutex_destroy(&memory_mutex);
 return 0;
}