/*
 * engine.cpp
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

#include "core/engine.h"
#include "core/scratchsprite.h"

/*! Constructs Engine. */
Engine::Engine(scratchSprite *sprite, QObject *parent) :
	QObject(parent),
	m_sprite(sprite) { }

/*! Runs blocks that can be run without screen refresh.*/
void Engine::frame(void)
{
	QStringList frameEventBlocks = m_sprite->frameEvents.keys();
	for(int i=0; i < frameEventBlocks.count(); i++)
	{
		QVariantMap block = m_sprite->frameEvents.value(frameEventBlocks[i]);
		QString opcode = block.value("opcode").toString();
		QMap<QString,QString> inputs = getInputs(block);
		if(opcode == "event_whengreaterthan")
		{
			if(inputs.value("WHENGREATERTHANMENU") == "LOUDNESS"); // TODO: Implement audio input loudness
			else if(inputs.value("WHENGREATERTHANMENU") == "TIMER")
				spriteTimerEvent();
		}
	}
	QList<QVariantMap> operationsToRemove;
	operationsToRemove.clear();
	newStack = nullptr;
	QList<QVariantMap*> newStacks;
	newStacks.clear();
	for(int frame_i=0; frame_i < m_sprite->currentExecPos.count(); frame_i++)
	{
		QString next = m_sprite->currentExecPos[frame_i]["id"].toString();
		bool frameEnd = false;
		while(!frameEnd)
		{
			// Load current block
			QString currentID = next;
			QVariantMap block = m_sprite->blocks.value(currentID);
			m_sprite->currentExecPos[frame_i]["id"] = currentID;
			if(block.value("topLevel").toBool())
			{
				QVariantMap posMap = m_sprite->currentExecPos[frame_i];
				posMap.insert("toplevelblock",currentID);
				m_sprite->currentExecPos[frame_i] = posMap;
			}
			QString opcode = block.value("opcode").toString();
			QMap<QString,QString> inputs = getInputs(block);
			bool processEnd = false;
			newStack = nullptr;
			// Run current block
			int previousLength = m_sprite->currentExecPos.count();
			if(!Blocks::runBlock(m_sprite, opcode, inputs, frame_i, &newStack, &frameEnd, &processEnd))
				qWarning() << "Warning: unsupported block:" << opcode;
			if(m_sprite->currentExecPos.count() != previousLength)
				goto end;
			if(m_sprite->currentExecPos[frame_i]["special"].toString() == "remove_operation")
			{
				operationsToRemove += m_sprite->currentExecPos[frame_i];
				frameEnd = true;
			}
			// Get next block
			QVariant nextValue = block.value("next");
			if(newStack != nullptr)
				newStacks += newStack;
			if(frameEnd)
				m_sprite->currentExecPos[frame_i]["id"] = currentID;
			else if(nextValue.isNull())
			{
				if(m_sprite->currentExecPos[frame_i].contains("loop_type"))
				{
					bool goBack = true;
					if(m_sprite->currentExecPos[frame_i]["special"].toString() == "loop")
					{
						QVariantMap *loopStack = (QVariantMap*) m_sprite->currentExecPos[frame_i]["loop_reference"].toLongLong();
						goBack = loopStack->value("loop_finished").toBool();
					}
					QString loopType = m_sprite->currentExecPos[frame_i]["loop_type"].toString();
					if(loopType == "repeat")
					{
						int loopCurrent = m_sprite->currentExecPos[frame_i]["loop_current"].toInt() + 1;
						m_sprite->currentExecPos[frame_i]["loop_current"] = loopCurrent;
						if(loopCurrent >= m_sprite->currentExecPos[frame_i]["loop_count"].toInt())
						{
							QVariantMap *loopStack = (QVariantMap*) m_sprite->currentExecPos[frame_i]["loop_ptr"].toLongLong();
							loopStack->insert("loop_finished",true);
							m_sprite->currentExecPos[frame_i]["loop_finished"] = true;
							QVariantMap posMap = m_sprite->currentExecPos[frame_i];
							posMap.remove("loop_type");
							m_sprite->currentExecPos[frame_i] = posMap;
							goBack = false;
						}
					}
					else if((loopType == "repeat_until") || (loopType == "while"))
					{
						QVariantMap *loopStack = (QVariantMap*) m_sprite->currentExecPos[frame_i]["loop_ptr"].toLongLong();
						auto loopInputs = getInputs(m_sprite->blocks[loopStack->value("loop_block_id").toString()]);
						bool cond;
						if(loopType == "repeat_until")
							cond = (loopInputs.value("CONDITION") == "true");
						else
							cond = (loopInputs.value("CONDITION") != "true");
						if(cond)
						{
							loopStack->insert("loop_finished",true);
							m_sprite->currentExecPos[frame_i]["loop_finished"] = true;
							QVariantMap posMap = m_sprite->currentExecPos[frame_i];
							posMap.remove("loop_type");
							m_sprite->currentExecPos[frame_i] = posMap;
							goBack = false;
						}
					}
					if(goBack)
					{
						next = m_sprite->currentExecPos[frame_i]["loop_start"].toString();
						m_sprite->currentExecPos[frame_i]["id"] = next;
						m_sprite->currentExecPos[frame_i]["special"] = "";
					}
					else
						operationsToRemove += m_sprite->currentExecPos[frame_i];
				}
				else
				{
					m_sprite->currentExecPos[frame_i]["id"] = currentID;
					QVariantMap *callerScript = (QVariantMap*) m_sprite->currentExecPos[frame_i]["callerptr"].toLongLong();
					if(callerScript != nullptr)
						callerScript->insert("activescripts",callerScript->value("activescripts").toInt()-1);
					operationsToRemove += m_sprite->currentExecPos[frame_i];
				}
				frameEnd = true;
			}
			else
			{
				next = nextValue.toString();
				if(processEnd)
				{
					m_sprite->currentExecPos[frame_i]["id"] = next;
					m_sprite->currentExecPos[frame_i]["special"] = "";
				}
			}
		}
	}
	end:
		for(int i=0; i < newStacks.count(); i++)
			m_sprite->currentExecPos += *newStacks[i];
		for(int i=0; i < operationsToRemove.count(); i++)
			m_sprite->currentExecPos.removeAll(operationsToRemove[i]);
}

