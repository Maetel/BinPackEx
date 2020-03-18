#pragma once
#include <QObject>

class SomeWindow : public QObject
{
	Q_OBJECT
public:
	SomeWindow();
	~SomeWindow();

signals:
	void speak();

public slots:
	void handleSpeak();
};