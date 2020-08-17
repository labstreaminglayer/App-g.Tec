#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QtDebug>

static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"},
		{QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
	QString logLevelName = msgLevelHash[type];

	QTime time = QTime::currentTime();
	QString formattedTime = time.toString("hh:mm:ss.zzz");

	QString txt = QString("%1 %2:\t%3").arg(formattedTime, logLevelName, msg);

	QFile outFile("gNEEDAccess.log");
	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream ts(&outFile);
	ts << txt << Qt::endl;

	// Call the default handler. This is important for aborting the app in case of fatal message.
	(*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}


int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageHandler);

    QApplication a(argc, argv);
	QCommandLineParser parser;
	parser.setApplicationDescription("LabStreamingLayer interface for g.tec g.NEEDaccess.");
	parser.addHelpOption();
	parser.addVersionOption();
	QCommandLineOption configFileOption(QStringList() << "c" << "config",
		QCoreApplication::translate("main", "Load configuration from <config>."),
		QCoreApplication::translate("main", "config"),
		default_config_fname);
	parser.addOption(configFileOption);
	parser.process(a);
	QString configFilename = parser.value(configFileOption);
	MainWindow w(0, configFilename);
    w.show();

	return a.exec();
}
