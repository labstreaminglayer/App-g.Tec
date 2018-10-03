#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "gUSBamp_LSL_interface.h"
#include "gUSB_dlg.h"
#include <fstream>
#include <string>
#include <vector>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <lsl_cpp.h>


MainWindow::MainWindow(QWidget* parent, const char* config_file)
    :
	QMainWindow(parent),
	recording_thread(nullptr),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	connect(ui->actionLoad_Configuration, &QAction::triggered, [this]() {
		load_config(QFileDialog::getOpenFileName(this, "Load Configuration File", "",
		                                         "Configuration Files (*.cfg)"));
	});
	connect(ui->actionSave_Configuration, &QAction::triggered, [this]() {
		save_config(QFileDialog::getSaveFileName(this, "Save Configuration File", "",
		                                         "Configuration Files (*.cfg)"));
	});
	connect(ui->actionEdit_Configuration, &QAction::triggered, [this]() {
		edit_config();
	});
	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actionAbout, &QAction::triggered, [this]() {
		QString infostr = QStringLiteral("LSL library version: ") +
		                  QString::number(lsl::library_version()) +
		                  "\nLSL library info:" + lsl::lsl_library_info();
		QMessageBox::about(this, "About g.Tec Connector", infostr);
	});
	connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

	// Create an empty sys_config
	m_sys_config = std::make_shared<gUSB_system_config>();
	scan_for_devices();  // Finds devices, populates m_sys_config.
	load_config(config_file);  // Use config file to fill in each device's configuration.
}

void MainWindow::load_config(const QString& filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("gUSBamp");
	m_sys_config->SampleRate = settings.value("SampleRate", 256).toUInt();
	uint32_t num_scans = settings.value("NumberOfScans").toUInt();
	if (num_scans > 0)
		m_sys_config->NumberOfScans = num_scans;
	int n_devices = settings.beginReadArray("Devices");
	for (int dev_ix = 0; dev_ix < n_devices; dev_ix++)
	{
		settings.setArrayIndex(dev_ix);
		std::string serial = settings.value("Serial").toString().toStdString();
		// Find the index of the amp with matching serial in m_sys_config
		int found_ix = -1;
		int cfg_ix = 0;
		for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, cfg_ix++) {
			if (dev_cfg->Serial == serial)
				found_ix = cfg_ix;
		}
		if (found_ix >= 0)
		{
			GDS_GUSBAMP_CONFIGURATION *dev_cfg = &(m_sys_config->Devices[found_ix]);
			dev_cfg->DeviceEnabled = settings.value("DeviceEnabled", true).toBool();
			dev_cfg->IsMaster = settings.value("IsMaster", true).toBool();
			dev_cfg->CounterEnabled = settings.value("CounterEnabled", false).toBool();
			dev_cfg->ShortCutEnabled = settings.value("ShortCutEnabled", true).toBool();
			dev_cfg->TriggerEnabled = settings.value("TriggerEnabled", true).toBool();

			settings.beginGroup("CommonReference");
			dev_cfg->CommonReference[0] = settings.value("A", false).toBool();
			dev_cfg->CommonReference[1] = settings.value("B", false).toBool();
			dev_cfg->CommonReference[2] = settings.value("C", false).toBool();
			dev_cfg->CommonReference[3] = settings.value("D", false).toBool();
			settings.endGroup();

			settings.beginGroup("CommonGround");
			dev_cfg->CommonGround[0] = settings.value("A", false).toBool();
			dev_cfg->CommonGround[1] = settings.value("B", false).toBool();
			dev_cfg->CommonGround[2] = settings.value("C", false).toBool();
			dev_cfg->CommonGround[3] = settings.value("D", false).toBool();
			settings.endGroup();


			int n_channels = settings.beginReadArray("Channels");
			for (int chan_ix = 0; chan_ix < n_channels; chan_ix++)
			{
				settings.setArrayIndex(chan_ix);
				dev_cfg->Channels[chan_ix].Acquire = settings.value("Acquire", true).toBool();
				dev_cfg->Channels[chan_ix].Label = settings.value("Label", QString::number(chan_ix)).toString().toStdString();
				dev_cfg->Channels[chan_ix].BandpassFilterIndex = settings.value("BandpassFilterIndex").toInt();
				dev_cfg->Channels[chan_ix].NotchFilterIndex = settings.value("NotchFilterIndex").toInt();
				dev_cfg->Channels[chan_ix].BipolarChannel = settings.value("BipolarChannel").toUInt();
			}
			settings.endArray();  // Channels
		}
	}
	settings.endArray();  // Devices

	if (settings.value("Autolink", false).toBool())
	{
		toggleRecording();
	}

	settings.endGroup();  // gUSBamp
}

