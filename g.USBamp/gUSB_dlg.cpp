#include "gUSB_dlg.h"
#include "ui_gUSB_dlg.h"
#include "gUSBamp_config.h"  // TODO: Should only use gUSBamp_LSL_interface.h
#include <QtWidgets>
#include <algorithm>

GUSBDlg::GUSBDlg(QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::GUSBDlg)
{
	ui->setupUi(this);

	for each (uint32_t srate in gUSBamp_sample_rates)
	{
		ui->sampleRate_combo->addItem(QString::number(srate));
	}
	connect(ui->sampleRate_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_sampleRateChange(int)));
}

void GUSBDlg::set_config(std::shared_ptr<gUSB_system_config> config)
{
	m_sys_config = config;
	create_widgets();
	update_ui();
}

void GUSBDlg::create_widgets()
{
	size_t dev_ix = 0;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {

		// Create widgets for binary switches.
		QCheckBox *enabled_box = new QCheckBox(tr("Enabled"));
		enabled_box->setEnabled(false);
		QCheckBox *master_box = new QCheckBox(tr("Master"));
		QCheckBox *shortcut_box = new QCheckBox(tr("ShortCut"));
		QCheckBox *counter_box = new QCheckBox(tr("Counter"));
		counter_box->setEnabled(false);
		QCheckBox *trigger_box = new QCheckBox(tr("Trigger"));
		
		// Create widgets for common grounds and references.
		QHBoxLayout *gr_layout = new QHBoxLayout;
		for (size_t grp_ix = 0; grp_ix < GDS_GUSBAMP_GROUPS_MAX; grp_ix++)
		{
			char box_label[2] = { 65 + (char)grp_ix };
			QCheckBox *ground_box = new QCheckBox(tr(box_label));
			gr_layout->addWidget(ground_box);
		}
		QHBoxLayout *ref_layout = new QHBoxLayout;
		for (size_t grp_ix = 0; grp_ix < GDS_GUSBAMP_GROUPS_MAX; grp_ix++)
		{
			char box_label[2] = { 65 + (char)grp_ix };
			QCheckBox *ref_box = new QCheckBox(tr(box_label));
			ref_layout->addWidget(ref_box);
		}

		// Channels
		QTableWidget *chan_table = new QTableWidget(GDS_GUSBAMP_CHANNELS_MAX, 6);
		QStringList h_labels;
		h_labels << "Channel" << "Acquire" << "Bipolar" << "Bandpass" << "Notch" << "Impedance";
		chan_table->setHorizontalHeaderLabels(h_labels);
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			chan_table->setItem(chan_ix, 0, new QTableWidgetItem(QString::fromStdString(dev_cfg->Channels[chan_ix].Label)));
			chan_table->setCellWidget(chan_ix, 1, new QCheckBox());
			QSpinBox* bipolar_spinbox = new QSpinBox();
			bipolar_spinbox->setRange(0, GDS_GUSBAMP_CHANNELS_MAX);
			chan_table->setCellWidget(chan_ix, 2, bipolar_spinbox);
			chan_table->setItem(chan_ix, 3, new QTableWidgetItem(tr("None")));  // bandpass filter
			chan_table->setItem(chan_ix, 4, new QTableWidgetItem(tr("None")));  // notch filter
			chan_table->setItem(chan_ix, 5, new QTableWidgetItem(tr("?")));		// impedance value
		}
		
		// Put all the widgets together on a layout.
		// page_widget > 
		//		dev_layout >
		//			sw_gr_ref:
		//				| status:status_group [enabled; master]
		//				| switches:switches_group [shortcut; counter; trigger]
		//				| gr_ref:ground_group,ref_group [[groundA groundB groundC groundD]; [refA refB refC refD]]
		//			chan_table
		QVBoxLayout *dev_layout = new QVBoxLayout;

		QHBoxLayout *sw_gr_ref_layout = new QHBoxLayout;

		QVBoxLayout *status_layout = new QVBoxLayout;
		status_layout->addWidget(enabled_box);
		status_layout->addWidget(master_box);
		QGroupBox *status_group = new QGroupBox(tr("Status"));
		status_group->setLayout(status_layout);
		sw_gr_ref_layout->addWidget(status_group);

		QVBoxLayout *switches_layout = new QVBoxLayout;
		switches_layout->addWidget(shortcut_box);
		switches_layout->addWidget(counter_box);
		switches_layout->addWidget(trigger_box);
		QGroupBox *switches_group = new QGroupBox(tr("Switches"));
		switches_group->setLayout(switches_layout);
		sw_gr_ref_layout->addWidget(switches_group);

		QVBoxLayout *gr_ref_layout = new QVBoxLayout;
		QGroupBox *ground_group = new QGroupBox(tr("Common Ground"));
		ground_group->setLayout(gr_layout);
		gr_ref_layout->addWidget(ground_group);
		QGroupBox *ref_group = new QGroupBox(tr("Common Reference"));
		ref_group->setLayout(ref_layout);
		gr_ref_layout->addWidget(ref_group);
		sw_gr_ref_layout->addLayout(gr_ref_layout);
		dev_layout->addLayout(sw_gr_ref_layout);

		// Channels widget
		dev_layout->addWidget(chan_table);

		// Device done. Add to widget, insert as tab.
		QWidget *page_widget = new QWidget();
		page_widget->setLayout(dev_layout);

		ui->devices_tabWidget->addTab(page_widget, QString::fromStdString(dev_cfg->Serial));
	}
}

