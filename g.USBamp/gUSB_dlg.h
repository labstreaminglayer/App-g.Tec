#pragma once
#include <QDialog>
#include <memory>
#include "gUSBamp_config.h"


namespace Ui {
	class GUSBDlg;
}


class GUSBDlg : public QDialog
{
	Q_OBJECT
public:
	explicit GUSBDlg(QWidget *parent = 0);
	void set_config(std::shared_ptr<gUSB_system_config> config);

	Ui::GUSBDlg *ui;

private slots:
	void handle_sampleRateChange(int index);
	void on_bandpass_pushButton_clicked();
	void on_notch_pushButton_clicked();
	void on_impedance_pushButton_clicked();
private:
	void create_widgets();
	void update_ui();
	void update_filters();
	void update_impedance_table();
	void accept() override;
	void apply_filter_to_enabled_chans(int widget_ix, QString value);

	std::shared_ptr<gUSB_system_config> m_sys_config;  // TODO: Make opaque.
	std::vector<std::string>* m_pChannel_labels;
	std::vector<double>* m_pChannel_impedances;
};