void MainWindow::save_config(const QString& filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("gUSBamp");
	
	// across-device settings.
	settings.setValue("SampleRate", m_sys_config->SampleRate);
	settings.setValue("NumberOfScans", m_sys_config->NumberOfScans);
	settings.setValue("DriverVersion", m_sys_config->DriverVersion);  // Info only. Will not be read on load_config

	// Settings for each device.
	settings.beginWriteArray("Devices");
	int dev_ix = 0;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {
		settings.setArrayIndex(dev_ix);

		settings.setValue("Serial", QString::fromStdString(dev_cfg->Serial));
		settings.setValue("DeviceEnabled", dev_cfg->DeviceEnabled);
		settings.setValue("IsMaster", dev_cfg->IsMaster);
		settings.setValue("CounterEnabled", dev_cfg->CounterEnabled);
		settings.setValue("ShortCutEnabled", dev_cfg->ShortCutEnabled);
		settings.setValue("TriggerEnabled", dev_cfg->TriggerEnabled);

		settings.beginGroup("CommonReference");
		settings.setValue("A", dev_cfg->CommonReference[0]);
		settings.setValue("B", dev_cfg->CommonReference[1]);
		settings.setValue("C", dev_cfg->CommonReference[2]);
		settings.setValue("D", dev_cfg->CommonReference[3]);
		settings.endGroup();

		settings.beginGroup("CommonGround");
		settings.setValue("A", dev_cfg->CommonGround[0]);
		settings.setValue("B", dev_cfg->CommonGround[1]);
		settings.setValue("C", dev_cfg->CommonGround[2]);
		settings.setValue("D", dev_cfg->CommonGround[3]);
		settings.endGroup();

		settings.beginWriteArray("Channels");
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			settings.setArrayIndex(chan_ix);
			settings.setValue("Acquire", dev_cfg->Channels[chan_ix].Acquire);
			settings.setValue("Label", QString::fromStdString(dev_cfg->Channels[chan_ix].Label));
			settings.setValue("BandpassFilterIndex", dev_cfg->Channels[chan_ix].BandpassFilterIndex);
			settings.setValue("NotchFilterIndex", dev_cfg->Channels[chan_ix]. NotchFilterIndex);
			settings.setValue("BipolarChannel", dev_cfg->Channels[chan_ix].BipolarChannel);
		}
		settings.endArray();
	}
	settings.endArray();  // Array of devices.
	settings.endGroup();  // gUSBamp parent group.
	settings.sync();
}

void MainWindow::edit_config()
{
	GUSBDlg cfg_dlg;
	cfg_dlg.set_config(m_sys_config);
	cfg_dlg.exec();
}


void MainWindow::closeEvent(QCloseEvent* ev) {
	if (recording_thread) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}

void MainWindow::scan_for_devices() {
	// Populate with available device names.
	gUSBamp_LSL_interface::enumerateDevices(m_sys_config);
	ui->availableListWidget->clear();
	for (auto & dev : m_sys_config->Devices) {
		ui->availableListWidget->addItem(QString::fromStdString(dev.Serial));
	}
}

void recording_thread_function(gUSB_system_config const &sys_config, std::atomic<bool>& shutdown) {
	gUSBamp_LSL_interface device(sys_config);  // Interface opens & configs the devices, starts acquisition.

	// Get info for outlet.
	size_t chunk_size, channel_count, marker_count;
	uint32_t srate;
	std::string serial;
	device.getAcquisitionParameters(chunk_size, channel_count, srate, serial, marker_count);
	std::vector<std::string> channel_labels;
	device.getChannelLabels(channel_labels);
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

	lsl::stream_info marker_info("g.USBamp-Markers", "Markers", 1, 0, lsl::cf_string, serial + "_markers");
	lsl::stream_outlet marker_outlet(marker_info);
	
	double lsl_timestamp;
	std::vector<std::vector<float> > buffer(chunk_size, std::vector<float>(channel_count));
	std::vector<std::pair<double, std::string>> marker_buffer;
	marker_buffer.clear();

	while (!shutdown) {
		if (device.getData(buffer, lsl_timestamp, marker_buffer)) {  // Get result of previously queued data fetch.
			outlet.push_chunk(buffer, lsl_timestamp);  // Push the result.
			if (marker_buffer.size() > 0)
			{
				for (auto mrk_pair : marker_buffer)
				{
					marker_outlet.push_sample(&mrk_pair.second, mrk_pair.first);
				}
			}
		} else {
			//break; // Acquisition was unsuccessful? -> Quit
		}
	}

	// When `device` goes out of scope, its destructor should clean up the connection and buffers.
}

void MainWindow::toggleRecording() {
	if (!recording_thread) {
		// read the configuration from the UI fields
		shutdown = false;
		// The thread passes the config to the gUSBamp_LSL_interface constructor which immediately makes a copy.
		// Below, std::ref isn't intended to let the thread modify m_sys_config in a way that makes it back to the main thread.
		// It simply avoids an unnecessary copy on thread creation, because the thread immediately passes the config to the
		// gUSBamp_LSL_interface constructor which makes a copy (via initializer list).
		recording_thread = std::make_unique<std::thread>(&recording_thread_function, std::ref(*m_sys_config), std::ref(shutdown));
		ui->linkButton->setText("Unlink");
	}
	else {
		shutdown = true;
		recording_thread->join();
		recording_thread.reset();
		ui->linkButton->setText("Link");
	}
}

MainWindow::~MainWindow() noexcept = default;
