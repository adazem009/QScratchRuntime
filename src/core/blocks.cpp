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

#include "core/blocks.h"

/*! Runs motion blocks. */
bool scratchSprite::motionBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd)
{
	if(frameEnd == nullptr)
		frameEnd = new bool;
	if(processEnd == nullptr)
		processEnd = new bool;
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
		setXPos(inputs.value("X").toDouble());
		setYPos(inputs.value("Y").toDouble());
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
				setXPos(QRandomGenerator::global()->bounded(-240,240));
				setYPos(QRandomGenerator::global()->bounded(-180,180));
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
					endX = QRandomGenerator::global()->bounded(-240,240);
					endY = QRandomGenerator::global()->bounded(-180,180);
				}
			}
			else
			{
				endX = targetSprite->spriteX;
				endY = targetSprite->spriteY;
			}
		}
		if(currentExecPos[processID]["special"].toString() != "glide")
		{
			currentExecPos[processID]["special"] = "glide";
			currentExecPos[processID]["startX"] = spriteX;
			currentExecPos[processID]["startY"] = spriteY;
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
	else
		return false;
	return true;
}

/*! Runs looks blocks. */
bool scratchSprite::looksBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd, bool *processEnd)
{
	if(frameEnd == nullptr)
		frameEnd = new bool;
	if(processEnd == nullptr)
		processEnd = new bool;
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
	else
		return false;
	return true;
}
