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

/// @brief Device for interfacing with analog soil moisture sensor
class DFSoilMoisture : public GenericAnalogInput {
	public:
		DFSoilMoisture(String Name, int Pin = A0, String ConfigFile = "SoilMoisture.json");
		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config, bool save);
		std::tuple<Sensor::calibration_response, String> calibrate(int step);
		
	protected:
		/// @brief Soil moisture sensor configuration
		struct {
			/// @brief The value for "air" (dry) 
			int AirValue;

			/// @brief The value for "water" (soaked)
			int WaterValue;
		} add_config;

		JsonDocument addAdditionalConfig();
};