#include "gUSBamp_LSL_interface.h"
#include <chrono>
#include <thread>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "gUSBamp.h"

gUSBamp_LSL_interface::gUSBamp_LSL_interface(int32_t device_param)
    : counter(device_param) {
	// Here we would connect to the device
}

gUSBamp_LSL_interface::~gUSBamp_LSL_interface() {
	// Close the connection
}

bool gUSBamp_LSL_interface::getData(std::vector<int32_t>& buffer) {
	// Acquire some data and return it
	for (auto& elem : buffer) elem = counter++;
	std::this_thread::sleep_for(std::chrono::milliseconds(buffer.size() * 100));
	return true;
}
