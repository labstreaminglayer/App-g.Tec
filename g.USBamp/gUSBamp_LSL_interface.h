#ifndef GUSBAMP_LSL_INTERFACE_H
#define GUSBAMP_LSL_INTERFACE_H
#include <cstdint>
#include <vector>
#include <memory>
#include <atomic>
#include "gUSBamp_config.h"

struct as_buf;  // Lightweight opaque wrapper around async buffer contents

class gUSBamp_LSL_interface {
public:
	explicit gUSBamp_LSL_interface(gUSB_system_config sys_config);
	~gUSBamp_LSL_interface();
	void getAcquisitionParameters(size_t &chunk_size, size_t &channel_count, uint32_t &sample_rate, std::string &serial, size_t &marker_count);
	void getChannelLabels(std::vector<std::string> &out_labels);
	bool getData(std::vector<std::vector<float> > &buffer, double &out_timestamp, std::vector<std::pair<double, std::string>> &out_markers);
	bool getStatus() { return true; }

	static void enumerateDevices(std::shared_ptr<gUSB_system_config> sys_config);
	void recording_thread_function(std::atomic<bool>& shutdown);
private:
	float_t counter;
	gUSB_system_config m_sys_config;
	as_buf *p_as_buf;
	bool queueFetch();
};

#endif // GUSBAMP_LSL_INTERFACE_H
