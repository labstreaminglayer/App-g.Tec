#include "gUSBamp_LSL_interface.h"
#include <chrono>
#include <thread>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#endif
#include "gUSBamp.h"
#include "lsl_cpp.h"
#include <iostream>
#include <atomic>

struct as_buf
{
	OVERLAPPED *p_ovl;
	HANDLE hEvent;
	BYTE *recv_buffer;  // 
	float *src_buffer;  // Shares memory with recv_buffer starting after HEADER_SIZE.
	float last_mark;
};

gUSBamp_LSL_interface::gUSBamp_LSL_interface(gUSB_system_config sys_config)
    : m_sys_config(sys_config) {

	// From sampling rate, determine optimal NumberOfScans (i.e., number of samples per chunk).
	ptrdiff_t pos = std::distance(gUSBamp_sample_rates.begin(), std::find(gUSBamp_sample_rates.begin(), gUSBamp_sample_rates.end(), m_sys_config.SampleRate));
	m_sys_config.NumberOfScans = gUSBamp_buffer_sizes[pos];

	size_t dev_ix = 0;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		dev_cfg->Handle = GT_OpenDeviceEx(const_cast<char *>(dev_cfg->Serial.c_str()));

		// Apply the config to the device.
		GT_SetSampleRate(dev_cfg->Handle, (WORD)m_sys_config.SampleRate);
		GT_SetBufferSize(dev_cfg->Handle, (WORD)m_sys_config.NumberOfScans);

		std::vector<UCHAR> chans_active;
		dev_cfg->ActiveChannelId.clear();
		for (size_t chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			if (dev_cfg->Channels[chan_ix].Acquire)
			{
				chans_active.push_back((UCHAR)(chan_ix + 1));
				dev_cfg->ActiveChannelId.push_back(chan_ix);
			}
		}
		GT_SetChannels(dev_cfg->Handle, chans_active.data(), (UCHAR)chans_active.size());
		dev_cfg->ActiveChannelCount = chans_active.size();

		GT_EnableTriggerLine(dev_cfg->Handle, dev_cfg->TriggerEnabled);

		for (auto chan_ix : chans_active)
		{
			GT_SetBandPass(dev_cfg->Handle, chan_ix, dev_cfg->Channels[chan_ix - 1].BandpassFilterIndex);
			GT_SetNotch(dev_cfg->Handle, chan_ix, dev_cfg->Channels[chan_ix - 1].NotchFilterIndex);
			CHANNEL bipoChannel = { chan_ix, (UCHAR)dev_cfg->Channels[chan_ix - 1].BipolarChannel };
			GT_SetBipolar(dev_cfg->Handle, bipoChannel);
		}
		
		GND CommonGround = { dev_cfg->CommonGround[0], dev_cfg->CommonGround[1], dev_cfg->CommonGround[2],dev_cfg->CommonGround[3] };
		GT_SetGround(dev_cfg->Handle, CommonGround);

		REF CommonReference = { dev_cfg->CommonReference[0], dev_cfg->CommonReference[1], dev_cfg->CommonReference[2],dev_cfg->CommonReference[3] };
		GT_SetReference(dev_cfg->Handle, CommonReference);

		// BOOL status = GT_SetDRLChannel(dev_cfg->Handle, CHANNEL drlChannel);
		GT_EnableSC(dev_cfg->Handle, dev_cfg->ShortCutEnabled);
		GT_SetSlave(dev_cfg->Handle, !dev_cfg->IsMaster);
		
		
	}

	// Start the slaves before the master.
	for (auto & dev_cfg : m_sys_config.Devices) {
		if (!dev_cfg.IsMaster)
			GT_Start(dev_cfg.Handle);
	}
	// Start the master
	for (auto & dev_cfg : m_sys_config.Devices) {
		if (dev_cfg.IsMaster)
			GT_Start(dev_cfg.Handle);
	}
}

gUSBamp_LSL_interface::~gUSBamp_LSL_interface() {
	for (auto & dev_cfg : m_sys_config.Devices) {
		if (!dev_cfg.IsMaster)
		{
			GT_Stop(dev_cfg.Handle);
			GT_ResetTransfer(dev_cfg.Handle);
			GT_CloseDevice(&dev_cfg.Handle);
		}
	}
	for (auto & dev_cfg : m_sys_config.Devices) {
		if (dev_cfg.IsMaster)
		{
			GT_Stop(dev_cfg.Handle);
			GT_ResetTransfer(dev_cfg.Handle);
			GT_CloseDevice(&dev_cfg.Handle);
		}
	}
}

