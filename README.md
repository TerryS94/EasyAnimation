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
    Animation* anim1 = manager.RegisterAnimation("forward",  nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Forward,  EaseFuncs::Linear);
    Animation* anim2 = manager.RegisterAnimation("backward", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::Backward, EaseFuncs::EaseInOutSine);
    Animation* anim3 = manager.RegisterAnimation("pingpong", nullptr, 0.0f, 1.0f, 1.0f, 0.0f, 1, EAnimDirection::PingPong, EaseFuncs::EaseInOutCubic);
    anim1->Restart();
    anim2->Restart();
    anim3->Restart();

    std::cout << "Running animations...\n";
    int i = 0;
    while (true)
    {
        manager.UpdateAll(0.1f);

        std::cout << "Step " << i << " | "
                  << "Forward: " << anim1->GetValue() << " | "
                  << "Backward: " << anim2->GetValue() << " | "
                  << "PingPong: " << anim3->GetValue() << "\n";

        if (!anim1->IsRunning() && !anim2->IsRunning() && !anim3->IsRunning())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        i++;
    }
    std::cout << "Animations finished.\n";
    std::cin.get();
    return 0;
}

```

