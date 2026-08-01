#ifndef POINTCLOUDAPP_ADDIMAGESOURCEDIALOG_H
#define POINTCLOUDAPP_ADDIMAGESOURCEDIALOG_H

#include "AddImageSourceDialog.h"
#include "QWidget"
#include "QVBoxLayout"
#include "QLabel"

class AddImageSourceDialog : public QWidget {
	Q_OBJECT
public:
	AddImageSourceDialog() : QWidget(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		mainLayout->addWidget(new QLabel("Test"));
	}

private:
};



#endif
