/*
 * blocks.cpp
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

/*
 * This file was created to make the scratchSprite class simpler to read.
 * All functions in this file are members of scratchSprite.
 */

#include "core/scratchsprite.h"

/*! Runs motion blocks. */
bool scratchSprite::motionBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(frameEnd == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
	if(processEnd == nullptr)
	{
		bool tmpProcessEnd;
		processEnd = &tmpProcessEnd;
	}
	if(returnValue == nullptr)
	{
		QString tmpReturnValue;
		returnValue = &tmpReturnValue;
	}
	*processEnd = false;
	if(opcode == "motion_movesteps")
	{
		qreal steps = inputs.value("STEPS").toDouble();
		// https://en.scratch-wiki.info/wiki/Move_()_Steps_(block)#Workaround
		setXPos(spriteX + qSin(qDegreesToRadians(direction))*steps);
		setYPos(spriteY + qCos(qDegreesToRadians(direction))*steps);
	}
	else if(opcode == "motion_turnright")
		setDirection(direction + inputs.value("DEGREES").toDouble());
	else if(opcode == "motion_turnleft")
		setDirection(direction - inputs.value("DEGREES").toDouble());
	else if(opcode == "motion_pointindirection")
		setDirection(inputs.value("DIRECTION").toDouble());
	else if(opcode == "motion_pointtowards")
	{
		QString targetName = inputs.value("TOWARDS");
		scratchSprite *targetSprite = getSprite(targetName);
		qreal deltaX = 0, deltaY = 0;
		if(targetSprite == nullptr)
		{
			deltaX = mouseX - spriteX;
			deltaY = mouseY - spriteY;
		}
		else
		{
			deltaX = targetSprite->spriteX - spriteX;
			deltaY = targetSprite->spriteY - spriteY;
		}
		// https://en.scratch-wiki.info/wiki/Point_Towards_()_(block)#Workaround
		if(deltaY == 0)
		{
			if(deltaX < 0)
				setDirection(-90);
			else
				setDirection(90);
		}
		else
		{
			qreal atanResult = qRadiansToDegrees(qAtan(deltaX/deltaY));
			if(deltaY < 0)
				setDirection(180 + atanResult);
			else
				setDirection(atanResult);
		}
	}
	else if(opcode == "motion_gotoxy")
	{
		if(inputs.contains("X"))
		{
				setXPos(inputs.value("X").toDouble());
				setYPos(inputs.value("Y").toDouble());
		}
	}
	else if(opcode == "motion_goto")
	{
		QString targetName = inputs.value("TO");
		scratchSprite *targetSprite = getSprite(targetName);
		if(targetSprite == nullptr)
		{
			if(targetName == "_mouse_")
			{
				setXPos(mouseX);
				setYPos(mouseY);
			}
			else if(targetName == "_random_")
			{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
				setXPos(QRandomGenerator::global()->bounded(-240,241));
				setYPos(QRandomGenerator::global()->bounded(-180,181));
#else
				setXPos(qrand()%481 - 240);
				setYPos(qrand()%361 - 180);
#endif
			}
		}
		else
		{
			setXPos(targetSprite->spriteX);
			setYPos(targetSprite->spriteY);
		}
	}
	else if((opcode == "motion_glidesecstoxy") || (opcode == "motion_glideto"))
	{
		*frameEnd = true;
		qreal endX = 0, endY = 0;
		if(currentExecPos[processID]["special"].toString() == "glide")
		{
			endX = currentExecPos[processID]["endX"].toDouble();
			endY = currentExecPos[processID]["endY"].toDouble();
		}
		else
		{
			if(opcode == "motion_glidesecstoxy")
			{
				endX = inputs.value("X").toDouble();
				endY = inputs.value("Y").toDouble();
			}
			else
			{
				QString targetName = inputs.value("TO");
				scratchSprite *targetSprite = getSprite(targetName);
				if(targetSprite == nullptr)
				{
					if(targetName == "_mouse_")
					{
						endX = mouseX;
						endY = mouseY;
					}
					else if(targetName == "_random_")
					{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
						endX = QRandomGenerator::global()->bounded(-240,241);
						endY = QRandomGenerator::global()->bounded(-180,181);
#else
						endX = qrand()%481 - 240;
						endY = qrand()%361 - 180;
#endif
					}
				}
				else
				{
					endX = targetSprite->spriteX;
					endY = targetSprite->spriteY;
				}
			}
			currentExecPos[processID]["special"] = "glide";
			currentExecPos[processID]["startX"] = spriteX;
			currentExecPos[processID]["startY"] = spriteY;
			currentExecPos[processID]["endX"] = endX;
			currentExecPos[processID]["endY"] = endY;
			currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addSecs(inputs.value("SECS").toDouble());
		}
		qreal startX = currentExecPos[processID]["startX"].toDouble();
		qreal startY = currentExecPos[processID]["startY"].toDouble();
		QDateTime startTime = currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			setXPos(endX);
			setYPos(endY);
			*processEnd = true;
			*frameEnd = false;
		}
		else
		{
			setXPos(startX + (endX-startX)*progress);
			setYPos(startY + (endY-startY)*progress);
		}
	}
	else if(opcode == "motion_changexby")
		setXPos(spriteX + inputs.value("DX").toDouble());
	else if(opcode == "motion_setx")
		setXPos(inputs.value("X").toDouble());
	else if(opcode == "motion_changeyby")
		setYPos(spriteY + inputs.value("DY").toDouble());
	else if(opcode == "motion_sety")
		setYPos(inputs.value("Y").toDouble());
	else if(opcode == "motion_ifonedgebounce")
	{
		QRectF spriteRect = boundingRect();
		// Right edge
		if(spriteX + (spriteRect.width()/2) > 240)
		{
			setDirection(-direction);
			setXPos(240 - (spriteRect.width()/2));
		}
		// Left edge
		if(spriteX - (spriteRect.width()/2) < -240)
		{
			setDirection(-direction);
			setXPos(-240 + (spriteRect.width()/2));
		}
		// Top edge
		if(spriteY + (spriteRect.height()/2) > 180)
		{
			setDirection(180-direction);
			setYPos(180 - (spriteRect.height()/2));
		}
		// Bottom edge
		if(spriteY - (spriteRect.height()/2) < -180)
		{
			setDirection(180-direction);
			setYPos(-180 + (spriteRect.height()/2));
		}
	}
	else if(opcode == "motion_setrotationstyle")
	{
		rotationStyle = inputs.value("STYLE");
		setDirection(direction);
	}
	// Reporter blocks
	else if(opcode == "motion_pointtowards_menu")
		*returnValue = inputs.value("TOWARDS");
	else if(opcode == "motion_goto_menu")
		*returnValue = inputs.value("TO");
	else if(opcode == "motion_glideto_menu")
		*returnValue = inputs.value("TO");
	else if(opcode == "motion_xposition")
		*returnValue = QString::number(spriteX);
	else if(opcode == "motion_yposition")
		*returnValue = QString::number(spriteY);
	else if(opcode == "motion_direction")
		*returnValue = QString::number(direction);
	else
		return false;
	return true;
}

