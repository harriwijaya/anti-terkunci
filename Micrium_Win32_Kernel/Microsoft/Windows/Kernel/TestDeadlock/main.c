/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                             uC/OS-III
*                                            EXAMPLE CODE
*
* Filename : main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>

#include  "os_app_hooks.h"
#include  "app_cfg.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
// Startup Task: send semaphore at certain interfal to trigger T1 and T2 execution
static  OS_TCB   StartupTaskTCB;
static  CPU_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];

// T1 Task: higher priority task, execute later, hold R2 then R1
static  OS_TCB   T1TaskTCB;
static  CPU_STK  T1TaskStk[APP_CFG_T1_TASK_STK_SIZE];
// T2 Task: lower priority task, execute first, hold R1 then R2
static  OS_TCB   T2TaskTCB;
static  CPU_STK  T2TaskStk[APP_CFG_T2_TASK_STK_SIZE];

// Event flag
static  OS_FLAG_GRP FlagGrpT1;
static  OS_FLAG_GRP FlagGrpT2;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  StartupTask (void  *p_arg);

static  void  T1Task(void  *p_arg);
static  void  T2Task(void  *p_arg);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  os_err;


    CPU_IntInit();

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit(&os_err);                                            /* Initialize uC/OS-III                                 */
    if (os_err != OS_ERR_NONE) {
        while (1);
    }

    App_OS_SetAllHooks();                                       /* Set all applications hooks                           */

    OSTaskCreate(&StartupTaskTCB,                               /* Create the startup task                              */
                 "Startup Task",
                  StartupTask,
                  0u,
                  APP_CFG_STARTUP_TASK_PRIO,
                 &StartupTaskStk[0u],
                  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE / 10u],
                  APP_CFG_STARTUP_TASK_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &os_err);
    if (os_err != OS_ERR_NONE) {
        while (1);
    }

	//
	//  Consider using OSTmrCreate instead...
	//     with periodic option.
	//

	// Create event flag
	OSFlagCreate(&FlagGrpT1,
		"Event flag for T1 trigger",
		(OS_FLAGS)0,
		&os_err);
	if (os_err != OS_ERR_NONE) {
		while (1);
	}
	OSFlagCreate(&FlagGrpT2,
		"Event flag for T2 trigger",
		(OS_FLAGS)0,
		&os_err);
	if (os_err != OS_ERR_NONE) {
		while (1);
	}

    OSStart(&os_err);                                           /* Start multitasking (i.e. give control to uC/OS-III)  */

    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
}


/*
*********************************************************************************************************
*                                            STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  StartupTask (void *p_arg)
{
    OS_ERR  os_err;


   (void)p_arg;

    OS_TRACE_INIT();                                            /* Initialize the uC/OS-III Trace recorder              */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err);                            /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    
    APP_TRACE_DBG(("uCOS-III is Running...\n\r"));


	OSTaskCreate(&T1TaskTCB,                               /* Create the startup task                              */
		"T1 Task",
		T1Task,
		0u,
		APP_CFG_T1_TASK_PRIO,
		&T1TaskStk[0u],
		T1TaskStk[APP_CFG_T1_TASK_STK_SIZE / 10u],
		APP_CFG_T1_TASK_STK_SIZE,
		0u,
		0u,
		0u,
		(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		&os_err);
	if (os_err != OS_ERR_NONE) {
		while (1);
	}

	OSTaskCreate(&T2TaskTCB,                               /* Create the startup task                              */
		"T2 Task",
		T2Task,
		0u,
		APP_CFG_T2_TASK_PRIO,
		&T2TaskStk[0u],
		T2TaskStk[APP_CFG_T2_TASK_STK_SIZE / 10u],
		APP_CFG_T2_TASK_STK_SIZE,
		0u,
		0u,
		0u,
		(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		&os_err);
	if (os_err != OS_ERR_NONE) {
		while (1);
	}


    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        //OSTimeDlyHMSM(0u, 0u, 1u, 0u,
        //              OS_OPT_TIME_HMSM_STRICT,
        //              &os_err);
        //APP_TRACE_DBG(("Time: %d\n\r", OSTimeGet(&os_err)));

		/* Trigger T2 */
		APP_TRACE_DBG(("T1\n\r"));

		// Flag to T2
		OSFlagPost(&FlagGrpT2,
			(OS_FLAGS)2, /* any value */
			(OS_OPT)OS_OPT_POST_FLAG_SET,
			&os_err);
		if (os_err != OS_ERR_NONE) {
			APP_TRACE_DBG(("Fail to FLAG to T2.\n\r"));
		}
		// sleep
		OSTimeDlyHMSM(0u, 0u, 5u, 0u,
		              OS_OPT_TIME_HMSM_STRICT,
		              &os_err);

		/* Trigger T1 */
		APP_TRACE_DBG(("T2\n\r"));

		// Flag to T1
		OSFlagPost(&FlagGrpT1,
			(OS_FLAGS)1, /* any value */
			(OS_OPT)OS_OPT_POST_FLAG_SET,
			&os_err);
		if (os_err != OS_ERR_NONE) {
			APP_TRACE_DBG(("Fail to FLAG to T1.\n\r"));
		}
		// sleep
		OSTimeDlyHMSM(0u, 0u, 5u, 0u,
			OS_OPT_TIME_HMSM_STRICT,
			&os_err);

    }
}

/* T1 */
static  void  T1Task(void *p_arg)
{
	OS_ERR  os_err;
	CPU_TS  ts;

	(void)p_arg;

	while (1) {
		OSFlagPend(&FlagGrpT1,
			(OS_FLAGS)0x0F,
			0, /* wait forever */
			OS_OPT_PEND_BLOCKING +
			OS_OPT_PEND_FLAG_SET_ANY +
			OS_OPT_PEND_FLAG_CONSUME,
			&ts,
			&os_err);
		if (os_err != OS_ERR_NONE) {
			APP_TRACE_DBG(("Fail to PEND on flag for T1.\n\r"));
		}

		APP_TRACE_DBG(("T1 executing.\n\r"));
	}

}

/* T2 */
static  void  T2Task(void *p_arg)
{
	OS_ERR  os_err;
	CPU_TS  ts;

	(void)p_arg;

	while (1) {
		OSFlagPend(&FlagGrpT2,
			(OS_FLAGS)0x0F,
			0, /* wait forever */
			OS_OPT_PEND_BLOCKING +
			OS_OPT_PEND_FLAG_SET_ANY +
			OS_OPT_PEND_FLAG_CONSUME,
			&ts,
			&os_err);
		if (os_err != OS_ERR_NONE) {
			APP_TRACE_DBG(("Fail to PEND on flag for T2.\n\r"));
		}

		APP_TRACE_DBG(("T2 executing.\n\r"));
	}

}