void gUSBamp_LSL_interface::getAcquisitionParameters(size_t &chunk_size, size_t &channel_count, uint32_t &sample_rate, std::string &serial, size_t &marker_count)
{
	sample_rate = m_sys_config.SampleRate;
	chunk_size = m_sys_config.NumberOfScans;
	channel_count = 0;
	marker_count = 0;
	size_t dev_ix = 0;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		channel_count += dev_cfg->ActiveChannelCount;
		if (dev_cfg->IsMaster)
			serial = dev_cfg->Serial;
		if (dev_cfg->TriggerEnabled)
			marker_count++;
	}
}

void gUSBamp_LSL_interface::getChannelLabels(std::vector<std::string> &out_labels) {
	size_t dev_ix = 0;
	out_labels.clear();
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		for (size_t chan_ix = 0; chan_ix < dev_cfg->ActiveChannelCount; chan_ix++)
		{
			out_labels.push_back(dev_cfg->Channels[dev_cfg->ActiveChannelId[chan_ix]].Label);
		}
	}
}

bool gUSBamp_LSL_interface::queueFetch() {
	size_t dev_ix = 0;
	bool success = true;
	// Send async io ops.
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		success &= (bool)GT_GetData(dev_cfg->Handle, p_as_buf[dev_ix].recv_buffer, (DWORD)dev_cfg->BufferSize, p_as_buf[dev_ix].p_ovl);
	}
	return success;
}

void ErrorDisplay(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

bool gUSBamp_LSL_interface::getData(std::vector<std::vector<float> > &out_buffer, double &out_timestamp, std::vector<std::pair<double, std::string>> &out_markers) {
	size_t dev_ix = 0;
	bool success = true;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		if (success)
		{
			DWORD result = WaitForSingleObject(p_as_buf[dev_ix].p_ovl->hEvent, 1000);
			success &= result == WAIT_OBJECT_0;
			if (!success)
			{
				if (result == WAIT_ABANDONED)
					ErrorDisplay(TEXT("WaitForSingleObject-Abandoned"));
				else if (result == WAIT_TIMEOUT)
					ErrorDisplay(TEXT("WaitForSingleObject-Timeout"));
				else if (result == WAIT_FAILED)
					ErrorDisplay(TEXT("WaitForSingleObject-Failed"));
			}
		}
	}

	size_t out_chan_offset = 0;
	dev_ix = 0;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		if (success)
		{
			if (dev_ix == 0)
				out_timestamp = lsl::local_clock();

			int add_trig = dev_cfg->TriggerEnabled ? 1 : 0;

			DWORD bytes_read;
			success &= GetOverlappedResult(dev_cfg->Handle, p_as_buf[dev_ix].p_ovl, &bytes_read, FALSE) != 0;
			if (!success)
				ErrorDisplay(TEXT("GetOverlappedResult"));
			else
			{
				success &= bytes_read == dev_cfg->BufferSize;
				if (!success)
					std::cout << "BufferSize: " << dev_cfg->BufferSize << "; bytes_read: " << bytes_read << std::endl;
				else {
					for (size_t sample_ix = 0; sample_ix < m_sys_config.NumberOfScans; sample_ix++)
					{
						for (size_t channel_ix = 0; channel_ix < dev_cfg->ActiveChannelCount; channel_ix++)
						{
							out_buffer[sample_ix][out_chan_offset + channel_ix] = dev_cfg->Scaling.Offset[dev_cfg->ActiveChannelId[channel_ix]]
								+ (dev_cfg->Scaling.ScalingFactor[dev_cfg->ActiveChannelId[channel_ix]]
									* p_as_buf[dev_ix].src_buffer[channel_ix + sample_ix * (dev_cfg->ActiveChannelCount + add_trig)]);
						}
					}
				}

			}
		}
		out_chan_offset += dev_cfg->ActiveChannelCount;

		if (dev_cfg->TriggerEnabled)
		{
			for (size_t sample_ix = 0; sample_ix < m_sys_config.NumberOfScans; sample_ix++)
			{
				float new_mark = p_as_buf[dev_ix].src_buffer[dev_cfg->ActiveChannelCount + sample_ix * (dev_cfg->ActiveChannelCount + 1)];
				if (new_mark != p_as_buf[dev_ix].last_mark)
				{
					double marker_time = out_timestamp + (sample_ix + 1 - m_sys_config.NumberOfScans) / m_sys_config.SampleRate;
					out_markers.push_back(std::pair<double, std::string>(marker_time, std::to_string(new_mark)));
					p_as_buf[dev_ix].last_mark = new_mark;
				}
			}
		}
	}
	return success;
}

