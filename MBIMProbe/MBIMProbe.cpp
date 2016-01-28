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

IOService * MBIMProbe::probe(IOService *provider, SInt32 *score){

    const IORegistryPlane * usbPlane = getPlane(kIOUSBPlane);
    IOUSBHostDevice       * device   = OSDynamicCast(IOUSBHostDevice, provider);
    IOReturn                status;
    
    if (device && usbPlane)
    {
        device->open(this);
        if (checkMsOsDescriptor(device)==kIOReturnSuccess){
            //Set the current configuration again. It might go away otherwise
            void     *dataBuffer;
            uint32_t  dataBufferSize;
            
          //Read the MS OS Compat Descriptor v1
            status=getMsDescriptor(device, MS_OS_10_REQUEST_EXTENDED_COMPATID, &dataBuffer, &dataBufferSize);

            switch (USBToHost64(*((uint64_t*)dataBuffer+18))){
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
                case MS_OS_10_ALTRCFG_COMPATIBLE_ID:
                    IOLog("MS Compatible Descriptor: MBIM");
                    switch (USBToHost64(*((uint64_t*)dataBuffer+18))){
                        case MS_OS_10_ALT2_SUBCOMPATIBLE_ID:
                        case MS_OS_10_ALT3_SUBCOMPATIBLE_ID:
                        case MS_OS_10_ALT4_SUBCOMPATIBLE_ID:
                            IOLog("\n");
                            //WARNING: We've found a fragile MBIM configuration.
                            //TODO: If El Capitan: Switch to the Windows 8
                            //      configuration now and afterwards publish
                            //      the MBIM configuration.
                            //      If Yosemite or older, switch to the AT
                            //      option.
                            //PublishMBIMConfiguration(device);
                            device->close(this);
                            break;
                        default:
                            IOLog(" incorrect Subcompatible descriptor\n");
                            break;
                    }
                    break;
                case MS_OS_10_BLUTUTH_COMPATIBLE_ID:
                    IOLog("Bluetooth");
                    switch (USBToHost64(*((uint64_t*)dataBuffer+18))){
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
        
        //It means that it still might be MBIM, but not
        //with the ALTCFG config descriptor defined
        //in Windows 8.
        if(searchMbimConfiguration(device)==kIOReturnSuccess){
            //PublishMBIMConfiguration(device);
            
            device->close(this);
            return NULL;
        }
        
        if(haveCDCinterfaces()){
            //PublishCDCConfiguration(device);
            
            device->close(this);
            return NULL;
        }
        
        if(haveRNDISInterfaces()){
            //PublishRNDISConfiguration(device);
            
            device->close(this);
            return NULL;
        }
        
        switch (USBToHost16(device->getDeviceDescriptor()->idVendor)){
            case USB_VENDOR_ID_ZTE_19D2:
            case USB_VENDOR_ID_QUALCOMM_05C6:
                //ejectCD(interface);
                break;
            case USB_VENDOR_ID_HUAWEI_12D1:
                //huaweiMode1(interface);
                //huaweiMode2(interface);
                break;
        }
        device->close(this);
        
    }
    return NULL;
}

IOReturn MBIMProbe::checkMsOsDescriptor(IOUSBHostDevice *device){
    IOUSBDevRequest	                       request;
    IOUSBConfigurationDescHeader           descriptorHdr;
    IOReturn                               kernelError;
    IOUSBConfigurationDescriptorPtr       *descriptor;
    const StringDescriptor                *msftString;
    
    msftString = device->getStringDescriptor(0xEE,0x0);
    uint64_t *highBytesString       = (uint64_t*)(void*)&msftString;
    uint32_t *medBytesString        = (uint32_t*)(void*)(&msftString+4);
    uint16_t *lowBytesString        = (uint16_t*)(void*)(&msftString+4);
    
    const char    msftRefString[8]     =  "MSFT100";
    const wchar_t msftRefWideString[8] = L"MSFT100";
    uint64_t *highBytesRefString       = (uint64_t*)(void*)&msftRefString;
    uint32_t *medBytesRefString        = (uint32_t*)(void*)(&msftRefString+4);
    uint16_t *lowBytesRefString        = (uint16_t*)(void*)(&msftRefString+4);
    
    // Let's see if we have MSFT100 in narrow
    // We should also compare in wide using msftRefStringUnicode
    if (msftString != NULL && msftString->bLength > StandardUSB::kDescriptorSize){

        //We should compare only the first 14 bytes. The last double byte
        //is the bRequest value + ContainerID and can be different.
        //Comparing two uint128_t would have been much more elegant.
        if((*highBytesRefString==*highBytesString)&&(*medBytesRefString==*medBytesString)&&(lowBytesRefString==lowBytesString))
        return kIOReturnSuccess;
    }
    
    return kIOReturnNotFound;
}


//Gets an MS OS Descriptor V1 for the Device
IOReturn MBIMProbe::getMsDescriptor(IOUSBHostDevice *device, const uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize){

    //Get the cookie
    const StringDescriptor *stringDescriptor = device->getStringDescriptor(0xEE,0x00);
    //If we've got the cookie, let's play with it
    if (stringDescriptor != NULL && stringDescriptor->bLength > StandardUSB::kDescriptorSize) {

        DeviceRequest    request;
        char             msftDescriptor[16] ={0};
        uint32_t         bytesTransferred   = 0 ;
        uint8_t          bRequest           = stringDescriptor->bString[15];
        
        request.bmRequestType = 0xC0;
        request.bRequest      = bRequest;                      // I can't explain why it's declared as single byte.
        request.wValue        = 0;                             // DeviceWide
        if (DescriptorType==MS_OS_10_REQUEST_EXTENDED_COMPATID)// If it's MS_OS_10_REQUEST_EXTENDED_COMPATID
            request.wLength   = 0x28;                          //    it only takes 40 bytes
        else                                                   // If it's MS_OS_10_REQUEST_EXTENDED_PROPERTIES
            request.wLength   = 0x10;                          //    we'll just take the header and make the request later.
        request.wIndex        = DescriptorType;                // The requested descriptor
        
        *dataBuffer = IOMalloc(request.wLength);
        if (dataBuffer==NULL) return kIOReturnVMError;
        
        IOReturn status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
        if(status!=kIOReturnSuccess) return status;
        
        //We can cast the variable like this regardless of the actual descriptor type
        //since both descriptors get transferred the same way and
        //they both start with dwLength
        MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER *header = (MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER*)*dataBuffer;

        switch (DescriptorType){

            //Both cases transfer the same way
            case MS_OS_10_REQUEST_EXTENDED_COMPATID:
            case MS_OS_10_REQUEST_EXTENDED_PROPERTIES:

                //If the descriptor is bigger than 4K we'll need to use paged requests.
                if (header->dwLength <= 0x1000) {
                           *dataBuffer    = IOMalloc(header->dwLength);
                           *dataBufferSize= header->dwLength;
                    request.bmRequestType = 0xC0;
                    request.bRequest      = bRequest;
                    request.wValue        = 0;
                    request.wLength       = header->dwLength;
                    request.wIndex        = DescriptorType;
                    status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
                    return kIOReturnSuccess;
                } else {
                    /* This was my first attempt at buffered transfers
                     * I am keeping it until we test.
                     uint16_t   numPages = 0;

                    if (header->dwLength % 0x1000){
                        numPages = (header->dwLength/0x1000)+1;
                    } else numPages = header->dwLength/0x1000;

                
                    void *dataBuffer[numPages];
                    void *tempBuffer;
                    
                    for (uint16_t i=0; i < numPages; i++) {
                        request.bmRequestType = 0xC0;
                        request.bRequest      = stringDescriptor->bString[15];
                        request.wValue        = 0;
                        request.wLength       = 0x1000;
                        request.wIndex        = DescriptorType;
                        status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                        dataBuffer[i] = IOMalloc(header->dwLength/0x1000);
                        bzero(dataBuffer[i], header->dwLength);
                        memcpy(tempBuffer, dataBuffer[i], bytesTransferred);
                    }
                     */
                    
                    /* This is the second attempt, based on Claus's algorithm
                     * It probably failes because page transfers might also
                     * include a header so we might need numFullPages+1

                    uint32_t   numFullPages   = (header->dwLength - 1) / 0x1000;
                    uint32_t   remainingBytes = header->dwLength;
                    uint8_t   *dataBuffer     = (uint8_t*) IOMalloc(header->dwLength);
                    uint8_t   *iterator       = dataBuffer;
                    void      *tempBuffer     = NULL;
                    
                    if (dataBuffer == NULL) return kIOReturnVMError;

                    for (uint32_t i=0; i<numFullPages; i++, iterator+=bytesTransferred, remainingBytes-=bytesTransferred) {
                        request.bmRequestType = 0xC0;
                        request.bRequest      = bRequest;
                        request.wValue        = numFullPages<<8;
                        request.wLength       = 0x1000;
                        request.wIndex        = DescriptorType;
                        status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                        if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator, bytesTransferred);
                    }
                    
                    if(remainingBytes > 0){
                        request.bmRequestType = 0xC0;
                        request.bRequest      = bRequest;
                        request.wValue        = numFullPages<<8;
                        request.wLength       = remainingBytes;
                        request.wIndex        = DescriptorType;
                        status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                        if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator, bytesTransferred);
                    }
                    */
                    
                    // Third attempt at paged transfers with iterator and remaining bytes calculation
                    
                    uint32_t   remainingBytes = header->dwLength;
                              *dataBuffer     = IOMalloc(header->dwLength);
                    uint8_t   *iterator       = (uint8_t*)*dataBuffer;
                    void      *tempBuffer     = NULL;
                    
                    if (dataBuffer == NULL) return kIOReturnVMError;
                    
                    while (remainingBytes>0){
                        request.bmRequestType = 0xC0;
                        request.bRequest      = bRequest;
                        if(remainingBytes>0x1000)
                            request.wValue    = 0x1000<<8;
                        else
                            request.wValue    = remainingBytes<<8;
                        request.wLength       = remainingBytes;
                        request.wIndex        = DescriptorType;
                        status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                        //XXX: Should we transfer everything behind the header or do the later pages exclude the header
                        if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator+sizeof(StandardUSB::Descriptor), bytesTransferred-sizeof(StandardUSB::Descriptor));
                        remainingBytes-=(bytesTransferred-sizeof(StandardUSB::Descriptor));
                    }//while
                   *dataBufferSize = header->dwLength;
                    return kIOReturnSuccess;
                }//else
                break;
            default:
                return kIOReturnUnsupported;
                break;
        }//switch
        
        return kIOReturnSuccess;
        
    } else return kIOReturnNotFound;//Cookie not found
    
}

//
// Gets an MS OS Descriptor V1 for the Interface
//
IOReturn MBIMProbe::getMsDescriptor(IOUSBHostDevice *device, uint16_t interfaceNumber, uint16_t DescriptorType, void **dataBuffer, uint32_t *dataBufferSize){
 
    const StringDescriptor *stringDescriptor = device->getStringDescriptor(0xEE,0x00);

    if (stringDescriptor != NULL && stringDescriptor->bLength > StandardUSB::kDescriptorSize) {
        
        DeviceRequest    request;
        char             msftDescriptor[16] ={0};
        uint32_t         bytesTransferred   = 0 ;
        uint8_t          bRequest           = stringDescriptor->bString[15];
        
        request.bmRequestType = 0xC0;
        request.bRequest      = bRequest;
        request.wValue        = 0;
        if (DescriptorType==MS_OS_10_REQUEST_EXTENDED_COMPATID)
            request.wLength   = 0x28;
        else
            request.wLength   = 0x10;
        request.wIndex        = DescriptorType;
        
        *dataBuffer = IOMalloc(request.wLength);
        if (dataBuffer==NULL) return kIOReturnVMError;
        
        IOReturn status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
        if(status!=kIOReturnSuccess) return status;
        
        MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER *header = (MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER*)*dataBuffer;
        
        switch (DescriptorType){
            case MS_OS_10_REQUEST_EXTENDED_COMPATID:
            case MS_OS_10_REQUEST_EXTENDED_PROPERTIES:
                
                if (header->dwLength <= 0x1000) {
                    *dataBuffer    = IOMalloc(header->dwLength);
                    *dataBufferSize= header->dwLength;
                    request.bmRequestType = 0xC0;
                    request.bRequest      = bRequest;
                    request.wValue        = 0;
                    request.wLength       = header->dwLength;
                    request.wIndex        = DescriptorType;
                    status = device->deviceRequest(this, request, *dataBuffer, *dataBufferSize, kUSBHostStandardRequestCompletionTimeout);
                    return kIOReturnSuccess;
                } else {
                    
                    uint32_t   remainingBytes = header->dwLength;
                    *dataBuffer     = IOMalloc(header->dwLength);
                    uint8_t   *iterator       = (uint8_t*)*dataBuffer;
                    void      *tempBuffer     = NULL;
                    
                    if (dataBuffer == NULL) return kIOReturnVMError;
                    
                    while (remainingBytes>0){
                        request.bmRequestType = 0xC0;
                        request.bRequest      = bRequest;
                        if(remainingBytes>0x1000)
                            request.wValue    = 0x1000<<8;
                        else
                            request.wValue    = remainingBytes<<8;
                        request.wLength       = remainingBytes;
                        request.wIndex        = DescriptorType;
                        status = device->deviceRequest(this, request, tempBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
                        if (status==kIOReturnSuccess) memcpy(tempBuffer, iterator+sizeof(StandardUSB::Descriptor), bytesTransferred-sizeof(StandardUSB::Descriptor));
                        remainingBytes-=(bytesTransferred-sizeof(StandardUSB::Descriptor));
                    }//while
                    *dataBufferSize = header->dwLength;
                    return kIOReturnSuccess;
                }//else
                break;
            default:
                return kIOReturnUnsupported;
                break;
        }//switch
        
        return kIOReturnSuccess;
        
    } else return kIOReturnNotFound;//Cookie not found
}

bool MBIMProbe::searchMbimConfiguration(IOUSBHostDevice *device){

    uint8_t                          numConfigs          = device->getDeviceDescriptor()->bNumConfigurations;
    const ConfigurationDescriptor	*configDescriptor    = device->getConfigurationDescriptor();		// configuration descriptor
    uint8_t                          configIndex         = configDescriptor->bConfigurationValue;

    if (numConfigs < 1){
        IOLog("MBIMProbe::SelectMBIMConfiguration not enough configurations: %d\n", numConfigs);
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
    return true;
}

IOReturn MBIMProbe::ejectCD(IOUSBHostInterface *interface){
    
}

IOReturn MBIMProbe::huaweiMode1(IOUSBHostInterface *interface){
    
}

IOReturn MBIMProbe::huaweiMode2(IOUSBHostInterface *interface){
    
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