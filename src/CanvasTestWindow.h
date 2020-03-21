
#include "TestRenderArea.h"

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

class CanvasTestWindow : public QMainWindow
{
	Q_OBJECT

public:
	CanvasTestWindow(QWidget* parent = nullptr);

private slots:
	void shapeChanged();
	void penChanged();
	void brushChanged();
	void handleLoadImage();

private:
	const int IdRole = Qt::UserRole;
	TestRenderArea* renderArea;
	QLabel* shapeLabel;
	QLabel* penWidthLabel;
	QLabel* penStyleLabel;
	QLabel* penCapLabel;
	QLabel* penJoinLabel;
	QLabel* brushStyleLabel;
	QLabel* otherOptionsLabel;
	QComboBox* shapeComboBox;
	QSpinBox* penWidthSpinBox;
	QComboBox* penStyleComboBox;
	QComboBox* penCapComboBox;
	QComboBox* penJoinComboBox;
	QComboBox* brushStyleComboBox;
	QCheckBox* antialiasingCheckBox;
	QCheckBox* transformationsCheckBox;
	QAction* loadImage;
};