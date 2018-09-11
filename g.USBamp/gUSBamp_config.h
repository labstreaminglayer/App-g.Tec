#ifndef GUSBAMP_CONFIG_H
#define GUSBAMP_CONFIG_H
#include <stdint.h>
#include <cstddef>

// ========================================================================================
// Constants
// ========================================================================================
#define GDS_GUSBAMP_CHANNELS_MAX 16
#define GDS_GUSBAMP_GROUPS_MAX 4
#define GDS_GUSBAMP_ASYNC_DIGITAL_IO_CHANNELS_MAX 4
#define GDS_GUSBAMP_DEVICE_INFORMATION_LENGTH_MAX 256
#define CHAINED_DEVICES_MAX 4
const std::vector<uint32_t> gUSBamp_sample_rates = { 32, 64, 128, 256, 512, 600, 1200, 2400, 4800, 9600, 19200, 38400 };

// ========================================================================================
// Enumerations
// ========================================================================================
typedef enum
{
	GDS_FILTER_TYPE_CHEBYSHEV = 0,
	GDS_FILTER_TYPE_BUTTERWORTH = 1,
	GDS_FILTER_TYPE_BESSEL = 2
} GDS_FILTER_TYPE;

typedef enum
{
	GDS_CHANNEL_DIRECTION_IN,
	GDS_CHANNEL_DIRECTION_OUT
} GDS_CHANNEL_DIRECTION;

typedef enum
{
	GDS_GUSBAMP_WAVESHAPE_SQUARE,
	GDS_GUSBAMP_WAVESHAPE_SAWTOOTH,
	GDS_GUSBAMP_WAVESHAPE_SINE,
	GDS_GUSBAMP_WAVESHAPE_DRL,
	GDS_GUSBAMP_WAVESHAPE_NOISE
} GDS_GUSBAMP_WAVESHAPE;

// ========================================================================================
// Structures
// ========================================================================================
typedef struct _GDS_GUSBAMP_SAMPLING_RATE_FEATURES
{
	uint32_t SamplingRate;
	uint16_t RecommendedNumberOfScans;
} GDS_GUSBAMP_SAMPLING_RATE_FEATURES, *PGDS_GUSBAMP_SAMPLING_RATE_FEATURES;

//! Describes the state of an asynchronous digital I/O channel.
typedef struct _GDS_GUSBAMP_ASYNC_DIGITAL_IO_CHANNEL
{
	//! The one-based channel number of the digital I/O channel described by this instance.
	unsigned int ChannelNumber;

	//! Indicates if the channel described by this instance is a digital input or output channel.
	GDS_CHANNEL_DIRECTION Direction;

	//! The digital value of the channel.
	bool Value;
} GDS_GUSBAMP_ASYNC_DIGITAL_IO_CHANNEL, *PGDS_GUSBAMP_ASYNC_DIGITAL_IO_CHANNEL;

//! The scaling values of each channel of a single g.USBamp device.
typedef struct _GDS_GUSBAMP_SCALING
{
	//! The offset for each chanel in microvolts.
	float Offset[GDS_GUSBAMP_CHANNELS_MAX];

	//! The scaling factor for each channel.
	float ScalingFactor[GDS_GUSBAMP_CHANNELS_MAX];
} GDS_GUSBAMP_SCALING, *PGDS_GUSBAMP_SCALING;

//! The configuration of the internal signal generator.
typedef struct _GDS_GUSBAMP_SIGNAL_GENERATOR_CONFIGURATION
{
	bool Enabled;
	GDS_GUSBAMP_WAVESHAPE WaveShape;
	uint32_t Frequency;  // Valid values range from 1 Hz to 100 Hz.
	int32_t Amplitude;  // Valid values range from -250 mV to +250 mV.
	int32_t Offset;  // Valid values range from -200 mV to +200 mV.

} GDS_GUSBAMP_SIGNAL_GENERATOR_CONFIGURATION, *PGDS_GUSBAMP_SIGNAL_GENERATOR_CONFIGURATION;

//! The configuration of a single channel.
typedef struct _GDS_GUSBAMP_CHANNEL_CONFIGURATION
{
	bool Acquire;

	//! The bipolar derivation setting for the channel.
	/*!
	Set this value to zero if unipolar derivation should be used with ground and reference electrodes that are connected to the ground and reference socket of the group that this channel belongs to.
	Otherwise, if bipolar derivation should be performed for the channel that this configuration belongs to, set \ref BipolarChannel to the one-based channel number whose signal should be subtracted from the measured signal of this channel.
	*/
	uint32_t BipolarChannel;

	//! The zero-based index of the bandpass filter to use for the channel.
	/*!
	The index is related to the list of available bandpass filters that are returned by \ref GDS_GUSBAMP_GetBandpassFilters.
	A value of -1 indicates that no bandpass filter is used for the channel.
	*/
	int32_t BandpassFilterIndex;

	//! The zero-based index of the notch filter to use for the channel.
	/*!
	The index is related to the list of available notch filters that are returned by \ref GDS_GUSBAMP_GetNotchFilters.
	A value of -1 indicates that no notch filter is used for the channel.
	*/
	int32_t NotchFilterIndex;
} GDS_GUSBAMP_CHANNEL_CONFIGURATION, *PGDS_GUSBAMP_CHANNEL_CONFIGURATION;

