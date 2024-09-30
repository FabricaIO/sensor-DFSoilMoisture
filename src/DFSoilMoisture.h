/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman and Garth Johnson
 * 
 * External libraries needed:
 * ArduinoJSON: https://arduinojson.org/
 *
 * Soil moisture sensor: https://www.dfrobot.com/product-1385.html
 *
 * Contributors: Sam Groveman, Garth Johnson
 */

#pragma once
#include <GenericAnalogInput.h>
#include <Storage.h>
#include <ArduinoJson.h>

class DFSoilMoisture : public GenericAnalogInput {
	public:
		DFSoilMoisture(int Pin = A0, String ConfigFile = "SoilMoisture.json");
		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config);
		std::tuple<Sensor::calibration_response, String> calibrate(int step);
		
	private:
		/// @brief Soil moisture sensor configuration
		struct {
			/// @brief The value for "air" (dry) 
			int AirValue;

			/// @brief The value for "water" (soaked)
			int WaterValue;
		} current_config;
};