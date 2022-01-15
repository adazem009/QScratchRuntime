/*
 * mainwindow.cpp
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2021-2022 - adazem009
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
	view->scale(2,2);
	view->setMouseTracking(true);
	ui->gridLayout->addWidget(view);
	view->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	view->setStyleSheet("QGraphicsView { background-color: rgb(255,255,255); }");
	view->show();
	// Connections
	connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
	connect(ui->loadFromUrlButton,SIGNAL(clicked()),this,SLOT(loadFromUrl()));
	connect(ui->greenFlag,&QPushButton::clicked,scene,&projectScene::greenFlag);
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
	if((fileName = QFileDialog::getOpenFileName(this,tr("Open Scratch project"),QString(),tr("Scratch project JSON file") + " (project.json)")) == "")
		return;
	parser = new projectParser(fileName);
	init();
}

/*!
 * Connected from loadFromUrlButton->clicked().\n
 * Loads a project from URL in urlEdit.
 */
void MainWindow::loadFromUrl(void)
{
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkReply *reply;
	QEventLoop requestLoop;
	connect(manager,&QNetworkAccessManager::finished,&requestLoop,&QEventLoop::quit);
	// Get project ID
	QString buffer = "", projectID = "";
	for(int i=0; i < ui->urlEdit->text().count(); i++)
	{
		if((ui->urlEdit->text().at(i) == '/') || (i+1 == ui->urlEdit->text().count()))
		{
			if(!(ui->urlEdit->text().at(i) == '/'))
				buffer += ui->urlEdit->text().at(i);
			projectID = buffer;
			buffer = "";
		}
		else
			buffer += ui->urlEdit->text().at(i);
	}
	// Download project.json
	reply = manager->get(QNetworkRequest(QUrl("https://projects.scratch.mit.edu/" + projectID)));
	requestLoop.exec(QEventLoop::ExcludeUserInputEvents);
	reply->waitForReadyRead(-1);
	parser = new projectParser("",reply->readAll());
	// Download assets
	QList<QMap<QString,QString>> assets = parser->assetIDs();
	projectAssets.clear();
	for(int i=0; i < assets.count(); i++)
	{
		qDebug() << QString("Downloading ") + QString("https://assets.scratch.mit.edu/internalapi/asset/") + assets[i]["assetId"] + "." + assets[i]["dataFormat"] + QString("/get/");
		reply = manager->get(QNetworkRequest(QUrl("https://assets.scratch.mit.edu/internalapi/asset/" + assets[i]["assetId"] + "." + assets[i]["dataFormat"] + "/get/")));
		requestLoop.exec(QEventLoop::ExcludeUserInputEvents);
		reply->waitForReadyRead(-1);
		QByteArray *assetData = new QByteArray(reply->readAll());
		projectAssets.insert(assets[i]["assetId"],assetData);
	}
	init();
}

/*! Reads project.json and initializes the project. */
void MainWindow::init(void)
{
	int i;
	QList<QGraphicsItem*> oldItems = scene->items();
	for(i=0; i < oldItems.count(); i++)
	{
		scene->removeItem(oldItems[i]);
		delete oldItems[i];
	}
	allSounds.clear();
	allSoundFiles.clear();
	sprites = parser->sprites();
	// Uncomment the following 2 lines to show X and Y axis
	//scene->addLine(-240,0,240,0);
	//scene->addLine(0,-180,0,180);
	// Add sprites
	for(i=0; i < sprites.count(); i++)
		scene->addItem(sprites[i]);
	// Add list to sprite pointers to every sprite
	for(i=0; i < sprites.count(); i++)
		sprites[i]->loadSpriteList(sprites);
	// Add sprite list to scene
	scene->loadSpriteList(sprites);
}
