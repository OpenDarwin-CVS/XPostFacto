/*

Copyright (c) 2001, 2002
Other World Computing
All rights reserved

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer as the first lines of
   each file.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

3. Redistributions in binary form must retain the link to Other World
   Computing's web site in the application's "About Box."

This software is provided by Other World Computing ``as is'' and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed. In no event
shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or
profits; or business interruption) however caused and on any theory of
liability, whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

*/

// This is all based on code from Don Bruder. Thanks!

// Unfortunately, it doesn't work with Mac OS 9.2. So I am leaving it out for the moment.

#include <Memory.h>
#include <Devices.h>
#include <Errors.h>

#include "L2Cache.h"

static unsigned readL2CRValue ();
static void EnterPPCSupervisorMode(void);
static void LeavePPCSupervisorMode(void);
static void InstallHandler(void);
static void RemoveHandler(void);
static void HandlerCode(void);
static void HandlerCodeEnd(void);

typedef struct MyHandlerData
{
   unsigned long  OriginalHandler;
   unsigned long  r4;
   unsigned long  cr;
   unsigned long  Code[];
} MyHandlerData;

static MyHandlerData*   GlobalHandlerData;

#define L2CR  1017

#define LOCATION(x) ((unsigned long*)x)[0]

unsigned int getCurrentL2CRValue ()
{
	unsigned int theValue;
	EnterPPCSupervisorMode ();
	theValue = readL2CRValue ();
	LeavePPCSupervisorMode ();
	return theValue;
}
  
static asm unsigned int readL2CRValue ()
{
	mfspr	r3,L2CR
	blr
}    
    
static void EnterPPCSupervisorMode(void)
{        
      GlobalHandlerData = NULL;
      InstallHandler(); /* Set up the exception handler we're about to invoke */
      asm {
           /* Set up caller signature */
            lis       r3,'RC';
            ori       r3,r3,'5C';
            /* invoke the exception handler */
            sc;
          }
      /* Now get rid of the handler */
      RemoveHandler();
      /* Theoretically, we could install the handler at init time, then uninstall it at shutdown */
      /* to save time. However, I feel that this way is a bit safer.                             */
}
    
static asm void LeavePPCSupervisorMode(void)

   {
      mfmsr r4;           /* Get current setting */
      ori   r4,r4,0xc000; /* Set PR bit and EE bit (User mode, and re-enable interrupts) */
      mtmsr r4;           /* Stuff modified back into MSR */
    					  /* We're now safely back in user mode */
   }
   
static void InstallHandler(void)

  {
   void                      *HandlerPhys;
   unsigned long            CodeLength, Count;
   MyHandlerData          *Handler;
   LogicalToPhysicalTable L2PTable;
   OSErr             Err;
   
   CodeLength =  LOCATION(HandlerCodeEnd) - LOCATION(HandlerCode); /* How big? */
   Handler =  (MyHandlerData*)NewPtrSys(sizeof(MyHandlerData) + CodeLength); /* Make a hole */
   Handler->OriginalHandler = *(unsigned long*)0x5FFFE450; /* save sc vector */
   GlobalHandlerData = Handler; /* copy out to global so we can access it later */
      
   BlockMove((void *)LOCATION(HandlerCode),Handler->Code,CodeLength); /* Move it into place */
   LockMemoryContiguous(Handler,sizeof(MyHandlerData) + CodeLength); /* Freeze, sucka! */
   MakeDataExecutable(Handler->Code,CodeLength); /* Sync caches (and other needed stuff) */

    /* Figure out where the handler lives */
   L2PTable.logical.address = &Handler->Code;
   L2PTable.logical.count   = 1024;
   Count = sizeof(L2PTable) / sizeof(MemoryBlock) - 1;
   Err = GetPhysical(&L2PTable, &Count);
   if (Err != noErr)
       HandlerPhys = &Handler->Code;
   else
       HandlerPhys = (Ptr)L2PTable.physical[0].address;
      /* Now that we know the location, stuff it into the vector */
    *(unsigned long*)0x5FFFE450 = (unsigned long)HandlerPhys; /* Chain in. This is where the code will probably break under OSX. */
  }

static void RemoveHandler(void)

  {
    unsigned long CodeLength;
      
    CodeLength = LOCATION(HandlerCodeEnd) - LOCATION(HandlerCode);    /* Figure out code length */
   *(unsigned long*)0x5FFFE450 = GlobalHandlerData->OriginalHandler; /* Put the old handler back. Again, the code will probably break here under OSX */
    UnlockMemory(GlobalHandlerData, sizeof(MyHandlerData) + CodeLength); /* Let things move again */
   DisposePtr((char *)GlobalHandlerData);                                /* Prevent memory leaks */
   GlobalHandlerData = NULL;                    /* Make sure it errors out if re-used after this */   
  }


static asm void HandlerCode(void)

  {
    /* When execution gets here, we're operating in Supervisor mode. As soon as the rfi happens,  */
    /* we'll lose that luxury unless we take some steps to hold onto it (kinda like freedom...)   */
   /* Make room for a local copy of MyHandlerData */
   subi  sp,sp,sizeof(MyHandlerData);
   
   /* Save r4 and cr in case the call isn't for us */ 
   stw       r4,MyHandlerData.r4(sp);
   mfcr    r4;
   stw       r4,MyHandlerData.cr(sp);  

   /* Test the signature. Is this call really meant for us? */
   lis       r4,'RC';
   ori       r4,r4,'5C';
   cmpw    r3,r4;
   bne     @callOriginalHandler; /* Not ours. Pass on */

   /* It's for us. Put cr back, and set up for the rfi */
   lwz       r4,MyHandlerData.cr(sp);
   mtcrf   0xFF,r4;
   mfspr   r4,SPRG2;
   mtlr    r4;
   mfspr    sp,SPRG1;
   /* We need to clear the PR bit of the MSR, but it's already clear, since we're executing as  */
   /* an exception handler. (This implicitly means that the PR bit is clear) But it will be put */ 
   /* back to whatever it was before we got called as part of the normal operation of the rfi   */
   /* instruction. So what do we do???? The answer:                                             */
   /* Mess with the saved (by the sc instruction that got us here) copy of the MSR that's       */
   /* stored in the srr1 register! This assures that our change will "stick" after the rfi      */
   /* instruction is executed.                                                                  */                               
   /* We're also going to shut down all maskable interrupts for the duration of our time in     */
   /* supervisor mode.                                                                          */
   mfsrr1   r4;
   li      r3,0x3fff;
   oris    r3,r3,0xffff;
   and     r4,r4,r3;
   mtsrr1  r4;
   rfi;

  @callOriginalHandler:
   /* Nope, not for us. Put r4/cr back like we found 'em, and pass through to whatever */
   /* handler we may have cut ahead of. */
   lwz       r4,MyHandlerData.cr(sp);
   mtcrf   0xFF,r4;
   lwz       r4,MyHandlerData.r4(sp);
   lwz       sp,MyHandlerData.OriginalHandler(sp);
   mtlr    sp;
   blrl;
  entry HandlerCodeEnd
  HandlerCodeEnd: 
}