/*! Reads block inputs and fields and returns a map. */
QMap<QString,QString> Engine::getInputs(QVariantMap block, bool readFields)
{
	QJsonObject blockInputs;
	if(readFields)
		blockInputs = block.value("fields").toJsonObject();
	else
		blockInputs = block.value("inputs").toJsonObject();
	QStringList blockInputsList = blockInputs.keys();
	QMap<QString,QString> out;
	out.clear();
	for(int i=0; i < blockInputsList.count(); i++)
	{
		QJsonValue inputValue = blockInputs.value(blockInputsList[i]).toArray().at(1);
		bool typeConverted = false;
		QJsonValue finalRawValue;
		QString finalValue = "";
		if(blockInputsList[i].contains("SUBSTACK"))
		{
			// Start of a blocks stack
			typeConverted = true;
			finalValue = blockInputs.value(blockInputsList[i]).toArray().at(1).toString();
		}
		else if(readFields)
		{
			// Input is in the first item
			finalRawValue = blockInputs.value(blockInputsList[i]).toArray().at(0);
		}
		else if(inputValue.isArray())
		{
			// Input representation as an array
			finalRawValue = inputValue.toArray().at(1);
		}
		else
		{
			typeConverted = true;
			if(!inputValue.isNull())
			{
				// Load reporter block
				// Note: Dropdown menus and color inputs are treated as reporter blocks
				QVariantMap reporterBlock = m_sprite->blocks.value(inputValue.toString());
				QString opcode = reporterBlock.value("opcode").toString();
				QMap<QString,QString> inputs = getInputs(reporterBlock);
				// Get reporter block value
				if(!Blocks::runBlock(m_sprite, opcode, inputs, 0, nullptr, nullptr, nullptr, &finalValue))
					qWarning() << "Warning: unsupported reporter block:" << opcode;
			}
		}
		if(!typeConverted)
		{
			if(finalRawValue.isString())
				finalValue = finalRawValue.toString();
			else if(finalRawValue.isDouble())
				finalValue = QString::number(finalRawValue.toDouble());
			else if(finalRawValue.isBool())
				finalValue = finalRawValue.toString();
		}
		out.insert(blockInputsList[i],finalValue);
	}
	if(!readFields)
	{
		// TODO: Equivalent to this is out.insert(getInputs(block,true)); but this works in Qt >= 5.15
		QMap<QString,QString> inputs = getInputs(block,true);
		QStringList inputList = inputs.keys();
		for(int i=0; i < inputList.count(); i++)
			out.insert(inputList[i],inputs.value(inputList[i]));
	}
	return out;
}

/*! Starts "when timer is greater than" event blocks if input time is greater than timer value (in seconds). */
void Engine::spriteTimerEvent(void)
{
	// frameEvents instead of blocks can be used here
	QStringList blocksList = m_sprite->frameEvents.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = m_sprite->frameEvents.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whengreaterthan")
		{
			QMap<QString,QString> inputs = getInputs(block);
			if((inputs.value("WHENGREATERTHANMENU") == "TIMER") && (m_sprite->timer.elapsed()/1000.0 > inputs.value("VALUE").toDouble())
				&& !block.value("special_timereventused").toBool())
			{
				// Stop running instances of this event
				QList<QVariantMap> operationsToRemove;
				operationsToRemove.clear();
				for(int i2=0; i2 < m_sprite->currentExecPos.count(); i2++)
				{
					if(m_sprite->currentExecPos[i2]["toplevelblock"] == blocksList[i])
						operationsToRemove += m_sprite->currentExecPos[i2];
				}
				for(int i2=0; i2 < operationsToRemove.count(); i2++)
					m_sprite->currentExecPos.removeAll(operationsToRemove[i2]);
				// Start the script
				block.insert("special_timereventused",true);
				m_sprite->frameEvents.insert(blocksList[i],block);
				QVariantMap blockMap;
				blockMap.clear();
				blockMap.insert("id",blocksList[i]);
				blockMap.insert("special","");
				m_sprite->currentExecPos += blockMap;
			}
		}
	}
}
