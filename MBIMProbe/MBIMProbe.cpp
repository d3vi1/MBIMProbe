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

    const IORegistryPlane * usbPlane = getPlane(kIOUSBPlane);
    IOUSBHostDevice       * device   = OSDynamicCast(IOUSBHostDevice, provider);
    IOReturn                status;
    
    if (device && usbPlane) {
        
        //Get exclusive access to the USB device
        device->open(this);
        
#ifdef DEBUG
        //Let's find out a few things about this device.
        //First: The number of configurations
        uint8_t configNumber  = 0;
        StandardUSB::DeviceRequest request;
        request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
        request.bRequest      = kDeviceRequestGetConfiguration;
        request.wValue        = 0;
        request.wIndex        = 0;
        request.wLength       = sizeof(configNumber);
        uint32_t bytesTransferred = 0;
        device->deviceRequest(this, request, &configNumber, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
        
        //And then, the idVendor, idProduct, etc.
        IOLog("-%s[%p]::probe We have the USB device exclusively. Vendor ID: %x. Product ID: %x. Config: %d. Now checking for the MSFT100 descriptor\n", getName(), this, USBToHost16(device->getDeviceDescriptor()->idVendor), USBToHost16(device->getDeviceDescriptor()->idProduct), configNumber);
#endif
        /* 
         * PRE:  There is a MSFT100 descriptor on device.
         * POST: Device is microsoft. Handle as MS device.
         */
        uint8_t cookie;
        if (checkMsOsDescriptor(device, &cookie) == kIOReturnSuccess){
            
            // TODO Set the current configuration again. It might go away otherwise
            void     *dataBuffer;
            uint32_t  dataBufferSize;
#ifdef DEBUG
            IOLog("-%s[%p]::%s We have a MSFT100 descriptor with cookie %x. We're now getting the COMPATID descriptor\n", getName(), this, __FUNCTION__, cookie);
#endif
            
            // Read the MS OS Compat Descriptor v1
            // dataBuffer and dataBufferSize set in callee
            status = getMsDescriptor(device, cookie, 0, MS_OS_10_REQUEST_EXTENDED_COMPATID, &dataBuffer, &dataBufferSize);
            if (status != kIOReturnSuccess) {
                IOLog("-%s[%p]::%s We couldn't get the MS_OS_10_REQUEST_EXTENDED_COMPATID descriptor with error: %x\n", getName(), this, __FUNCTION__, status);
                device->close(this);
                return NULL;
            }
            
            IOLog("-%s[%p]::%s We actually succeeded in getting MS_OS_10_REQUEST_EXTENDED_COMPATID\n", getName(), this, __FUNCTION__);
            device->close(this);
            return NULL;
            
            // First 18 bytes are USB header. Data at byte 19
            uint64_t descriptorData = *((uint64_t*)dataBuffer + 18);
            uint64_t subDescriptorData = *((uint64_t*)dataBuffer + 18 + 8);
            
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
            } //switch(descriptorData)

        } //if(checkMsOsDescriptor)

        device->close(this);
        
    } //if(device && usbPlane)

    return NULL;
}


