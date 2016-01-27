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
#include <IOKit/IOKitKeys.h>
#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBHostInterface.h>

class MBIMProbe : public IOService
{

    OSDeclareDefaultStructors(MBIMProbe)

private:
    virtual bool        MergeDictionaryIntoProvider(IOService *  provider, OSDictionary *  mergeDict);
    virtual bool        MergeDictionaryIntoDictionary(OSDictionary *  sourceDictionary,  OSDictionary *  targetDictionary);
    virtual bool        SelectMsMbimConfiguration(IOUSBHostDevice	*device);
    virtual bool        CheckMsOSDescriptor(IOUSBHostDevice *device);
    virtual bool        getMsDescriptorDevice(IOUSBHostDevice *device);
    virtual bool        getMsDescriptorInterface(IOUSBHostInterface *interface);
public:
    IOUSBHostDevice	   *fpDevice;
    virtual bool        init(OSDictionary *properties = 0);
    virtual void        free(void);
    IOService          *probe(IOService *provider, SInt32 *score);
    virtual bool        start(IOService *provider);
    virtual void        stop(IOService *provider);

};

#endif
