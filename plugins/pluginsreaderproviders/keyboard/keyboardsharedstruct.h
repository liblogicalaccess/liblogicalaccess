/**
 * \file keyboardsharedstruct.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard shared struct. 
 */

#ifndef LOGICALACCESS_KEYBOARDSHAREDSTRUCT_H
#define LOGICALACCESS_KEYBOARDSHAREDSTRUCT_H

namespace logicalaccess
{
	#define KEYBOARD_SHAREDDATA			"Global\\ISLOGKbdHook"
	#define KEYBOARD_EVENT				"Global\\ISLOGKbdEvent"
	#define KEYBOARD_EVENTPROCESEED		"Global\\ISLOGKbdEventProcessed"
	#define KEYBOARD_HOSTEVENT			"Global\\ISLOGHostEvent"
	#define KEYBOARD_STILLALIVEEVENT	"Global\\ISLOGStillAliveEvent"

	#define MAX_KEYBOARD_DEVICES		10
	#define DEVICE_NAME_MAXLENGTH		128

	struct KeyboardEntry
	{
		unsigned long long handle;

		char name[DEVICE_NAME_MAXLENGTH];
	};

	struct KeyboardSharedStruct
	{
		KeyboardEntry devices[MAX_KEYBOARD_DEVICES];

		char enteredKeyChar;

		char selectedDeviceName[DEVICE_NAME_MAXLENGTH];

		char keyboardLayout[9];
	};
}

#endif

