/*
 *  AudioHardwarePower.h
 *  Apple02Audio
 *
 *  Created by lcerveau on Tue Feb 20 2001.
 *  Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 */

#ifndef _AUDIOHARDWAREPOWER_H_
#define _AUDIOHARDWAREPOWER_H_

#include "AudioHardwareCommon.h"
#include "Apple02Audio.h"

enum {
    kBasePowerObject,     			// basic mute 
    kProj6PowerObject = 6,			// for PowerBook G3
    kProj7PowerObject,	  			// for Screamer based G4 tower and iMac DVs
    kProj8PowerObject,	  			// for iBook with Clamshell
    kProj10PowerObject = 10,    	// for Pismo and Titanium
    kProj14PowerObject = 14,		// for Texas 3001 based Tower
    kProj16PowerObject = 16  		// for dual USB iBook
};

#pragma mark -
class AudioPowerObject : public OSObject {
    OSDeclareDefaultStructors(AudioPowerObject);

public:
    static  AudioPowerObject* createAudioPowerObject(Apple02Audio *pluginRef);

    virtual IOReturn setHardwarePowerOn();
    virtual IOReturn setHardwarePowerOff();
	virtual Boolean wantsIdleCalls (void) {return FALSE;}
	virtual void setIdlePowerState (void) {return;}
	virtual void setFullPowerState (void) {return;}
	virtual void setHardwarePowerIdleOn ( void ) { setHardwarePowerOn (); }
	virtual UInt32 GetTimeToChangePowerState (IOAudioDevicePowerState oldPowerState, IOAudioDevicePowerState newPowerState);

protected:    
    virtual bool init(Apple02Audio  *pluginRef);
    virtual void free();
    
    IODeviceMemory *powerReg;
    Apple02Audio *audioPluginRef;
    short 	hardwareType;
    OSArray *OutputPortMuteStates;
	UInt32	mMicroSecondsRequired;
};

#if 0
// For FW PB and PowerBook G4
#pragma mark -
class AudioProj10PowerObject : public AudioPowerObject {
     OSDeclareDefaultStructors(AudioProj10PowerObject);

public:
     static AudioProj10PowerObject* createAudioProj10PowerObject(Apple02Audio *pluginRef);

    virtual IOReturn setHardwarePowerOn();
    virtual IOReturn setHardwarePowerOff();
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

private:
    bool init(Apple02Audio *pluginRef);
    enum {
        durationMillisecond = 1,
        kTime10ms = durationMillisecond * 10,	// a delay of 10 ms is required after 
                                                //starting clocks before doing a recalibrate
        kDefaultPowerObjectReg	= 0x80800000,	// default base address
        kPowerObjectOffset	= 0x0000006F,		// offset to GPIO5 register
        kPowerObjectMask	= 0x02,				// determine if PWD is high or low
        kPowerOn		= 0x05,					// sound power enable
        kPowerOff		= 0x04					// sound power disable
    };
};
    
// For PowerBook G3
#pragma mark -
class AudioProj6PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj6PowerObject);

public:
    static AudioProj6PowerObject* createAudioProj6PowerObject(Apple02Audio *pluginRef);                                                            
    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

    bool init(Apple02Audio *pluginRef);

private:
    UInt32	*powerObjectReg;	// register Objectling power
	
    enum {
        durationMillisecond = 1,
        kTime10ms =	durationMillisecond * 10,   // a delay of 10 ms is required after 
												// starting clocks before doing a recalibrate
        kDefaultPowerObjectReg	= 0xF3000000,	// default base address
        kPowerObjectOffset		= 0x00000038,	// offset to feature Object register
        kPowerObjectMask		= 0x00003000,	// enable sound clock and power, big endian
        kPowerOn				= 0x00002000,	// sound clock enable, sound power enable, big endian
        kPowerOff				= 0x00001000,	// sound clock disable, sound power disable, big endian
        kPowerPWDBit			= 0x00001000,	// sound PWD bit on/off (PWD asserted low), big endian
        kPowerClkBit	        = 0x00002000	// sound SND_CLK_EN bit on/off (clocks on high), big endian

/*		These are the original version of these values, don't know if we need to use the little endian or big endian constants
        kPowerObjectMask		= 0x00300000,	// enable sound clock and power, little endian
        kPowerOn				= 0x00200000,	// sound clock enable, sound power enable, little endian
        kPowerOff				= 0x00100000,	// sound clock disable, sound power disable, little endian
        kPowerPWDBit			= 0x00100000,	// sound PWD bit on/off (PWD asserted low), little endian
        kPowerClkBit	        = 0x00200000	// sound SND_CLK_EN bit on/off (clocks on high), little endian */
    };
};