/*! Runs looks blocks. */
bool scratchSprite::looksBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(frameEnd == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
	if(processEnd == nullptr)
	{
		bool tmpProcessEnd;
		processEnd = &tmpProcessEnd;
	}
	if(returnValue == nullptr)
	{
		QString tmpReturnValue;
		returnValue = &tmpReturnValue;
	}
	*processEnd = false;
	if(opcode == "looks_sayforsecs")
	{
		*frameEnd = true;
		showBubble(inputs.value("MESSAGE"));
		if(currentExecPos[processID]["special"].toString() != "wait")
		{
			currentExecPos[processID]["special"] = "wait";
			currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addSecs(inputs.value("SECS").toDouble());
		}
		QDateTime startTime = currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			showBubble("");
			*processEnd = true;
			*frameEnd = false;
		}
	}
	else if(opcode == "looks_say")
		showBubble(inputs.value("MESSAGE"));
	else if(opcode == "looks_thinkforsecs")
	{
		*frameEnd = true;
		showBubble(inputs.value("MESSAGE"),true);
		if(currentExecPos[processID]["special"].toString() != "wait")
		{
			currentExecPos[processID]["special"] = "wait";
			currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addSecs(inputs.value("SECS").toDouble());
		}
		QDateTime startTime = currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			showBubble("");
			*processEnd = true;
			*frameEnd = false;
		}
	}
	else if(opcode == "looks_think")
		showBubble(inputs.value("MESSAGE"),true);
	else if(opcode == "looks_show")
		setVisible(true);
	else if(opcode == "looks_hide")
		setVisible(false);
	else if(opcode == "looks_changeeffectby")
	{
		graphicEffects[inputs.value("EFFECT")] += inputs.value("CHANGE").toDouble();
		installGraphicEffects();
	}
	else if(opcode == "looks_seteffectto")
	{
		graphicEffects[inputs.value("EFFECT")] = inputs.value("VALUE").toDouble();
		installGraphicEffects();
	}
	else if(opcode == "looks_cleargraphiceffects")
		resetGraphicEffects();
	else if(opcode == "looks_changesizeby")
		setSize(size + inputs.value("CHANGE").toDouble());
	else if(opcode == "looks_setsizeto")
		setSize(inputs.value("SIZE").toDouble());
	else if(opcode == "looks_switchcostumeto")
	{
		int newCostume = currentCostume;
		for(int i=0; i < costumes.count(); i++)
		{
			if((costumes[i].contains("name")) && (costumes[i].value("name").toString() == inputs.value("COSTUME")))
				newCostume = i;
		}
		setCostume(newCostume);
	}
	else if(opcode == "looks_nextcostume")
	{
		int newCostume = currentCostume + 1;
		if(newCostume >= costumes.count())
			newCostume = 0;
		setCostume(newCostume);
	}
	else if((opcode == "looks_switchbackdropto") || (opcode == "looks_switchbackdroptoandwait"))
	{
		scratchSprite *stagePtr = getSprite("Stage");
		QList<QVariantMap> *backdrops = &stagePtr->costumes;
		int newCostume = stagePtr->currentCostume;
		bool backdropFound = false;
		for(int i=0; i < backdrops->count(); i++)
		{
			if(backdrops->value(i).value("name").toString() == inputs.value("BACKDROP"))
			{
				newCostume = i;
				backdropFound = true;
				break;
			}
		}
		if(!backdropFound)
		{
			if(inputs.value("BACKDROP") == "next backdrop")
			{
				newCostume = stagePtr->currentCostume + 1;
				if(newCostume >= stagePtr->costumes.count())
					newCostume = 0;
			}
			else if(inputs.value("BACKDROP") == "previous backdrop")
			{
				newCostume = stagePtr->currentCostume - 1;
				if(newCostume < 0)
					newCostume = stagePtr->costumes.count() - 1;
			}
			else if(inputs.value("BACKDROP") == "random backdrop")
			{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
				newCostume = QRandomGenerator::global()->bounded(0,stagePtr->costumes.count());
#else
				newCostume = qrand() % stagePtr->costumes.count();
#endif
			}
		}
		if(opcode == "looks_switchbackdroptoandwait")
		{
			*frameEnd = true;
			if(currentExecPos[processID]["special"].toString() != "waituntilend")
			{
				currentExecPos[processID]["special"] = "waituntilend";
				currentExecPos[processID]["activescripts"] = 0;
				stagePtr->setCostume(newCostume,&currentExecPos[processID]);
			}
			else
			{
				if(currentExecPos[processID]["activescripts"].toInt() == 0)
				{
					*processEnd = true;
					*frameEnd = false;
				}
			}
		}
		else
			stagePtr->setCostume(newCostume);
	}
	else if(opcode == "looks_nextbackdrop")
	{
		scratchSprite *stagePtr = getSprite("Stage");
		int newCostume = stagePtr->currentCostume + 1;
		if(newCostume >= stagePtr->costumes.count())
			newCostume = 0;
		stagePtr->setCostume(newCostume);
	}
	else if(opcode == "looks_gotofrontback")
	{
		if(inputs.value("FRONT_BACK") == "front")
		{
			int maxLayer = 0;
			for(int i=0; i < spriteList.count(); i++)
			{
				if(spriteList[i]->zValue() > maxLayer)
					maxLayer = spriteList[i]->zValue();
			}
			setZValue(maxLayer+1);
		}
		else
		{
			for(int i=0; i < spriteList.count(); i++)
				spriteList[i]->setZValue(spriteList[i]->zValue() + 1);
			setZValue(1);
		}
	}
	else if(opcode == "looks_goforwardbackwardlayers")
	{
		int delta = inputs.value("NUM").toInt();
		if(inputs.value("FORWARD_BACKWARD") == "backward")
			delta *= -1;
		if(delta < 0)
		{
			for(int i=0; i < spriteList.count(); i++)
				spriteList[i]->setZValue(spriteList[i]->zValue() + 1);
		}
		setZValue(zValue() + delta);
		if(zValue() < 1)
			setZValue(1);
	}
	// Reporter blocks
	else if(opcode == "looks_size")
		*returnValue = QString::number(size);
	else if(opcode == "looks_costume")
		*returnValue = inputs.value("COSTUME");
	else if(opcode == "looks_backdrops")
		*returnValue = inputs.value("BACKDROP");
	else if(opcode == "looks_backdropnumbername")
	{
		scratchSprite *stagePtr = getSprite("Stage");
		if(inputs.value("NUMBER_NAME") == "number")
			*returnValue = QString::number(stagePtr->currentCostume);
		else
			*returnValue = stagePtr->costumes[stagePtr->currentCostume].value("name").toString();
	}
	else if(opcode == "looks_costumenumbername")
	{
		if(inputs.value("NUMBER_NAME") == "number")
			*returnValue = QString::number(currentCostume);
		else
			*returnValue = costumes[currentCostume].value("name").toString();
	}
	else
		return false;
	return true;
}

