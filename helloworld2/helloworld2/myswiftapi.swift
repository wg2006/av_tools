//
//  myswiftapi.swift
//  helloworld2
//
//  Created by wanggang on 04/05/2017.
//  Copyright © 2017 wanggang. All rights reserved.
//

import Foundation

//in order to call this api from object C, the class must be from NSObject
class MyShape:NSObject {
    var numberOfSides = 0
    func simpleDescription() -> String {
        print("This api is from swift code\n");
        return "A shape with \(numberOfSides) sides."
    }
}
