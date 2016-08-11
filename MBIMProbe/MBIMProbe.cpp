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
    uint8_t configNumber  = 0;
    
    if(!(device && usbPlane)){
        return 0;
    }
    
    
    //Get exclusive access to the USB device
    bool openSucc = device->open(this);
    if(!openSucc){
        log("Failed to open device\n");
        return 0;
    }
    
    
    
    discoverDevice(device, &configNumber);
    
    
    /*
     * PRE:  There is a MSFT100 descriptor on device.
     * POST: Device is Microsoft. Handle as MS device.
     */
    uint8_t cookie;
    status = checkMsOsDescriptor(device, &cookie);
    if(status != kIOReturnSuccess){
        log("Device is not MSFT100\n");
        if(device->isOpen()){
            device->close(this);
        }
        return this;
    }
    
    log("We have a MSFT100 descriptor with cookie %x. We're now getting the COMPATID descriptor\n", cookie);
    // TODO Set the current configuration again. It might go away otherwise
    void     *dataBuffer;
    uint32_t  dataBufferSize;
    
    
    // Read the MS OS Compat Descriptor v1
    // dataBuffer and dataBufferSize set in callee
    status = getMsDescriptor(device, cookie, 0, MS_OS_10_REQUEST_EXTENDED_COMPATID, &dataBuffer, &dataBufferSize);
    if (status != kIOReturnSuccess) {
        log("We couldn't get the MS_OS_10_REQUEST_EXTENDED_COMPATID descriptor with error: %x\n", status);
        if(device->isOpen()){
            device->close(this);
        }
        return this;
    }
    
    log("We succeeded in getting MS_OS_10_REQUEST_EXTENDED_COMPATID\n");
    
    configNumber = parseMSDescriptor(dataBuffer, configNumber);
    if(configNumber < 0) {
        log("Failed to discover a valid configuration\n");
        if(device->isOpen()){
            device->close(this);
        }
        return this;
    }
    IOFree(dataBuffer, dataBufferSize);
    
    log("Activating configuration: %x\n", configNumber);
    status = device->setConfiguration(configNumber, true);
    if(device->isOpen()) {
        device->close(this);
    }
    if(status != kIOReturnSuccess){
        log("Failed to activate configuration: %x\n", configNumber);
        return 0;
    }
    
    return this;
    
}

void MBIMProbe::discoverDevice(IOUSBHostDevice *device, uint8_t *configNumber) {
    //Let's find out a few things about this device.
    //First: The number of configurations
    StandardUSB::DeviceRequest request;
    request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest      = kDeviceRequestGetConfiguration;
    request.wValue        = 0;
    request.wIndex        = 0;
    request.wLength       = sizeof(*configNumber);
    uint32_t bytesTransferred = 0;
    device->deviceRequest(this, request, configNumber, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
    //And then, the idVendor, idProduct, etc.
#ifdef DEBUG
    log("We have the USB device exclusively. Vendor ID: %x. Product ID: %x. Config: %d. Now checking for the MSFT100 descriptor\n", USBToHost16(device->getDeviceDescriptor()->idVendor), USBToHost16(device->getDeviceDescriptor()->idProduct), *configNumber);
#endif
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
        log("Couldn't get the MSFT100 descriptor, transferred %d bytes: %x\n", bytesTransferred, kernelError);
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
        if((*highBytesString==MS_OS_SIGNATURE_REF_STRING_1) &&
           (*medBytesString ==MS_OS_SIGNATURE_REF_STRING_2) &&
           (*lowBytesString ==MS_OS_SIGNATURE_REF_STRING_3)) {
            log("We have a confirmed MSFT100 Descriptor\n");
            IOFree(msftString, msftStringSize);
            return kIOReturnSuccess;
        }
    }
    log("MSFT100 was not found\n");
    IOFree(msftString, msftStringSize);
    return kIOReturnNotFound;
}


