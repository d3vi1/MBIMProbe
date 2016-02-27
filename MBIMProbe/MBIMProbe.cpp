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

    IOLog("Hello World!");
    const IORegistryPlane * usbPlane = getPlane(kIOUSBPlane);
    IOUSBHostDevice       * device   = OSDynamicCast(IOUSBHostDevice, provider);
    IOReturn                status;
    
    if (device && usbPlane) {
        
        //Get exclusive access to the USB device
        device->open(this);
        
        
        IOLog("We have the USB device exclusively. Now checking for the MSFT100 descriptor");
        IOSleep(10000);
        
        /* 
         * PRE:  There is a MSFT100 descriptor on device.
         * POST: Device is microsoft. Handle as MS device.
         */
        if (checkMsOsDescriptor(device)==kIOReturnSuccess){
            
            // TODO Set the current configuration again. It might go away otherwise
            void     *dataBuffer;
            uint32_t  dataBufferSize;
            
            IOLog("We have a MSFT100 descriptor. We're now getting the COMPATID descriptor");
            IOSleep(1000);

            
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
                    IOLog("MS Compatible Descriptor: Null Compatible ID\n");
                    break;
                case MS_OS_10_RNDIS_COMPATIBLE_ID:
                    //Let's also look for something else before we
                    //PublishRNDISConfiguration(device);
                    IOLog("MS Compatible Descriptor: RNDIS\n");
                    break;
                case MS_OS_10_MTP_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: MTP\n");
                    break;
                case MS_OS_10_PTP_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: PTP\n");
                    break;
                case MS_OS_10_WINUSB_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: WINUSB\n");
                    break;
                case MS_OS_10_XUSB20_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: X USB 2.0\n");
                    break;
                case MS_OS_10_MBIM_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: MBIM on default config.\n");
                    //setconfig(currentConfig);
                    //PublishMBIMConfiguration(device)
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                case MS_OS_10_ALTRCFG_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: MBIM");
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
                            IOLog(" on config 2");
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
                    IOLog("MS Compatible Descriptor: CDC-WMC on default config.\n");
                    //setconfig(currentConfig);
                    //PublishMBIMConfiguration(device)
                    IOFree(dataBuffer, dataBufferSize);
                    device->close(this);
                    return NULL;
                case MS_OS_10_WMCALTR_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: CDC-WMC");
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
                            IOLog(" on config 2");
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
                case MS_OS_10_BLUTUTH_COMPATIBLE_ID:
                    IOLog("Bluetooth");
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
                            IOLog(" incorrect Subcompatible descriptor\n");
                            break;
                    }
                default:
                    IOLog("Incorrect Compatible descriptor\n");
                    break;
            }

        }
        
        IOLog("We shouldn't even get here on K4201-Z");

        device->close(this);
        
    }

    return NULL;
}


//
// Check for the presence of an MS OS Descriptor.
//
IOReturn MBIMProbe::checkMsOsDescriptor(IOUSBHostDevice *device){  
    IOUSBDevRequest	                       request;
    IOUSBConfigurationDescHeader           descriptorHdr;
    IOReturn                               kernelError;
    IOUSBConfigurationDescriptorPtr       *descriptor;
    const StringDescriptor                *msftString;
    
    msftString = device->getStringDescriptor(0xEE,0x0);
    uint64_t *highBytesString       = (uint64_t*)(void*) &msftString;
    uint32_t *medBytesString        = (uint32_t*)(void*)(&msftString+2);
    uint16_t *lowBytesString        = (uint16_t*)(void*)(&msftString+4);

    const wchar_t msftRefString[8]     = L"MSFT100";
    uint64_t  *highBytesRefString      = (uint64_t*)(void*) &msftRefString;
    uint32_t  *medBytesRefString       = (uint32_t*)(void*)(&msftRefString+2);
    uint16_t  *lowBytesRefString       = (uint16_t*)(void*)(&msftRefString+4);
    
    // Let's see if we have MSFT100 in
    // We should also compare in wide using msftRefStringUnicode
    if (msftString != NULL && msftString->bLength > StandardUSB::kDescriptorSize){

        // We should compare only the first 14 bytes. The last double byte
        // is the bRequest value + ContainerID and can be different.
        // Comparing two uint128_t would have been much more elegant.
        if((*highBytesRefString== *highBytesString) &&
           (*medBytesRefString == *medBytesString ) &&
           (*lowBytesRefString == *lowBytesString))
        return kIOReturnSuccess;
    }
    
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