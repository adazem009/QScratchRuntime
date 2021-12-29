/*
 * mainwindow.cpp
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2021 - adazem009
 *
 * QScratchRuntime is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * QScratchRuntime is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QScratchRuntime. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*! Constructs MainWindow. */
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scene = new projectScene;
	view = new QGraphicsView(scene,ui->centralwidget);
	view->setMouseTracking(true);
	ui->gridLayout->addWidget(view);
	view->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	view->setMinimumWidth(480);
	view->setMinimumHeight(360);
	view->setStyleSheet("QGraphicsView { background-color: rgb(255,255,255); }");
	view->show();
	// Connections
	connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
}

/*! Destroys MainWindow. */
MainWindow::~MainWindow()
{
	delete ui;
}

/*!
 * Connected from actionOpen->clicked().\n
 * Opens a Scratch project.
 */
void MainWindow::openFile(void)
{
	fileName = QFileDialog::getOpenFileName(this,tr("Open Scratch project"),QString(),tr("Scratch project JSON file") + " (project.json)");
	init();
}

/*! Reads project.json and initializes the project. */
void MainWindow::init(void)
{
	parser = new projectParser(fileName);
	sprites = parser->sprites();
	// Add stage
	scene->addItem(parser->stage());
	// Uncomment the following 2 lines to show X and Y axis
	//scene->addLine(-240,0,240,0);
	//scene->addLine(0,-180,0,180);
	int i;
	// Add sprites
	for(i=0; i < sprites.count(); i++)
	{
		connect(ui->greenFlag,SIGNAL(clicked()),sprites[i],SLOT(greenFlagClicked()));
		if(!sprites[i]->isStage)
			scene->addItem(sprites[i]);
	}
	// Add list to sprite pointers to every sprite
	for(i=0; i < sprites.count(); i++)
		sprites[i]->loadSpriteList(sprites);
	// Add sprite list to scene
	scene->loadSpriteList(sprites);
}
