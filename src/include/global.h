/*
 * global.h
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2022 - adazem009
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

#include <QList>
#include <QPointer>
#include <QSoundEffect>
#include <QTemporaryFile>
#include <QMap>

extern QList<QPointer<QSoundEffect>> allSounds;
extern QList<QTemporaryFile*> allSoundFiles;
extern QMap<QString,QByteArray*> projectAssets;
extern bool __run_frame_again;
