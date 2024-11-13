//this is a starter file

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//Define core CPU registers
int PC = 0;
int ACC = 0;
int IR = 0;
//Status register flags
struct StatusRegister {
 int ZF;
 int CF;
} Status;

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
 memory[0] = LOAD_operation;
 memory[1] = 15;
 memory[2] = MUL_operation;
 memory[3] = 3;
}
//Function to simulate instruction fetching
void fetch() {
 IR = memory[PC]; //Fetch the next instruction
}

void decode(){
    //Decode the instruction in IR
    //Implement logic to extract the opcode and operands
    opcode = IR;
    operand = memory[PC + 1];
}

void execute(){

    decode();
    int result;
    int prev;

    switch (IR) {
        case ADD_operation: 
            prev = ACC;
            result = ACC + operand;
            ACC = result;
            printf("Adding: %d + %d = %d\n", prev, operand, ACC);
            break;
        case SUB_operation:
            prev = ACC;
            result = ACC - operand;
            ACC = result;
            printf("Subtracting: %d - %d = %d\n", prev, operand, ACC);
            break;
        case MUL_operation:
            prev = ACC;
            result = ACC * operand;
            ACC = result;
            printf("Multiplying: %d * %d = %d\n", prev, operand, ACC);
            break;
        case DIV_operation:
            if(operand != 0){
                prev = ACC;
                result = ACC / operand;
                ACC = result;
                printf("Dividing: %d * %d = %d\n", prev, operand, ACC);
                break;
            }
            else{
                printf("Division Error: Division by zero\n");
                errorFlag = 1;
                break;
            }
        case LOAD_operation:
            prev = ACC;
            ACC = operand;
			printf("Loading data in ACC: %d -> %d\n", prev, ACC);
			break;
        case STORE_operation:
            memory[operand] = ACC;
            printf("Storing data into memory from ACC to Adress: %d -> %d\n", ACC, operand);
			break;
        case AND_operation:
            prev = ACC;
            result = ACC & operand;
            ACC = result;
			printf("AND operation: %d & %d = %d \n", prev, operand, ACC);
            break;
        case OR_operation:
            prev = ACC;
            result = ACC | operand;
            ACC = result;
            printf("OR operation: %d | %d = %d \n", prev, operand, ACC);
            break;
        case JMP_operation:
            if (operand >= 0 && operand < 256){
                prev = PC;
                PC = operand - 2;
                printf("Jump: PC has been changed from %d to %d\n", prev, PC);
                break;
            }
            else{
                printf("Error: Invalid Jump attempt. Out of Bounds Error\n");
                errorFlag = 1;
                break;
            }
        case JZ_operation:
            if(zeroFlag == 1){
                if(operand >= 0 && operand < 256){
                    prev = PC;
                    PC = operand - 2;
                    printf("Jump Zero: PC has been changed from %d to %d\n", prev, PC);
                    break;
                    }
                    else{
                        printf("Error: Invalid Jump attempt. Out of Bounds Error\n");
                        errorFlag = 1;
                        break;
                    }
                
            }
            else{
                printf("Jump Zero: Unsuccessful jump, no zero flag present.\n");
                errorFlag = 1;
                break;
            }
		default:
			// Handle undefined/invalid opcodes
			printf("Invalid opcode given. Please try again.\n");
            errorFlag = 1;
			break;
        }
    
    PC += 2; //Move to the next instruction

}

//Function to simulate the instruction cycle with concurrency
void* cpuCore(void* arg) {
 while (1) {
if(errorFlag == 1){
    break;
}
 //Lock memory for exclusive access
 pthread_mutex_lock(&memory_mutex);
 fetch(); //fetch instruction
 if(!IR){
    break;
 }
 execute(); //decode and execute instruction
 pthread_mutex_unlock(&memory_mutex); //unlock memory
 //Stop condition (example: check PC bounds)
 if (PC >= 256 || PC < 0) break;
 }
 return NULL;
}
int main() {
 loadProgram(); //initialize memory with instructions
 //initialize mutex for memory access
 pthread_mutex_init(&memory_mutex, NULL);
 //create a thread for concurrent processing
 pthread_t cpu_thread;
 pthread_create(&cpu_thread, NULL, cpuCore, NULL);
 //wait for the thread to complete
 pthread_join(cpu_thread, NULL);
 //clean up mutex
 pthread_mutex_destroy(&memory_mutex);
 return 0;
}