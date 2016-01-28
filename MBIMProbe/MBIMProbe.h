//
//  MBIMProbe.h
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 30.04.2014.
//  Copyright (c) 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#ifndef _MBIMProbe_h
#define _MBIMProbe_h

#include "MSOSDescriptorV1.h"
#include "MSOSDescriptorV2.h"
#include "3GModems.h"
#include <IOKit/IOKitKeys.h>
#include <IOKit/usb/USB.h>
#include <IOKit/usb/StandardUSB.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
//#include <IOKit/usb/IOUSBMassStorageClass.h>
#include <sys/_endian.h>

class MBIMProbe : public IOService
{

    OSDeclareDefaultStructors(MBIMProbe)

private:
    virtual bool        MergeDictionaryIntoProvider(IOService *  provider, OSDictionary *  mergeDict);
    virtual bool        MergeDictionaryIntoDictionary(OSDictionary *  sourceDictionary,  OSDictionary *  targetDictionary);
    virtual bool        selectMbimConfiguration(IOUSBHostDevice	*device);
    virtual IOReturn    checkMsOsDescriptor(IOUSBHostDevice *device);
    virtual IOReturn    getMsDescriptor(IOUSBHostDevice *device, uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize);
    virtual IOReturn    getMsDescriptor(IOUSBHostDevice *device, uint16_t interfaceNumber, uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize);
    virtual IOReturn    huaweiMode1(IOUSBHostInterface *interface);
    virtual IOReturn    huaweiMode2(IOUSBHostInterface *interface);
    virtual IOReturn    ejectCD(IOUSBHostInterface *interface);
    virtual IOReturn    haveCDCinterfaces();
    virtual IOReturn    haveRNDISInterfaces();
public:
    IOUSBHostDevice	   *fpDevice;
    virtual bool        init(OSDictionary *properties = 0);
    virtual void        free(void);
    IOService          *probe(IOService *provider, SInt32 *score);
    virtual bool        start(IOService *provider);
    virtual void        stop(IOService *provider);

};

#endif
