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
#include <algorithm>
#include <cmath>

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
	int initialIterations;
	EAnimDirection direction;
	EAnimDirection initialDirection;
	bool movingForward;//internal for PingPong mode
	std::function<float(float progress)> easeFunc {};
	float* optionalTargetValue = nullptr;
public:

public:
	inline Animation(float* optionalTargetValue, float minValue, float maxValue, float duration, float delay, int iterations, EAnimDirection direction, std::function<float(float)> easeFunc)
		: initialDirection(direction),
		  optionalTargetValue(optionalTargetValue), 
		  minValue(minValue), maxValue(maxValue), 
		  duration(std::max(duration, 0.001f)), 
		  delayLeft(delay), 
		  initialDelay(delay), 
		  iterations(iterations),
		  initialIterations(iterations),
		  direction(direction), 
		  easeFunc(std::move(easeFunc)), movingForward(true)
	{
		progressValue = (direction == EAnimDirection::Backward) ? maxValue : minValue;
	};
	inline ~Animation() {};
    inline void Update(float dt_seconds)
    {
        if (animState == EAnimState::STOPPED) return;
        if (dt_seconds <= 0.0f) return;
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
			if (optionalTargetValue) *optionalTargetValue = progressValue;
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
	//start/restart the animation in the desired direction that was passed in
	inline void Play()
	{
		iterations = initialIterations;
		direction = initialDirection;
		delayLeft = initialDelay;
		t = 0.0f;
		movingForward = true;
		progressValue = (direction == EAnimDirection::Backward) ? maxValue : minValue;
		if (direction == EAnimDirection::PingPong) { movingForward = true; progressValue = minValue; }
		if (optionalTargetValue) *optionalTargetValue = progressValue;
		animState = EAnimState::RUNNING;
	}
	//PlayReverse() does NOT permanently change the configured direction.
    //it computes a temporary "play reversed" state using initialDirection.
	inline void PlayReverse()
	{
		iterations = initialIterations;
		direction = initialDirection;
		delayLeft = initialDelay;
		t = 0.0f;
		if (direction == EAnimDirection::Forward)
		{
			direction = EAnimDirection::Backward;
			progressValue = maxValue;
		}
		else if (direction == EAnimDirection::Backward)
		{
			direction = EAnimDirection::Forward;
			progressValue = minValue;
		}
		else//PingPong
		{
			movingForward = false;
			progressValue = maxValue;
		}

		if (optionalTargetValue) *optionalTargetValue = progressValue;
		animState = EAnimState::RUNNING;
	}
	//stop and reset to the beginning
	inline void Stop()
	{
		animState = EAnimState::STOPPED;
		delayLeft = 0.0f;
		t = 0.0f;
		direction = initialDirection;
		iterations = initialIterations;
		movingForward = true;
		switch (direction)
		{
			case EAnimDirection::Forward:  progressValue = minValue; break;
			case EAnimDirection::Backward: progressValue = maxValue; break;
			case EAnimDirection::PingPong: progressValue = minValue; break;
		}
		if (optionalTargetValue) *optionalTargetValue = progressValue;
	}
	inline EAnimState GetState() const { return animState; }
	inline bool IsAnimationInfinite() const { return iterations == -1; }
	inline bool IsRunning() const { return animState == EAnimState::RUNNING; }
};
class EasyAnimation
{
private:
	std::mutex animationMutex;
	std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
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
	inline std::shared_ptr<Animation> RegisterAnimation(const std::string& animName, float* targetValue = nullptr, float minValue = 0.0f, float maxValue = 1.0f, float duration = 1.0f, float delay = 0.0f, int iterations = 1, EAnimDirection direction = EAnimDirection::Forward, std::function<float(float progress)> easeFunc = EaseFuncs::Linear)
	{
		if (!easeFunc) easeFunc = EaseFuncs::Linear;
		auto anim = std::make_shared<Animation>(targetValue, minValue, maxValue, duration, delay, iterations, direction, std::move(easeFunc));
		std::lock_guard<std::mutex> lock(animationMutex);
		animations.insert_or_assign(animName, anim);
		return anim;
	}
	inline std::shared_ptr<Animation> GetAnimation(const std::string& animName)
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		auto it = animations.find(animName);
		return (it == animations.end()) ? nullptr : it->second;
	}
	inline float GetValueForAnimation(const std::string& animName)
	{
		auto anim = GetAnimation(animName);
		return anim ? anim->GetValue() : 0.0f;
	}
	inline void Shutdown()
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		for (const auto& [_, anim] : animations) anim->Stop();
		animations.clear();
	}
	//call per frame.
	//for example, if using ImGui, call this before ImGui_ImplX_NewFrame() and pass ImGui::GetIO.DeltaTime into the parameter
	inline void UpdateAll(float dt_seconds)
	{
		std::lock_guard<std::mutex> lock(animationMutex);
		for (const auto& [_, anim] : animations) anim->Update(dt_seconds);
	}
};