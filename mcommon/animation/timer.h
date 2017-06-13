#pragma once

enum class TimerSample
{
    None,
    Restart
};

class Timer
{
public:
    Timer();
    static Timer& GlobalTimer();
    float GetTime() const;

    void Restart();

    float GetDelta(TimerSample sample = TimerSample::Restart);

private:
    int64_t m_startTime;
};