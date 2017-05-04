//
//  cplusapi.hpp
//  helloworld2
//
//  Created by wanggang on 04/05/2017.
//  Copyright © 2017 wanggang. All rights reserved.
//

#ifndef cplusapi_hpp
#define cplusapi_hpp

#include <stdio.h>

class Rectangle {
    int width, height;
public:
    void set_values (int,int);
    int area() {return width*height;}
};

#endif /* cplusapi_hpp */
