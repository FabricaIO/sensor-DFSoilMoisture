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
#include <Sensor.h>
#include <Storage.h>
#include <ArduinoJson.h>

class DFSoilMoisture : public Sensor {
	public:
		DFSoilMoisture(String ConfigFile = "SoilMoisture.json");
		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config);
		
	private:
		/// @brief Soil moisture sensor configuration
		struct {
			/// @brief The value for "air" (dry) 
			int AirValue;

			/// @brief The value for "water" (soaked)
			int WaterValue;

			/// @brief The analog pin to use
			int Pin;

		} current_config;

		/// @brief Path to settings file
		String path;
};