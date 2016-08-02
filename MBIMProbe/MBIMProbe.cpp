//
//  MBIMProbe.cpp
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 30.04.2014.
//  Copyright © 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//


#include "MBIMProbe.h"

#define super IOService

OSDefineMetaClassAndStructors(MBIMProbe, IOService)

//
// Here's our entry point
//
IOService * MBIMProbe::probe(IOService *provider, SInt32 *score){

#ifdef DEBUG
    IOLog("-%s[%p]::probe Hello World!\n", getName(), this);
#endif
    const IORegistryPlane * usbPlane = getPlane(kIOUSBPlane);
    IOUSBHostDevice       * device   = OSDynamicCast(IOUSBHostDevice, provider);
    IOReturn                status;
    
    if (device && usbPlane) {
        
        //Get exclusive access to the USB device
        device->open(this);
        
        //
        //Let's find out a few things about this device.
        //First: The number of configurations
        //
        uint8_t configNumber  = 0;
        StandardUSB::DeviceRequest request;
        request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
        request.bRequest      = kDeviceRequestGetConfiguration;
        request.wValue        = 0;
        request.wIndex        = 0;
        request.wLength       = sizeof(configNumber);
        
        uint32_t bytesTransferred = 0;
        
        device->deviceRequest(this, request, &configNumber, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
        
//        const StandardUSB::ConfigurationDescriptor* configDescriptor = device->getConfigurationDescriptor(configNumber);
        
#ifdef DEBUG
        IOLog("-%s[%p]::probe We have the USB device exclusively. Vendor ID: %x. Product ID: %x. Config: %d. Now checking for the MSFT100 descriptor\n", getName(), this, USBToHost16(device->getDeviceDescriptor()->idVendor), USBToHost16(device->getDeviceDescriptor()->idProduct), configNumber);
#endif
        /* 
         * PRE:  There is a MSFT100 descriptor on device.
         * POST: Device is microsoft. Handle as MS device.
         */
        if (checkMsOsDescriptor(device)==kIOReturnSuccess){
            
            // TODO Set the current configuration again. It might go away otherwise
            void     *dataBuffer;
            uint32_t  dataBufferSize;
#ifdef DEBUG
            IOLog("-%s[%p]::probe We have a MSFT100 descriptor. We're now getting the COMPATID descriptor\n", getName(), this);
            IOSleep(1000);
#endif
            
            // Read the MS OS Compat Descriptor v1
            // dataBuffer and dataBufferSize set in callee
            status = getMsDescriptor(device, 0, MS_OS_10_REQUEST_EXTENDED_COMPATID, &dataBuffer, &dataBufferSize);
            if (status!=kIOReturnSuccess) {
                device->close(this);
                return NULL;
            }


            // First 18 bytes are USB header. Data at byte 19
            uint64_t descriptorData = USBToHost64(*((uint64_t*)dataBuffer + 18));
            uint64_t subDescriptorData = USBToHost64(*((uint64_t*)dataBuffer + 18 + 8));
            
            //Now let's act upon the descriptor.
            switch (descriptorData){
                case 0:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: Null Compatible ID\n", getName(), this);
                    break;
                case MS_OS_10_RNDIS_COMPATIBLE_ID:
                    //Let's also look for something else before we
                    //PublishRNDISConfiguration(device);
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: RNDIS\n", getName(), this);
                    break;
                case MS_OS_10_MTP_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: MTP\n", getName(), this);
                    break;
                case MS_OS_10_PTP_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: PTP\n", getName(), this);
                    break;
                case MS_OS_10_WINUSB_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: WINUSB\n", getName(), this);
                    break;
                case MS_OS_10_XUSB20_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: X USB 2.0\n", getName(), this);
                    break;
                case MS_OS_10_MBIM_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: MBIM on default config.\n", getName(), this);
                    //setconfig(currentConfig);
                    //PublishMBIMConfiguration(device)
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                case MS_OS_10_ALTRCFG_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: MBIM", getName(), this);
                    switch (subDescriptorData){
                        case MS_OS_10_ALT2_SUBCOMPATIBLE_ID:
                            IOLog(" on config 2");
                            //setconfig(2);
                            //PublishMBIMConfiguration(device);
                            break;
                        case MS_OS_10_ALT3_SUBCOMPATIBLE_ID:
                            IOLog(" on config 3");
                            //setconfig(3);
                            //PublishMBIMConfiguration(device);
                            break;
                        case MS_OS_10_ALT4_SUBCOMPATIBLE_ID:
                            IOLog(" on config 4");
                            //setconfig(4);
                            //PublishMBIMConfiguration(device);
                            break;
                        default:
                            IOLog(" incorrect Subcompatible descriptor\n");
                            break;
                    }
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                    break;
                case MS_OS_10_CDC_WMC_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: CDC-WMC on default config.\n", getName(), this);
                    //setconfig(currentConfig);
                    //PublishWMCConfiguration(device)
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                case MS_OS_10_WMCALTR_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe MS Compatible Descriptor: CDC-WMC", getName(), this);
                    switch (subDescriptorData){
                        case MS_OS_10_ALT2_SUBCOMPATIBLE_ID:
                            IOLog(" on config 2");
                            //setconfig(2);
                            //PublishWMCConfiguration(device);
                            break;
                        case MS_OS_10_ALT3_SUBCOMPATIBLE_ID:
                            IOLog(" on config 3");
                            //setconfig(3);
                            //PublishWMCConfiguration(device);
                            break;
                        case MS_OS_10_ALT4_SUBCOMPATIBLE_ID:
                            IOLog(" on config 4");
                            //setconfig(4);
                            //PublishWMCConfiguration(device);
                            break;
                        default:
                            IOLog(" with incorrect Subcompatible descriptor\n");
                            break;
                    }
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                    break;
                case MS_OS_10_BLUTUTH_COMPATIBLE_ID:
                    IOLog("-%s[%p]::probe Bluetooth", getName(), this);
                    switch (descriptorData) {
                        case MS_OS_10_NULL_SUBCOMPATIBLE_ID:
                            IOLog(" incorrect Subcompatible descriptor\n");
                            break;
                        case MS_OS_10_BT11_SUBCOMPATIBLE_ID:
                            IOLog(" v1.1\n");
                            break;
                        case MS_OS_10_BT12_SUBCOMPATIBLE_ID:
                            IOLog(" v1.2\n");
                            break;
                        case MS_OS_10_EDR2_SUBCOMPATIBLE_ID:
                            IOLog(" v2.0+EDR\n");
                            break;
                        default:
                            IOLog(" with incorrect Subcompatible descriptor\n");
                            break;
                    }
                default:
                    IOLog("-%s[%p]::probe Incorrect Compatible descriptor\n", getName(), this);
                    break;
            }

        }
        
        IOLog("-%s[%p]::probe We shouldn't even get here on K4201-Z\n", getName(), this);

        device->close(this);
        
    }

    return NULL;
}


//
// Check for the presence of an MS OS Descriptor.
//
IOReturn MBIMProbe::checkMsOsDescriptor(IOUSBHostDevice *device){  
    uint16_t                               msftStringSize = 18;
    char                                  *msftString = (char *)IOMalloc(msftStringSize);
//    StandardUSB::Descriptor               *msftStringDescriptor;
    bzero(msftString, msftStringSize);
    
    StandardUSB::DeviceRequest request;
    request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest      = kDeviceRequestGetDescriptor;
    request.wValue        = (0x3 << 8) | 0xEE;
    request.wIndex        = 0;
    request.wLength       = 0x12;
    
    uint32_t bytesTransferred = 0;
    uint32_t completionTimeoutMs = kUSBHostDefaultControlCompletionTimeoutMS;
    IOReturn Error = device->deviceRequest(this, request, (void *)msftString, bytesTransferred, completionTimeoutMs);
    
    if (Error){
        IOLog("-%s[%p]::CheckMsOsDescriptor: Couldn't get the descriptor, transferred %d bytes: %x\n",getName(), this, bytesTransferred, Error);
        return kIOReturnNotFound;
    }
    IOLog("-%s[%p]::CheckMsOsDescriptor: Got the MSFT100 descriptor, transferred %d bytes\n",getName(), this, bytesTransferred);
    
    IOLog("-%s[%p]::CheckMsOsDescriptor: ", getName(), this);
    for(int i = 0; i < 18; i++) {
        IOLog("0x%0x(%c) ", *(uint8_t*)(msftString + i), *(char*)(msftString + i));
    }
    IOLog ("\n");
    
    uint64_t *highBytesString       = (uint64_t*)(void*)(msftString+2);
    uint32_t *medBytesString        = (uint32_t*)(void*)(msftString+10);
    uint16_t *lowBytesString        = (uint16_t*)(void*)(msftString+14);

    uint64_t  highBytesRefString;
    uint32_t  medBytesRefString;
    uint16_t  lowBytesRefString;
    //TLV: 0x12(\^R) 0x3(\^C)
    //High:   0x4d(M) 0x0() 0x53(S) 0x0() 0x46(F) 0x0() 0x54(T) 0x0()
    //Med:    0x31(1) 0x0() 0x30(0) 0x0()
    //Low:    0x30(0) 0x0()
    //Cookie: 0x4(\^D) 0x0()
    highBytesRefString=0x005400460053004d; //M\0S\0\F\0T\0 in some order
    medBytesRefString =0x00300031; //1\00\00
    lowBytesRefString =0x0030;
    
    
    IOLog("-%s[%p]::CheckMsOsDescriptor: High: 0x%llx 0x%llx \nMed: 0x%x 0x%x\nLow: 0x%x 0x%x\n", getName(), this, *highBytesString, highBytesRefString, *medBytesString, medBytesRefString, *lowBytesString, lowBytesRefString);
    
    // Let's see if we have MSFT100 in
    // We should also compare in wide using msftRefStringUnicode
    // FIXME: Also check  && msftString->bLength > StandardUSB::kDescriptorSize
    if (msftString != NULL){
        IOLog("-%s[%p]::CheckMsOsDescriptor: found\n", getName(), this);
        
        // We should compare only the first 14 bytes. The last double byte
        // is the bRequest value + ContainerID and can be different.
        // Comparing two uint128_t would have been much more elegant.
        if((highBytesRefString== *highBytesString) &&
           (medBytesRefString == *medBytesString ) &&
           (lowBytesRefString == *lowBytesString)) {
            IOLog("-%s[%p]::CheckMsOsDescriptor: IT WOOOORKS\n", getName(), this);
            return kIOReturnSuccess;
        }
    }
    IOLog("-%s[%p]::CheckMsOsDescriptor: not found\n", getName(), this);
    return kIOReturnNotFound;
}


//
// Gets an MS OS Descriptor V1 for the Device if interface is NULL
// or interface if an interface number is specified.
//
IOReturn MBIMProbe::getMsDescriptor(IOUSBHostDevice *device, uint16_t interfaceNumber, const uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize){
    //
    //Validating the input parameters
    //
    //First one: We can only request COMPATID descriptor device-wide
    if ((interfaceNumber>0) && (DescriptorType==MS_OS_10_REQUEST_EXTENDED_COMPATID)){
        return kIOReturnBadArgument;
    }
    //Second one: We can only request COMPATID or Extended Properties. Genre is not supported/specified by MS
    if ((DescriptorType!=MS_OS_10_REQUEST_EXTENDED_COMPATID)||(DescriptorType!=MS_OS_10_REQUEST_EXTENDED_PROPERTIES)){
        return kIOReturnBadArgument;
    }

    //Get the cookie
    const StringDescriptor *stringDescriptor = device->getStringDescriptor(0xEE,0x00);
    
    
    //If we've got the cookie, let's play with it
    if (stringDescriptor != NULL && stringDescriptor->bLength > StandardUSB::kDescriptorSize) {
        
        // Prepare request for DescriptorType and write into dataBuffer
        
        DeviceRequest    request;
        uint8_t          bRequest  = stringDescriptor->bString[15];
        
        request.bmRequestType      = 0xC0;
        request.bRequest           = bRequest;                     // I can't explain why it's declared as single byte.
        request.wIndex             = DescriptorType;               // The requested descriptor.
        if (interfaceNumber > 0) {                                 // This only applies to Extended Properties as.
            request.bmRequestType = 0xC1;                          // CompatID doesn't have a per-interface
        } else {
            request.bmRequestType = 0xC0;
        }
        if (DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID){ // If it's MS_OS_10_REQUEST_EXTENDED_COMPATID
            request.wValue         = 0;                            //    we request DeviceWide
            request.wLength        = 0x28;                         //    and it only takes 40 bytes
           *dataBufferSize         = 0x28;
        } else {                                                   // If it's MS_OS_10_REQUEST_EXTENDED_PROPERTIES
            request.wValue         = interfaceNumber;              //    we specify the interface (if one is requested) in the low
                                                                   //    bytes and the page number in the high bytes.
                                                                   //    Since we're just getting the header, high=0 so we just put
                                                                   //    the interface number in USB order.
            request.wLength   = 0x10;                              //    The actual request will be made later.
           *dataBufferSize    = 0x10;
        }
        
        
        *dataBuffer = IOMalloc(request.wLength);
        
        if (dataBuffer == NULL)
            return kIOReturnVMError;
        
        IOReturn status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
        
        if(status != kIOReturnSuccess)
            return status;

        // The CompatID descriptor is a single request of known size so we can finish here
        // It's clear that below we're talking about Extended Properties
        if (DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID)
            return kIOReturnSuccess;

        // We can cast the variable like this regardless of the actual descriptor type
        // since both descriptors get transferred the same way and they both provide the
        // much needed dwLength.
        
        MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER *header = (MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER*) *dataBuffer;

        //If the descriptor is smaller than 4K we'll get it in one shot
        //It should be 4K-sizeof(usbTransferHeader)
        if (header->dwLength <= 0x1000) {
            *dataBuffer           = IOMalloc(header->dwLength);
            *dataBufferSize       = header->dwLength;
            request.bRequest      = bRequest;
            request.wValue        = interfaceNumber;
            request.wLength       = header->dwLength;
            request.wIndex        = DescriptorType;
            //If requesting for an interface, we have a different bmRequestType
            if (interfaceNumber > 0) {
                request.bmRequestType = 0xC1;
            } else {
                request.bmRequestType = 0xC0;
            }

            status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
            return kIOReturnSuccess;
                    
        //If it's bigger than 4K, we need to make paged transfers.
        } else {
                    
            uint32_t   bytesTransferred   = 0;
            uint32_t   remainingBytes     = header->dwLength;
                      *dataBuffer         = IOMalloc(header->dwLength);
                      *dataBufferSize     = header->dwLength;
            uint8_t   *iterator           = (uint8_t*)*dataBuffer;
            void      *tempBuffer         = NULL;
                    
            if (dataBuffer == NULL) return kIOReturnVMError;
                    
            while (remainingBytes > 0) {
                
                //High Bytes are interfaceNumber, low bytes are page number (bytes DIV pagesize +1)
                request.wValue        = interfaceNumber|((remainingBytes % 0x1000 + 1) << 8);
                request.bRequest      = bRequest;
                request.wIndex        = DescriptorType;
                
                //Let's request at most the remaining number of bytes
                if(remainingBytes > 0x1000) {
                    request.wLength   = 0x1000;
                } else {
                    request.wLength   = remainingBytes;
                }
                //If requesting for an interface, we have a different bmRequestType
                if (interfaceNumber > 0) {
                    request.bmRequestType = 0xC1;
                } else {
                    request.bmRequestType = 0xC0;
                }
                
                status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                //
                // TODO: Should we transfer everything after the header or do the later pages exclude the header?
                //
                if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator+sizeof(StandardUSB::Descriptor), bytesTransferred-sizeof(StandardUSB::Descriptor));
                        remainingBytes -= (bytesTransferred-sizeof(StandardUSB::Descriptor));
            }

            return kIOReturnSuccess;
        }//else
        
    } else return kIOReturnNotFound;//Cookie not found
    
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
