/*
 * mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QGraphicsView>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "projectscene.h"
#include "core/projectparser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/*! The MainWindow class is the main window of QScratchRuntime. */
class MainWindow : public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow(QWidget *parent = nullptr);
		~MainWindow();

	private:
		Ui::MainWindow *ui;
		QString fileName;
		projectParser *parser;
		projectScene *scene;
		QGraphicsView *view;
		QList<scratchSprite*> sprites;
		QNetworkAccessManager *manager = nullptr;
		QNetworkReply *currentReply = nullptr;
		QVector<QNetworkReply*> assetReplies;
		bool projectDataLoaded;
		QList<QMap<QString,QString>> assets;
		int loadedAssets;
		void continueLoading(QNetworkReply* reply);
		void init(void);

	private slots:
		void openFile(void);
		void loadFromUrl(void);
};

#endif // MAINWINDOW_H
