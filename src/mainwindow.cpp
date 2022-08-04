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
	scene = new projectScene(1);
	view = new QGraphicsView(scene,ui->centralwidget);
	view->setMouseTracking(true);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->projectLayout->addWidget(view);
	view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	view->setStyleSheet("QGraphicsView { background-color: rgb(255,255,255); }");
	view->hide();
	QMetaObject::invokeMethod(this, "adjustSceneSize", Qt::QueuedConnection);
#ifndef Q_OS_WASM
	QOpenGLWidget *gl = new QOpenGLWidget();
	QSurfaceFormat format;
	format.setSamples(4);
	gl->setFormat(format);
	view->setViewport(gl);
#else
	ui->actionMultithreading->setEnabled(false);
#endif // Q_OS_WASM
	ui->loaderFrame->hide();
	ui->greenFlag->setEnabled(false);
	ui->stopButton->setEnabled(false);
	setCurrentFps(0);
	ui->actionMultithreading->setChecked(settings.value("main/multithreading", false).toBool());
	ui->actionSvgUpscale->setChecked(settings.value("main/hqsvg", true).toBool());
	// Connections
	connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
	connect(ui->actionFps, &QAction::triggered, this, &MainWindow::changeFps);
	connect(ui->actionMultithreading, &QAction::triggered, this, &MainWindow::toggleMultithreading);
	connect(ui->actionSvgUpscale, &QAction::triggered, this, &MainWindow::toggleSvgUpscale);
	connect(ui->loadFromUrlButton,SIGNAL(clicked()),this,SLOT(loadFromUrl()));
	connect(ui->greenFlag,&QPushButton::clicked,scene,&projectScene::greenFlag);
	connect(ui->stopButton,&QPushButton::clicked,scene,&projectScene::stop);
	connect(scene ,&projectScene::currentFpsChanged, this, &MainWindow::setCurrentFps);
}

/*! Destroys MainWindow. */
MainWindow::~MainWindow()
{
	delete ui;
	for(int i=0; i < assetPointers.count(); i++)
	{
		if(assetPointers[i])
			delete assetPointers[i];
	}
}

/*!
 * Connected from actionOpen->clicked().\n
 * Opens a Scratch project.
 */
void MainWindow::openFile(void)
{
	if((fileName = QFileDialog::getOpenFileName(this,tr("Open Scratch project"),QString(),tr("Scratch project JSON file") + " (project.json)")) == "")
		return;
	if(manager != nullptr)
	{
		disconnect(manager,nullptr,nullptr,nullptr);
		if(currentReply != nullptr)
			currentReply->abort();
		manager->deleteLater();
		currentReply->deleteLater();
		manager = nullptr;
		currentReply = nullptr;
		for(int i=0; i < assetReplies.count(); i++)
			assetReplies[i]->deleteLater();
		assetReplies.clear();
	}
	ui->loaderFrame->hide();
	view->show();
	parser = new projectParser(fileName, "", this);
	init();
}

/*!
 * Connected from loadFromUrlButton->clicked().\n
 * Loads a project from URL in urlEdit.
 */
void MainWindow::loadFromUrl(void)
{
	scene->clearSpriteList();
	QList<QGraphicsItem*> oldItems = scene->items();
	for(int i=0; i < oldItems.count(); i++)
	{
		scene->removeItem(oldItems[i]);
		delete oldItems[i];
	}
	allSounds.clear();
	ui->greenFlag->setEnabled(false);
	ui->stopButton->setEnabled(false);
	view->hide();
	ui->loaderFrame->show();
	ui->loadingProgressBar->setRange(0,1);
	ui->loadingProgressBar->setValue(0);
	if(manager != nullptr)
	{
		disconnect(manager,nullptr,nullptr,nullptr);
		if(currentReply != nullptr)
			currentReply->abort();
		manager->deleteLater();
		currentReply->deleteLater();
		manager = nullptr;
		currentReply = nullptr;
		for(int i=0; i < assetReplies.count(); i++)
			assetReplies[i]->deleteLater();
		assetReplies.clear();
	}
	manager = new QNetworkAccessManager(this);
	connect(manager,&QNetworkAccessManager::finished,this,&MainWindow::continueLoading);
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
	ui->loadingProgressLabel->setText(tr("Loading project data..."));
	projectDataLoaded = false;
	currentReply = manager->get(QNetworkRequest(QUrl("https://projects.scratch.mit.edu/" + projectID)));
}

