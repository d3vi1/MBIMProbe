//
//  MBIMProbe.c
//  MBIMProbe
//
//  Created by Răzvan Corneliu C.R. VILT on 30.04.2014.
//  Copyright (c) 2014 Răzvan Corneliu C.R. VILT. All rights reserved.
//

#include <mach/mach_types.h>

kern_return_t MBIMProbe_start(kmod_info_t * ki, void *d);
kern_return_t MBIMProbe_stop(kmod_info_t *ki, void *d);

kern_return_t MBIMProbe_start(kmod_info_t * ki, void *d)
{
    return KERN_SUCCESS;
}

kern_return_t MBIMProbe_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
}
