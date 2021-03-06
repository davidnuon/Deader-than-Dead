#include "clock.h"

Clock::Clock()
{
	curTimeStamp = 0;
	prevTimeStamp = 0;
	totalMsCount = 0;
	timeElapsed = 0;
}

bool Clock::init()
{
	if(SDL_Init( SDL_INIT_TIMER ) == -1 )
		return false;

	curTimeStamp = 0;
	prevTimeStamp = 0;
	totalMsCount = 0;
	timeElapsed = 0;

	return true;
}

void Clock::update()
{
	prevTimeStamp = curTimeStamp; //Saves the previous frame's data.
	curTimeStamp = SDL_GetTicks(); //Gather the current frame's data.
	totalMsCount += (curTimeStamp -prevTimeStamp); //increments the raw total ms count.

	if(totalMsCount >= TIME_FOR_60_FPS)
		timeElapsed = totalMsCount;
}

bool Clock::allowTick()
{
	if(totalMsCount >= TIME_FOR_60_FPS)
	{
		totalMsCount = 0;
		return true;
	}
	return false;
}

