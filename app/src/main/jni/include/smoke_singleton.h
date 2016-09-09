//
//  smoke_singleton.hpp
//  smoke
//
//  Created by Maxim Ermakov on 9/8/16.
//  Copyright © 2016 Maxim Ermakov. All rights reserved.
//

#ifndef smoke_singleton_hpp
#define smoke_singleton_hpp

#include <stdio.h>
//
//  smoke_singleton.cpp
//  smoke
//
//  Created by Maxim Ermakov on 9/7/16.
//  Copyright © 2016 Maxim Ermakov. All rights reserved.
//
#include <iostream>
#include "smoke.h"

using namespace std;

class smoke_singleton
{
private:
    static bool instanceFlag;
    static smoke_singleton *single;
    smoke *smokeInstance;
    smoke_singleton()
    {
        //private constructor
    }
public:
    static smoke_singleton* getInstance();
    void run();
    void stop();
    ~smoke_singleton()
    {
        instanceFlag = false;
    }
};

#endif /* smoke_singleton_hpp */