//! The configuration of a g.USBamp device.
typedef struct _GDS_GUSBAMP_CONFIGURATION
{
	bool DeviceEnabled = true;
	//! The sample rate of the analog inputs in hertz.
	/*!
	Valid values are: 32, 64, 128, 256, 512, 600, 1200, 2400, 4800, 9600, 19200, 38400.
	Eight-channel g.USBamp devices support the following sample rates only: 128, 256, 512.
	*/
	uint32_t SampleRate;

	//! The number of scans to retrieve from the device at once, or zero to determine the recommended number of scans for the specified sampling rate automatically.
	/*!
	This value depends on the value of \ref SampleRate. The following table lists the recommended values of \ref NumberOfScans for all possible sample rates.

	\li \c 1 for a sample rate of 32 Hz
	\li \c 2 for a sample rate of 64 Hz
	\li \c 4 for a sample rate of 128 Hz
	\li \c 8 for a sample rate of 256 Hz
	\li \c 16 for a sample rate of 512 Hz
	\li \c 32 for a sample rate of 600 Hz
	\li \c 64 for a sample rate of 1200 Hz
	\li \c 128 for a sample rate of 2400 Hz
	\li \c 256 for a sample rate of 4800 Hz
	\li \c 512 for a sample rate of 9600 Hz
	\li \c 512 for a sample rate of 19200 Hz
	\li \c 512 for a sample rate of 38400 Hz
	*/
	size_t NumberOfScans;

	//! Indicates whether the short cut socket is enabled.
	/*!
	If short cut is enabled, a HIGH level on the SC input socket of the amplifier disconnects the electrodes from the amplifier input stage and holds the lastly measured values as long as the HIGH level is applied.
	If short cut is disabled, the level on the SC input socket of the amplifier is ignored and the amplifier always delivers the currently measured values on its input channels.
	*/
	bool ShortCutEnabled;

	//! Indicates whether a sample counter should be applied on channel 16 instead of the measured signal if selected for acquisition (overrun at 1000000 samples).
	bool CounterEnabled;

	//! Indicates whether the synchronously sampled digital trigger input channels should be included in data acquisition in an additional logical channel that is appended to the synchronously sampled analog input channels.
	/*!
	If enabled, the values of the digital trigger input channels come encoded in a single value as an additional channel appended to the analog channels.

	In g.USBamp version 2.0 there is just one trigger line, so the values of the trigger channel can be 0 (LOW) or 250000 (HIGH).
	In version 3.0 there are 8 trigger lines coming bit-encoded as UINT8 value on the trigger channel. If all of the 8 trigger lines are HIGH, the value of the channel equals 255.0. If e.g. trigger line 0 to 3 are HIGH the value of the channel equals 15.0.
	*/
	bool TriggerEnabled;

	//! The configuration of the internal signal generator.
	GDS_GUSBAMP_SIGNAL_GENERATOR_CONFIGURATION InternalSignalGenerator;

	//! The common ground setting for each of the four groups A, B, C, and D.
	/*!
	Each element in the array indicates whether the ground socket of the corresponding group (A, B, C, or D) should be connected to common ground. The common ground setting for group A (channels 1 to 4) is represented by the element at index zero, group B (channels 5 to 8) at index 1, group C (channels 9 to 12) at index 2, and group D (channels 13 to 16) at index 3.
	A value of zero of an element in the array indicates that the ground socket of the corresponding group is not connected to common ground.
	A non-zero element indicates that the ground socket of the corresponding group is connected to common ground.
	*/
	bool CommonGround[GDS_GUSBAMP_GROUPS_MAX];

	//! The common reference setting for each of the four groups A, B, C, and D.
	/*!
	Each element in the array indicates whether the reference socket of the corresponding group (A, B, C, or D) should be connected to common reference. The common reference setting for group A (channels 1 to 4) is represented by the element at index zero, group B (channels 5 to 8) at index 1, group C (channels 9 to 12) at index 2, and group D (channels 13 to 16) at index 3.
	A value of zero of an element in the array indicates that the reference socket of the corresponding group is not connected to common reference.
	A non-zero element indicates that the reference socket of the corresponding group is connected to common reference.
	*/
	bool CommonReference[GDS_GUSBAMP_GROUPS_MAX];

	//! The configuration of each channel.
	/*!
	The element at zero-based index <i>i</i> holds the configuration for the channel with one-based channel number <i>i + 1</i>.
	*/
	GDS_GUSBAMP_CHANNEL_CONFIGURATION Channels[GDS_GUSBAMP_CHANNELS_MAX];

	bool IsMaster = true;

} GDS_GUSBAMP_CONFIGURATION, *PGDS_GUSBAMP_CONFIGURATION;

typedef struct _SYSTEM_CONFIGURATION
{
	GDS_GUSBAMP_CONFIGURATION Devices[CHAINED_DEVICES_MAX];
} gUSB_system_config, *PgUSB_system_config;

#endif  // GUSBAMP_CONFIG_H