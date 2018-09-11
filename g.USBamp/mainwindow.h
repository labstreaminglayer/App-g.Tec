#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <atomic>
#include <memory> //for std::unique_ptr
#include <thread>
#include <QMainWindow>
#include "gUSBamp_config.h"


namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
	explicit MainWindow(QWidget *parent, const char* config_file);
	~MainWindow() noexcept override;

private slots:
	void closeEvent(QCloseEvent *ev) override;
	void toggleRecording(void);

private:
	// function for loading / saving the config file
	void load_config(const QString& filename);
	void save_config(const QString& filename);
	void edit_config();
	std::unique_ptr<std::thread> recording_thread;
	std::unique_ptr<Ui::MainWindow> ui;	// window pointer
	std::shared_ptr<gUSB_system_config> sys_config;
	std::atomic<bool> shutdown{false};  // flag indicating whether the recording thread should quit
};

#endif // MAINWINDOW_H
