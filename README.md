## EasyAnimation
A small, lightweight single-header for animating floating point values over time using easing functions.

## Simple Example
```cpp
#include <iostream>
#include "EasyAnimation/EasyAnimation.h"
int main()
{
    EasyAnimation& manager = EasyAnimation::Instance();
                                          //animName //target*//from //to //dur //delay//iterations      //direction //easeFunction
    auto anim1 = manager.RegisterAnimation("forward",  nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Forward,  EaseFuncs::Linear);
    auto anim2 = manager.RegisterAnimation("backward", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Backward, EaseFuncs::Linear);
    auto anim3 = manager.RegisterAnimation("pingpong", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::PingPong, EaseFuncs::Linear);
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
Step 0 | Forward: 0.1 | Backward: 0.9 | PingPong: 0.2
Step 1 | Forward: 0.2 | Backward: 0.8 | PingPong: 0.4
Step 2 | Forward: 0.3 | Backward: 0.7 | PingPong: 0.6
Step 3 | Forward: 0.4 | Backward: 0.6 | PingPong: 0.8
Step 4 | Forward: 0.5 | Backward: 0.5 | PingPong: 1.0
Step 5 | Forward: 0.6 | Backward: 0.4 | PingPong: 0.8
Step 6 | Forward: 0.7 | Backward: 0.3 | PingPong: 0.6
Step 7 | Forward: 0.8 | Backward: 0.2 | PingPong: 0.4
Step 8 | Forward: 0.9 | Backward: 0.1 | PingPong: 0.2
Step 9 | Forward: 1.0 | Backward: 0.0 | PingPong: 0.0
Animations finished.
```