// For B&W G3
#pragma mark -
class AudioProj4PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj4PowerObject);

public:
    static AudioProj4PowerObject* createAudioProj4PowerObject(Apple02Audio *pluginRef);
								
    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();

private:
    UInt32 *powerObjectReg;	// register Objectling power
	
    enum {
        durationMillisecond =1,
        kTime10ms	= durationMillisecond * 10,		// a delay of 10 ms is required after 
                                                    // starting clocks before doing a recalibrate
        kDefaultPowerObjectReg	= 0x80800000,		// default base address
        kPowerObjectOffset	= 0x00000038,			// offset to feature Object register
        kPowerObjectMask	= 0x00100000,			// enable sound clock and power, endian swapped
        kPowerOn		= 0x00100000,				// sound power enable
        kPowerOff		= 0x00100000,				// sound power disable
        kPowerYosemite		= 0x00100000			// sound power enable for Yosemite
    };
};

// for iBooks in Clamshell
#pragma mark -
class AudioProj8PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj8PowerObject);
    
public:
    static AudioProj8PowerObject* createAudioProj8PowerObject(Apple02Audio *pluginRef);
								
    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

private:
    bool init(Apple02Audio *pluginRef);

};

// for Screamer based G4 and iMac DVs
#pragma mark -
class AudioProj7PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj7PowerObject);
    
public:
    static AudioProj7PowerObject* createAudioProj7PowerObject(Apple02Audio *pluginRef);
								
    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

private:
    bool init(Apple02Audio *pluginRef);
    UInt8	*powerObjectReg; 	// register Objectling power
    UInt32	layoutID;	 	// layout id of built in hardware.
    Boolean	restoreProgOut;		// remember and restore ProgOut
    UInt32	oldProgOut;
	
    enum {
        durationMillisecond = 1,
        kTime10ms = durationMillisecond * 10,		// a delay of 10 ms is required after 
													// starting clocks before doing a recalibrate
        kTime100ms = durationMillisecond * 100,		// a delay of 100 ms is required to let the 
													// anti-pop circuit do its thing
        kTime500ms = durationMillisecond * 500,		// a delay of 500 ms is required to let the 
													// anti-pop circuit do its thing
        kDefaultPowerObjectReg	= 0x80800000,		// default base address
        kPowerObjectOffset	= 0x0000006F,			// offset to GPIO5 register
        kPowerObjectMask	= 0x02,					// determine if PWD is high or low
        kPowerOn		= 0x05,						// sound power enable
        kPowerOff		= 0x04						// sound power disable
    };
};
    
// for Texas3001 Tower
#pragma mark -
class AudioProj14PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj14PowerObject);
    
public:
    static AudioProj14PowerObject* createAudioProj14PowerObject(Apple02Audio *pluginRef);

    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

private:
	bool init(Apple02Audio *pluginRef);
};
    
// for iBook dual USB
#pragma mark -
class AudioProj16PowerObject : public AudioPowerObject {
    OSDeclareDefaultStructors(AudioProj16PowerObject);
    
public:
    static AudioProj16PowerObject* createAudioProj16PowerObject(Apple02Audio *pluginRef);
								
    virtual IOReturn setHardwarePowerOff();	
    virtual IOReturn setHardwarePowerOn();
	virtual Boolean wantsIdleCalls (void);
	virtual void setIdlePowerState (void);
	virtual void setFullPowerState (void);
	virtual void setHardwarePowerIdleOn ( void );

private:
	bool init(Apple02Audio *pluginRef);
};

#endif
#endif

