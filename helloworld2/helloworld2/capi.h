//
//  capi.h
//  helloworld2
//
//  Created by wanggang on 04/05/2017.
//  Copyright © 2017 wanggang. All rights reserved.
//

#ifndef capi_h
#define capi_h

//compatible with c++ code call this api
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

void myprint();

#ifdef __cplusplus
}
#endif
    
#endif /* capi_h */