void GUSBDlg::update_ui()
{
	int dev_ix = 0;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {
		QWidget *dev_page = ui->devices_tabWidget->widget(dev_ix);
		QVBoxLayout *dev_layout = (QVBoxLayout*)dev_page->layout();

		QHBoxLayout *sw_gr_ref_layout = (QHBoxLayout*)dev_layout->itemAt(0)->layout();

		QGroupBox *status_group = (QGroupBox*)sw_gr_ref_layout->itemAt(0)->widget();
		QCheckBox *enabled_box = (QCheckBox*)status_group->layout()->itemAt(0)->widget();
		enabled_box->setChecked(dev_cfg->DeviceEnabled);
		QCheckBox *master_box = (QCheckBox*)status_group->layout()->itemAt(1)->widget();
		master_box->setChecked(dev_cfg->IsMaster);
		
		QGroupBox *switches_group = (QGroupBox*)sw_gr_ref_layout->itemAt(1)->widget();
		QCheckBox *shortcut_box = (QCheckBox*)switches_group->layout()->itemAt(0)->widget();
		shortcut_box->setChecked(dev_cfg->ShortCutEnabled);

		QCheckBox *counter_box = (QCheckBox*)switches_group->layout()->itemAt(1)->widget();
		counter_box->setChecked(dev_cfg->CounterEnabled);

		QCheckBox *trigger_box = (QCheckBox*)switches_group->layout()->itemAt(2)->widget();
		trigger_box->setChecked(dev_cfg->TriggerEnabled);

		QVBoxLayout *gr_ref_layout = (QVBoxLayout*)sw_gr_ref_layout->itemAt(2)->layout();
		QGroupBox *ground_group = (QGroupBox*)gr_ref_layout->itemAt(0)->widget();
		QHBoxLayout *gr_layout = (QHBoxLayout*)ground_group->layout();
		QGroupBox *ref_group = (QGroupBox*)gr_ref_layout->itemAt(1)->widget();
		QHBoxLayout *ref_layout = (QHBoxLayout*)ref_group->layout();
		for (int grp_ix = 0; grp_ix < GDS_GUSBAMP_GROUPS_MAX; grp_ix++)
		{
			QCheckBox *ground_box = (QCheckBox*)gr_layout->itemAt(grp_ix)->widget();
			ground_box->setChecked(dev_cfg->CommonGround[grp_ix]);
			QCheckBox *ref_box = (QCheckBox*)ref_layout->itemAt(grp_ix)->widget();
			ref_box->setChecked(dev_cfg->CommonReference[grp_ix]);
		}

		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(1)->widget();
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			QCheckBox *chan_acquire = (QCheckBox*)chan_table->cellWidget(chan_ix, 1);
			chan_acquire->setChecked(dev_cfg->Channels[chan_ix].Acquire);

			QSpinBox* chan_bipolar = (QSpinBox*)chan_table->cellWidget(chan_ix, 2);
			chan_bipolar->setValue(dev_cfg->Channels[chan_ix].BipolarChannel);
		}
	}

	// Update samplerate, number of scans, driver
	ui->driver_value->setText(QString::number(m_sys_config->DriverVersion));
	int pos = std::distance(gUSBamp_sample_rates.begin(), std::find(gUSBamp_sample_rates.begin(), gUSBamp_sample_rates.end(), m_sys_config->SampleRate));
	ui->numScans_spinBox->setValue(gUSBamp_buffer_sizes[pos]);
	ui->sampleRate_combo->setCurrentIndex(pos);  // Triggers handle_sampleRateChange -> calls update_filters()
}

