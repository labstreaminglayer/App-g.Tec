#ifndef GUSBAMP_LSL_INTERFACE_H
#define GUSBAMP_LSL_INTERFACE_H
#include <cstdint>
#include <vector>
#include <memory>
#include <atomic>
#include "gUSBamp_config.h"


class gUSBamp_LSL_interface {
public:
	explicit gUSBamp_LSL_interface(gUSB_system_config sys_config);
	~gUSBamp_LSL_interface();
	void getAcquisitionParameters(size_t &chunk_size, size_t &channel_count, uint32_t &sample_rate, std::string &serial, size_t &marker_count);
	void getChannelLabels(std::vector<std::string> &out_labels);
	bool getStatus() { return true; }
	static void enumerateDevices(std::shared_ptr<gUSB_system_config> sys_config);
	void recording_thread_function(std::atomic<bool>& shutdown);
private:
	float_t counter;
	gUSB_system_config m_sys_config;
};

#endif // GUSBAMP_LSL_INTERFACE_H