//
// Gets an MS OS Descriptor V1 for the Device if interface is NULL
// or for the Interface if an interface number is specified.
//
IOReturn MBIMProbe::getMsDescriptor(IOUSBHostDevice *device, const uint8_t cookie, uint16_t interfaceNumber, const uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize){
    // First input validation:
    // We can only request the COMPATID descriptor device-wide
    if (interfaceNumber > 0 && DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID){
        log("Can only request Extended CompatID Device-wide\n");
        return kIOReturnBadArgument;
    }
    // Second input validation:
    // We can only request the COMPATID or Extended Properties.
    // Genre is not supported/documented by Microsoft. It just is.
    if (!(DescriptorType == MS_OS_10_REQUEST_EXTENDED_COMPATID || DescriptorType == MS_OS_10_REQUEST_EXTENDED_PROPERTIES)){
        log("Unknown Microsoft Descriptor Type. Check Endianness: %00000x\n", DescriptorType);
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
    // FIXME: interfacenumber only 8 bits long in the low bytes of wValue
    request.wValue         = interfaceNumber;
    request.wLength        = 0x10;
    
    void* interimDataBuffer = IOMalloc(request.wLength);
    
    if (interimDataBuffer == NULL) {
        log("VM Allocation Error.\n");
        return kIOReturnVMError;
    }
    
#ifdef DEBUG
    log("Making the request: bRequest: %x, wIndex: %x, bmRequestType: %x, wValue: %x, wLength: %x\n", request.bRequest, request.wIndex, request.bmRequestType, request.wValue, request.wLength);
#endif
    uint32_t bytesTransfered;
    IOReturn status = device->deviceRequest(this, request, interimDataBuffer, bytesTransfered, kUSBHostStandardRequestCompletionTimeout);
    if(status != kIOReturnSuccess) {
        log("Could not perform request Extended CompatID: %x, %x bytes transfered\n", status, bytesTransfered);
        IOFree(interimDataBuffer, 0x10);
        return status;
    }
    log("Performed request Extended CompatID: %x, %x bytes transfered\n", status, bytesTransfered);
    
    // We can cast the variable like this regardless of the actual descriptor type
    // since both descriptors get transferred the same way and they both provide the
    // much needed dwLength.
    
    MS_OS_10_EXTENDED_COMPAT_DESCRIPTOR_HEADER *header = (MS_OS_10_EXTENDED_COMPAT_DESCRIPTOR_HEADER*) interimDataBuffer;
    
    log("Header bits: dwLength: %x bcdVersion: %x wIndex: %x bCount: %x\n", header->dwLength, header->bcdVersion, header->wIndex, header->bCount);
    
    // If the descriptor is smaller than 4K we'll get it in one shot
    // It might actually be 4K - StandardUSB::kDescriptorSize but
    // only Redmond might be able to explain this to us.
    uint16_t transferSize = USBToHost16(header->dwLength);
    if (transferSize <= 0x1000) {
        *dataBuffer = IOMalloc(header->dwLength);
        if (*dataBuffer == NULL) {
            log("VM Error.\n");
            return kIOReturnVMError;
        }
        
        request.bRequest      = cookie;
        request.wValue        = interfaceNumber;
        request.wLength       = HostToUSB16(transferSize);
        request.wIndex        = DescriptorType;
        //If requesting for an interface, we have a different bmRequestType
        if (interfaceNumber > 0) {
            request.bmRequestType = 0xC1;
        } else {
            request.bmRequestType = 0xC0;
        }
#ifdef DEBUG
        log("Making the 2nd request: bRequest: %x, wIndex: %x, bmRequestType: %x, wValue: %x, wLength: %x\n", request.bRequest, request.wIndex, request.bmRequestType, request.wValue, request.wLength);
#endif
        
        status = device->deviceRequest(this, request, *dataBuffer, bytesTransfered, kUSBHostVendorRequestCompletionTimeout);
        if(status){
            log("Request failed: %x, %x bytes transfered\n", status, bytesTransfered);
            header = NULL;
            IOFree(interimDataBuffer, 0x10);
            return status;
        }
        log("Performed 2nd request with status: %x\n", status);
        
        log("Content is ");
        for (int i=0; i < header->dwLength; i++){
            log_cont("0x%0x ", (*(uint8_t**)dataBuffer)[i]);
        }
        log_cont("\n");
        IOSleep(10000);
        
        header = NULL;
        IOFree(interimDataBuffer, 0x10);
        
        return kIOReturnSuccess;
        
        //If it's bigger than 4K, we need to make paged transfers.
    } else {
        uint32_t   bytesTransferred   = 0;
        uint32_t   remainingBytes     = header->dwLength;
        *dataBuffer         = IOMalloc(header->dwLength);
        *dataBufferSize     = header->dwLength;
        uint8_t   *iterator           = (uint8_t*)*dataBuffer;
        void      *tempBuffer         = NULL;
        
        if (dataBuffer == NULL) {
            return kIOReturnVMError;
        }
        
        while (remainingBytes > 0) {
            
            //High Bytes are interfaceNumber, low bytes are page number (bytes DIV pagesize +1)
            request.wValue        = interfaceNumber|((remainingBytes % 0x1000 + 1) << 8);
            request.bRequest      = cookie;
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
            if (status==kIOReturnSuccess){
              memcpy(tempBuffer, iterator+sizeof(StandardUSB::Descriptor), bytesTransferred-sizeof(StandardUSB::Descriptor));
            }
            remainingBytes -= (bytesTransferred-sizeof(StandardUSB::Descriptor));
        }
        
        return kIOReturnSuccess;
    }//else
    
}

uint8_t MBIMProbe::parseMSDescriptor(void *dataBuffer, uint8_t currentConfigNumber) {
    uint8_t nextConfigNumber = -1;
    
    uint64_t descriptorData = **(uint64_t**)((uint8_t*) dataBuffer + 24);
    uint64_t subDescriptorData = *(&descriptorData + 1);
    
    //Now let's act upon the descriptor.
    switch (descriptorData){
        case 0:
            log("MS Compatible Descriptor: Null Compatible ID\n");
            break;
        case MS_OS_10_RNDIS_COMPATIBLE_ID:
            //Let's also look for something else before we
            //PublishRNDISConfiguration(device);
            log("MS Compatible Descriptor: RNDIS\n");
            break;
        case MS_OS_10_MTP_COMPATIBLE_ID:
            log("MS Compatible Descriptor: MTP\n");
            break;
        case MS_OS_10_PTP_COMPATIBLE_ID:
            log("MS Compatible Descriptor: PTP\n");
            break;
        case MS_OS_10_WINUSB_COMPATIBLE_ID:
            log("MS Compatible Descriptor: WINUSB\n");
            break;
        case MS_OS_10_XUSB20_COMPATIBLE_ID:
            log("MS Compatible Descriptor: X USB 2.0\n");
            break;
        case MS_OS_10_MBIM_COMPATIBLE_ID:
            log("MS Compatible Descriptor: MBIM on default config.\n");
            //Let's pointlessly switch to the current config.
            nextConfigNumber = currentConfigNumber;
            // TODO PublishMBIMConfiguration(device)
            break;
        case MS_OS_10_ALTRCFG_COMPATIBLE_ID:
            log("MS Compatible Descriptor: MBIM");
            switch (subDescriptorData){
                case MS_OS_10_ALT2_SUBCOMPATIBLE_ID:
                    log_cont(" on config 2");
                    nextConfigNumber = 2;
                    // TODO PublishMBIMConfiguration(device);
                    break;
                case MS_OS_10_ALT3_SUBCOMPATIBLE_ID:
                    log_cont(" on config 3");
                    nextConfigNumber = 3;
                    // TODO PublishMBIMConfiguration(device);
                    break;
                case MS_OS_10_ALT4_SUBCOMPATIBLE_ID:
                    log_cont(" on config 4");
                    nextConfigNumber = 4;
                    // TODO PublishMBIMConfiguration(device);
                    break;
                default:
                    log_cont(" incorrect Subcompatible descriptor\n");
                    break;
            }
            break;
        case MS_OS_10_CDC_WMC_COMPATIBLE_ID:
            log_cont("MS Compatible Descriptor: CDC-WMC on default config.\n");
            nextConfigNumber = currentConfigNumber;
            // TODO PublishWMCConfiguration(device)
            break;
        case MS_OS_10_WMCALTR_COMPATIBLE_ID:
            log("MS Compatible Descriptor: CDC-WMC");
            switch (subDescriptorData){
                case MS_OS_10_ALT2_SUBCOMPATIBLE_ID:
                    log_cont(" on config 2\n");
                    nextConfigNumber = 2;
                    // TODO PublishWMCConfiguration(device);
                    break;
                case MS_OS_10_ALT3_SUBCOMPATIBLE_ID:
                    log_cont(" on config 3\n");
                    nextConfigNumber = 3;
                    // TODO PublishWMCConfiguration(device);
                    break;
                case MS_OS_10_ALT4_SUBCOMPATIBLE_ID:
                    log_cont(" on config 4\n");
                    nextConfigNumber = 4;
                    // TODO PublishWMCConfiguration(device);
                    break;
                default:
                    log_cont(" with illegal Subcompatible descriptor\n");
                    break;
            }
            break;
        case MS_OS_10_BLUTUTH_COMPATIBLE_ID:
            log("Bluetooth");
            switch (subDescriptorData) {
                case MS_OS_10_NULL_SUBCOMPATIBLE_ID:
                    log_cont(" incorrect Subcompatible descriptor\n");
                    break;
                case MS_OS_10_BT11_SUBCOMPATIBLE_ID:
                    log_cont(" v1.1\n");
                    break;
                case MS_OS_10_BT12_SUBCOMPATIBLE_ID:
                    log_cont(" v1.2\n");
                    break;
                case MS_OS_10_EDR2_SUBCOMPATIBLE_ID:
                    log_cont(" v2.0+EDR\n");
                    break;
                default:
                    log(" with illegal Subcompatible descriptor\n");
                    break;
            }
        default:
            log("Illegal Compatible descriptor: %llx. Subcompatible: %llx \n", descriptorData, subDescriptorData);
            break;
    }
    return nextConfigNumber;
}

//
// Just call the superclass to do the starting
//
bool MBIMProbe::start(IOService *provider){
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
void MBIMProbe::stop(IOService *provider){
    
    log("calling super::stop\n");
    super::stop(provider);
}


//
// Just call the superclass to do the initializing
//
bool MBIMProbe::init(OSDictionary *properties){
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
void MBIMProbe::free(){
    super::free();
    return;
}