void GUSBDlg::update_filters()
{
	int srate_ix = ui->sampleRate_combo->currentIndex();
	uint32_t current_srate = gUSBamp_sample_rates[srate_ix];

	std::vector<int> bp_filt_id;
	bp_filt_id.push_back(-1);
	ui->bandpass_comboBox->clear();
	ui->bandpass_comboBox->addItem("None");
	int filt_ix = 0;
	for (auto filt_cfg = m_sys_config->available_bandpass_filters.begin(); filt_cfg != m_sys_config->available_bandpass_filters.end(); filt_cfg++, filt_ix++)
	{
		if ((uint32_t)filt_cfg->SamplingRate == current_srate)
		{
			bp_filt_id.push_back(filt_ix);
			QString filt_type = filt_cfg->TypeId == GDS_FILTER_TYPE_CHEBYSHEV ? "Cheby" : "Butter";
			ui->bandpass_comboBox->addItem(QString("%1:%2,%3,%4-%5").arg(
				QString::number(filt_ix),
				filt_type,
				QString::number(filt_cfg->Order),
				QString::number(filt_cfg->LowerCutoffFrequency),
				QString::number(filt_cfg->UpperCutoffFrequency)));
		}
	}

	std::vector<int> notch_filt_id;
	notch_filt_id.push_back(-1);
	ui->notch_comboBox->clear();
	ui->notch_comboBox->addItem("None");
	filt_ix = 0;
	for (auto filt_cfg = m_sys_config->available_notch_filters.begin(); filt_cfg != m_sys_config->available_notch_filters.end(); filt_cfg++, filt_ix++)
	{
		if ((uint32_t)filt_cfg->SamplingRate == current_srate)
		{
			notch_filt_id.push_back(filt_ix);
			QString filt_type = filt_cfg->TypeId == GDS_FILTER_TYPE_CHEBYSHEV ? "Cheby" : "Butter";
			ui->notch_comboBox->addItem(QString("%1:%2,%3,%4-%5").arg(
				QString::number(filt_ix),
				filt_type,
				QString::number(filt_cfg->Order),
				QString::number(filt_cfg->LowerCutoffFrequency),
				QString::number(filt_cfg->UpperCutoffFrequency)));
		}
	}

	// Update ui with config contents
	int dev_ix = 0;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {
		QWidget *dev_page = ui->devices_tabWidget->widget(dev_ix);
		QVBoxLayout *dev_layout = (QVBoxLayout*)dev_page->layout();
		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(1)->widget();
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			// Where in list of bp_filt_id is this channels filter index, if anywhere?
			std::vector<int>::iterator bp_it = std::find(bp_filt_id.begin(), bp_filt_id.end(), dev_cfg->Channels[chan_ix].BandpassFilterIndex);
			int bp_ix = bp_it != bp_filt_id.end() ? std::distance(bp_filt_id.begin(), bp_it) : 0;  // Default to 0th (None)
			chan_table->item(chan_ix, 3)->setText(ui->bandpass_comboBox->itemText(bp_ix));

			std::vector<int>::iterator notch_it = std::find(notch_filt_id.begin(), notch_filt_id.end(), dev_cfg->Channels[chan_ix].NotchFilterIndex);
			int notch_ix = notch_it != notch_filt_id.end() ? std::distance(notch_filt_id.begin(), notch_it) : 0;
			chan_table->item(chan_ix, 4)->setText(ui->notch_comboBox->itemText(notch_ix));
		}
	}
}

