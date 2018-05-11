#pragma once


enum E_ITME_STATES
{
	ITEM_STATES_UNKNOWN,
	ITEM_STATES_WAIT,
	ITEM_STATES_TRANSCODING,
	ITEM_STATES_COMPLETE,
	ITEM_STATES_ERROR,
	ITEM_STATES_PAUSE,
	//ITEM_STATES_PAUSE2,
	ITEM_STATES_STOP,
};


enum E_ENGINE_STATES
{
	ENGINE_STATES_STOP,
	ENGINE_STATES_TRANSCODING,
	ENGINE_STATES_PAUSE,
};


enum E_RESULT
{
	E_NO_ERROR,
	E_ERROR_FILE_EXISTS,
	E_ERROR_FILE_UNSUPPORTS,
	E_ERROR_FILE_UNEXISTS,
	E_ERROR_UNKNOWN,
};