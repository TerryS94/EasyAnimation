## EasyAnimation
A small, lightweight single-header for animating floating point values over time using easing functions.

## Simple Example
```cpp
#include <iostream>
#include "EasyAnimation/EasyAnimation.h"
int main()
{
    EasyAnimation& manager = EasyAnimation::Instance();
                                          //animName //target*//min //max //dur //delay//iterations      //direction //easeFunction
    auto anim1 = manager.RegisterAnimation("forward",  nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Forward,  EaseFuncs::Linear);
    auto anim2 = manager.RegisterAnimation("backward", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Backward, EaseFuncs::EaseInOutSine);
    auto anim3 = manager.RegisterAnimation("pingpong", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::PingPong, EaseFuncs::EaseInOutCubic);
    anim1->Play();
    anim2->Play();
    anim3->Play();
    std::cout << "Running animations...\n";
    int i = 0;
    while (true)
    {
        manager.UpdateAll(0.1f);
        std::cout << "Step " << i << " | "
                  << "Forward: "  << anim1->GetValue() << " | "
                  << "Backward: " << anim2->GetValue() << " | "
                  << "PingPong: " << anim3->GetValue() << "\n";

        if (!anim1->IsRunning() && !anim2->IsRunning() && !anim3->IsRunning()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        i++;
    }
    std::cout << "Animations finished.\n";
    std::cin.get();
    return 0;
}
```
## output of the code above
```
Running animations...
Step 0  | Forward: 0.1 | Backward: 0.975528  | PingPong: 0.004
Step 1  | Forward: 0.2 | Backward: 0.904508  | PingPong: 0.032
Step 2  | Forward: 0.3 | Backward: 0.793893  | PingPong: 0.108
Step 3  | Forward: 0.4 | Backward: 0.654508  | PingPong: 0.256
Step 4  | Forward: 0.5 | Backward: 0.5       | PingPong: 0.5
Step 5  | Forward: 0.6 | Backward: 0.345491  | PingPong: 0.744
Step 6  | Forward: 0.7 | Backward: 0.206107  | PingPong: 0.892
Step 7  | Forward: 0.8 | Backward: 0.0954914 | PingPong: 0.968
Step 8  | Forward: 0.9 | Backward: 0.0244716 | PingPong: 0.996
Step 9  | Forward: 1   | Backward: 0         | PingPong: 1
Step 10 | Forward: 1   | Backward: 0         | PingPong: 0.996
Step 11 | Forward: 1   | Backward: 0         | PingPong: 0.968
Step 12 | Forward: 1   | Backward: 0         | PingPong: 0.892
Step 13 | Forward: 1   | Backward: 0         | PingPong: 0.744
Step 14 | Forward: 1   | Backward: 0         | PingPong: 0.5
Step 15 | Forward: 1   | Backward: 0         | PingPong: 0.256
Step 16 | Forward: 1   | Backward: 0         | PingPong: 0.108
Step 17 | Forward: 1   | Backward: 0         | PingPong: 0.0319999
Step 18 | Forward: 1   | Backward: 0         | PingPong: 0.00399995
Step 19 | Forward: 1   | Backward: 0         | PingPong: 0
Animations finished.
```
