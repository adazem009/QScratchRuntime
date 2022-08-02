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

#include "core/blocks.h"

/*! Runs a block. */
bool Blocks::runBlock(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	if(newStackPtr == nullptr)
	{
		bool tmpFrameEnd;
		frameEnd = &tmpFrameEnd;
	}
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
	if(opcode.startsWith("motion"))
		return motionBlocks(sprite, opcode, inputs, processID, newStackPtr, frameEnd, processEnd, returnValue);
	else if(opcode.startsWith("looks"))
		return looksBlocks(sprite, opcode, inputs, processID, newStackPtr, frameEnd, processEnd, returnValue);
	else if(opcode.startsWith("sound"))
		return soundBlocks(sprite, opcode, inputs, processID, newStackPtr, frameEnd, processEnd, returnValue);
	else if(opcode.startsWith("event"))
		return eventBlocks(sprite, opcode, inputs, processID, newStackPtr, frameEnd, processEnd, returnValue);
	else if(opcode.startsWith("control"))
		return controlBlocks(sprite, opcode, inputs, processID, newStackPtr, frameEnd, processEnd, returnValue);
	else
		return false;
}

/*! Runs motion blocks. */
bool Blocks::motionBlocks(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	Q_UNUSED(newStackPtr);
	Q_UNUSED(returnValue);
	if(opcode == "motion_movesteps")
	{
		qreal steps = inputs.value("STEPS").toDouble();
		// https://en.scratch-wiki.info/wiki/Move_()_Steps_(block)#Workaround
		sprite->setXPos(sprite->spriteX + qSin(qDegreesToRadians(sprite->direction))*steps);
		sprite->setYPos(sprite->spriteY + qCos(qDegreesToRadians(sprite->direction))*steps);
	}
	else if(opcode == "motion_turnright")
		sprite->setDirection(sprite->direction + inputs.value("DEGREES").toDouble());
	else if(opcode == "motion_turnleft")
		sprite->setDirection(sprite->direction - inputs.value("DEGREES").toDouble());
	else if(opcode == "motion_pointindirection")
		sprite->setDirection(inputs.value("DIRECTION").toDouble());
	else if(opcode == "motion_pointtowards")
	{
		QString targetName = inputs.value("TOWARDS");
		scratchSprite *targetSprite = sprite->getSprite(targetName);
		qreal deltaX = 0, deltaY = 0;
		if(targetSprite == nullptr)
		{
			deltaX = sprite->mouseX - sprite->spriteX;
			deltaY = sprite->mouseY - sprite->spriteY;
		}
		else
		{
			deltaX = targetSprite->spriteX - sprite->spriteX;
			deltaY = targetSprite->spriteY - sprite->spriteY;
		}
		// https://en.scratch-wiki.info/wiki/Point_Towards_()_(block)#Workaround
		if(deltaY == 0)
		{
			if(deltaX < 0)
				sprite->setDirection(-90);
			else
				sprite->setDirection(90);
		}
		else
		{
			qreal atanResult = qRadiansToDegrees(qAtan(deltaX/deltaY));
			if(deltaY < 0)
				sprite->setDirection(180 + atanResult);
			else
				sprite->setDirection(atanResult);
		}
	}
	else if(opcode == "motion_gotoxy")
	{
		if(inputs.contains("X"))
		{
				sprite->setXPos(inputs.value("X").toDouble());
				sprite->setYPos(inputs.value("Y").toDouble());
		}
	}
	else if(opcode == "motion_goto")
	{
		QString targetName = inputs.value("TO");
		scratchSprite *targetSprite = sprite->getSprite(targetName);
		if(targetSprite == nullptr)
		{
			if(targetName == "_mouse_")
			{
				sprite->setXPos(sprite->mouseX);
				sprite->setYPos(sprite->mouseY);
			}
			else if(targetName == "_random_")
			{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
				sprite->setXPos(QRandomGenerator::global()->bounded(-240,241));
				sprite->setYPos(QRandomGenerator::global()->bounded(-180,181));
#else
				sprite->setXPos(qrand()%481 - 240);
				sprite->setYPos(qrand()%361 - 180);
#endif
			}
		}
		else
		{
			sprite->setXPos(targetSprite->spriteX);
			sprite->setYPos(targetSprite->spriteY);
		}
	}
	else if((opcode == "motion_glidesecstoxy") || (opcode == "motion_glideto"))
	{
		*frameEnd = true;
		qreal endX = 0, endY = 0;
		if(sprite->currentExecPos[processID]["special"].toString() == "glide")
		{
			endX = sprite->currentExecPos[processID]["endX"].toDouble();
			endY = sprite->currentExecPos[processID]["endY"].toDouble();
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
				scratchSprite *targetSprite = sprite->getSprite(targetName);
				if(targetSprite == nullptr)
				{
					if(targetName == "_mouse_")
					{
						endX = sprite->mouseX;
						endY = sprite->mouseY;
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
			sprite->currentExecPos[processID]["special"] = "glide";
			sprite->currentExecPos[processID]["startX"] = sprite->spriteX;
			sprite->currentExecPos[processID]["startY"] = sprite->spriteY;
			sprite->currentExecPos[processID]["endX"] = endX;
			sprite->currentExecPos[processID]["endY"] = endY;
			sprite->currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			sprite->currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addMSecs(inputs.value("SECS").toDouble() * 1000);
		}
		qreal startX = sprite->currentExecPos[processID]["startX"].toDouble();
		qreal startY = sprite->currentExecPos[processID]["startY"].toDouble();
		QDateTime startTime = sprite->currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = sprite->currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			sprite->setXPos(endX);
			sprite->setYPos(endY);
			*processEnd = true;
			*frameEnd = false;
		}
		else
		{
			sprite->setXPos(startX + (endX-startX)*progress);
			sprite->setYPos(startY + (endY-startY)*progress);
		}
	}
	else if(opcode == "motion_changexby")
		sprite->setXPos(sprite->spriteX + inputs.value("DX").toDouble());
	else if(opcode == "motion_setx")
		sprite->setXPos(inputs.value("X").toDouble());
	else if(opcode == "motion_changeyby")
		sprite->setYPos(sprite->spriteY + inputs.value("DY").toDouble());
	else if(opcode == "motion_sety")
		sprite->setYPos(inputs.value("Y").toDouble());
	else if(opcode == "motion_ifonedgebounce")
	{
		QRectF spriteRect = sprite->boundingRect();
		// Right edge
		if(sprite->spriteX + (spriteRect.width()/2) > 240)
		{
			sprite->setDirection(-sprite->direction);
			sprite->setXPos(240 - (spriteRect.width()/2));
		}
		// Left edge
		if(sprite->spriteX - (spriteRect.width()/2) < -240)
		{
			sprite->setDirection(-sprite->direction);
			sprite->setXPos(-240 + (spriteRect.width()/2));
		}
		// Top edge
		if(sprite->spriteY + (spriteRect.height()/2) > 180)
		{
			sprite->setDirection(180-sprite->direction);
			sprite->setYPos(180 - (spriteRect.height()/2));
		}
		// Bottom edge
		if(sprite->spriteY - (spriteRect.height()/2) < -180)
		{
			sprite->setDirection(180-sprite->direction);
			sprite->setYPos(-180 + (spriteRect.height()/2));
		}
	}
	else if(opcode == "motion_setrotationstyle")
	{
		sprite->rotationStyle = inputs.value("STYLE");
		sprite->setDirection(sprite->direction);
	}
	// Reporter blocks
	else if(opcode == "motion_pointtowards_menu")
		*returnValue = inputs.value("TOWARDS");
	else if(opcode == "motion_goto_menu")
		*returnValue = inputs.value("TO");
	else if(opcode == "motion_glideto_menu")
		*returnValue = inputs.value("TO");
	else if(opcode == "motion_xposition")
		*returnValue = QString::number(sprite->spriteX);
	else if(opcode == "motion_yposition")
		*returnValue = QString::number(sprite->spriteY);
	else if(opcode == "motion_direction")
		*returnValue = QString::number(sprite->direction);
	else
		return false;
	return true;
}

/*! Runs looks blocks. */
bool Blocks::looksBlocks(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	Q_UNUSED(newStackPtr);
	Q_UNUSED(returnValue);
	if(opcode == "looks_sayforsecs")
	{
		*frameEnd = true;
		sprite->showBubble(inputs.value("MESSAGE"));
		if(sprite->currentExecPos[processID]["special"].toString() != "wait")
		{
			sprite->currentExecPos[processID]["special"] = "wait";
			sprite->currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			sprite->currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addMSecs(inputs.value("SECS").toDouble() * 1000);
		}
		QDateTime startTime = sprite->currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = sprite->currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			sprite->showBubble("");
			*processEnd = true;
			*frameEnd = false;
		}
	}
	else if(opcode == "looks_say")
		sprite->showBubble(inputs.value("MESSAGE"));
	else if(opcode == "looks_thinkforsecs")
	{
		*frameEnd = true;
		sprite->showBubble(inputs.value("MESSAGE"),true);
		if(sprite->currentExecPos[processID]["special"].toString() != "wait")
		{
			sprite->currentExecPos[processID]["special"] = "wait";
			sprite->currentExecPos[processID]["startTime"] = QDateTime::currentDateTimeUtc();
			sprite->currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addMSecs(inputs.value("SECS").toDouble() * 1000);
		}
		QDateTime startTime = sprite->currentExecPos[processID]["startTime"].toDateTime();
		QDateTime endTime = sprite->currentExecPos[processID]["endTime"].toDateTime();
		QDateTime currentTime = QDateTime::currentDateTimeUtc();
		qreal progress = (startTime.msecsTo(endTime) - currentTime.msecsTo(endTime)) / (inputs.value("SECS").toDouble() * 1000.0);
		if(progress >= 1)
		{
			sprite->showBubble("");
			*processEnd = true;
			*frameEnd = false;
		}
	}
	else if(opcode == "looks_think")
		sprite->showBubble(inputs.value("MESSAGE"),true);
	else if(opcode == "looks_show")
		sprite->setVisible(true);
	else if(opcode == "looks_hide")
		sprite->setVisible(false);
	else if(opcode == "looks_changeeffectby")
	{
		sprite->graphicEffects[inputs.value("EFFECT")] += inputs.value("CHANGE").toDouble();
		sprite->installGraphicEffects();
	}
	else if(opcode == "looks_seteffectto")
	{
		sprite->graphicEffects[inputs.value("EFFECT")] = inputs.value("VALUE").toDouble();
		sprite->installGraphicEffects();
	}
	else if(opcode == "looks_cleargraphiceffects")
		sprite->resetGraphicEffects();
	else if(opcode == "looks_changesizeby")
		sprite->setSize(sprite->size + inputs.value("CHANGE").toDouble());
	else if(opcode == "looks_setsizeto")
		sprite->setSize(inputs.value("SIZE").toDouble());
	else if(opcode == "looks_switchcostumeto")
	{
		int newCostume = sprite->currentCostume;
		for(int i=0; i < sprite->costumes.count(); i++)
		{
			if((sprite->costumes[i].contains("name")) && (sprite->costumes[i].value("name").toString() == inputs.value("COSTUME")))
				newCostume = i;
		}
		sprite->setCostume(newCostume);
	}
	else if(opcode == "looks_nextcostume")
	{
		int newCostume = sprite->currentCostume + 1;
		if(newCostume >= sprite->costumes.count())
			newCostume = 0;
		sprite->setCostume(newCostume);
	}
	else if((opcode == "looks_switchbackdropto") || (opcode == "looks_switchbackdroptoandwait"))
	{
		scratchSprite *stagePtr = sprite->getSprite("Stage");
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
			if(sprite->currentExecPos[processID]["special"].toString() != "waituntilend")
			{
				sprite->currentExecPos[processID]["special"] = "waituntilend";
				sprite->currentExecPos[processID]["activescripts"] = 0;
				stagePtr->setCostume(newCostume,&sprite->currentExecPos[processID]);
			}
			else
			{
				if(sprite->currentExecPos[processID]["activescripts"].toInt() == 0)
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
		scratchSprite *stagePtr = sprite->getSprite("Stage");
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
			for(int i=0; i < sprite->spriteList.count(); i++)
			{
				if(sprite->spriteList[i]->zValue() > maxLayer)
					maxLayer = sprite->spriteList[i]->zValue();
			}
			sprite->setZValue(maxLayer+1);
		}
		else
		{
			for(int i=0; i < sprite->spriteList.count(); i++)
				sprite->spriteList[i]->setZValue(sprite->spriteList[i]->zValue() + 1);
			sprite->setZValue(1);
		}
	}
	else if(opcode == "looks_goforwardbackwardlayers")
	{
		int delta = inputs.value("NUM").toInt();
		if(inputs.value("FORWARD_BACKWARD") == "backward")
			delta *= -1;
		if(delta < 0)
		{
			for(int i=0; i < sprite->spriteList.count(); i++)
				sprite->spriteList[i]->setZValue(sprite->spriteList[i]->zValue() + 1);
		}
		sprite->setZValue(sprite->zValue() + delta);
		if(sprite->zValue() < 1)
			sprite->setZValue(1);
	}
	// Reporter blocks
	else if(opcode == "looks_size")
		*returnValue = QString::number(sprite->size);
	else if(opcode == "looks_costume")
		*returnValue = inputs.value("COSTUME");
	else if(opcode == "looks_backdrops")
		*returnValue = inputs.value("BACKDROP");
	else if(opcode == "looks_backdropnumbername")
	{
		scratchSprite *stagePtr = sprite->getSprite("Stage");
		if(inputs.value("NUMBER_NAME") == "number")
			*returnValue = QString::number(stagePtr->currentCostume);
		else
			*returnValue = stagePtr->costumes[stagePtr->currentCostume].value("name").toString();
	}
	else if(opcode == "looks_costumenumbername")
	{
		if(inputs.value("NUMBER_NAME") == "number")
			*returnValue = QString::number(sprite->currentCostume);
		else
			*returnValue = sprite->costumes[sprite->currentCostume].value("name").toString();
	}
	else
		return false;
	return true;
}

