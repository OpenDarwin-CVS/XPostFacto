// This is based on Apple's AudioDeviceTreeUpdater, with a few convenience
// elements thrown in to convert from OSStrings to OSData's. So the "model"
// property can be a string (instead of data), and the "sound-objects" property
// can be an array of strings (rather than a series of null-terminated strings
// in a data element). The "ConvertToOSData" function will convert to what the
// parser expects.

// Note the -DAudioDeviceTreeUpdater=OpenAudioDeviceTreeUpdater in the compiler settings --
// this avoids duplicate class names when trying to load the kext. (And keeps it easy to compare
// this file with any changes Apple makes in their kext).

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>

#include "AppleDTUpdater.h"


OSDefineMetaClassAndStructors(AudioDeviceTreeUpdater, IOService)

#define super IOService
// Guts
void
AudioDeviceTreeUpdater::mergeProperties(OSObject* inDest, OSObject* inSrc)
{
	OSDictionary*	dest = OSDynamicCast(OSDictionary, inDest) ;
	OSDictionary*	src = OSDynamicCast(OSDictionary, inSrc) ;

	if (!src || !dest)
		return ;

	OSCollectionIterator*	srcIterator = OSCollectionIterator::withCollection(src) ;
	
	OSSymbol*	keyObject = NULL ;
	OSObject*	destObject = NULL ;
	while (NULL != (keyObject = OSDynamicCast(OSSymbol, srcIterator->getNextObject())))
	{
		if (NULL != (destObject = dest->getObject(keyObject)) && (OSDynamicCast(OSDictionary, src->getObject(keyObject))))
			mergeProperties(destObject, src->getObject(keyObject)) ;
		else
			dest->setObject(keyObject, src->getObject(keyObject)) ;

		//keyObject->release() ;
	}
        srcIterator->release();
}

// IOService method overrides
// Always return false, because we load purely to update the device tree.
bool AudioDeviceTreeUpdater::start(IOService * provider )
{
    super::start(provider);

	OSArray*	arrayObj = OSDynamicCast(OSArray, getProperty("DeviceTreePatches"));
    
    do {
        if(!arrayObj)
            break;
            
        OSCollectionIterator*	patchIterator = OSCollectionIterator::withCollection(arrayObj) ;
    
        OSDictionary *	patchDict;
        while (NULL != (patchDict = OSDynamicCast(OSDictionary, patchIterator->getNextObject()))) {
            // Find the device tree node to patch.
            OSString *parentPath;
            OSString *nodeName;
            char nodePath[128];
            
            parentPath = OSDynamicCast(OSString, patchDict->getObject("Parent"));
            if(!parentPath)
                break;
            nodeName = OSDynamicCast(OSString, patchDict->getObject("Node"));
            if(!nodeName)
                break;
            
            if((parentPath->getLength() + nodeName->getLength() + 2) > sizeof(nodePath))
                    break;
            strcpy(nodePath, parentPath->getCStringNoCopy());
            strcat(nodePath, "/");
            strcat(nodePath, nodeName->getCStringNoCopy());
            
            IORegistryEntry *patchParent;
            IORegistryEntry *patch;
            patchParent = IORegistryEntry::fromPath(parentPath->getCStringNoCopy(), gIODTPlane);
            if(!patchParent)
                break;
            patch = IORegistryEntry::fromPath(nodePath, gIODTPlane);
            
            if(!patch) {
                // Have to create the node as well as set its properties
                patch = new IORegistryEntry;
                if(!patch)
                    break;                    
                if(!patch->init(OSDynamicCast(OSDictionary, patchDict->getObject("NodeData")))) {
                    patch->release();	
                    patch = NULL;
                    break;
                }
                // And add it into the device tree
                if(!patch->attachToParent(patchParent, gIODTPlane)) {
                    patch->release();
                    patch = NULL;
                    break;
                }
                patch->release ();
                patch->setName(nodeName->getCStringNoCopy());
            }
            else {
                mergeProperties(patch->getPropertyTable(), patchDict->getObject("NodeData"));
            }
            convertToOSData (patch);
        }
        patchIterator->release ();
    } while (false);
    return false;
}

void
AudioDeviceTreeUpdater::convertToOSData (IORegistryEntry *soundEntry)
{
	// Convert the model and soundobjects properties to OSData
	// This is a convenience method, to make editing the property lists easier
	
	OSString *model = OSDynamicCast(OSString, soundEntry->getProperty ("model"));
	if (model) {
		OSData *data = OSData::withBytes (model->getCStringNoCopy(), model->getLength());
		if (data) {
			soundEntry->setProperty ("model", data);
			data->release ();
		}	
	}
	
	OSArray *soundobjects = OSDynamicCast(OSArray, soundEntry->getProperty ("sound-objects"));
	if (soundobjects) {
		OSCollectionIterator* iter = OSCollectionIterator::withCollection (soundobjects);
		if (iter) {
			OSString *nextString;
			unsigned length = 0;
			while (nextString = OSDynamicCast (OSString, iter->getNextObject())) {
				length += nextString->getLength() + 1;
			}
			OSData *data = OSData::withCapacity (length + 1);
			if (data) {
				iter->reset ();
				while (nextString = OSDynamicCast (OSString, iter->getNextObject())) {
					data->appendBytes (nextString->getCStringNoCopy(), nextString->getLength());
					data->appendByte (0, 1);
				}
				data->appendByte (0, 1);
				soundEntry->setProperty ("sound-objects", data);
				data->release ();
			}
			iter->release ();
		}
	}
}
