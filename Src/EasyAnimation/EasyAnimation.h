#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

//https://easings.net/
namespace EaseFuncs
{
	static constexpr float PI = 3.14159265358979323846f;
	inline float Linear(float p) { return p; }
	inline float EaseInSine(float p) { return 1.0f - std::cosf(p * PI / 2.0f); }
	inline float EaseOutSine(float p) { return std::sinf(p * PI / 2.0f); }
	inline float EaseInOutSine(float p) { return -(std::cosf(PI * p) - 1.0f) / 2.0f; }
	inline float EaseOutCubic(float p) { const float reciprocal = 1.0f - p; return 1.0f - (reciprocal * reciprocal * reciprocal); }
	inline float EaseInOutCubic(float p) { if (p < 0.5f) return 4.0f * p * p * p; return 1.0f - (powf(-2.0f * p + 2.0f, 3.0f) / 2.0f); }
	inline float EaseOutQuint(float p) { const float reciprocal = 1.0f - p; return 1.0f - (reciprocal * reciprocal * reciprocal * reciprocal * reciprocal); }
	inline float EaseInOutCirc(float p) { return p < 0.5f ? (1.0f - sqrtf(1.0f - powf(2.0f * p, 2.0f))) / 2.0f : (sqrtf(1.0f - powf(-2.0f * p + 2.0f, 2.0f)) + 1.0f) / 2.0f; }
	inline float EaseInOutQuad(float p) { return p < 0.5f ? 2.0f * p * p : 1.0f - powf(-2.0f * p + 2.0f, 2.0f) / 2.0f; }
	inline float EaseInExpo(float p) { return p == 0.0f ? 0.0f : powf(2.0f, 10.0f * p - 10.0f); }
	inline float EaseOutExpo(float p) { return p == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * p); }
}
enum class EAnimState : uint8_t
{
	STOPPED,
	RUNNING,
};
enum class EAnimDirection : uint8_t
{
	Forward,  //1 iteration = min -> max
	Backward, //1 iteration = max -> min
	PingPong  //1 iteration = min -> max -> min
};
class Animation
{
private:
	float t = 0.0f;//normalized time progress for the current phase
	float initialDelay;
	float delayLeft;
	float minValue;
	float maxValue;
	float progressValue;
	float duration;
	EAnimState animState = EAnimState::STOPPED;
	int iterations;//-1 for inf iterations
	EAnimDirection direction;
	bool movingForward;//internal for PingPong mode
	std::function<float(float progress)> easeFunc {};
	float* optionalTargetValue = nullptr;
public:

public:
	inline Animation(float* optionalTargetValue, float minValue, float maxValue, float duration, float delay, int iterations, EAnimDirection direction, std::function<float(float)> easeFunc)
		: optionalTargetValue(optionalTargetValue), minValue(minValue), maxValue(maxValue), duration(duration), delayLeft(delay), initialDelay(delay), iterations(iterations), direction(direction), easeFunc(std::move(easeFunc)), movingForward(true)
	{
		progressValue = (direction == EAnimDirection::Backward) ? maxValue : minValue;
	};
	inline ~Animation() {};
    inline void Update(float dt_seconds)
    {
        if (animState == EAnimState::STOPPED) return;
        if (dt_seconds <= 0.0f) return;
        duration = std::max(duration, 0.001f);
        if (delayLeft > 0.0f)
        {
            delayLeft -= dt_seconds;
            if (delayLeft > 0.0f) return;
			dt_seconds = -delayLeft;
            delayLeft = 0.0f;
            if (dt_seconds <= 0.0f) return;
        }
        const float range = maxValue - minValue;
        if (range == 0.0f)
        {
            progressValue = minValue;
            animState = EAnimState::STOPPED;
            return;
        }
        float remainingTime = dt_seconds;
        while (remainingTime > 0.0f && animState == EAnimState::RUNNING)
        {
            const float dtToNorm = remainingTime / duration;
            const float normLeft = 1.0f - t;
            if (dtToNorm < normLeft)
            {
                t += dtToNorm;
                remainingTime = 0.0f;
            }
            else
            {
                t = 1.0f;
				const float timeToFinishPhase = normLeft * duration;
                remainingTime -= timeToFinishPhase;
            }
            const float eased = easeFunc ? easeFunc(t) : t;
            if (direction == EAnimDirection::Forward) progressValue = minValue + range * eased;
            else if (direction == EAnimDirection::Backward) progressValue = maxValue - range * eased;
            else progressValue = movingForward ? (minValue + range * eased) : (maxValue - range * eased);

            if (t < 1.0f) break;
            if (direction == EAnimDirection::PingPong)
            {
                if (movingForward)
                {
                    movingForward = false;
                    t = 0.0f;
                }
                else
                {
                    movingForward = true;
                    t = 0.0f;
                    if (iterations != -1)
                    {
                        --iterations;
                        if (iterations <= 0)
                        {
                            animState = EAnimState::STOPPED;
                            progressValue = minValue;
                            break;
                        }
                    }
                }
            }
            else
            {
                t = 0.0f;
                if (iterations != -1)
                {
                    --iterations;
                    if (iterations <= 0)
                    {
                        animState = EAnimState::STOPPED;
                        if (direction == EAnimDirection::Forward)  progressValue = maxValue;
                        if (direction == EAnimDirection::Backward) progressValue = minValue;
                        break;
                    }
                }
            }
        }
		if (optionalTargetValue) *optionalTargetValue = progressValue;
    }
	inline float GetValue() const { return progressValue; }
	inline void Restart()
	{ 
		movingForward = true;
		t = 0.0f;
		progressValue = (direction == EAnimDirection::Backward) ? maxValue : minValue;
		if (direction == EAnimDirection::Backward) movingForward = false;
		animState = EAnimState::RUNNING;
	}
	inline void Stop()
	{
		switch (direction)
		{
			case EAnimDirection::Forward:  progressValue = maxValue; break;
			case EAnimDirection::Backward: progressValue = minValue; break;
			case EAnimDirection::PingPong: progressValue = minValue; movingForward = true; break;
		}
		animState = EAnimState::STOPPED;
	}
	inline EAnimState GetState() const { return animState; }
	inline bool IsAnimationInfinite() const { return iterations < 1; }
	inline bool IsRunning() const { return animState == EAnimState::RUNNING; }
};
class EasyAnimation
{
private:
	std::mutex animationMutex;
	std::unordered_map<std::string, std::unique_ptr<Animation>> animations;
	std::thread::id UpdateAll_ThreadID{};
public:
	EasyAnimation() = default;
	~EasyAnimation() = default;
	EasyAnimation(const EasyAnimation&) = delete;
	EasyAnimation& operator=(const EasyAnimation&) = delete;
	EasyAnimation(EasyAnimation&&) = delete;
	EasyAnimation& operator=(EasyAnimation&&) = delete;