/*! Runs sound blocks. */
bool Blocks::soundBlocks(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	Q_UNUSED(newStackPtr);
	Q_UNUSED(returnValue);
	if(opcode == "sound_play")
		sprite->playSound(inputs.value("SOUND_MENU"));
	else if(opcode == "sound_playuntildone")
	{
		*frameEnd = true;
		if(sprite->currentExecPos[processID]["special"].toString() != "soundwait")
		{
			sprite->currentExecPos[processID]["special"] = "soundwait";
			sprite->currentExecPos[processID]["sound"] = (qlonglong) (intptr_t) sprite->playSound(inputs.value("SOUND_MENU"));
		}
		else
		{
			QPointer<QMediaPlayer> sound = *((QPointer<QMediaPlayer>*) sprite->currentExecPos[processID]["sound"].toLongLong());
			if((sound == nullptr) || (sound->state() == QMediaPlayer::StoppedState))
			{
				*processEnd = true;
				*frameEnd = false;
				sound->deleteLater();
			}
		}
	}
	else if(opcode == "sound_stopallsounds")
		sprite->stopAllSounds();
	else if(opcode == "sound_seteffectto");
		// TODO: Add sound effects (see QAudioDecoder)
	else if(opcode == "sound_changeeffectby");
	else if(opcode == "sound_cleareffects");
	else if(opcode == "sound_changevolumeby")
		sprite->setVolume(sprite->volume + inputs.value("VOLUME").toDouble());
	else if(opcode == "sound_setvolumeto")
		sprite->setVolume(inputs.value("VOLUME").toDouble());
	// Reporter blocks
	else if(opcode == "sound_sounds_menu")
		*returnValue = inputs.value("SOUND_MENU");
	else if(opcode == "sound_volume")
		*returnValue = QString::number(sprite->volume);
	else
		return false;
	return true;
}

