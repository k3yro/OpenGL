#pragma once

#include "KameraFPS.hpp"

class KameraFloating : public KameraFPS
{
public:
	KameraFloating(float fov, float width, float height) : KameraFPS(fov, width, height){}

	void moveUp(float amount)
	{
		translate(up * amount);
	}
};

