/**
 * test will excersize bkni under nucleus.
 */
#include "nucleus.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include <stdio.h>

NU_TASK         Task_0;
NU_TASK         Task_1;

void    task_0(UNSIGNED argc, VOID *argv);
void    task_1(UNSIGNED argc, VOID *argv);

#define ST_SIZE 0x800

/* entry point from nucleus*/
void    Application_Initialize(void *first_available_memory)
{
     BERR_Code err;
     STATUS st;
     void * pointer;
     BKNI_Nucleus_Config(first_available_memory);
     err = BKNI_Init();
     if(BERR_SUCCESS != err){
	  printf("Unable to initialize KNI!\n");
	  return;
     }
/* create 2 tasks, allocating stack from system pool */
     pointer = BKNI_Malloc(ST_SIZE);
     if(NULL == pointer){
	  printf("Unable to allocate memory\n");
	  return;
     }
     st = NU_Create_Task(&Task_0, "TASK0", task_0, 0, NU_NULL,
			 pointer, ST_SIZE, 50, 2, NU_PREEMPT, NU_START);
     if(NU_SUCCESS != st){
	  printf("Unable to create task 0.\n");
	  /*we dont need to clean up here we are dead anyway*/
	  return;
     }
     /*second task ins suspended state*/
     pointer = BKNI_Malloc(ST_SIZE);
     if(NULL == pointer){
	  printf("Unable to allocate memory\n");
	  return;
     }
     st = NU_Create_Task(&Task_1, "TASK1", task_1, 0, NU_NULL,
			 pointer, ST_SIZE, 50, 2, NU_PREEMPT, NU_NO_START);
     if(NU_SUCCESS != st){
	  printf("Unable to create task 1.\n");
	  /*we dont need to clean up here we are dead anyway*/
	  return;
     }
     /*start the whole thing*/
     NU_Control_Interrupts(NU_ENABLE_INTERRUPTS);
}

BKNI_EventHandle event1;
BKNI_EventHandle event2;

void task_0(UNSIGNED argc, VOID * argv)
{
     BERR_Code err;
     STATUS st;
     /*create event1*/
     err = BKNI_CreateEvent(&event1);
     if(BERR_SUCCESS != err){
	  BKNI_Printf("Unable to create event 1.\n");
	  return;
     }
     err = BKNI_CreateEvent(&event2);
     if(BERR_SUCCESS != err){
	  BKNI_Printf("Unable to create event 2.\n");
	  return;
     }
     /* start second task*/
     st = NU_Resume_Task(&Task_1);
     if(NU_SUCCESS != st){
	  BKNI_Printf("Unable to resume task 2.\n");
     }
     while(1){
	  BKNI_Printf("Ping\n");
	  BKNI_SetEvent(event1);
	  err = BKNI_WaitForEvent(event2, BKNI_INFINITE);
	  if(BERR_SUCCESS != err){
	       BKNI_Printf("Error while wating for event 2\n");
	       return;
	  }
	  BKNI_Printf("Pong\n");
     }
}

void task_1(UNSIGNED argc, VOID * argv)
{
     BERR_Code err;
     void * ptr;
     int i;
     while(1){
	  err = BKNI_WaitForEvent(event1, BKNI_INFINITE);
	  if(BERR_SUCCESS != err){
	       BKNI_Printf("Error while wating for event 1\n");
	       return;
	  }
	  /* do some tests now */
	  for(i = 1; i <=10; i++){
	       ptr = BKNI_Malloc(0x400 * i);
	       if(NULL == ptr){
		    BKNI_Printf("Error while allocating %dkb of memory\n", i);
		    return;
	       }
	       BKNI_Free(ptr);
	  }
	  BKNI_ResetEvent(event1);
	  BKNI_Printf("Sleeping for 2 seconds\n");
	  BKNI_Sleep(2000);
	  BKNI_Printf("Sendig event\n");
	  BKNI_SetEvent(event2);
     }
}

int main (int argc, char * argv[])
{
     INT_Initialize();
}