// Static function to scan available devices and get basic config information for each.
void gUSBamp_LSL_interface::enumerateDevices(std::shared_ptr<gUSB_system_config> sys_config)
{
	sys_config->Devices.clear();
	for (int dev_ix = 0; dev_ix < CHAINED_DEVICES_MAX; dev_ix++)
	{
		HANDLE hDevice = GT_OpenDevice(dev_ix);
		if (hDevice)
		{
			char serial[16];
			if (GT_GetSerial(hDevice, serial, 16))
			{
				GDS_GUSBAMP_CONFIGURATION dev_cfg;
				dev_cfg.Serial = serial;
				
				SCALE scale;
				if (GT_GetScale(hDevice, &scale))
				{
					for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
					{
						dev_cfg.Scaling.Offset[chan_ix] = scale.offset[chan_ix];
						dev_cfg.Scaling.ScalingFactor[chan_ix] = scale.factor[chan_ix];
					}
				}
				GND CommonGround;
				if (GT_GetGround(hDevice, &CommonGround))
				{
					dev_cfg.CommonGround[0] = CommonGround.GND1;
					dev_cfg.CommonGround[1] = CommonGround.GND2;
					dev_cfg.CommonGround[2] = CommonGround.GND3;
					dev_cfg.CommonGround[3] = CommonGround.GND4;
				}
				REF CommonReference;
				if (GT_GetReference(hDevice, &CommonReference))
				{
					dev_cfg.CommonReference[0] = CommonReference.ref1;
					dev_cfg.CommonReference[1] = CommonReference.ref2;
					dev_cfg.CommonReference[2] = CommonReference.ref3;
					dev_cfg.CommonReference[3] = CommonReference.ref4;
				}
				sys_config->Devices.push_back(dev_cfg);
			}
			
			GT_CloseDevice(&hDevice);
		}
	}

	// Non device-specific things.
	sys_config->DriverVersion = GT_GetDriverVersion();

	int number_of_filters;
	if (GT_GetNumberOfFilter(&number_of_filters))
	{
		std::vector<FILT> bp_spec(number_of_filters);
		if (GT_GetFilterSpec(bp_spec.data()))
		{
			sys_config->available_bandpass_filters.clear();
			for (int filt_ix = 0; filt_ix < number_of_filters; filt_ix++)
			{
				GDS_FILTER_INFO this_filter;
				this_filter.Order = (uint32_t)bp_spec[filt_ix].order;
				this_filter.SamplingRate = bp_spec[filt_ix].fs;
				this_filter.LowerCutoffFrequency = bp_spec[filt_ix].fu;
				this_filter.UpperCutoffFrequency = bp_spec[filt_ix].fo;
				if (bp_spec[filt_ix].type == 2)
				{
					this_filter.TypeId = GDS_FILTER_TYPE_CHEBYSHEV;
				}
				else if (bp_spec[filt_ix].type == 1)
				{
					this_filter.TypeId = GDS_FILTER_TYPE_BUTTERWORTH;
				}
				sys_config->available_bandpass_filters.push_back(this_filter);
			}
		}
	}

	int number_of_notches;
	if (GT_GetNumberOfNotch(&number_of_notches))
	{
		std::vector<FILT> notch_spec(number_of_notches);
		if (GT_GetNotchSpec(notch_spec.data()))
		{
			sys_config->available_notch_filters.clear();
			for (int filt_ix = 0; filt_ix < number_of_notches; filt_ix++)
			{
				GDS_FILTER_INFO this_filter;
				this_filter.Order = (uint32_t)notch_spec[filt_ix].order;
				this_filter.SamplingRate = notch_spec[filt_ix].fs;
				this_filter.LowerCutoffFrequency = notch_spec[filt_ix].fu;
				this_filter.UpperCutoffFrequency = notch_spec[filt_ix].fo;
				if (notch_spec[filt_ix].type == 2)
				{
					this_filter.TypeId = GDS_FILTER_TYPE_CHEBYSHEV;
				}
				else if (notch_spec[filt_ix].type == 1)
				{
					this_filter.TypeId = GDS_FILTER_TYPE_BUTTERWORTH;
				}
				sys_config->available_notch_filters.push_back(this_filter);
			}
		}
	}
}

