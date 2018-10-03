#include "mainwindow.h"
#include <QApplication>
/*
#ifdef _WIN32
#include <Windows.h>
#endif
*/
int main(int argc, char* argv[]) {

/*
#ifdef _WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
*/


	QApplication a(argc, argv);
	MainWindow w(nullptr, argc > 1 ? argv[1] : "gUSBampLSL.cfg");
	w.show();
	return a.exec();
}
