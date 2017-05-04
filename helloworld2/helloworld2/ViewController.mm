//
//  ViewController.m
//  helloworld2
//
//  Created by wanggang on 04/05/2017.
//  Copyright © 2017 wanggang. All rights reserved.
//

#import "ViewController.h"
#import "cplusapi.hpp"
#import "capi.h"

//head file need call swift API
#import "helloworld2-Swift.h"

#include <iostream>

using namespace std;

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    NSLog(@"hello world");
    
    //call c++ api, just rename xx.m to xx.mm
    Rectangle rect;
    rect.set_values (3,4);
    //cout << "area: " << rect.area() <<endl;

    //call c api
    myprint();
    
    //call swift code
    MyShape *shape1 = [[MyShape alloc] init];
    [shape1 simpleDescription];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end