void gUSBamp_LSL_interface::recording_thread_function(std::atomic<bool>& shutdown) {
	
	// Get info for outlet.
	size_t chunk_size, channel_count, marker_count;
	uint32_t srate;
	std::string serial;
	getAcquisitionParameters(chunk_size, channel_count, srate, serial, marker_count);

	// Create brain signal outlet.
	std::vector<std::string> channel_labels;
	getChannelLabels(channel_labels);
	lsl::stream_info info("g.USBamp", "EEG", (int32_t)channel_count, (double)srate, lsl::cf_float32, serial);
	lsl::xml_element channels = info.desc().append_child("channels");
	for (int chan_ix = 0; chan_ix < channel_count; chan_ix++)
		channels.append_child("channel")
		.append_child_value("label", channel_labels[chan_ix].c_str())
		.append_child_value("type", "EEG")
		.append_child_value("unit", "microvolts");
	info.desc().append_child("acquisition")
		.append_child_value("manufacturer", "g.Tec")
		.append_child_value("serial_number", serial.c_str());
	lsl::stream_outlet outlet(info);

	// Create marker outlet.
	std::string marker_uid = serial + "_markers";
	lsl::stream_info marker_info("g.USBamp-Markers", "Markers", 1, lsl::IRREGULAR_RATE, lsl::cf_string, marker_uid);
	marker_info.desc().append_child("acquisition")
		.append_child_value("manufacturer", "g.Tec")
		.append_child_value("serial_number", serial.c_str());
	lsl::stream_outlet marker_outlet(marker_info);

	double lsl_timestamp = lsl::local_clock();
	std::vector<std::vector<float> > buffer(chunk_size, std::vector<float>(channel_count));
	std::vector<std::pair<double, std::string>> marker_buffer;
	marker_buffer.clear();

	// Async IO structures.
	// The only reason we have them as a member variable is so that they are available to getData,
	// because getData was previously called by an outside function.
	p_as_buf = new as_buf[CHAINED_DEVICES_MAX];
	size_t dev_ix = 0;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		p_as_buf[dev_ix].hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		p_as_buf[dev_ix].p_ovl = new OVERLAPPED;
		memset(p_as_buf[dev_ix].p_ovl, 0, sizeof(OVERLAPPED));
		p_as_buf[dev_ix].p_ovl->hEvent = p_as_buf[dev_ix].hEvent;
		p_as_buf[dev_ix].p_ovl->Offset = 0;
		p_as_buf[dev_ix].p_ovl->OffsetHigh = 0;
		ResetEvent(p_as_buf[dev_ix].hEvent);

		// Buffers
		int trig_ch = dev_cfg->TriggerEnabled ? 1 : 0;
		dev_cfg->BufferSize = (unsigned int)(m_sys_config.NumberOfScans * (dev_cfg->ActiveChannelCount + trig_ch) * sizeof(float) + HEADER_SIZE);
		p_as_buf[dev_ix].recv_buffer = new BYTE[dev_cfg->BufferSize];
		p_as_buf[dev_ix].src_buffer = reinterpret_cast<float*>(p_as_buf[dev_ix].recv_buffer + HEADER_SIZE);

		p_as_buf[dev_ix].last_mark = 0;
	}

	while (!shutdown) {
		queueFetch();
		if (getData(buffer, lsl_timestamp, marker_buffer)) {  // Get result of previously queued data fetch.
			outlet.push_chunk(buffer, lsl_timestamp);  // Push the result.
			if (marker_buffer.size() > 0)
			{
				for (auto mrk_pair : marker_buffer)
				{
					marker_outlet.push_sample(&mrk_pair.second, mrk_pair.first);
				}
			}
		}
		else {
			//break; // Acquisition was unsuccessful? -> Quit
		}
	}

	dev_ix = 0;
	for (auto dev_cfg = m_sys_config.Devices.begin(); dev_cfg != m_sys_config.Devices.end(); dev_cfg++, dev_ix++) {
		// Clear and delete buffers.
		if (p_as_buf[dev_ix].hEvent)
		{
			CloseHandle(p_as_buf[dev_ix].hEvent);
			p_as_buf[dev_ix].hEvent = NULL;
		}
		if (p_as_buf[dev_ix].p_ovl)
		{
			delete p_as_buf[dev_ix].p_ovl;
			p_as_buf[dev_ix].p_ovl = NULL;
		}
		if (p_as_buf[dev_ix].recv_buffer)
		{
			delete p_as_buf[dev_ix].recv_buffer;
			p_as_buf[dev_ix].recv_buffer = NULL;
		}
	}
	if (p_as_buf)
	{
		delete p_as_buf;
		p_as_buf = NULL;
	}

	// When `device` goes out of scope, its destructor should clean up the connection and buffers.
}