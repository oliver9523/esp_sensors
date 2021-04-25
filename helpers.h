#pragma once

#ifndef _HELPERS_h
#define _HELPERS_h

#include "arduino.h"

//converts ms to s
#define SECONDS(X) (X*1000)

//enumns used throughout
enum SENSOR_TYPE { NOTSET, ANALOG, DIGITAL };
enum TRIGGER_ZONE { LOWER, NONE, UPPER };
enum STATE { ON, OFF };

//list of available mappings
enum MAPPINGS {
	NO_MAPPING,
	TMP36,
	BATTERY,
	NORM_3V,
	INV_NORM_3V
};


namespace helper_functions
{
	String enum2str(MAPPINGS _mapping) {
		switch (_mapping)
		{
		case NO_MAPPING:
			return "none";
		case TMP36:
			return "tmp36";
		case BATTERY:
			return "battery";
		case NORM_3V:
			return "norm3v";
		case INV_NORM_3V:
			return "inv_norm3v";
		default:
			return "none";
		}
	}

#pragma region mappings
	//mapping functions that take the ADC voltage 0-3.3v and converts them to another range

	float v2tmp36(float v) {
		//maps ADC voltage to TMP36
		return ((v * 1000) - 500) / 10; //temp_c
	}

	float battery(float v) {
		//scales Vout to Vin
		//scale based on potential divider Vin---[56k]--|Vout|--[10k]---0v 
		//Vout = Vin * R2/(R1+R2)
		return v * 6.6;
	}

	float none(float v) {
		return v;
	}

	float norm3v(float v) {
		return v / 3.3;
	}

	float inv_norm3v(float v) {
		return 1.0 - (v / 3.3);
	}

#pragma endregion mappings
}



#endif