void GUSBDlg::handle_sampleRateChange(int index)
{
	ui->numScans_spinBox->setValue(gUSBamp_buffer_sizes[index]);
	update_filters();
}

void GUSBDlg::on_bandpass_pushButton_clicked()
{
	apply_filter_to_enabled_chans(3, ui->bandpass_comboBox->currentText());
}

void GUSBDlg::on_notch_pushButton_clicked()
{
	apply_filter_to_enabled_chans(4, ui->notch_comboBox->currentText());
}

void GUSBDlg::apply_filter_to_enabled_chans(int widget_ix, QString value)
{
	int dev_ix = 0;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {
		QWidget *dev_page = ui->devices_tabWidget->widget(dev_ix);
		QVBoxLayout *dev_layout = (QVBoxLayout*)dev_page->layout();

		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(1)->widget();
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			QCheckBox *chan_acquire = (QCheckBox*)chan_table->cellWidget(chan_ix, 1);
			if (chan_acquire->isChecked())
			{
				chan_table->item(chan_ix, widget_ix)->setText(value);
			}
		}
	}
}

void GUSBDlg::update_impedance_table() 
{
	// Open device with name in position 0.
	// BOOL status = GT_SetMode(HANDLE hDevice, M_IMPEDANCE);

	/*
	// Must open each device then close it upon completion.
	for (size_t cfg_ix = 0; cfg_ix < m_configs.size(); cfg_ix++)
	{
		QVBoxLayout *dev_layout = (QVBoxLayout*)ui->devices_layout->itemAtPosition((int)cfg_ix, 0)->layout();
		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(2)->widget();
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			if (m_pChannel_impedances->size() > (cfg_ix*GDS_GUSBAMP_CHANNELS_MAX) + chan_ix)
			{
				chan_table->item(chan_ix, 5)->setText(QString::number(m_pChannel_impedances->at(cfg_ix*GDS_GUSBAMP_CHANNELS_MAX) + chan_ix));
			}
		}
	}
	*/
}

void GUSBDlg::on_impedance_pushButton_clicked() 
{
	ui->impedance_pushButton->setEnabled(false);
	qApp->processEvents();
	m_pChannel_impedances->clear();
	/*
	for (size_t cfg_ix = 0; cfg_ix < m_configs.size(); cfg_ix++)
	{
		GDS_GUSBAMP_CONFIGURATION* dev_cfg = (GDS_GUSBAMP_CONFIGURATION*)m_configs[cfg_ix].Configuration;

		char(*device_name)[DEVICE_NAME_LENGTH_MAX] = new char[1][DEVICE_NAME_LENGTH_MAX];
		std::strcpy(device_name[0], m_configs[cfg_ix].DeviceInfo.Name);

		QVBoxLayout *dev_layout = (QVBoxLayout*)ui->devices_layout->itemAtPosition((int)cfg_ix, 0)->layout();
		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(2)->widget();

		for (size_t chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			double impedance = 0;
			GDS_RESULT res = GDS_GUSBAMP_GetImpedance(*m_pHandle, device_name, (int)(chan_ix + 1), &impedance);
			m_pChannel_impedances->push_back(impedance);

			chan_table->item(chan_ix, 5)->setText(QString::number(m_pChannel_impedances->at(cfg_ix*GDS_GUSBAMP_CHANNELS_MAX + chan_ix)));
			qApp->processEvents();
		}
		
		delete[] device_name;
		device_name = NULL;
	}
	*/
	ui->impedance_pushButton->setEnabled(true);
	// update_impedance_table();
}

