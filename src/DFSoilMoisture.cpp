#include "DFSoilMoisture.h"

/// @brief Creates a soil moisture sensor
/// @param Pin The analog pin to use
/// @param ConfigFile The name of the configuration file to use
DFSoilMoisture::DFSoilMoisture(int Pin, String ConfigFile) {
	path = "/settings/sen/" + ConfigFile;
	current_config.Pin = Pin;
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
	if (!checkConfig(path)) {
		// Set defaults
		current_config.AirValue = 500;
		current_config.WaterValue = 200;
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
	values[0] = map(rawMeasurement(), current_config.AirValue, current_config.WaterValue, 0, 100);
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

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> DFSoilMoisture::calibrate(int step) {
	Serial.println("Calibrating soil moisture sensor, step " + String(step));
	std::tuple<Sensor::calibration_response, String> response;
	int new_value;
	switch (step) {
		case 0:
			response = { Sensor::calibration_response::next, "Ensure sensor is completely dry, then click next." };
			break;
		case 1:
			new_value = rawMeasurement();
			for (int i = 0; i < 9; i++) {
				int temp_value = rawMeasurement();
				new_value = temp_value < new_value ? temp_value : new_value;
				delay(50);
			}
			current_config.AirValue = new_value;
			Serial.println("New air value: " + String(current_config.AirValue));
			response = { Sensor::calibration_response::next, "Submerge sensor in water to indicated max line, then click next." };
			break;
		case 2:
			new_value = rawMeasurement();
			for (int i = 0; i < 9; i++) {
				int temp_value = rawMeasurement();
				new_value = temp_value > new_value ? temp_value : new_value;
				delay(50);
			}
			current_config.WaterValue = new_value;
			Serial.println("New water value: " + String(current_config.WaterValue));
			if (saveConfig(path, getConfig())) {
				response = { Sensor::calibration_response::done, "Calibration successful" };
			} else {
				response = { Sensor::calibration_response::error, "Couldn't save new configuration" };
			}
			break;
		default:
		response = { Sensor::calibration_response::error, "No such calibration step: " + String(step) };
		break;
	}
	return response;
}

/// @brief Takes a war analog reading
/// @return The analog value
uint16_t DFSoilMoisture::rawMeasurement() {
	return analogRead(current_config.Pin);
}