	static EasyAnimation& Instance()
	{
		static EasyAnimation instance;
		return instance;
	}
	//creates/recreates your animation and returns a pointer to it for convenience
	inline Animation* RegisterAnimation(const std::string& animName, float* targetValue = nullptr, float minValue = 0.0f, float maxValue = 1.0f, float duration = 1.0f, float delay = 0.0f, int iterations = 1, EAnimDirection direction = EAnimDirection::Forward, std::function<float(float progress)> easeFunc = EaseFuncs::Linear)
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		duration = std::max(duration, 0.001f);
		if (!easeFunc) easeFunc = EaseFuncs::Linear;
		auto anim = std::make_unique<Animation>(targetValue, minValue, maxValue, duration, delay, iterations, direction, std::move(easeFunc));
		animations[animName] = std::move(anim);
		return animations[animName].get();
	}
	inline const Animation* GetAnimation(const std::string& animName)
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		auto it = animations.find(animName);
		if (it == animations.end()) return nullptr;
		return it->second.get();
	}
	inline void Shutdown()
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		for (const auto& [_, anim] : animations) anim.get()->Stop();
		animations.clear();
	}
	//if the animation iterations is not inf and the current thread id is different than the thread id that UpdateAll is called on, then sleep until the animation is over.
	//A scenario this would be useful for is if you unloaded a dll but wanted to wait for an anim to finish on the render thread before your ejection code runs in your main thread.
	inline void WaitForAnimationFinish(const std::string& animName)
	{
		const Animation* anim = GetAnimation(animName);
		if (!anim)
		{
			MessageBox(nullptr, std::format("[EasyAnimation] WaitForAnimationFinish was called for \"{}\" but it doesn't exist! Bailing out...", animName).c_str(), "Oops...", MB_OK);
			return;
		}
		const bool isDifferentThread = UpdateAll_ThreadID != std::this_thread::get_id();
		if (!isDifferentThread)
		{
			MessageBox(nullptr, std::format("[EasyAnimation] WaitForAnimationFinish was called for \"{}\" but it was called on the same thread as Update function! Bailing out...", animName).c_str(), "Oops...", MB_OK);
			return;
		}
		if (anim->IsAnimationInfinite())
		{
			MessageBox(nullptr, std::format("[EasyAnimation] WaitForAnimationFinish was called for \"{}\" but it's an inf animation! Bailing out...", animName).c_str(), "Oops...", MB_OK);
			return;
		}
		while (anim->IsRunning())
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	//call per frame.
	//for example, if using ImGui, call this before ImGui_ImplX_NewFrame() and pass ImGui::GetIO.DeltaTime into the parameter
	inline void UpdateAll(float dt_seconds)
	{
		UpdateAll_ThreadID = std::this_thread::get_id();
		std::lock_guard<std::mutex> lock(animationMutex);
		for (const auto& [_, anim] : animations) anim.get()->Update(dt_seconds);
	}
};