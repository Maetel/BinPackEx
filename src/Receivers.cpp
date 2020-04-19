#pragma once

#include "Receivers.h"
#include "BinpackMainWindow.h"
#include "Utils.h"
//#include "Karlsun.h"

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
	QLabel* inRealUnits = 0;
	int curDPI;
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
	pImpl->curDPI = pImpl->Owner->DPI();

	auto* editLayout = new QGridLayout;
	auto* validator = new QIntValidator(0, 9999, this);
	pImpl->widEdit = new QLineEdit(this);
	pImpl->widEdit->setValidator(validator);
	pImpl->widEdit->setText(QString::number(pImpl->Owner->canvasSize().width()));
	pImpl->hiEdit = new QLineEdit(this);
	pImpl->hiEdit->setValidator(validator);
	pImpl->hiEdit->setText(QString::number(pImpl->Owner->canvasSize().height()));
	auto* warnLbl = new QLabel(KorStr(" * 기존 데이터가 지워집니다"), this);
	auto* widLbl = new QLabel(KorStr("가로"), this);
	auto* hiLbl = new QLabel(KorStr("세로"), this);
	auto* widPxLbl = new QLabel("px", this);
	auto* hiPxLbl = new QLabel("px", this);
	pImpl->inRealUnits = new QLabel(this);
	auto* updateBtn = new QPushButton(KorStr("확인"), this);
	auto* cancelBtn = new QPushButton(KorStr("취소"), this);

	enum Rows { WARNER = 0, WID, HI, IN_REAL_UNITS, BUTTONS };

	editLayout->addWidget(warnLbl, WARNER, 0, 1, 4);
	editLayout->addWidget(widLbl, WID, 0, 1, 1);
	editLayout->addWidget(pImpl->widEdit, WID, 1, 1, 2);
	editLayout->addWidget(widPxLbl, WID, 3, 1, 1);
	editLayout->addWidget(hiLbl, HI, 0, 1, 1);
	editLayout->addWidget(pImpl->hiEdit, HI, 1, 1, 2);
	editLayout->addWidget(hiPxLbl, HI, 3, 1, 1);
	editLayout->addWidget(pImpl->inRealUnits, IN_REAL_UNITS, 0, 1, 4);
	editLayout->addWidget(updateBtn, BUTTONS, 0, 1, 2);
	editLayout->addWidget(cancelBtn, BUTTONS, 2, 1, 2);

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(editLayout);

	updateRealUnit();
	//QObject::connect(btnWidget, &QPushButton::clicked, this, &updateValue);
	connect(pImpl->widEdit, &QLineEdit::textChanged, this, &SizeReceiver::updateRealUnit);
	connect(pImpl->hiEdit, &QLineEdit::textChanged, this, &SizeReceiver::updateRealUnit);
	connect(updateBtn, &QPushButton::released, this, &SizeReceiver::handleValues);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	this->setCentralWidget(mainWidget);
	this->setWindowTitle(QString(KorStr("사이즈 설정")));
}

void SizeReceiver::handleValues()
{
	const auto curSize = pImpl->Owner->canvasSize();

	int wid = pImpl->widEdit->text().toInt();
	int hi = pImpl->hiEdit->text().toInt();

	if (wid <= 0)
		wid = curSize.width();
	if (hi <= 0)
		hi = curSize.height();

	pImpl->Owner->setCanvasSize({ wid,hi });

	this->close();
}