/*! Continues loading the project. */
void MainWindow::continueLoading(QNetworkReply* reply)
{
	QString loadingAssetsText = tr("Loading assets...");
	int currentAsset;
	if(!projectDataLoaded)
	{
		parser = new projectParser("",reply->readAll());
		ui->loadingProgressLabel->setText(loadingAssetsText);
		assets = parser->assetIDs();
		ui->loadingProgressBar->setRange(0,assets.count());
		for(int i=0; i < projectAssets.count(); i++)
		{
			assetPointers.removeAll(projectAssets[projectAssets.keys().at(i)]);
			delete projectAssets[projectAssets.keys().at(i)];
		}
		projectAssets.clear();
		projectDataLoaded = true;
		currentAsset = 0;
		loadedAssets = 0;
		assetReplies.clear();
		for(int i=0; i < assets.count(); i++)
			assetReplies += manager->get(QNetworkRequest(QUrl("https://assets.scratch.mit.edu/internalapi/asset/" + assets[i]["assetId"] + "." + assets[i]["dataFormat"] + "/get/")));
	}
	else
	{
		currentAsset = assetReplies.indexOf(reply);
		ui->loadingProgressBar->setValue(loadedAssets+1);
		ui->loadingProgressLabel->setText(loadingAssetsText + " (" + QString::number(loadedAssets+1) + "/" + QString::number(assets.count()) + ")");
		QByteArray *assetData = new QByteArray(reply->readAll());
		projectAssets.insert(assets[currentAsset]["assetId"],assetData);
		assetPointers.append(assetData);
		loadedAssets++;
	}
	for(int i=0; i < assetReplies.count(); i++)
	{
		if(assetReplies[i]->isRunning())
			return;
	}
	ui->loaderFrame->hide();
	view->show();
	init();
}

/*! Reads project.json and initializes the project. */
void MainWindow::init(void)
{
	int i;
	scene->clearSpriteList();
	QList<QGraphicsItem*> oldItems = scene->items();
	for(i=0; i < oldItems.count(); i++)
	{
		scene->removeItem(oldItems[i]);
		delete oldItems[i];
	}
	allSounds.clear();
	sprites = parser->sprites();
	// Uncomment the following 2 lines to show X and Y axis
	//scene->addLine(-240,0,240,0);
	//scene->addLine(0,-180,0,180);
	// Add sprites
	for(i=0; i < sprites.count(); i++)
		scene->addItem(sprites[i]);
	// Add sprite list to scene
	scene->loadSpriteList(sprites);
	// Enable control buttons
	ui->greenFlag->setEnabled(true);
	ui->stopButton->setEnabled(true);
}

/*! Sets scene scale based on window size. */
void MainWindow::adjustSceneSize(void)
{
	scene->setScale(std::min(ui->projectFrame->width() / 480.0, ui->projectFrame->height() / 360.0));
	view->setMaximumSize(QSize(480 * scene->sceneScale(), 360 * scene->sceneScale()));
}

/*! Overrides QMainWindow#resizeEvent(). */
void MainWindow::resizeEvent(QResizeEvent *event)
{
	adjustSceneSize();
	QMainWindow::resizeEvent(event);
}

/*! Allows the user to change FPS. */
void MainWindow::changeFps(void)
{
	QInputDialog *dialog = new QInputDialog(this);
	dialog->setInputMode(QInputDialog::IntInput);
	dialog->setIntMinimum(1);
	dialog->setIntMaximum(QGuiApplication::primaryScreen()->refreshRate());
	dialog->setIntValue(settings.value("main/fps", 30).toInt());
	dialog->setWindowModality(Qt::WindowModal);
	connect(dialog, &QDialog::accepted, this, [this, dialog]() {
		scene->setFps(dialog->intValue());
	});
	dialog->open();
}

/*! Shows the given FPS value on the FPS label. */
void MainWindow::setCurrentFps(int fps)
{
	ui->fpsLabel->setText("FPS: " + QString::number(fps));
}

/*! Toggles multithreading. */
void MainWindow::toggleMultithreading(bool state)
{
	scene->setMultithreading(state);
}

/*! Toggles SVG upscaling. */
void MainWindow::toggleSvgUpscale(bool state)
{
	settings.setValue("main/hqsvg", state);
	// Set scale to refresh sprites
	scene->setScale(scene->sceneScale());
}