/*! Runs event blocks. */
bool Blocks::eventBlocks(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	Q_UNUSED(newStackPtr);
	Q_UNUSED(returnValue);
	if(opcode == "event_broadcast")
		sprite->emitBroadcast(inputs.value("BROADCAST_INPUT"));
	else if(opcode == "event_broadcastandwait")
	{
		*frameEnd = true;
		if(sprite->currentExecPos[processID]["special"].toString() != "waituntilend")
		{
			sprite->currentExecPos[processID]["special"] = "waituntilend";
			sprite->currentExecPos[processID]["activescripts"] = 0;
			sprite->emitBroadcast(inputs.value("BROADCAST_INPUT"),&sprite->currentExecPos[processID]);
		}
		else
		{
			if(sprite->currentExecPos[processID]["activescripts"].toInt() == 0)
			{
				*processEnd = true;
				*frameEnd = false;
			}
		}
	}
	// Reporter blocks
	else if(opcode == "event_broadcast_menu")
		*returnValue = inputs.value("BROADCAST_OPTION");
	else if((opcode != "event_whenflagclicked") &&
		(opcode != "event_whenkeypressed") &&
		(opcode != "event_whenthisspriteclicked") &&
		(opcode != "event_whenstageclicked") &&
		(opcode != "event_whenbackdropswitchesto") &&
		(opcode != "event_whengreaterthan") &&
		(opcode != "event_whenbroadcastreceived"))
		return false;
	return true;
}

