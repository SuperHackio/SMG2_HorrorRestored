#include "HorrorLimitArea.h"

HorrorLimitArea::HorrorLimitArea(const char* pName) : AreaObj(pName)
{

}

const char* HorrorLimitArea::getManagerName() const
{
	return "HorrorLimit";
}