/////////////////////////////////////////////////////////////
// Author: Patrick Wuerflinger
// Filename: APDS9301.cpp
// Description: Source file for the APDS9301 sensor
// Revision: 0
/////////////////////////////////////////////////////////////

#include "APDS9301.h"
#include "Visitor.h"
#include <math.h> 
#include <iostream>

const size_t RAW_VAL_LENGTH = 4;

void APDS9301::measure()
{
	std::string rawDataString = readDeviceDriver();
	if (!rawDataString.empty() && rawDataString.size() == RAW_VAL_LENGTH)
	{
		// read both 16 bit channel registers and perform calculation according to datasheet (page 4/20)		
		auto rawDataArray = reinterpret_cast<const uint8_t*>(&rawDataString[0]);	
		// Channel 0: 1st & 2nd Byte
		uint16_t rawCH0 = ((((uint16_t)rawDataArray[1]) << 8) | (uint16_t)rawDataArray[0]);	
		// Channel 1: 3st & 4th Byte
		uint16_t rawCH1 = ((((uint16_t)rawDataArray[3]) << 8) | (uint16_t)rawDataArray[2]);
		float channelRatio = (float)rawCH1 / (float)rawCH0;
		float brightness = 0.0;
		
		if (channelRatio > 0 && channelRatio <= 0.5)
			brightness = (0.0304 * rawCH0) - (0.062 * rawCH0 * pow(channelRatio, 1.4));
		else if (channelRatio > 0.5 && channelRatio <= 0.61)
			brightness = (0.0224 * rawCH0) - (0.031 * rawCH1);
		else if (channelRatio > 0.61 && channelRatio <= 0.8)
			brightness = (0.0128 * rawCH0) - (0.0153 * rawCH1);
		else if (channelRatio > 0.8 && channelRatio <= 1.3)
			brightness = (0.00146 * rawCH0) - (0.00112 * rawCH1);
		else
			brightness = 0.0;		
		
		mBrightness = static_cast <uint32_t>(brightness);
		
		// set the brightness for the seven seg display
		mSevenSeg->setBrightness(calcBrightnessForSevenSeg());		
	}
	else
	{
		throw std::string{ "APDS9301:measure: Device driver return invalid raw data! " };	
	}
}

PublishDataVec_t APDS9301::accept(std::shared_ptr<Visitor> const & visitor)
{
	if (visitor != nullptr)
	{
		return visitor->visitSensor(shared_from_this());
	}
	else
	{
		throw std::string{ "APDS9301:accept: no nullptr allowed! " };
	}
}

uint32_t APDS9301::getBrightness() const
{
	return mBrightness;
}


uint8_t APDS9301::calcBrightnessForSevenSeg() const
{
	uint8_t brightness = 0;
	if (mBrightness < MAX_BRIGHTNESS && mBrightness > MIN_VISIBLE_BRIGHTNESS)
	{
		brightness = mBrightness;
	}
	else if (mBrightness < MIN_VISIBLE_BRIGHTNESS)
	{
		brightness = MAX_BRIGHTNESS;
	}
	else if (mBrightness > MAX_BRIGHTNESS)
	{
		brightness = MIN_VISIBLE_BRIGHTNESS;
	}
	return brightness;
}
