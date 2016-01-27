//
//  MBIMProbe.cpp
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 30.04.2014.
//  Copyright (c) 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//


#include "MBIMProbe.h"

#define super IOService

OSDefineMetaClassAndStructors(MBIMProbe, IOService)

IOService * MBIMProbe::probe(IOService *provider, SInt32 *score){
    const IORegistryPlane            *usbPlane = getPlane(kIOUSBPlane);
    
    IOUSBHostDevice *device = OSDynamicCast(IOUSBHostDevice, provider);
    
    if (device && usbPlane)
    {
        
        if(SelectMsMbimConfiguration(device)){
            //PublishMBIMConfiguration(device);
            return NULL;
        }
        
        
    }
    return NULL;
}

bool MBIMProbe::CheckMsOSDescriptor(IOUSBHostDevice *device){
    IOUSBDevRequest	                       request;
    IOUSBConfigurationDescHeader           descriptorHdr;
    IOReturn                               kernelError;
    IOUSBConfigurationDescriptorPtr       *descriptor;
    const StringDescriptor                *msftString;
    
    msftString = device->getStringDescriptor(0xEE,0x0);
    
    const char msftRefString[8] = "MSFT100";
    const wchar_t msftRefWideString[8] = L"MSFT100";
    
    //Let's see if we have MSFT100 in narrow
    //We should also compare in wide using msftRefStringUnicode
    if (msftString->bLength > 0){

        for (int i=0; i<14; i++) {
            IOLog("Comparing MSFT OS Descriptor Wide String wchar %i\n", i);
            if(msftString->bString[i]!=msftRefWideString[i]) goto nodescriptor;
        }

        IOLog("Wide Descriptor succeeded.\n");
        IOLog("Descriptor bMS_VendorCode: %x\n", msftString->bString[14]);
        
        for (int i=0; i<7; i++) {
            IOLog("Comparing MSFT OS Descriptor String char %i\n", i);
            if(msftString->bString[i]!=msftRefString[i]) goto nodescriptor;
        }
        
        IOLog("Narrow and incorrect descriptor.\n");
        IOLog("Descriptor bMS_VendorCode: %x\n", msftString->bString[7]);
        
    }
    
    
nodescriptor:
    IOLog("Device doesn't have MSFT String Descriptor\n.");

}

bool MBIMProbe::getMsDescriptorDevice(IOUSBHostDevice *device){

    //bmRequestType=0xC0
    //bRequest=bMS_VendorCode
    //wValue=PageNumber start with 0 << InterfaceNumber always 0
    //wIndex = 0x04
    //wLength = 0x10 (just the tip ^H^H^H^Hheader)
}

bool MBIMProbe::getMsDescriptorInterface(IOUSBHostInterface *interface){
    
}

bool MBIMProbe::SelectMsMbimConfiguration(IOUSBHostDevice	*device){

    uint8_t                          numConfigs          = device->getDeviceDescriptor()->bNumConfigurations;
    const ConfigurationDescriptor	*configDescriptor = device->getConfigurationDescriptor();		// configuration descriptor
    uint8_t                          configIndex       = configDescriptor->bConfigurationValue;

    if (numConfigs < 1){
        IOLog("MBIMProbe::SelectMBIMConfiguration not enough configurations: %d\n", numConfigs);
    }
    
    device->open(this);
    if (CheckMsOSDescriptor(device)){
        //Check for MSOS Descriptor v1
        //Check for MSOS Descriptor v2
    }
    
    //
    //Loop through all the configurations starting from the current one.
    //
    for (configIndex=configDescriptor->bConfigurationValue; configIndex==(configDescriptor->bConfigurationValue-1); configIndex++){
        IOLog("MBIMProbe::SelectMBIMConfiguration Checking configuration %d\n", configIndex);
        configDescriptor = device->getConfigurationDescriptor(configIndex);

        
        if (!configDescriptor) {
            IOLog("MBIMProbe::SelectMBIMConfiguration Could not get full configuration descriptor for config %d\n", configIndex);
        } else {
            
            //If we've reached the max number of configurations,
            //let's reset to the first one and continue from there
            if(configIndex==numConfigs){
                configIndex=1;
            }
            
        }
        
        
    }
    device->close(this);
    return true;
}

bool MBIMProbe::MergeDictionaryIntoProvider(IOService * provider, OSDictionary * dictionaryToMerge){
    return true;
}

bool MBIMProbe::MergeDictionaryIntoDictionary(OSDictionary * parentSourceDictionary,  OSDictionary * parentTargetDictionary){
    return true;
}

bool MBIMProbe::start(IOService *provider){
	bool ret;
	   
	IOLog("-%s[%p]::start - This is handled by the superclass\n", getName(), this);
	ret = super::start(provider);
	if (!ret){
		IOLog("-%s[%p]::start -  super::start failed\n", getName(), this);
	}
	
	return ret;
}

void MBIMProbe::stop(IOService *provider){
    
    IOLog("-%s[%p]::stop - calling super::stop\n", getName(), this);
    super::stop(provider);
}

bool MBIMProbe::init(OSDictionary *properties){
    if (super::init(properties) == false)
    {
        IOLog("-%s[%p]::init - initialize super failed\n", getName(), this);
        return false;
    }
    return true;
}

void MBIMProbe::free(){
    super::free();
    return;
}