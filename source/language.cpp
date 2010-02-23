#include "language.h"
#include <ogc/conf.h>
const char* TEXT_InitializingNetwork;
const char* TEXT_CouldNotConnect;
const char* TEXT_ConnectingTo;
const char* TEXT_ConnectionFailed;
const char* TEXT_EnterPassword;
const char* TEXT_Cancel;
const char* TEXT_Connect;
const char* TEXT_Exit;
const char* TEXT_Enter;
const char* TEXT_Up;
const char* TEXT_Down;
const char* TEXT_Left;
const char* TEXT_Right;

void InitializeLanguage()
{
	switch(CONF_GetLanguage())
	{
		default:
		case CONF_LANG_ENGLISH:
			TEXT_InitializingNetwork = "Initializing network";
			TEXT_CouldNotConnect = "Could not connect to network";
			TEXT_ConnectingTo = "Connecting to:";
			TEXT_ConnectionFailed = "Connection failed";
			TEXT_EnterPassword = "Enter Password";
			TEXT_Cancel = "Cancel";
			TEXT_Connect = "Connect";
			TEXT_Exit = "Exit";
			TEXT_Enter = "Enter";
			TEXT_Up = "Up";
			TEXT_Down = "Down";
			TEXT_Left = "Left";
			TEXT_Right = "Right";
			break;

		case CONF_LANG_DUTCH:
			TEXT_InitializingNetwork = "Netwerk initialiseren";
			TEXT_CouldNotConnect = "Kon geen verbinding maken met netwerk";
			TEXT_ConnectingTo = "Verbinding maken met:";
			TEXT_ConnectionFailed = "Verbinding mislukt";
			TEXT_EnterPassword = "Voer wachtwoord in";
			TEXT_Cancel = "Annuleren";
			TEXT_Connect = "Maak verbinding";
			TEXT_Exit = "Afsluiten";
			TEXT_Enter = "OK";
			TEXT_Up = "Omhoog";
			TEXT_Down = "Omlaag";
			TEXT_Left = "Links";
			TEXT_Right = "Rechts";
			break;
			
		case CONF_LANG_GERMAN:
			TEXT_InitializingNetwork = "Netzwerk initialisieren";
			TEXT_CouldNotConnect = "Konnte keine verbindung machen zum netzwerk";
			TEXT_ConnectingTo = "Verbindung machen mit:";
			TEXT_ConnectionFailed = "Verbindung fehler";
			TEXT_EnterPassword = "Passwort eingeben:";
			TEXT_Cancel = "Abbrechen";
			TEXT_Connect = "Verbindung";
			TEXT_Exit = "Beënden";
			TEXT_Enter = "OK";
			TEXT_Up = "Oben";
			TEXT_Down = "Rünter";
			TEXT_Left = "Links";
			TEXT_Right = "Rechts";
			break;
			
		//TODO:
		//	CONF_LANG_JAPANESE,
		//	CONF_LANG_GERMAN,
		//	CONF_LANG_FRENCH,
		//	CONF_LANG_SPANISH,
		//	CONF_LANG_ITALIAN,
		//	CONF_LANG_SIMP_CHINESE,
		//	CONF_LANG_TRAD_CHINESE,
		//	CONF_LANG_KOREAN
	}
}