void SizeReceiver::updateRealUnit()
{
	const auto wid = pImpl->widEdit->text().toInt();
	const auto hi = pImpl->hiEdit->text().toInt();

	const double widInMm = util::px2mm(wid, pImpl->curDPI);
	const double hiInMm = util::px2mm(hi, pImpl->curDPI);

	pImpl->inRealUnits->setText(QString("%4 : %1mm * %2mm (DPI : %3)").arg(QString::number(widInMm, 'f', 1)).arg(QString::number(hiInMm, 'f', 1)).arg(pImpl->curDPI).arg(KorStr("사이즈")));
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

	auto* chkAllLbl = new QLabel(KorStr("전체선택"), this);
	auto* chkBoxAll = new QCheckBox(this);
	chkBoxAll->setChecked(false);
	cbLayout->addWidget(chkAllLbl, Title, 0, 1, 1);
	cbLayout->addWidget(chkBoxAll, Title, 1, 1, 1);

	for (int idx = 0; idx < cnt; ++idx)
	{
		auto* chkLbl = new QLabel(QString("%1%2").arg(KorStr("이미지")).arg(idx), this);
		auto* chkBox = new QCheckBox(this);
		chkBox->setChecked(false);
		cbLayout->addWidget(chkLbl, ChkBox + idx, 0, 1, 1);
		cbLayout->addWidget(chkBox, ChkBox + idx, 1, 1, 1);
		pImpl->checkBoxes.push_back(chkBox);
	}

	auto* updateBtn = new QPushButton(KorStr("확인"), this);
	auto* cancelBtn = new QPushButton(KorStr("취소"), this);
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
	this->setWindowTitle(QString(KorStr("이미지 제거")));
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

#pragma region KarlsunStyleReceiver

class KarlsunStyleReceiver::PImpl
{
public:
	PImpl(BinpackMainWindow* owner) : Owner(owner) {}
	~PImpl() {}

public:
	BinpackMainWindow* Owner = 0;
	QLineEdit* offsetEdit = 0, *roundingEdit = 0;
	QLabel* offsetPxLbl = 0, *roundingPxLbl = 0;
	int curDPI;
};

KarlsunStyleReceiver::KarlsunStyleReceiver(BinpackMainWindow* owner)
	: BaseReceiver(owner)
	, pImpl(new PImpl(owner))
{
	initialize();
}
KarlsunStyleReceiver::~KarlsunStyleReceiver()
{
	util::HandyDelete(pImpl);
}


void KarlsunStyleReceiver::initialize()
{
	auto globalKarlsunStyle = pImpl->Owner->karlsunStyle();
	const int offset = globalKarlsunStyle.offset;
	const int rounding = globalKarlsunStyle.roundPixel;
	pImpl->curDPI = pImpl->Owner->DPI();

	auto* editLayout = new QGridLayout;
	auto* validator = new QIntValidator(0, 999, this);
	pImpl->offsetEdit = new QLineEdit(this);
	pImpl->offsetEdit->setValidator(validator);
	pImpl->offsetEdit->setText(QString::number(offset));
	pImpl->roundingEdit = new QLineEdit(this);
	pImpl->roundingEdit->setValidator(validator);
	pImpl->roundingEdit->setText(QString::number(rounding));
	auto* DPIInfoLbl = new QLabel(QString("DPI : %1").arg(QString::number(pImpl->curDPI)), this);
	auto* widLbl = new QLabel(KorStr("오프셋"), this);
	auto* hiLbl = new QLabel(KorStr("라운딩"), this);
	pImpl->offsetPxLbl = new QLabel(QString("px"), this);
	pImpl->roundingPxLbl = new QLabel("px", this);
	auto* updateBtn = new QPushButton(KorStr("확인"), this);
	auto* cancelBtn = new QPushButton(KorStr("취소"), this);

	enum Rows { DPIINFO, OFFSET, ROUNDING, BUTTONS };

	editLayout->addWidget(DPIInfoLbl, DPIINFO, 3, 1, 1);
	editLayout->addWidget(widLbl, OFFSET, 0, 1, 1);
	editLayout->addWidget(pImpl->offsetEdit, OFFSET, 1, 1, 2);
	editLayout->addWidget(pImpl->offsetPxLbl, OFFSET, 3, 1, 1);
	editLayout->addWidget(hiLbl, ROUNDING, 0, 1, 1);
	editLayout->addWidget(pImpl->roundingEdit, ROUNDING, 1, 1, 2);
	editLayout->addWidget(pImpl->roundingPxLbl, ROUNDING, 3, 1, 1);
	editLayout->addWidget(updateBtn, BUTTONS, 0, 1, 2);
	editLayout->addWidget(cancelBtn, BUTTONS, 2, 1, 2);

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(editLayout);

	updateRealUnit();
	connect(pImpl->offsetEdit, &QLineEdit::textChanged, this, &KarlsunStyleReceiver::updateRealUnit);
	connect(pImpl->roundingEdit, &QLineEdit::textChanged, this, &KarlsunStyleReceiver::updateRealUnit);
	connect(updateBtn, &QPushButton::released, this, &KarlsunStyleReceiver::handleValues);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	this->setCentralWidget(mainWidget);
	this->setWindowTitle(KorStr("칼선"));
}

void KarlsunStyleReceiver::handleValues()
{
	const auto offset = pImpl->offsetEdit->text().toInt();
	const auto rounding = pImpl->roundingEdit->text().toInt();

	pImpl->Owner->setGlobalKarlsunStyle ({ offset,rounding });

	this->close();
}

void KarlsunStyleReceiver::updateRealUnit()
{
	const auto offset = pImpl->offsetEdit->text().toInt();
	const auto rounding = pImpl->roundingEdit->text().toInt();

	const double offsetInMm = util::px2mm(offset, pImpl->curDPI);
	const double roundingInMm = util::px2mm(rounding, pImpl->curDPI);

	pImpl->offsetPxLbl->setText(QString("px (%1mm)").arg(QString::number(offsetInMm, 'f', 2)));
	pImpl->roundingPxLbl->setText(QString("px (%1mm)").arg(QString::number(roundingInMm, 'f', 2)));
}

#pragma endregion

#pragma region DPIReceiver

class DPIReceiver::PImpl
{
public:
	PImpl(BinpackMainWindow* owner) : Owner(owner) {}
	~PImpl() {}

public:
	BinpackMainWindow* Owner = 0;
	QLineEdit* DPIEdit = 0;
};

DPIReceiver::DPIReceiver(BinpackMainWindow* owner)
	: BaseReceiver(owner)
	, pImpl(new PImpl(owner))
{
	initialize();
}
DPIReceiver::~DPIReceiver()
{
	util::HandyDelete(pImpl);
}


void DPIReceiver::initialize()
{
	auto curDPI = pImpl->Owner->DPI();
	
	auto* editLayout = new QGridLayout;
	auto* validator = new QIntValidator(0, 300, this);
	pImpl->DPIEdit = new QLineEdit(this);
	pImpl->DPIEdit->setValidator(validator);
	pImpl->DPIEdit->setText(QString::number(curDPI));
	auto* widLbl = new QLabel("DPI", this);
	auto* dpiLbl= new QLabel("", this);
	auto* updateBtn = new QPushButton(KorStr("확인"), this);
	auto* cancelBtn = new QPushButton(KorStr("취소"), this);

	enum Rows { DPI_INDICATOR = 0, BUTTONS };

	editLayout->addWidget(widLbl, DPI_INDICATOR, 0, 1, 1);
	editLayout->addWidget(pImpl->DPIEdit, DPI_INDICATOR, 1, 1, 2);
	editLayout->addWidget(dpiLbl, DPI_INDICATOR, 3, 1, 1);
	editLayout->addWidget(updateBtn, BUTTONS, 0, 1, 2);
	editLayout->addWidget(cancelBtn, BUTTONS, 2, 1, 2);

	auto* mainWidget = new QWidget(this);
	mainWidget->setLayout(editLayout);

	connect(updateBtn, &QPushButton::released, this, &DPIReceiver::handleValues);
	connect(cancelBtn, &QPushButton::released, [=]() {this->close(); });

	this->setCentralWidget(mainWidget);
	this->setWindowTitle(KorStr("DPI 설정"));
}

void DPIReceiver::handleValues()
{
	int DPI = pImpl->DPIEdit->text().toInt();
	if (DPI <= 0)
		DPI = pImpl->Owner->DPI();

	pImpl->Owner->setDPI(DPI);

	this->close();
}

#pragma endregion