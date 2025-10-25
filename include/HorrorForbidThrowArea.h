#pragma once

#include "syati.h"

class HorrorForbidThrowArea : public AreaObj
{
public:
	HorrorForbidThrowArea(const char* pName);

	virtual const char* getManagerName() const;
};