//
// Check for the presence of an MS OS Descriptor.
// The descriptor is used in both v1 and v2 of the Microsoft OS Descriptors.
//
IOReturn MBIMProbe::checkMsOsDescriptor(IOUSBHostDevice *device, uint8_t *cookie){
    
    // We know that the MSFT String Descriptor is 16 byte long plus the Type and Length
    size_t        msftStringSize   = 18;
    char         *msftString       = (char *)IOMalloc(msftStringSize);
    if (msftString == NULL) return kIOReturnVMError;
    uint32_t      bytesTransferred = 0;
    bzero(msftString, msftStringSize);
    
    // GetDescriptor(kDescriptorTypeString, kDescriptorIndexEE, kDescriptorLangNull)
    StandardUSB::DeviceRequest request;
    request.bmRequestType    = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest         = kDeviceRequestGetDescriptor;
    request.wValue           = (kDescriptorTypeString << 8) | 0xEE;
    request.wIndex           = 0;
    request.wLength          = 0x12;
    IOReturn kernelError = device->deviceRequest(this, request, (void *)msftString, bytesTransferred, kUSBHostDefaultControlCompletionTimeoutMS);
    
    if (kernelError){
        IOLog("-%s[%p]::%s: Couldn't get the MSFT100 descriptor, transferred %d bytes: %x\n",getName(), this, __FUNCTION__, bytesTransferred, kernelError);
        IOFree(msftString, msftStringSize);
        return kIOReturnNotFound;
    }

    // Let's typecast to appropriate uints to make the comparison easier
    uint64_t *highBytesString       = (uint64_t*)(void*)(msftString+2);
    uint32_t *medBytesString        = (uint32_t*)(void*)(msftString+10);
    uint16_t *lowBytesString        = (uint16_t*)(void*)(msftString+14);
    *cookie = *(uint8_t*)(void*)(msftString+16);
    
    //
    // Let's see if we have MSFT100 in
    // We should also compare in wide using msftRefStringUnicode
    // FIXME: Also check  && msftString->bLength > StandardUSB::kDescriptorSize
    //
    if (msftString != NULL){
        IOLog("-%s[%p]::%s: found\n", getName(), this, __FUNCTION__);
        
        if((*highBytesString==MS_OS_SIGNATURE_REF_STRING_1) &&
           (*medBytesString ==MS_OS_SIGNATURE_REF_STRING_2) &&
           (*lowBytesString ==MS_OS_SIGNATURE_REF_STRING_3)) {
            IOLog("-%s[%p]::%s: We have a confirmed MSFT100 Descriptor\n", getName(), this, __FUNCTION__);
            IOFree(msftString, msftStringSize);
            return kIOReturnSuccess;
        }
    }
    IOLog("-%s[%p]::%s: MSFT100 was not found\n", getName(), this, __FUNCTION__);
    IOFree(msftString, msftStringSize);
    return kIOReturnNotFound;
}


