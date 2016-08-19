//
//  HuaweiProbe.cpp
//  WWAN Drivers
//
//  Created by Răzvan Corneliu C.R. VILT on 19/08/2016.
//  Copyright © 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#include "HuaweiProbe.hpp"

#define super IOService

OSDefineMetaClassAndStructors(HuaweiProbe, IOService)

//
// Here's our entry point
//
IOService * HuaweiProbe::probe(IOService *provider, SInt32 *score){
    
    const IORegistryPlane * usbPlane = getPlane(kIOUSBPlane);
    IOUSBHostDevice       * device   = OSDynamicCast(IOUSBHostDevice, provider);
    IOReturn                status;
    
    if(!(device && usbPlane)){
        return 0;
    }
    
    //Get exclusive access to the USB device
    bool openSucc = device->open(this);
    if(!openSucc){
        log("Failed to open device\n");
        return 0;
    }
    
    uint8_t configNumber  = getConfig(device);
    
    log("Looking for USB Mass Storage config\n");
    
    
    
    return this;
    
}



uint8_t HuaweiProbe::getConfig(IOUSBHostDevice *device) {
    //Let's find out a few things about this device.
    //First: The number of configurations
    uint8_t configNumber;
    StandardUSB::DeviceRequest request;
    request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest      = kDeviceRequestGetConfiguration;
    request.wValue        = 0;
    request.wIndex        = 0;
    request.wLength       = sizeof(configNumber);
    uint32_t bytesTransferred = 0;
    device->deviceRequest(this, request, &configNumber, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
    //And then, the idVendor, idProduct, etc.
#ifdef DEBUG
    log("We have the USB device exclusively. Vendor ID: %x. Product ID: %x. Config: %d. Now checking for the MSFT100 descriptor\n", USBToHost16(device->getDeviceDescriptor()->idVendor), USBToHost16(device->getDeviceDescriptor()->idProduct), configNumber);
#endif
    return configNumber;
}


//
// Find all MassStorage/SCSI/BulkOnly Interfaces and send a message to them
//
uint8_t HuaweiProbe::ParseMassStorage(IOUSBHostDevice *device){
    const IOUSBDeviceDescriptor *device_descriptor = (IOUSBDeviceDescriptor *) device->getDeviceDescriptor();
    IOLog("-%s[%p]::%s @0 \n", getName(), this, __FUNCTION__);
    
    for(int j = 0; j < device_descriptor->bNumConfigurations; j++){
        IOLog("-%s[%p]::%s @1 \n", getName(), this, __FUNCTION__);
        
        uint8_t currentConfig = discoverDevice(device);
        const ConfigurationDescriptor *configDescriptor = device->getConfigurationDescriptor(j);
        
        currentConfig = getConfig(device);
        
        if(configDescriptor == NULL){
            IOLog("-%s[%p]::%s Could not get getConfigurationDescriptor: %x\n", getName(), this, __FUNCTION__, j);
            continue;
        }
        IOLog("-%s[%p]::%s @2: at configuration %x\n", getName(), this, __FUNCTION__, config->bConfigurationValue);
        
        const InterfaceDescriptor *interface_descriptor;
        do {
            interface_descriptor = getNextInterfaceDescriptor(configDescriptor, interface_descriptor);
            if(interface_descriptor != NULL) {
                IOLog("-%s[%p]::%s got an iface descriptor class %x subclass %x\n", getName(), this, __FUNCTION__, interface_descriptor->bInterfaceClass, interface_descriptor->bInterfaceSubClass);
                
                if(interface_descriptor->bInterfaceClass == kUSBMassStorageInterfaceClass && interface_descriptor->bInterfaceSubClass == kUSBMassStorageSCSISubClass && interface_descriptor->bInterfaceProtocol == kMSCProtocolBulkOnly) {
                    
                    
                }
            }
        } while(interface_descriptor != NULL);
    }
    return NULL;
}



//
// Just call the superclass to do the starting
//
bool HuaweiProbe::start(IOService *provider){
    bool ret;
	   
    log("This is handled by the superclass\n");
    ret = super::start(provider);
    if (!ret){
        log("super::start failed\n");
    }
    
    return ret;
}



//
// Just call the superclass to do the stoping
//
void HuaweiProbe::stop(IOService *provider){
    
    log("calling super::stop\n");
    super::stop(provider);
}



//
// Just call the superclass to do the initializing
//
bool HuaweiProbe::init(OSDictionary *properties){
    if (super::init(properties) == false)
    {
        log("initialize super failed\n");
        return false;
    }
    return true;
}



//
// Just call the superclass to do the freeing
//
void HuaweiProbe::free(){
    super::free();
    return;
}
