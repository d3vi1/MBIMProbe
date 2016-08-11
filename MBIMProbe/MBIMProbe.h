//
//  MBIMProbe.h
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 30.04.2014.
//  Copyright © 2016:
//        Răzvan Corneliu C.R. VILT
//        Vlad A. VERGU
//  All rights reserved.
//

#ifndef _MBIMProbe_h
#define _MBIMProbe_h

#include "MSOSDescriptorV1.h"
#include "MSOSDescriptorV2.h"
#include "3GModems.h"
#include <IOKit/IOKitKeys.h>
//#include <IOKit/usb/USB.h>
//#include <IOKit/usb/StandardUSB.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
#include <sys/_endian.h>

#ifdef DEBUG
#define log(x, ...)  IOLog("%s::%s " x , getName(), __FUNCTION__, ##__VA_ARGS__)
#define log_cont(x, ...)  IOLog( x, ##__VA_ARGS__)
#else
#define log(x, ...)
#define log_cont(x, ...)
#endif

class MBIMProbe : public IOService
{

    OSDeclareDefaultStructors(MBIMProbe)

private:
    virtual uint8_t     discoverDevice               (IOUSBHostDevice *device);
    virtual IOReturn    checkMsOsDescriptor          (IOUSBHostDevice *device,
                                                      uint8_t *cookie);
    virtual IOReturn   getSmallMsDescriptor          (IOUSBHostDevice *device,
                                                      uint8_t cookie,
                                                      uint16_t interfaceNumber,
                                                      const uint16_t DescriptorType,
                                                      void **dataBuffer,
                                                      uint32_t *dataBufferSize);
    virtual IOReturn    getMsDescriptor              (IOUSBHostDevice *device,
                                                      uint8_t cookie,
                                                      uint16_t interfaceNumber,
                                                      uint16_t DescriptorType,
                                                      void **dataBuffer,
                                                      uint32_t *dataBufferSize);
    virtual uint8_t     parseMSDescriptor            (void *dataBuffer, uint8_t currentConfigNumber);
public:
    IOUSBHostDevice	   *fpDevice;
    virtual bool        init(OSDictionary *properties = 0);
    virtual void        free(void);
    IOService          *probe(IOService *provider, SInt32 *score);
    virtual bool        start(IOService *provider);
    virtual void        stop(IOService *provider);

};

#endif
