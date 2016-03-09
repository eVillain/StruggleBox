//
//  Timer.h
//  DungeonSmith
//
//  Created by The Drudgerist on 20/04/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#ifndef __DungeonSmith__Timer__
#define __DungeonSmith__Timer__

#include <string>

class Timer {
public:
    // Universal timers
    static long Microseconds();
    static double Milliseconds();
    static double Seconds();
    static std::string TimeStamp();
};

#endif /* defined(__DungeonSmith__Timer__) */
