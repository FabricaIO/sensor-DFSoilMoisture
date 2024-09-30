#include "DFSoilMoisture.h"

/// @brief Creates a soil moisture sensor
/// @param Pin The analog pin to use
/// @param ConfigFile The name of the configuration file to use
DFSoilMoisture::DFSoilMoisture(int Pin, String ConfigFile) : GenericAnalogInput(Pin, ConfigFile) {}

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
	if (!checkConfig(config_path)) {
		// Set defaults
		current_config.AirValue = 500;
		current_config.WaterValue = 200;
		result = saveConfig(config_path, getConfig());
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path));
	}
	return result;
}

/// @brief Takes a measurement
/// @return True on success
bool DFSoilMoisture::takeMeasurement() {
	values[0] = map(getAnalogValue(analog_config.RollingAverage), current_config.AirValue, current_config.WaterValue, 0, 100);
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
	doc["Pin"] = analog_config.Pin;
	doc["ADC_Voltage_mv"] = analog_config.ADC_Voltage_mv;
	doc["ADC_Resolution"] = analog_config.ADC_Resolution;
	doc["RollingAverage"] = analog_config.RollingAverage;
	doc["AverageSize"] = analog_config.AverageSize;

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
	analog_config.Pin = doc["Pin"].as<int>();
	analog_config.ADC_Voltage_mv = doc["ADC_Voltage_mv"].as<int>();
	analog_config.ADC_Resolution = doc["ADC_Resolution"].as<int>();
	analog_config.RollingAverage = doc["RollingAverage"].as<bool>() ;
	analog_config.AverageSize = doc["AverageSize"].as<int>();
	configureInput();
	return saveConfig(config_path, config);
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
			new_value = getAnalogValue(false);
			for (int i = 0; i < 9; i++) {
				int temp_value = getAnalogValue(false);
				new_value = temp_value < new_value ? temp_value : new_value;
				delay(50);
			}
			current_config.AirValue = new_value;
			Serial.println("New air value: " + String(current_config.AirValue));
			response = { Sensor::calibration_response::next, "Submerge sensor in water to indicated max line, then click next." };
			break;
		case 2:
			new_value = getAnalogValue(false);
			for (int i = 0; i < 9; i++) {
				int temp_value = getAnalogValue(false);
				new_value = temp_value > new_value ? temp_value : new_value;
				delay(50);
			}
			current_config.WaterValue = new_value;
			Serial.println("New water value: " + String(current_config.WaterValue));
			if (saveConfig(config_path, getConfig())) {
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