/*! Runs control blocks. */
bool Blocks::controlBlocks(scratchSprite *sprite, QString opcode, QMap<QString,QString> inputs, int processID, QVariantMap **newStackPtr, bool *frameEnd, bool *processEnd, QString *returnValue)
{
	Q_UNUSED(newStackPtr);
	Q_UNUSED(returnValue);
	if((opcode == "control_forever") || (opcode == "control_repeat") || (opcode == "control_repeat_until") || (opcode == "control_while"))
	{
		if(sprite->currentExecPos[processID]["special"].toString() == "loop")
		{
			QVariantMap *loopStack = (QVariantMap*) sprite->currentExecPos[processID]["loop_reference"].toLongLong();
			if(loopStack->value("loop_finished").toBool() == true)
			{
				sprite->currentExecPos[processID]["special"] = "";
				*processEnd = true;
				__run_frame_again = true;
			}
			else
				*frameEnd = true;
		}
		else
		{
			if((opcode == "control_forever") ||
				((opcode == "control_repeat") && (inputs.value("TIMES").toInt() > 0)) ||
				((opcode == "control_repeat_until") && (inputs.value("CONDITION") != "true")) ||
				((opcode == "control_while") && (inputs.value("CONDITION") == "true")))
			{
				*frameEnd = true;
				QVariantMap *newStack = new QVariantMap;
				*newStackPtr = newStack;
				newStack->clear();
				newStack->insert("id",inputs.value("SUBSTACK"));
				newStack->insert("toplevelblock",sprite->currentExecPos[processID]["toplevelblock"]);
				newStack->insert("special","");
				newStack->insert("loop_start",inputs.value("SUBSTACK"));
				newStack->insert("loop_finished",false);
				newStack->insert("loop_ptr", (qlonglong) (intptr_t) newStack);
				newStack->insert("loop_block_id", sprite->currentExecPos[processID]["id"]);
				sprite->currentExecPos[processID]["special"] = "loop";
				sprite->currentExecPos[processID]["loop_reference"] = (qlonglong) (intptr_t) newStack;
				sprite->stackPointers.append(newStack);
				if(opcode == "control_forever")
					newStack->insert("loop_type","forever");
				else if(opcode == "control_repeat")
				{
					newStack->insert("loop_type","repeat");
					newStack->insert("loop_count",inputs.value("TIMES").toInt());
					newStack->insert("loop_current",0);
				}
				else if(opcode == "control_repeat_until")
					newStack->insert("loop_type","repeat_until");
				else if(opcode == "control_while")
					newStack->insert("loop_type","while");
				// TODO: Add for each (obsolete) block after variables are added
				// Avoid screen refresh after starting the loop
				__run_frame_again = true;
			}
		}
	}
	else if((opcode == "control_if") || (opcode == "control_if_else"))
	{
		if(sprite->currentExecPos[processID]["special"].toString() == "loop")
		{
			QVariantMap *loopStack = (QVariantMap*) sprite->currentExecPos[processID]["loop_reference"].toLongLong();
			if(loopStack->value("loop_finished").toBool() == true)
			{
				sprite->currentExecPos[processID]["special"] = "";
				*processEnd = true;
				__run_frame_again = true;
			}
			else
				*frameEnd = true;
		}
		else
		{
			bool isIfElse = (opcode == "control_if_else");
			bool condition = (inputs.value("CONDITION") == "true");
			if((condition && (inputs.value("SUBSTACK") != "")) || (isIfElse && !condition && (inputs.value("SUBSTACK2") != "")))
			{
				// Using a repeat(1) loop if the condition is true
				*frameEnd = true;
				QVariantMap *newStack = new QVariantMap;
				*newStackPtr = newStack;
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
				newStack->insert("toplevelblock", sprite->currentExecPos[processID]["toplevelblock"]);
				newStack->insert("special", "");
				newStack->insert("loop_finished", false);
				newStack->insert("loop_ptr", (qlonglong) (intptr_t) newStack);
				sprite->currentExecPos[processID]["special"] = "loop";
				sprite->currentExecPos[processID]["loop_reference"] = (qlonglong) (intptr_t) newStack;
				sprite->stackPointers.append(newStack);
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
			for(int i=0; i < sprite->spriteList.count(); i++)
				sprite->spriteList[i]->stopSprite();
		}
		else if(inputs.value("STOP_OPTION") == "this script")
			sprite->currentExecPos[processID]["special"] = "remove_operation";
		else if(inputs.value("STOP_OPTION") == "other scripts in sprite")
		{
			QList<QVariantMap> operationsToRemove;
			operationsToRemove.clear();
			for(int i=0; i < sprite->currentExecPos.count(); i++)
			{
				// TODO: This may not work if there are e.g multiple instances of the same custom block running
				if(sprite->currentExecPos[i]["toplevelblock"].toString() != sprite->currentExecPos[processID]["toplevelblock"].toString())
					operationsToRemove += sprite->currentExecPos[i];
			}
			for(int i=0; i < operationsToRemove.count(); i++)
				sprite->currentExecPos.removeAll(operationsToRemove[i]);
		}
	}
	else if(opcode == "control_wait")
	{
		if(sprite->currentExecPos[processID]["special"].toString() == "wait_secs")
		{
			QDateTime currentTime = QDateTime::currentDateTimeUtc();
			if(currentTime >= sprite->currentExecPos[processID]["endTime"].toDateTime())
			{
				sprite->currentExecPos[processID]["special"] = "";
				*processEnd = true;
			}
			else
				*frameEnd = true;
		}
		else
		{
			*frameEnd = true;
			sprite->currentExecPos[processID]["special"] = "wait_secs";
			sprite->currentExecPos[processID]["endTime"] = QDateTime::currentDateTimeUtc().addMSecs(inputs.value("DURATION").toDouble() * 1000);
			__run_frame_again = true;
		}
	}
	else if(opcode == "control_wait_until")
	{
		if(sprite->currentExecPos[processID]["special"].toString() == "wait_until")
		{
			if(inputs.value("CONDITION") == "true")
			{
				sprite->currentExecPos[processID]["special"] = "";
				*processEnd = true;
			}
			else
				*frameEnd = true;
		}
		else
		{
			*frameEnd = true;
			sprite->currentExecPos[processID]["special"] = "wait_until";
			__run_frame_again = true;
		}
	}
	else
		return false;
	return true;
}
