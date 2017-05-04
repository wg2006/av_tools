//
//  cplusapi.cpp
//  helloworld2
//
//  Created by wanggang on 04/05/2017.
//  Copyright © 2017 wanggang. All rights reserved.
//

#include "cplusapi.hpp"
#include <iostream>

void Rectangle::set_values (int x, int y) {
    printf("This api is from C++ code\n\n");
    width = x;
    height = y;
}
