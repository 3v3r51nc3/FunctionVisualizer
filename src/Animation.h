#pragma once
#include <cmath>

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float EaseLinear(float t) { return t; }
inline float EaseInQuad(float t) { return t * t; }
inline float EaseOutQuad(float t) { return t * (2 - t); }
inline float EaseInOutQuad(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

struct ScaleAnimation {
    bool active = false;
    float start = 1.0f;
    float target = 1.0f;
    double startTime = 0.0;
    double duration = 0.5; // seconds
    float lastValue = 1.0f;

    void Start(float current, float newTarget, double now, float (*easingFn)(float) = EaseInOutQuad) {
        if (active) {
            // compute current value at "now"
            float t = float((now - startTime) / duration);
            if (t >= 1.0f) {
                // finished anyway, just start new
                start = target;
            }
            else {
                start = Lerp(start, target, easingFn(t));
            }
        }
        else {
            start = current;
        }
        target = newTarget;
        startTime = now;
        active = true;
    }

    float Update(double now, float (*easingFn)(float)) {
        if (!active) return target;
        float t = float((now - startTime) / duration);
        if (t >= 1.0f) {
            active = false;
            lastValue = target;
            return target;
        }
        lastValue = Lerp(start, target, easingFn(t));
        return lastValue;
    }
};
