#pragma once

#include "Receivers.h"
#include "BinpackMainWindow.h";
#include "Utils.h"

#include <QLineEdit>
#include <QHboxLayout>
#include <QValidator>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QKeyEvent>
#include <QDebug>

#pragma region BaseReceiver

BaseReceiver::BaseReceiver(BinpackMainWindow* owner) : QMainWindow(owner)
{
	//call child initialize
	//this->initialize(); //this grammar is forbidden
}

BaseReceiver::~BaseReceiver()
{
}

void BaseReceiver::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		qDebug() << "Esc pressed from sub window. Closing...";
		close();
		break;
	case Qt::Key_Return:	// main enter key
	case Qt::Key_Enter:		// numpad enter key
		qDebug() << "Enter pressed from sub window. call handleValues()";
		handleValues();
		break;
	default:
		QMainWindow::keyPressEvent(event);
	}
}

void BaseReceiver::closeEvent(QCloseEvent* event)
{
	//do something when closing
}

#pragma endregion



#pragma region SizeReceiver

class SizeReceiver::PImpl
{
public:
	PImpl(BinpackMainWindow* owner) : Owner(owner) {}
	~PImpl() {}

public:
	BinpackMainWindow* Owner = 0;
	QLineEdit* widEdit = 0, *hiEdit = 0;
};

SizeReceiver::SizeReceiver(BinpackMainWindow* owner)
	: BaseReceiver(owner)
	, pImpl(new PImpl(owner))
{
	initialize();
}
SizeReceiver::~SizeReceiver()
{
	util::HandyDelete(pImpl);
}


void SizeReceiver::initialize()
{
	auto* editLayout = new QGridLayout;
	auto* validator = new QIntValidator(0, 9999, this);
	pImpl->widEdit = new QLineEdit(this);
	pImpl->widEdit->setValidator(validator);
	pImpl->widEdit->setText(QString::number(pImpl->Owner->canvasSize().width()));
	pImpl->hiEdit = new QLineEdit(this);
	pImpl->hiEdit->setValidator(validator);
	pImpl->hiEdit->setText(QString::number(pImpl->Owner->canvasSize().height()));
	auto* warnLbl = new QLabel(" * Canvas will be cleared", this);
	auto* widLbl = new QLabel("Width", this);
	auto* hiLbl = new QLabel("Height", this);
	auto* widPxLbl = new QLabel("px", this);
	auto* hiPxLbl = new QLabel("px", this);
	auto* updateBtn = new QPushButton("Confirm", this);
	auto* cancelBtn = new QPushButton("Cancel", this);

	enum Rows { WARNER = 0, WID, HI, BUTTONS };

	editLayout->addWidget(warnLbl, WARNER, 0, 1, 4);
	editLayout->addWidget(widLbl, WID, 0, 1, 1);
	editLayout->addWidget(pImpl->widEdit, WID, 1, 1, 2);
	editLayout->addWidget(widPxLbl, WID, 3, 1, 1);
	editLayout->addWidget(hiLbl, HI, 0, 1, 1);
	editLayout->addWidget(pImpl->hiEdit, HI, 1, 1, 2);
	editLayout->addWidget(hiPxLbl, HI, 3, 1, 1);
	editLayout->addWidget(updateBtn, BUTTONS, 0, 1, 2);
	editLayout->addWidget(cancelBtn, BUTTONS, 2, 1, 2);

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(editLayout);

	//QObject::connect(btnWidget, &QPushButton::clicked, this, &updateValue);
	connect(updateBtn, &QPushButton::released, this, &SizeReceiver::handleValues);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	this->setCentralWidget(mainWidget);
	this->setWindowTitle(QString("Resize canvas"));
}

void SizeReceiver::handleValues()
{
	const auto wid = pImpl->widEdit->text().toInt();
	const auto hi = pImpl->hiEdit->text().toInt();

	pImpl->Owner->setCanvasSize({ wid,hi });

	this->close();
}

#pragma endregion


#pragma region RemoveIndexReceiver

class RemoveIndexReceiver::PImpl
{
public:
	PImpl(BinpackMainWindow* owner, int imageCount) : Owner(owner), maxImages(imageCount) {}
	~PImpl() {}

public:
	std::vector<QCheckBox*> checkBoxes;
	BinpackMainWindow* Owner = 0;
	int maxImages;
};

RemoveIndexReceiver::RemoveIndexReceiver(BinpackMainWindow* owner, int imageCount)
	: BaseReceiver(owner)
	, pImpl(new PImpl(owner, imageCount))
{
	initialize();
}
RemoveIndexReceiver::~RemoveIndexReceiver()
{
	util::HandyDelete(pImpl);
}

void RemoveIndexReceiver::initialize()
{
	auto cbLayout = new QGridLayout;
	const int cnt = pImpl->maxImages;

	enum { Title = 0, ChkBox, Btns };

	auto* chkAllLbl = new QLabel("Check all", this);
	auto* chkBoxAll = new QCheckBox(this);
	chkBoxAll->setChecked(false);
	cbLayout->addWidget(chkAllLbl, Title, 0, 1, 1);
	cbLayout->addWidget(chkBoxAll, Title, 1, 1, 1);

	for (int idx = 0; idx < cnt; ++idx)
	{
		auto* chkLbl = new QLabel(QString("%1%2").arg("Index").arg(idx), this);
		auto* chkBox = new QCheckBox(this);
		chkBox->setChecked(false);
		cbLayout->addWidget(chkLbl, ChkBox + idx, 0, 1, 1);
		cbLayout->addWidget(chkBox, ChkBox + idx, 1, 1, 1);
		pImpl->checkBoxes.push_back(chkBox);
	}

	auto* updateBtn = new QPushButton("Confirm", this);
	auto* cancelBtn = new QPushButton("Cancel", this);
	cbLayout->addWidget(updateBtn, Btns + cnt, 0, 1, 1);
	cbLayout->addWidget(cancelBtn, Btns + cnt, 1, 1, 1);
	connect(chkBoxAll, &QCheckBox::stateChanged, [=](int c)
		{
			for (auto box : pImpl->checkBoxes)
				box->setChecked(c);
		}
	);
	connect(updateBtn, &QPushButton::released, this, &RemoveIndexReceiver::handleValues);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(cbLayout);
	this->setCentralWidget(mainWidget);
	this->setWindowTitle(QString("Remove images"));
}

void RemoveIndexReceiver::handleValues()
{
	std::vector<int> retval;

	for (int boxIdx = 0; boxIdx < pImpl->checkBoxes.size(); ++boxIdx)
		if (pImpl->checkBoxes.at(boxIdx)->isChecked())
			retval.push_back(boxIdx);

	pImpl->Owner->setRemoveImages(retval);

	this->close();
}

#pragma endregion
