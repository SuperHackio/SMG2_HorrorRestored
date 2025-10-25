#pragma once

#include "syati.h"

class HorrorLimitArea : public AreaObj
{
public:
	HorrorLimitArea(const char* pName);

	virtual const char* getManagerName() const;
};