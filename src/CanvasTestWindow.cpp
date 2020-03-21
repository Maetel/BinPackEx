#include "CanvasTestWindow.h"
#include <QGridLayout>
#include <QBrush>
#include <QAction>
#include <QFileDialog>
#include <QMenu>


CanvasTestWindow::CanvasTestWindow(QWidget* parent)
	: QMainWindow(parent)
{
	renderArea = new TestRenderArea(parent);

	shapeComboBox = new QComboBox(parent);
	shapeComboBox->addItem(tr("Polygon"), TestRenderArea::Polygon);
	shapeComboBox->addItem(tr("Rectangle"), TestRenderArea::Rect);
	shapeComboBox->addItem(tr("Rounded Rectangle"), TestRenderArea::RoundedRect);
	shapeComboBox->addItem(tr("Ellipse"), TestRenderArea::Ellipse);
	shapeComboBox->addItem(tr("Pie"), TestRenderArea::Pie);
	shapeComboBox->addItem(tr("Chord"), TestRenderArea::Chord);
	shapeComboBox->addItem(tr("Path"), TestRenderArea::Path);
	shapeComboBox->addItem(tr("Line"), TestRenderArea::Line);
	shapeComboBox->addItem(tr("Polyline"), TestRenderArea::Polyline);
	shapeComboBox->addItem(tr("Arc"), TestRenderArea::Arc);
	shapeComboBox->addItem(tr("Points"), TestRenderArea::Points);
	shapeComboBox->addItem(tr("Text"), TestRenderArea::Text);
	shapeComboBox->addItem(tr("Pixmap"), TestRenderArea::Pixmap);

	shapeLabel = new QLabel(tr("&Shape:"), parent);
	shapeLabel->setBuddy(shapeComboBox);

	penWidthSpinBox = new QSpinBox(parent);
	penWidthSpinBox->setRange(0, 20);
	penWidthSpinBox->setSpecialValueText(tr("0 (cosmetic pen)"));

	penWidthSpinBox = new QSpinBox(parent);
	penWidthLabel = new QLabel(tr("Pen &Width:"), parent);
	penWidthLabel->setBuddy(penWidthSpinBox);

	penStyleComboBox = new QComboBox(parent);
	penStyleComboBox->addItem(tr("Solid"), static_cast<int>(Qt::SolidLine));
	penStyleComboBox->addItem(tr("Dash"), static_cast<int>(Qt::DashLine));
	penStyleComboBox->addItem(tr("Dot"), static_cast<int>(Qt::DotLine));
	penStyleComboBox->addItem(tr("Dash Dot"), static_cast<int>(Qt::DashDotLine));
	penStyleComboBox->addItem(tr("Dash Dot Dot"), static_cast<int>(Qt::DashDotDotLine));
	penStyleComboBox->addItem(tr("None"), static_cast<int>(Qt::NoPen));

	penStyleLabel = new QLabel(tr("&Pen Style:"), parent);
	penStyleLabel->setBuddy(penStyleComboBox);

	penCapComboBox = new QComboBox(parent);
	penCapComboBox->addItem(tr("Flat"), Qt::FlatCap);
	penCapComboBox->addItem(tr("Square"), Qt::SquareCap);
	penCapComboBox->addItem(tr("Round"), Qt::RoundCap);

	penCapLabel = new QLabel(tr("Pen &Cap:"), parent);
	penCapLabel->setBuddy(penCapComboBox);

	penJoinComboBox = new QComboBox(parent);
	penJoinComboBox->addItem(tr("Miter"), Qt::MiterJoin);
	penJoinComboBox->addItem(tr("Bevel"), Qt::BevelJoin);
	penJoinComboBox->addItem(tr("Round"), Qt::RoundJoin);

	penJoinLabel = new QLabel(tr("Pen &Join:"), parent);
	penJoinLabel->setBuddy(penJoinComboBox);

	brushStyleComboBox = new QComboBox(parent);
	brushStyleComboBox->addItem(tr("Linear Gradient"),
		static_cast<int>(Qt::LinearGradientPattern));
	brushStyleComboBox->addItem(tr("Radial Gradient"),
		static_cast<int>(Qt::RadialGradientPattern));
	brushStyleComboBox->addItem(tr("Conical Gradient"),
		static_cast<int>(Qt::ConicalGradientPattern));
	brushStyleComboBox->addItem(tr("Texture"), static_cast<int>(Qt::TexturePattern));
	brushStyleComboBox->addItem(tr("Solid"), static_cast<int>(Qt::SolidPattern));
	brushStyleComboBox->addItem(tr("Horizontal"), static_cast<int>(Qt::HorPattern));
	brushStyleComboBox->addItem(tr("Vertical"), static_cast<int>(Qt::VerPattern));
	brushStyleComboBox->addItem(tr("Cross"), static_cast<int>(Qt::CrossPattern));
	brushStyleComboBox->addItem(tr("Backward Diagonal"), static_cast<int>(Qt::BDiagPattern));
	brushStyleComboBox->addItem(tr("Forward Diagonal"), static_cast<int>(Qt::FDiagPattern));
	brushStyleComboBox->addItem(tr("Diagonal Cross"), static_cast<int>(Qt::DiagCrossPattern));
	brushStyleComboBox->addItem(tr("Dense 1"), static_cast<int>(Qt::Dense1Pattern));
	brushStyleComboBox->addItem(tr("Dense 2"), static_cast<int>(Qt::Dense2Pattern));
	brushStyleComboBox->addItem(tr("Dense 3"), static_cast<int>(Qt::Dense3Pattern));
	brushStyleComboBox->addItem(tr("Dense 4"), static_cast<int>(Qt::Dense4Pattern));
	brushStyleComboBox->addItem(tr("Dense 5"), static_cast<int>(Qt::Dense5Pattern));
	brushStyleComboBox->addItem(tr("Dense 6"), static_cast<int>(Qt::Dense6Pattern));
	brushStyleComboBox->addItem(tr("Dense 7"), static_cast<int>(Qt::Dense7Pattern));
	brushStyleComboBox->addItem(tr("None"), static_cast<int>(Qt::NoBrush));

	brushStyleLabel = new QLabel(tr("&Brush:"), parent);
	brushStyleLabel->setBuddy(brushStyleComboBox);

	otherOptionsLabel = new QLabel(tr("Options:"), parent);
	antialiasingCheckBox = new QCheckBox(tr("&Antialiasing"), parent);

	transformationsCheckBox = new QCheckBox(tr("&Transformations"), parent);

	auto icon = QIcon("wrongpath");
	this->loadImage = new QAction(icon, "load image", this);
	loadImage->activate(QAction::Trigger);
	loadImage->setCheckable(true);
	loadImage->setEnabled(true);

	connect(shapeComboBox, QOverload<int>::of(&QComboBox::activated),this, &CanvasTestWindow::shapeChanged);
	connect(penWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),this, &CanvasTestWindow::penChanged);
	connect(penStyleComboBox, QOverload<int>::of(&QComboBox::activated),this, &CanvasTestWindow::penChanged);
	connect(penCapComboBox, QOverload<int>::of(&QComboBox::activated),this, &CanvasTestWindow::penChanged);
	connect(penJoinComboBox, QOverload<int>::of(&QComboBox::activated),this, &CanvasTestWindow::penChanged);
	connect(brushStyleComboBox, QOverload<int>::of(&QComboBox::activated),this, &CanvasTestWindow::brushChanged);
	connect(antialiasingCheckBox, &QAbstractButton::toggled,renderArea, &TestRenderArea::setAntialiased);
	connect(transformationsCheckBox, &QAbstractButton::toggled,renderArea, &TestRenderArea::setTransformed);
	connect(loadImage, &QAction::triggered, this, &CanvasTestWindow::handleLoadImage);


	QLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(renderArea);
	mainLayout->addWidget(shapeLabel);
	mainLayout->addWidget(shapeComboBox);
	mainLayout->addWidget(penWidthLabel);
	mainLayout->addWidget(penWidthSpinBox);
	mainLayout->addWidget(penStyleLabel);
	mainLayout->addWidget(penStyleComboBox);
	mainLayout->addWidget(penCapLabel);
	mainLayout->addWidget(penCapComboBox);
	mainLayout->addWidget(penJoinLabel);
	mainLayout->addWidget(penJoinComboBox);
	mainLayout->addWidget(brushStyleLabel);
	mainLayout->addWidget(brushStyleComboBox);
	mainLayout->addWidget(otherOptionsLabel);
	mainLayout->addWidget(antialiasingCheckBox);
	mainLayout->addWidget(transformationsCheckBox);
	auto* loadWidget = new QWidget(this);
	loadWidget->addAction(loadImage);
	mainLayout->addWidget(loadWidget);

	auto* tempWidget = new QWidget(parent);
	tempWidget->setLayout(mainLayout);
	
	setCentralWidget(tempWidget);
	QMenu* menu = new QMenu(parent);
	menu->addAction(loadImage);
	addAction(loadImage);
	setMenuWidget(menu);

	shapeChanged();
	penChanged();
	brushChanged();
	antialiasingCheckBox->setChecked(true);

	setWindowTitle(tr("Basic Drawing"));
}

