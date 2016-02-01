/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(test_module);


void print_messages_1(void);
void print_messages_2(void);
static void print_messages(void);
static void print_inst_messages(void *instance);


static void *instance1=&instance1;
static void *instance2=&instance2;
static void *instance3=&instance3;

int 
main(void)
{

    BKNI_Init();
    BDBG_Init();

    print_messages_1();/* this should trigger static allocation */

    BDBG_REGISTER_INSTANCE(instance1);
    BDBG_REGISTER_INSTANCE(instance2);

    BDBG_SetInstanceName(instance1, "instance1");
    BDBG_SetInstanceName(instance2, "instance2");

    BDBG_ERR(("Default level"));
    print_messages();

    BDBG_SetLevel(BDBG_eTrace);
    BDBG_ERR(("TRACE Level"));
    print_messages();


    BDBG_SetLevel(BDBG_eMsg);
    BDBG_ERR(("MSG Level"));
    print_messages();

    BDBG_SetLevel(BDBG_eWrn);
    BDBG_ERR(("WRN Level"));
    print_messages();

    BDBG_SetLevel(BDBG_eErr);
    BDBG_ERR(("ERR Level"));
    print_messages();


    BDBG_SetModuleLevel("test_module", BDBG_eMsg);
    BDBG_ERR(("module MSG Level"));
    print_messages();
    BDBG_SetModuleLevel("test_module", BDBG_eWrn);
    BDBG_ERR(("module WRN Level"));
    print_messages();
    BDBG_SetModuleLevel("test_module", BDBG_eErr);
    BDBG_ERR(("module ERR Level"));
    print_messages();

    BDBG_SetInstanceLevel(instance1, BDBG_eMsg);
    BDBG_ERR(("instance %#x MSG Level", (unsigned)instance1));
    print_messages();
    BDBG_SetInstanceLevel(instance1, BDBG_eWrn);
    BDBG_ERR(("instance %#x WRN Level", (unsigned)instance1));
    print_messages();
    BDBG_SetInstanceLevel(instance1, BDBG_eErr);
    BDBG_ERR(("instance %#x ERR Level", (unsigned)instance1));
    print_messages();

    BDBG_Uninit();

    BDBG_Init();
    BDBG_Uninit();

    BDBG_Init();

    BDBG_ASSERT("Not Failed");
    BDBG_ASSERT(1==1);
    BDBG_ASSERT(0==1);

	/* not reached */
    BDBG_Uninit();
    
    return 0;
}



void
print_messages(void)
{
    BDBG_ENTER(print_messages);
    
    BDBG_MSG(("Debug messages - Level MSG"));
    BDBG_WRN(("Debug messages - Level WRN"));
    BDBG_ERR(("Debug messages - Level ERR"));

    print_messages_1();
    print_messages_2();

    print_inst_messages(instance1);
    print_inst_messages(instance2);
    print_inst_messages(instance3);

    BDBG_LEAVE(print_messages);
    return;
}

void
print_inst_messages(void *instance)
{
    BDBG_ENTER(print_inst_messages);

    BDBG_INSTANCE_MSG(instance, ("Debug messages - Level MSG - instance %#x", (unsigned)instance));
    BDBG_INSTANCE_WRN(instance, ("Debug messages - Level WRN - instance %#x", (unsigned)instance));
    BDBG_INSTANCE_ERR(instance, ("Debug messages - Level ERR - instance %#x", (unsigned)instance));

    BDBG_LEAVE(print_inst_messages);
    return;
}

