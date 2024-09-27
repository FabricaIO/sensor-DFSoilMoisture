#include "DFSoilMoisture.h"

/// @brief Creates a soil moisture sensor
/// @param ConfigFile The name of the configuration file to use
DFSoilMoisture::DFSoilMoisture(String ConfigFile) {
	path = "/settings/sen/" + ConfigFile;
}

/// @brief Starts a Data Template object
/// @return True on success
bool DFSoilMoisture::begin() {
	values.resize(1);
	Description.parameterQuantity = 1;
	Description.type = "Environmental Sensor";
	Description.name = "Soil Moisture Sensor";
	Description.parameters = {"Soil Moisture"};
	Description.units = {"%Moisture"};
	Description.id = 2;
	bool result = false;
	// Create settings directory if necessary
	if (!checkConfig(path)){
		// Set defaults
		current_config = { .AirValue = 500, .WaterValue = 200, .Pin = A0};
		result = saveConfig(path, getConfig());
	} else {
		// Load settings
		result = setConfig(Storage::readFile(path));
	}
	return result;
}

/// @brief Takes a measurement
/// @return True on success
bool DFSoilMoisture::takeMeasurement() {
	int rawValue = analogRead(A0);
	int calValue = map(rawValue, current_config.AirValue, current_config.WaterValue, 0, 100);
	values[0] = calValue;
	return true;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFSoilMoisture::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["AirValue"] = current_config.AirValue;
	doc["WaterValue"] = current_config.WaterValue;
	doc["Pin"] = current_config.Pin;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool DFSoilMoisture::setConfig(String config) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		return false;
	}
	// Assign loaded values
	current_config.AirValue = doc["AirValue"].as<int>();
	current_config.WaterValue = doc["WaterValue"].as<int>();
	current_config.Pin = doc["Pin"].as<int>();
	pinMode(current_config.Pin, INPUT);
	return saveConfig(path, config);
}
