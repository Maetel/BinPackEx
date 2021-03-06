#pragma once

//#include "BinpackMainWindow.h"

#include <QMainWindow>
class BinpackMainWindow;

class BaseReceiver : public QMainWindow
{
	Q_OBJECT
protected slots:
	virtual void handleValues() = 0;
public:
	virtual ~BaseReceiver();
protected:
	BaseReceiver(BinpackMainWindow* owner);
	virtual void initialize() = 0;
	void keyPressEvent(QKeyEvent* event) override;	// esc : close(), enter : handleValues()
	void closeEvent(QCloseEvent* event) override;	// do nothing
};

// takes width/height and returns
// calls : BinpackMainWindow::setResultSize
class SizeReceiver : public BaseReceiver
{
	Q_OBJECT
public:
	SizeReceiver(BinpackMainWindow* owner);
	~SizeReceiver();
protected:
	void initialize() override;
protected slots:
	void handleValues() override;
	void updateRealUnit();

private:
	class PImpl;
	PImpl* pImpl;
};

// takes width/height and returns
// calls : BinpackMainWindow::setResultSize
class RemoveIndexReceiver : public BaseReceiver
{
	Q_OBJECT
public:
	RemoveIndexReceiver(BinpackMainWindow* owner, int imageCount);
	~RemoveIndexReceiver();

protected:
	void initialize() override;
protected slots:
	void handleValues() override;

private:
	class PImpl;
	PImpl* pImpl;
};

// takes offset/rounding pixel and returns
// calls : BinpackMainWindow::setGlobalKarlsunStyle
class KarlsunStyleReceiver : public BaseReceiver
{
	Q_OBJECT
public:
	KarlsunStyleReceiver(BinpackMainWindow* owner);
	~KarlsunStyleReceiver();
protected:
	void initialize() override;
protected slots:
	void handleValues() override;
	void updateRealUnit();

private:
	class PImpl;
	PImpl* pImpl;
};

// takes DPI returns
// calls : BinpackMainWindow::setDPI
class DPIReceiver : public BaseReceiver
{
	Q_OBJECT
public:
	DPIReceiver(BinpackMainWindow* owner);
	~DPIReceiver();
protected:
	void initialize() override;
protected slots:
	void handleValues() override;

private:
	class PImpl;
	PImpl* pImpl;
};