void CanvasTestWindow::shapeChanged()
{
	TestRenderArea::Shape shape = TestRenderArea::Shape(shapeComboBox->itemData(
		shapeComboBox->currentIndex(), IdRole).toInt());
	renderArea->setShape(shape);
}

void CanvasTestWindow::penChanged()
{
	int width = penWidthSpinBox->value();
	Qt::PenStyle style = Qt::PenStyle(penStyleComboBox->itemData(
		penStyleComboBox->currentIndex(), IdRole).toInt());
	Qt::PenCapStyle cap = Qt::PenCapStyle(penCapComboBox->itemData(
		penCapComboBox->currentIndex(), IdRole).toInt());
	Qt::PenJoinStyle join = Qt::PenJoinStyle(penJoinComboBox->itemData(
		penJoinComboBox->currentIndex(), IdRole).toInt());

	renderArea->setPen(QPen(Qt::blue, width, style, cap, join));
}

void CanvasTestWindow::brushChanged()
{
	Qt::BrushStyle style = Qt::BrushStyle(brushStyleComboBox->itemData(penJoinComboBox->currentIndex(), IdRole).toInt());

	if (style == Qt::LinearGradientPattern) {
		QLinearGradient linearGradient(0, 0, 100, 100);
		linearGradient.setColorAt(0.0, Qt::white);
		linearGradient.setColorAt(0.2, Qt::green);
		linearGradient.setColorAt(1.0, Qt::black);
		renderArea->setBrush(linearGradient);
	}
	else if (style == Qt::RadialGradientPattern) {
		QRadialGradient radialGradient(50, 50, 50, 70, 70);
		radialGradient.setColorAt(0.0, Qt::white);
		radialGradient.setColorAt(0.2, Qt::green);
		radialGradient.setColorAt(1.0, Qt::black);
		renderArea->setBrush(radialGradient);
	}
	else if (style == Qt::ConicalGradientPattern) {
		QConicalGradient conicalGradient(50, 50, 150);
		conicalGradient.setColorAt(0.0, Qt::white);
		conicalGradient.setColorAt(0.2, Qt::green);
		conicalGradient.setColorAt(1.0, Qt::black);
		renderArea->setBrush(conicalGradient);
	}
	else if (style == Qt::TexturePattern) {
		renderArea->setBrush(QBrush(QPixmap(":/images/brick.png")));
	}
	else {
		renderArea->setBrush(QBrush(Qt::green, style));
	}

}

void CanvasTestWindow::handleLoadImage()
{
	QFileDialog dia;
	QString file = dia.getOpenFileName(this, "Load image", QString(), "image (*.jpg *.png *.bmp)");
	renderArea->setBrush(QBrush(QImage(file)));
}