void GUSBDlg::accept()
{
	m_sys_config->SampleRate = ui->sampleRate_combo->currentText().toULong();
	m_sys_config->NumberOfScans = ui->numScans_spinBox->value();
	int dev_ix = 0;
	bool master_set = false;
	for (auto dev_cfg = m_sys_config->Devices.begin(); dev_cfg != m_sys_config->Devices.end(); dev_cfg++, dev_ix++) {
		QWidget *dev_page = ui->devices_tabWidget->widget(dev_ix);
		QVBoxLayout *dev_layout = (QVBoxLayout*)dev_page->layout();

		QHBoxLayout *sw_gr_ref_layout = (QHBoxLayout*)dev_layout->itemAt(0)->layout();

		QGroupBox *status_group = (QGroupBox*)sw_gr_ref_layout->itemAt(0)->widget();
		QCheckBox *enabled_box = (QCheckBox*)status_group->layout()->itemAt(0)->widget();
		dev_cfg->DeviceEnabled = enabled_box->isChecked();
		
		QCheckBox *master_box = (QCheckBox*)status_group->layout()->itemAt(1)->widget();
		dev_cfg->IsMaster = master_box->isChecked() & !master_set;
		master_set |= dev_cfg->IsMaster;

		QGroupBox *switches_group = (QGroupBox*)sw_gr_ref_layout->itemAt(1)->widget();
		QCheckBox *shortcut_box = (QCheckBox*)switches_group->layout()->itemAt(0)->widget();
		dev_cfg->ShortCutEnabled = shortcut_box->isChecked();

		QCheckBox *counter_box = (QCheckBox*)switches_group->layout()->itemAt(1)->widget();
		dev_cfg->CounterEnabled = counter_box->isChecked();

		QCheckBox *trigger_box = (QCheckBox*)switches_group->layout()->itemAt(2)->widget();
		dev_cfg->TriggerEnabled = trigger_box->isChecked();

		QVBoxLayout *gr_ref_layout = (QVBoxLayout*)sw_gr_ref_layout->itemAt(2)->layout();
		QGroupBox *ground_group = (QGroupBox*)gr_ref_layout->itemAt(0)->widget();
		QHBoxLayout *gr_layout = (QHBoxLayout*)ground_group->layout();
		QGroupBox *ref_group = (QGroupBox*)gr_ref_layout->itemAt(1)->widget();
		QHBoxLayout *ref_layout = (QHBoxLayout*)ref_group->layout();
		for (int grp_ix = 0; grp_ix < GDS_GUSBAMP_GROUPS_MAX; grp_ix++)
		{
			QCheckBox *ground_box = (QCheckBox*)gr_layout->itemAt(grp_ix)->widget();
			dev_cfg->CommonGround[grp_ix] = ground_box->isChecked();
			QCheckBox *ref_box = (QCheckBox*)ref_layout->itemAt(grp_ix)->widget();
			dev_cfg->CommonReference[grp_ix] = ref_box->isChecked();
		}

		QTableWidget *chan_table = (QTableWidget*)dev_layout->itemAt(1)->widget();
		for (int chan_ix = 0; chan_ix < GDS_GUSBAMP_CHANNELS_MAX; chan_ix++)
		{
			dev_cfg->Channels[chan_ix].Label = chan_table->item(chan_ix, 0)->text().toStdString();

			QCheckBox *chan_acquire = (QCheckBox*)chan_table->cellWidget(chan_ix, 1);
			dev_cfg->Channels[chan_ix].Acquire = chan_acquire->isChecked();

			QSpinBox* chan_bipolar = (QSpinBox*)chan_table->cellWidget(chan_ix, 2);
			dev_cfg->Channels[chan_ix].BipolarChannel = chan_bipolar->value();

			int32_t bp_ix, notch_ix;
			bp_ix = chan_table->item(chan_ix, 3)->text().split(':').at(0).toLong();
			dev_cfg->Channels[chan_ix].BandpassFilterIndex = bp_ix;
			notch_ix = chan_table->item(chan_ix, 4)->text().split(':').at(0).toLong();
			dev_cfg->Channels[chan_ix].NotchFilterIndex = notch_ix;
		}

	}
	

	QDialog::accept();
}