/*! Runs sound blocks. */
bool scratchSprite::soundBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(frameEnd == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
	if(processEnd == nullptr)
	{
		bool tmpProcessEnd;
		processEnd = &tmpProcessEnd;
	}
	if(returnValue == nullptr)
	{
		QString tmpReturnValue;
		returnValue = &tmpReturnValue;
	}
	*processEnd = false;
	if(opcode == "sound_play")
		playSound(inputs.value("SOUND_MENU"));
	else if(opcode == "sound_playuntildone")
	{
		*frameEnd = true;
		if(currentExecPos[processID]["special"].toString() != "soundwait")
		{
			currentExecPos[processID]["special"] = "soundwait";
			currentExecPos[processID]["sound"] = (qlonglong) (intptr_t) playSound(inputs.value("SOUND_MENU"));
		}
		else
		{
			QSoundEffect *sound = (QSoundEffect*) currentExecPos[processID]["sound"].toLongLong();
			if(!sound->isPlaying())
			{
				*processEnd = true;
				*frameEnd = false;
				sound->deleteLater();
			}
		}
	}
	else if(opcode == "sound_stopallsounds")
		stopAllSounds();
	else if(opcode == "sound_seteffectto");
		// TODO: Add sound effects (see QAudioDecoder)
	else if(opcode == "sound_changeeffectby");
	else if(opcode == "sound_cleareffects");
	else if(opcode == "sound_changevolumeby")
		setVolume(volume + inputs.value("VOLUME").toDouble());
	else if(opcode == "sound_setvolumeto")
		setVolume(inputs.value("VOLUME").toDouble());
	// Reporter blocks
	else if(opcode == "sound_sounds_menu")
		*returnValue = inputs.value("SOUND_MENU");
	else if(opcode == "sound_volume")
		*returnValue = QString::number(volume);
	else
		return false;
	return true;
}

