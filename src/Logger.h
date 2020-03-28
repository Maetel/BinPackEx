#pragma once

#include <QtGlobal>
#include <QString>
#include <QByteArray>
#include <QMainWindow>
#include <QTextEdit>

static QTextEdit* g_logEdit = 0;

static
void Binpacklog(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	static QString message;

	QByteArray localMsg = msg.toLocal8Bit();
	QString typeStr;

	bool abort = false;

	switch (type) {
	case QtDebugMsg:
		typeStr = "[Debug] ";
		break;
	case QtInfoMsg:
		typeStr = "[Info] ";
		break;
	case QtWarningMsg:
		typeStr = "[Warning] ";
		break;
	case QtCriticalMsg:
		typeStr = "[Critical] ";
		break;
	case QtFatalMsg:
		typeStr = "[Fatal] ";
	default:
		typeStr = "[Fatal(default)] ";
		abort = true;
		break;
	}

	message += QString("%1: %2 (%3:%4, %5)\n").arg(typeStr).arg(msg).arg(context.file).arg(context.line).arg(context.function);
	
	//do something with log
	if (!g_logEdit)
		g_logEdit = new QTextEdit;
	g_logEdit->setText(message);
	
	if (abort)
		throw;
}
