//
//  MSOSDescriptorV1.h
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 27.01.2016.
//  Copyright © 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#ifndef MSOSDescriptorV1_h
#define MSOSDescriptorV1_h

#include <IOKit/usb/USB.h>

/*
 
 MSFT100 Descriptor Description:
 
     bLength             1  0x12        Length of the descriptor.
     bDescriptorType     1  0x03        Descriptor type. A value of 0x03 indi-
                                        -cates a Microsoft OS string descriptor.
     qwSignature        14  L"MSFT100"  Signature field.
     bMS_VendorCode      1  0x??        Vendor Code.
     bFlags	             1  0x02        Bit 1: ContainerID Support Bool. Bits
                                        0,2–7: Reserved
 
 Vendor Code: Save it for later as it's needed for each MS Request. Works like
              a cookie.
 ContainerID: Show all the devices a single one regardless of function/interface
              grouping.
              Think of multifunction printers. Each one of them contains a few
              USB devices such as SD/CF Readers, Scanners, HID (keypad),
              Printer, Fax. Show the Multi Function Printer as a single device
              even if the HID and scanners are grouped together.
              Container IDs are actually used with MSOSDescriptor V2. Oddly
              enough, they were introduced in MS OS Descriptor v2 and used in
              MS OS Descriptor v1. But it's Microsoft, so no surprises.
 
*/

//Unused
#define MS_OS_10_REQUEST_GENRE               0x1
//Compatible IDs
#define MS_OS_10_REQUEST_EXTENDED_COMPATID   0x4
//Icons, URLs, Registry Entries, Help Pages, etc.
#define MS_OS_10_REQUEST_EXTENDED_PROPERTIES 0x5


/*
 
 Microsoft Compatible ID Descriptor
 
 Values are: RNDIS, PTP, MTP, Xusb20, Bluetooth, Winusb, ALTRCFG
             Subvalues are only for Bluetooth and MBIM/ALTRCFG
 
 Except BTooth and Alternate all compatible IDs use NULL as a subcompatible ID.
 
 Bluetooth can only use BT11, BT12 and EDR as a subcompatible ID.
 
 The definitions below are from:
 "MBIM-Based Mobile Broadband Requirements for Windows"
 
 MBIM means that it defaults to MBIM (current config). No subcompatible ID
 allowed. ALTRCFG means that the subcompatible ID indicates which config
 contains the MBIM.
 
 CDC_WMC means that it defaults to CDC_WMC (current config). No subcompatible ID
 allowed. WMCALTR means that the subcompatible ID indicates which config
 contains the WMC.

 The compatible ID Descriptor has a header and bCount x Sections.
 
 Warning, this is network order padded ASCII. Always use the USBTOHLL() endian
 conversion macro.
 
 */

#define   MS_OS_10_NULL_COMPATIBLE_ID    0x0000000000000000
#define   MS_OS_10_RNDIS_COMPATIBLE_ID   0x524E444953000000
#define   MS_OS_10_PTP_COMPATIBLE_ID     0x5054500000000000
#define   MS_OS_10_MTP_COMPATIBLE_ID     0x4D54500000000000
#define   MS_OS_10_XUSB20_COMPATIBLE_ID  0x5855534232300000
#define   MS_OS_10_BLUTUTH_COMPATIBLE_ID 0x424C555455544800
#define   MS_OS_10_WINUSB_COMPATIBLE_ID  0x57494E5553420000

#define   MS_OS_10_MBIM_COMPATIBLE_ID    0x4D42494D00000000
#define   MS_OS_10_ALTRCFG_COMPATIBLE_ID 0x414C545243464700
#define   MS_OS_10_CDC_WMC_COMPATIBLE_ID 0x4344435F574D4300
#define   MS_OS_10_WMCALTR_COMPATIBLE_ID 0x574D43414C545200

#define   MS_OS_10_NULL_SUBCOMPATIBLE_ID 0x0000000000000000
#define   MS_OS_10_BT11_SUBCOMPATIBLE_ID 0x3131000000000000
#define   MS_OS_10_BT12_SUBCOMPATIBLE_ID 0x3132000000000000
#define   MS_OS_10_EDR2_SUBCOMPATIBLE_ID 0x4544520000000000
#define   MS_OS_10_ALT2_SUBCOMPATIBLE_ID 0x3200000000000000
#define   MS_OS_10_ALT3_SUBCOMPATIBLE_ID 0x3300000000000000
#define   MS_OS_10_ALT4_SUBCOMPATIBLE_ID 0x3400000000000000

typedef struct {
    uint32_t  dwLength;
    uint16_t  bcdVersion;
    uint16_t  wIndex;
    uint8_t   bCount;
    uint8_t   reserved[7];
} MS_OS_10_EXTENDED_COMPAT_DESCRIPTOR_HEADER;

typedef struct {
    uint8_t   bFirstInterfaceNumber;
    uint8_t   Reserved;
    uint8_t   compatibleId[8];
    uint8_t   subCompatibleId[8];
    uint8_t   Reserved2[6];
} MS_OS_10_EXTENDED_COMPAT_DESCRIPTOR_FUNCTION;

/*

 Microsoft Extended Properties Descriptor
 
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
 
 */

#define   MS_OS_REG_RESERVED             0
#define   MS_OS_REG_SZ                   1
#define   MS_OS_REG_EXPAND_SZ            2
#define   MS_OS_REG_BINARY               3
#define   MS_OS_REG_DWORD_LITTLE_ENDIAN  4
#define   MS_OS_REG_DWORD_BIG_ENDIAN     5
#define   MS_OS_REG_LINK                 6
#define   MS_OS_REG_MULTI_SZ             7

typedef struct {
    uint32_t  dwLength;
    uint16_t  bcdVersion;
    uint16_t  wIndex;
    uint16_t  wCount;
} MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_HEADER;

typedef struct {
    uint32_t  dwSize;
    uint32_t  dwPropertyDataType;
    uint16_t  wPropertyNameLength;
    uint8_t   bPropertyName;
} MS_OS_10_EXTENDED_PROPERTIES_DESCRIPTOR_PROPERTY_HEADER;

typedef uint32_t dwPropertyDataLength;

#endif /* MSOSDescriptorV1_h */
