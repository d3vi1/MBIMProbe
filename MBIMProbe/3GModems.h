//
//  3GModems.h
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 28.01.2016.
//  Copyright © 2016 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#ifndef _GModems_h
#define _GModems_h

/*
 In the drivers included by Vodafone, these are all the included vendor IDs. Did you notice that
 Huawei's ID is just an anagram of Qualcomm's? Did you also notice that Qualcomm's VID is used
 on some ZTE Devices?
 
 find . -type f -name Info.plist -print0 | xargs -0 grep -A 1 idVendor | grep -v idVendor | sort -u

 ./10.10/MBBDataCardECMDriver_10_9.kext/Contents/Info.plist-                                                        <integer>4817</integer>
 ./10.10/MBBDataCardECMDriver_10_9.kext/Contents/PlugIns/MBBAppUSBCDCECMControl.kext/Contents/Info.plist-           <integer>4817</integer>
 ./10.10/MBBDataCardECMDriver_10_9.kext/Contents/PlugIns/MBBAppUSBCDCECMData.kext/Contents/Info.plist-			    <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/Info.plist-                                                         <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardACMControl.kext/Contents/Info.plist-			<integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardACMData.kext/Contents/Info.plist-             <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardDMM.kext/Contents/Info.plist-                 <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardECMControl.kext/Contents/Info.plist-          <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardECMData.kext/Contents/Info.plist-             <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardHidPort.kext/Contents/Info.plist-             <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiDataCardJUBusDriver.kext/Contents/Info.plist-         <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/HuaweiLogLevel.kext/Contents/Info.plist-                    <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/MBBACMData.kext/Contents/Info.plist-                        <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/MBBActivateDriver.kext/Contents/Info.plist-                 <integer>4817</integer>
 ./10.9/HuaweiDataCardDriver_10_9.kext/Contents/PlugIns/MBBEthernetData.kext/Contents/Info.plist-                   <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/Info.plist-                                                            <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardACMControl.kext/Contents/Info.plist-             <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardACMData.kext/Contents/Info.plist-                <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardDMM.kext/Contents/Info.plist-                    <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardECMControl.kext/Contents/Info.plist-             <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardECMData.kext/Contents/Info.plist-                <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardHidPort.kext/Contents/Info.plist-                <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiDataCardJUBusDriver.kext/Contents/Info.plist-            <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/HuaweiLogLevel.kext/Contents/Info.plist-                       <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/MBBACMData.kext/Contents/Info.plist-                           <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/MBBActivateDriver.kext/Contents/Info.plist-                    <integer>4817</integer>
 ./Huawei/HuaweiDataCardDriver.kext/Contents/PlugIns/MBBEthernetData.kext/Contents/Info.plist-                      <integer>4817</integer>
 ./Huawei/USBExpressCardCantWake_Huawei.kext/Contents/Info.plist-                                                   <integer>4817</integer>
 ./ZTE/ZTEUSBCDCACMData.kext/Contents/Info.plist-                                                                   <integer>1478</integer>
 ./10.9/ZTEUSBCDCACMData_10_9.kext/Contents/Info.plist-                                                             <integer>1478</integer>
 ./ZTE/ZTEUSBCDCACMData.kext/Contents/Info.plist-                                                                   <integer>6610</integer>
 ./10.9/ZTEUSBCDCACMData_10_9.kext/Contents/Info.plist-                                                             <integer>6610</integer>
 ./ZTE/ZTEUSBMassStorageFilter.kext/Contents/Info.plist-                                                            <integer>6610</integer>
 ./10.9/ZTEUSBMassStorageFilter_10_9.kext/Contents/Info.plist-                                                      <integer>6610</integer>
 ./ZTE/cdc.kext/Contents/Info.plist-                                                                                <integer>6610</integer>
 ./10.9/cdc_10_9.kext/Contents/Info.plist-                                                                          <integer>6610</integer>
 ./10.10/ZTEUSBCDC_10.10.kext/Contents/Info.plist-                                                                  <integer>6610</integer>
 */

#define USB_VENDOR_ID_HUAWEI_12D1   4817
#define USB_VENDOR_ID_ZTE_19D2      6610
#define USB_VENDOR_ID_QUALCOMM_05C6 1478

#endif /* _GModems_h */
