/*
* Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
*
* @APPLE_LICENSE_HEADER_START@
* 
* The contents of this file constitute Original Code as defined in and
* are subject to the Apple Public Source License Version 1.1 (the
* "License").  You may not use this file except in compliance with the
* License.  Please obtain a copy of the License at
* http://www.apple.com/publicsource and read it before using this file.
* 
* This Original Code and all software distributed under the License are
* distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
* INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
* License for the specific language governing rights and limitations
* under the License.
* 
* @APPLE_LICENSE_HEADER_END@
*/
#include "OpenPMUUserClient.h"

#ifndef NULL
#define NULL	0
#endif

#define super	IOUserClient

OSDefineMetaClassAndStructors(OpenPMUUserClient, IOUserClient)

/* static */ OpenPMUUserClient*
OpenPMUUserClient::withTask(task_t owningTask)
{
    OpenPMUUserClient *client;

    client = new OpenPMUUserClient;
    if (client != NULL) {
        if (client->init() == false) {
            client->release();
            client = NULL;
        }
    }
    if (client != NULL) {
        client->fTask = owningTask;
    }
    return (client);
}

bool
OpenPMUUserClient::start(IOService *provider)
{
    bool result = false;

    theInterface = OSDynamicCast(OpenPMUInterface, provider);

    if (theInterface != NULL)
        result = super::start(provider);
    else
        result = false;

    if (result == false) {
        IOLog("OpenPMUUserClient: provider start failed\n");
    }

    return (result);
}

IOReturn
OpenPMUUserClient::clientClose(void)
{
    detach(theInterface);
    return (kIOReturnSuccess);
}

IOReturn
OpenPMUUserClient::clientDied(void)
{
    return (clientClose());
}

IOReturn
OpenPMUUserClient::connectClient(IOUserClient *client)
{
    return (kIOReturnSuccess);
}

IOReturn
OpenPMUUserClient::registerNotificationPort(mach_port_t port, UInt32 type)
{
    return (kIOReturnUnsupported);
}

// --------------------------------------------------------------------------
// Method: setProperties
//
// Purpose:
//       sets the property from the dictionary to the airport properties.

// Directly from the ApplePlatfromExpert:
#define	SECS_BETWEEN_1904_1970	2082844800

IOReturn
OpenPMUUserClient::setProperties( OSObject * properties )
{
    OSDictionary *	dict;

    dict = OSDynamicCast( OSDictionary, properties );
    if ((dict) && (theInterface != NULL)) {
        OSData *data;

        // Sets the wake on ring:
        if( (data = OSDynamicCast( OSData, dict->getObject("WakeOnRing")))) {
            UInt8 myBool = *((UInt8*)data->getBytesNoCopy());
            theInterface->setWakeOnRing(myBool);

            IOLog("OpenPMUUserClient::setProperties WakeOnRing %d\n", myBool);
            
            // returns success:
            return kIOReturnSuccess;
        }

        // Sets the file-server mode:
        if( (data = OSDynamicCast( OSData, dict->getObject("FileServer")))) {
            UInt8 myBool = *((UInt8*)data->getBytesNoCopy());
            theInterface->setFileServerMode(myBool);

            IOLog("OpenPMUUserClient::setProperties FileServer %d\n", myBool != 0);
            
            // returns success:
            return kIOReturnSuccess;
        }

        // Sets the self-wake time:
        if( (data = OSDynamicCast( OSData, dict->getObject("AutoWake")))) {
            IOByteCount len = data->getLength();
            UInt32 newTime = *((UInt32*)data->getBytesNoCopy());

            if ((len == 4) && (newTime != 0)) {
                newTime += SECS_BETWEEN_1904_1970;
                theInterface->sendMiscCommand (kPMUtimerSet, len, (UInt8*)&newTime, NULL, NULL);
            }
            else {
                theInterface->sendMiscCommand (kPMUtimerDisable, 0, NULL, NULL, NULL);
                newTime = 0;
            }

            theInterface->setProperty("AutoWake", newTime);
            IOLog("OpenPMUUserClient::setProperties AutoWake 0x%08lx\n", newTime);   

            // returns success:
            return kIOReturnSuccess;
        }

        // Sets the file-server mode:
        if( (data = OSDynamicCast( OSData, dict->getObject("ReflashPMU")))) {
            IOReturn returnValue = theInterface->downloadFirmware(data);

            // Checks the result:
            if (returnValue == kIOReturnSuccess) {
                UInt8 iBuffer = 0;
                IOByteCount iLen = sizeof(iBuffer);
                if (theInterface->sendMiscCommand (kPMUdownloadStatus, (IOByteCount)0, NULL, &iLen, &iBuffer) == kIOReturnSuccess) {
                    IOLog("OpenPMUUserClient::setProperties kPMUdownloadStatus returns 0x%02x\n", iBuffer);

                    if (iBuffer != 0x62)
                        returnValue = kIOReturnBadArgument;
                }
                else
                    returnValue = kIOReturnBadArgument;
            }

            IOLog("OpenPMUUserClient::setProperties ReflashPMU returns error # 0x%08lx\n", returnValue);

            // returns the result:
            return returnValue;
        }
    }

    return(kIOReturnBadArgument);
}