//
// Gets an MS OS Descriptor V1 for the Device if interface is NULL
// or for the Interface if an interface number is specified.
//
IOReturn MBIMProbe::getMsDescriptor(IOUSBHostDevice *device, const uint8_t cookie, uint16_t interfaceNumber, const uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize){
    // First input validation:
    // We can only request COMPATID descriptor device-wide
    if (interfaceNumber > 0 && DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID){
        IOLog("-%s[%p]::getMsDescriptor Can only request Extended CompatID Device-wide\n", getName(), this);
        return kIOReturnBadArgument;
    }
    // Second input validation:
    // We can only request COMPATID or Extended Properties.
    // Genre is not supported/documented by Microsoft. It just is.
    if (!(DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID || DescriptorType == MS_OS_10_REQUEST_EXTENDED_PROPERTIES)){
        IOLog("-%s[%p]::%s Unknown Microsoft Descriptor: %00000x\n", getName(), this, __FUNCTION__, DescriptorType);
        return kIOReturnBadArgument;
    }

    DeviceRequest                request;
    request.bRequest           = cookie;                      // I can't explain why it's declared as single byte.
    request.wIndex             = DescriptorType;                // The requested descriptor.
    if (interfaceNumber > 0) {                                  // This only applies to Extended Properties as.
        request.bmRequestType = 0xC1;                           // CompatID doesn't have a per-interface
    } else {
        request.bmRequestType = 0xC0;
    }
    
    // We first request the header to figure out the length of data to expect (probably 0x28)
    
    request.wValue         = interfaceNumber;
    request.wLength        = 0x10;
    
    void* interimDataBuffer = IOMalloc(request.wLength);
    
    if (interimDataBuffer == NULL) {
        IOLog("-%s[%p]::%s VM Error.\n", getName(), this, __FUNCTION__);
        return kIOReturnVMError;
    }
    
#ifdef DEBUG
    IOLog("-%s[%p]::%s Making the request: bRequest: %x, wIndex: %x, bmRequestType: %x, wValue: %x, wLength: %x\n", getName(), this, __FUNCTION__, request.bRequest, request.wIndex, request.bmRequestType, request.wValue, request.wLength);
#endif
    
    IOReturn status = device->deviceRequest(this, request, interimDataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
    if(status != kIOReturnSuccess) {
        IOLog("-%s[%p]::%s Could not perform request Extended CompatID: %x\n", getName(), this, __FUNCTION__, status);
        IOSleep(10000);
        IOFree(interimDataBuffer, 0x10);
        return status;
    }
    
    // We can cast the variable like this regardless of the actual descriptor type
    // since both descriptors get transferred the same way and they both provide the
    // much needed dwLength.
    
    MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER *header = (MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER*) interimDataBuffer;
    
    IOLog("-%s[%p]::%s Header bits: dwLength: %x bcdVersion: %x wIndex: %x wCount: %x\n", getName(), this, __FUNCTION__, header->dwLength, header->bcdVersion, header->wIndex, header->wCount);

    //If the descriptor is smaller than 4K we'll get it in one shot
    //It should be 4K-sizeof(usbTransferHeader)
    if (header->dwLength <= 0x1000) {
        *dataBuffer = IOMalloc(header->dwLength);
        if (dataBuffer == NULL) {
            IOLog("-%s[%p]::%s VM Error.\n", getName(), this, __FUNCTION__);
            return kIOReturnVMError;
        }
        
        request.bRequest      = cookie;
        request.wValue        = interfaceNumber;
        request.wLength       = header->dwLength;
        request.wIndex        = DescriptorType;
        //If requesting for an interface, we have a different bmRequestType
        if (interfaceNumber > 0) {
            request.bmRequestType = 0xC1;
        } else {
            request.bmRequestType = 0xC0;
        }

        status = device->deviceRequest(this, request, *dataBuffer, header->dwLength, kUSBHostStandardRequestCompletionTimeout);
        IOLog("-%s[%p]::%s Performed 2nd request with status: %x\n", getName(), this, __FUNCTION__, status);
        IOSleep(10000);
        
        header = NULL;
        IOFree(interimDataBuffer, 0x10);
        
        return kIOReturnSuccess;
        
    //If it's bigger than 4K, we need to make paged transfers.
    } else {
        IOLog("-%s[%p]::%s Don't know how to transfer: %x bytes\n", getName(), this, __FUNCTION__, header->dwLength);
        return kIOReturnBadArgument;
//        uint32_t   bytesTransferred   = 0;
//        uint32_t   remainingBytes     = header->dwLength;
//                  *dataBuffer         = IOMalloc(header->dwLength);
//                  *dataBufferSize     = header->dwLength;
//        uint8_t   *iterator           = (uint8_t*)*dataBuffer;
//        void      *tempBuffer         = NULL;
//                    
//        if (dataBuffer == NULL) return kIOReturnVMError;
//                    
//        while (remainingBytes > 0) {
//            
//            //High Bytes are interfaceNumber, low bytes are page number (bytes DIV pagesize +1)
//            request.wValue        = interfaceNumber|((remainingBytes % 0x1000 + 1) << 8);
//            request.bRequest      = cookie;
//            request.wIndex        = DescriptorType;
//                
//            //Let's request at most the remaining number of bytes
//            if(remainingBytes > 0x1000) {
//                request.wLength   = 0x1000;
//            } else {
//                request.wLength   = remainingBytes;
//            }
//            //If requesting for an interface, we have a different bmRequestType
//            if (interfaceNumber > 0) {
//                request.bmRequestType = 0xC1;
//            } else {
//                request.bmRequestType = 0xC0;
//            }
//                
//            status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
//            //
//            // TODO: Should we transfer everything after the header or do the later pages exclude the header?
//            //
//            if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator+sizeof(StandardUSB::Descriptor), bytesTransferred-sizeof(StandardUSB::Descriptor));
//                    remainingBytes -= (bytesTransferred-sizeof(StandardUSB::Descriptor));
//        }
//
//        return kIOReturnSuccess;
    }//else
    
}


//
// Just call the superclass to do the starting
//
bool MBIMProbe::start(IOService *provider){
	bool ret;
	   
	IOLog("-%s[%p]::start - This is handled by the superclass\n", getName(), this);
	ret = super::start(provider);
	if (!ret){
		IOLog("-%s[%p]::start -  super::start failed\n", getName(), this);
	}
	
	return ret;
}


//
// Just call the superclass to do the stoping
//
void MBIMProbe::stop(IOService *provider){
    
    IOLog("-%s[%p]::stop - calling super::stop\n", getName(), this);
    super::stop(provider);
}


//
// Just call the superclass to do the initializing
//
bool MBIMProbe::init(OSDictionary *properties){
    if (super::init(properties) == false)
    {
        IOLog("-%s[%p]::init - initialize super failed\n", getName(), this);
        return false;
    }
    return true;
}


//
// Just call the superclass to do the freeing
//
void MBIMProbe::free(){
    super::free();
    return;
}
