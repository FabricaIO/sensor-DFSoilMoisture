#include "DFSoilMoisture.h"

/// @brief Creates a soil moisture sensor
/// @param Name The device name
/// @param Pin The analog pin to use
/// @param ConfigFile The name of the configuration file to use
DFSoilMoisture::DFSoilMoisture(String Name, int Pin, String ConfigFile) : GenericAnalogInput(Name, Pin, ConfigFile) {}

/// @brief Starts a soil moisture sensor object
/// @return True on success
bool DFSoilMoisture::begin() {
	bool result = false;
	if (GenericAnalogInput::begin()) {
		// Set description
		Description.type = "Environmental Sensor";
		Description.parameters = {"Soil Moisture", "Soil Moisture Raw"};
		Description.parameterQuantity = 2;
		Description.units = {"%Moisture", "mV"};
		values.resize(Description.parameterQuantity);
		if (!checkConfig(config_path)) {
			// Set defaults
			add_config.AirValue = 500;
			add_config.WaterValue = 200;
			result = saveConfig(config_path, getConfig());
		} else {
			// Load settings
			result = setConfig(Storage::readFile(config_path), false);
		}
	}
	return result;
}

/// @brief Takes a measurement
/// @return True on success
bool DFSoilMoisture::takeMeasurement() {
	int raw_value = getAnalogValue(analog_config.RollingAverage);
	values[0] = map(raw_value, add_config.AirValue, add_config.WaterValue, 0, 100);
	values[1] = raw_value;
	return true;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFSoilMoisture::getConfig() {
	// Allocate the JSON document
	JsonDocument doc = addAdditionalConfig();

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool DFSoilMoisture::setConfig(String config, bool save) {
	if (GenericAnalogInput::setConfig(config, false)) {
		// Allocate the JSON document
		JsonDocument doc;
		// Deserialize file contents
		DeserializationError error = deserializeJson(doc, config);
		// Test if parsing succeeds.
		if (error) {
			Logger.print(F("Deserialization failed: "));
			Logger.println(error.f_str());
			return false;
		}
		// Assign loaded values
		add_config.AirValue = doc["AirValue"].as<int>();
		add_config.WaterValue = doc["WaterValue"].as<int>();
		Description.parameters[0] = doc["ParamName"].as<String>();
		if (save) {
			return saveConfig(config_path, config);
		}
		return true;
	}
	return false;
}

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> DFSoilMoisture::calibrate(int step) {
	Logger.println("Calibrating soil moisture sensor, step " + String(step));
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
			add_config.AirValue = new_value;
			Logger.println("New air value: " + String(add_config.AirValue));
			response = { Sensor::calibration_response::next, "Submerge sensor in water to indicated max line, then click next." };
			break;
		case 2:
			new_value = getAnalogValue(false);
			for (int i = 0; i < 9; i++) {
				int temp_value = getAnalogValue(false);
				new_value = temp_value > new_value ? temp_value : new_value;
				delay(50);
			}
			add_config.WaterValue = new_value;
			Logger.println("New water value: " + String(add_config.WaterValue));
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

/// @brief Collects all the base class parameters and additional parameters
/// @return a JSON document with all the parameters
JsonDocument DFSoilMoisture::addAdditionalConfig() {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, GenericAnalogInput::getConfig());
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return doc;
	}
	doc["AirValue"] = add_config.AirValue;
	doc["WaterValue"] = add_config.WaterValue;
	doc["ParamName"] = Description.parameters[0];
	return doc;
}