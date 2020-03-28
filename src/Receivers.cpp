#pragma once

#include "Receivers.h"
#include "Utils.h"

#include <QLineEdit>
#include <QHboxLayout>
#include <QValidator>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

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
	: QMainWindow(owner)
	, pImpl(new PImpl(owner))
{
	popup();
}
SizeReceiver::~SizeReceiver()
{
	util::HandyDelete(pImpl);
}


void SizeReceiver::popup()
{
	auto* editLayout = new QGridLayout;
	auto* validator = new QIntValidator(0, 9999, this);
	pImpl->widEdit = new QLineEdit(this);
	pImpl->widEdit->setValidator(validator);
	pImpl->hiEdit = new QLineEdit(this);
	pImpl->hiEdit->setValidator(validator);
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
	connect(updateBtn, &QPushButton::released, this, &SizeReceiver::updateValue);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	this->setCentralWidget(mainWidget);
	this->setWindowTitle(QString("Resize canvas"));
}

void SizeReceiver::closeEvent(QCloseEvent* event)
{
	//pImpl->Owner->setEnabled(true);
}

void SizeReceiver::updateValue()
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
	: QMainWindow(owner)
	, pImpl(new PImpl(owner, imageCount))
{
	popup();
}
RemoveIndexReceiver::~RemoveIndexReceiver()
{
	util::HandyDelete(pImpl);
}

void RemoveIndexReceiver::popup()
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
	connect(updateBtn, &QPushButton::released, this, &RemoveIndexReceiver::updateValue);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(cbLayout);
	this->setCentralWidget(mainWidget);
	this->setWindowTitle(QString("Remove images"));
}

void RemoveIndexReceiver::updateValue()
{
	std::vector<int> retval;

	for (int boxIdx = 0; boxIdx < pImpl->checkBoxes.size(); ++boxIdx)
		if (pImpl->checkBoxes.at(boxIdx)->isChecked())
			retval.push_back(boxIdx);

	pImpl->Owner->setRemoveImages(retval);

	this->close();
}

#pragma endregion
