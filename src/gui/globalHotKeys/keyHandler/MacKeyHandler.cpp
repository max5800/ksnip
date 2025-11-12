/*
 * Copyright (C) 2019 Damir Porobic <damir.porobic@gmx.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MacKeyHandler.h"

int MacKeyHandler::mNextId = 0;

MacKeyHandler::MacKeyHandler() : mHotKeyRef(nullptr)
{
	mHotKeyId.signature = 'ksnp';
	mHotKeyId.id = 0;
}

MacKeyHandler::~MacKeyHandler()
{
	unregisterKey();
}

bool MacKeyHandler::registerKey(const QKeySequence &keySequence)
{
	// Unregister any existing hotkey
	unregisterKey();

	auto keyCodeCombo = mKeyCodeMapper.map(keySequence);
	
	if (keyCodeCombo.key == 0) {
		return false;
	}

	// Generate unique ID for this hotkey
	mHotKeyId.id = ++mNextId;

	// Register the hotkey using Carbon API
	OSStatus status = RegisterEventHotKey(
		keyCodeCombo.key,
		keyCodeCombo.modifier,
		mHotKeyId,
		GetApplicationEventTarget(),
		0,
		&mHotKeyRef
	);

	return status == noErr;
}

bool MacKeyHandler::isKeyPressed(void *message)
{
	if (!message) {
		return false;
	}

	auto event = static_cast<EventRef>(message);
	
	if (GetEventClass(event) != kEventClassKeyboard) {
		return false;
	}

	if (GetEventKind(event) != kEventHotKeyPressed) {
		return false;
	}

	EventHotKeyID eventHotKeyId;
	OSStatus status = GetEventParameter(
		event,
		kEventParamDirectObject,
		typeEventHotKeyID,
		nullptr,
		sizeof(EventHotKeyID),
		nullptr,
		&eventHotKeyId
	);

	if (status != noErr) {
		return false;
	}

	return eventHotKeyId.signature == mHotKeyId.signature && 
	       eventHotKeyId.id == mHotKeyId.id;
}

void MacKeyHandler::unregisterKey() const
{
	if (mHotKeyRef != nullptr) {
		UnregisterEventHotKey(mHotKeyRef);
		mHotKeyRef = nullptr;
	}
}

OSStatus MacKeyHandler::hotKeyHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
	// This handler can be used for custom event handling if needed
	return CallNextEventHandler(nextHandler, event);
}