/*! Runs event blocks. */
bool scratchSprite::eventBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(frameEnd == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
	if(processEnd == nullptr)
	{
		bool tmpProcessEnd;
		processEnd = &tmpProcessEnd;
	}
	if(returnValue == nullptr)
	{
		QString tmpReturnValue;
		returnValue = &tmpReturnValue;
	}
	*processEnd = false;
	if(opcode == "event_broadcast")
		emit broadcast(inputs.value("BROADCAST_INPUT"));
	else if(opcode == "event_broadcastandwait")
	{
		*frameEnd = true;
		if(currentExecPos[processID]["special"].toString() != "waituntilend")
		{
			currentExecPos[processID]["special"] = "waituntilend";
			currentExecPos[processID]["activescripts"] = 0;
			emit broadcast(inputs.value("BROADCAST_INPUT"),&currentExecPos[processID]);
		}
		else
		{
			if(currentExecPos[processID]["activescripts"].toInt() == 0)
			{
				*processEnd = true;
				*frameEnd = false;
			}
		}
	}
	// Reporter blocks
	else if(opcode == "event_broadcast_menu")
		*returnValue = inputs.value("BROADCAST_OPTION");
	else if(opcode != "event_whenflagclicked")
		return false;
	return true;
}

/*! Runs control blocks. */
bool scratchSprite::controlBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(frameEnd == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
	if(processEnd == nullptr)
	{
		bool tmpProcessEnd;
		processEnd = &tmpProcessEnd;
	}
	if(returnValue == nullptr)
	{
		QString tmpReturnValue;
		returnValue = &tmpReturnValue;
	}
	*processEnd = false;
	if((opcode == "control_forever") || (opcode == "control_repeat"))
	{
		if(currentExecPos[processID]["special"].toString() == "loop")
		{
			QVariantMap *loopStack = (QVariantMap*) currentExecPos[processID]["loop_reference"].toLongLong();
			if(loopStack->value("loop_finished").toBool() == true)
			{
				currentExecPos[processID]["special"] = "";
				*processEnd = true;
			}
			else
				*frameEnd = true;
		}
		else
		{
			if((opcode == "control_forever") || ((opcode == "control_repeat") && (inputs.value("TIMES").toInt() > 0)))
			{
				*frameEnd = true;
				newStack = new QVariantMap;
				newStack->clear();
				newStack->insert("id",inputs.value("SUBSTACK"));
				newStack->insert("toplevelblock",currentExecPos[processID]["toplevelblock"]);
				newStack->insert("special","");
				newStack->insert("loop_start",inputs.value("SUBSTACK"));
				newStack->insert("loop_finished",false);
				newStack->insert("loop_ptr", (qlonglong) (intptr_t) newStack);
				currentExecPos[processID]["special"] = "loop";
				currentExecPos[processID]["loop_reference"] = (qlonglong) (intptr_t) newStack;
				stackPointers.append(newStack);
				if(opcode == "control_forever")
					newStack->insert("loop_type","forever");
				else if(opcode == "control_repeat")
				{
					newStack->insert("loop_type","repeat");
					newStack->insert("loop_count",inputs.value("TIMES").toInt());
					newStack->insert("loop_current",0);
				}
				// Avoid screen refresh after starting the loop
				__run_frame_again = true;
			}
		}
	}
	else if((opcode == "control_if") || (opcode == "control_if_else"))
	{
		if(currentExecPos[processID]["special"].toString() == "loop")
		{
			currentExecPos[processID]["special"] = "";
			*processEnd = true;
		}
		else
		{
			bool isIfElse = (opcode == "control_if_else");
			bool condition = (inputs.value("CONDITION") == "true");
			if((condition && (inputs.value("SUBSTACK") != "")) || (isIfElse && !condition && (inputs.value("SUBSTACK2") != "")))
			{
				// Using a repeat(1) loop if the condition is true
				*frameEnd = true;
				newStack = new QVariantMap;
				newStack->clear();
				if(condition)
				{
					newStack->insert("id", inputs.value("SUBSTACK"));
					newStack->insert("loop_start", inputs.value("SUBSTACK"));
				}
				else
				{
					newStack->insert("id", inputs.value("SUBSTACK2"));
					newStack->insert("loop_start", inputs.value("SUBSTACK2"));
				}
				newStack->insert("toplevelblock", currentExecPos[processID]["toplevelblock"]);
				newStack->insert("special", "");
				newStack->insert("loop_finished", false);
				newStack->insert("loop_ptr", (qlonglong) (intptr_t) newStack);
				currentExecPos[processID]["special"] = "loop";
				currentExecPos[processID]["loop_reference"] = (qlonglong) (intptr_t) newStack;
				stackPointers.append(newStack);
				newStack->insert("loop_type", "repeat");
				newStack->insert("loop_count", 1);
				newStack->insert("loop_current", 0);
				// Avoid screen refresh after creating the substack
				__run_frame_again = true;
			}
			else
				*processEnd = true;
		}
	}
	else if(opcode == "control_stop")
	{
		if(inputs.value("STOP_OPTION") == "all")
		{
			for(int i=0; i < spriteList.count(); i++)
				spriteList[i]->stopSprite();
		}
		else if(inputs.value("STOP_OPTION") == "this script")
			currentExecPos[processID]["special"] = "remove_operation";
		else if(inputs.value("STOP_OPTION") == "other scripts in sprite")
		{
			QList<QVariantMap> operationsToRemove;
			operationsToRemove.clear();
			for(int i=0; i < currentExecPos.count(); i++)
			{
				// TODO: This may not work if there are e.g multiple instances of the same custom block running
				if(currentExecPos[i]["toplevelblock"].toString() != currentExecPos[processID]["toplevelblock"].toString())
					operationsToRemove += currentExecPos[i];
			}
			for(int i=0; i < operationsToRemove.count(); i++)
				currentExecPos.removeAll(operationsToRemove[i]);
		}
	}
	else
		return false;
	return true;
}
