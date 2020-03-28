#pragma once

#include "BinpackMainWindow.h"

// takes width/height and returns
// calls : BinpackMainWindow::setResultSize
class SizeReceiver : public QMainWindow
{
	Q_OBJECT
public:
	SizeReceiver(BinpackMainWindow* owner);
	~SizeReceiver();

protected:
	void popup();
	void closeEvent(QCloseEvent* event) override;
protected slots:
	void updateValue();

private:
	class PImpl;
	PImpl* pImpl;
};