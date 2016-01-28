//
//  MSOSDescriptorV2.h
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 27.01.2016.
//  Copyright © 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#ifndef MSOSDescriptorV2_h
#define MSOSDescriptorV2_h

//MS OS 2.0 Platform ID "D8DD60DF-4589-4CC7-9CD2-659D9E648A9F"
#define MS_OS_20_DESCRIPTOR_PLATFORM_ID      0xDF 0x60 0xDD 0xD8 0x89 0x45 0xC7 0x4C 0x9C 0xD2 0x65 0x9D 0x9E 0x64 0x8A 0x9F

//Windows 8.1 was the first one. NT Kernel 6.3
#define MS_OS_20_WINDOWS_VERSION_BLUE        0x00 0x00 0x03 0x06
#define MS_OS_20_WINDOWS_VERSION_THRESHOLD   0x00 0x00 0x00 0x10
#define MS_OS_20_WINDOWS_VERSION_REDSTONE    0x00 0x00 0x01 0x10

//wIndex Values
#define MS_OS_20_DESCRIPTOR_INDEX            0x07
#define MS_OS_20_SET_ALT_ENUMERATION         0x08

//wDescriptorType Values
#define MS_OS_20_SET_HEADER_DESCRIPTOR       0x00
#define MS_OS_20_SUBSET_HEADER_CONFIGURATION 0x01
#define MS_OS_20_SUBSET_HEADER_FUNCTION      0x02
#define MS_OS_20_FEATURE_COMPATBLE_ID        0x03
#define MS_OS_20_FEATURE_REG_PROPERTY        0x04
#define MS_OS_20_FEATURE_MIN_RESUME_TIME     0x05
#define MS_OS_20_FEATURE_MODEL_ID            0x06
#define MS_OS_20_FEATURE_CCGP_DEVICE         0x07

/*

 Microsoft OS 2.0 Registry Property Descriptor
        is basically identical to:
 Microsoft OS 1.0 Extended Properties Descriptor
 
 It contains data with names formatted as:
 
 * REG_SZ                  (Unicode String)
 * REG_EXPAND_SZ           (Unicode String with expandable Variables)
 * REG_BINARY              (Binary)
 * REG_DWORD_LITTLE_ENDIAN (Little Endian uint32_t)
 * REG_DWORD_BIG_ENDIAN    (Big Endian uint32_t)
 * REG_LINK                (Unicode Symbolic Link)
 * REG_MULTI_SZ            (Multiple Unicode Strings)
 
 The Extended Properties Descriptor is formed by:
 Header, followed by wCount x Sections.
 Each Section has a size, LengthValue Name, TLV Data.
 
 Not exactly suitable for parsing with structs unless
 you use buffers and pointers to the data.
 
 The end results end up in the registry entry of the device
 or interface on Windows and we can do the same (pointlessly)
 on Mac OS X.
 
 */

// This won't work, but it's here for posterity.
// PropertyName and PropertyData need to be voids
// The defines above should also be part of an enum
// with 16-bit values.
typedef struct {
    uint16_t   wLength;
    uint16_t   wDescriptorType;
    uint16_t   wPropertyDataType;
    uint8_t   *PropertyName;
    uint16_t   wPropertyDataLength;
    uint8_t   *PropertyData;
} MS_OS_20_REGISTRY_PROPERTY_DESCRIPTOR;

#endif /* MSOSDescriptorV2_h */
