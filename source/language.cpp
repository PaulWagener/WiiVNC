#include "language.h"
#include <ogc/conf.h>
const wchar_t* TEXT_InitializingNetwork;
const wchar_t* TEXT_CouldNotConnect;
const wchar_t* TEXT_ConnectingTo;
const wchar_t* TEXT_ConnectionFailed;
const wchar_t* TEXT_EnterPassword;
const wchar_t* TEXT_Cancel;
const wchar_t* TEXT_Connect;
const wchar_t* TEXT_Exit;
const wchar_t* TEXT_Enter;
const wchar_t* TEXT_Up;
const wchar_t* TEXT_Down;
const wchar_t* TEXT_Left;
const wchar_t* TEXT_Right;

void InitializeLanguage()
{
	switch(CONF_GetLanguage())
	{
		default:
		case CONF_LANG_ENGLISH:
			TEXT_InitializingNetwork = L"Initializing network";
			TEXT_CouldNotConnect = L"Could not connect to network";
			TEXT_ConnectingTo = L"Connecting to:";
			TEXT_ConnectionFailed = L"Connection failed";
			TEXT_EnterPassword = L"Enter Password";
			TEXT_Cancel = L"Cancel";
			TEXT_Connect = L"Connect";
			TEXT_Exit = L"Exit";
			TEXT_Enter = L"Enter";
			TEXT_Up = L"Up";
			TEXT_Down = L"Down";
			TEXT_Left = L"Left";
			TEXT_Right = L"Right";
			break;

		case CONF_LANG_DUTCH:
			TEXT_InitializingNetwork = L"Netwerk initialiseren";
			TEXT_CouldNotConnect = L"Kon geen verbinding maken met netwerk";
			TEXT_ConnectingTo = L"Verbinding maken met:";
			TEXT_ConnectionFailed = L"Verbinding mislukt";
			TEXT_EnterPassword = L"Voer wachtwoord in";
			TEXT_Cancel = L"Annuleren";
			TEXT_Connect = L"Maak verbinding";
			TEXT_Exit = L"Afsluiten";
			TEXT_Enter = L"OK";
			TEXT_Up = L"Omhoog";
			TEXT_Down = L"Omlaag";
			TEXT_Left = L"Links";
			TEXT_Right = L"Rechts";
			break;
			
		case CONF_LANG_GERMAN:
			TEXT_InitializingNetwork = L"Netzwerk initialisieren";
			TEXT_CouldNotConnect = L"Konnte keine verbindung machen zum netzwerk";
			TEXT_ConnectingTo = L"Verbindung machen mit:";
			TEXT_ConnectionFailed = L"Verbindung fehler";
			TEXT_EnterPassword = L"Passwort eingeben:";
			TEXT_Cancel = L"Abbrechen";
			TEXT_Connect = L"Verbindung machen";
			TEXT_Exit = L"Beënden";
			TEXT_Enter = L"OK";
			TEXT_Up = L"Oben";
			TEXT_Down = L"Rünter";
			TEXT_Left = L"Links";
			TEXT_Right = L"Rechts";
			break;
			
		case CONF_LANG_SPANISH:
			TEXT_InitializingNetwork = L"Inciando la Conexión";
			TEXT_CouldNotConnect = L"No Se Pudo Conectar a la Red";
			TEXT_ConnectingTo = L"Conectándose a:";
			TEXT_ConnectionFailed = L"Falló la Conexión";
			TEXT_EnterPassword = L"Introducir Clave";
			TEXT_Cancel = L"Cancelar";
			TEXT_Connect = L"Connectar";
			TEXT_Exit = L"Salir";
			TEXT_Enter = L"Entrar";
			TEXT_Up = L"Arriba";
			TEXT_Down = L"Abajo";
			TEXT_Left = L"Izquierda";
			TEXT_Right = L"Derecha";
			break;
			
		case CONF_LANG_ITALIAN:
			TEXT_InitializingNetwork = L"Inizializzando la rete";
			TEXT_CouldNotConnect = L"Impossibile connettersi";
			TEXT_ConnectingTo = L"Connessione a:";
			TEXT_ConnectionFailed = L"Connessione fallita";
			TEXT_EnterPassword = L"Inserire la password";
			TEXT_Cancel = L"Annulla";
			TEXT_Connect = L"Connetti";
			TEXT_Exit = L"Esci";
			TEXT_Enter = L"Invio";
			TEXT_Up = L"Su";
			TEXT_Down = L"Giu";
			TEXT_Left = L"Sinistra";
			TEXT_Right = L"Destra";
			break;
			
		case CONF_LANG_FRENCH:
			TEXT_InitializingNetwork = L"Initialisation du réseau";
			TEXT_CouldNotConnect = L"Impossible de se connecter au réseau";
			TEXT_ConnectingTo = L"Connexion à:";
			TEXT_ConnectionFailed = L"Connexion échouée";
			TEXT_EnterPassword = L"Entrer le mot de passe";
			TEXT_Cancel = L"Annuler";
			TEXT_Connect = L"Connexion";
			TEXT_Exit = L"Quitter";
			TEXT_Enter = L"Entrer";
			TEXT_Up = L"Haut";
			TEXT_Down = L"Bas";
			TEXT_Left = L"Gauche";
			TEXT_Right = L"Droite";
			break;
			
			/* Need a helvetica font that supports japanese
		case CONF_LANG_JAPANESE:
			TEXT_InitializingNetwork = L"ネットワークの初期化";
			TEXT_CouldNotConnect = L"ネットワークに接続できませんでした。";
			TEXT_ConnectingTo = L"への接続:";
			TEXT_ConnectionFailed = L"接続に失敗しました";
			TEXT_EnterPassword = L"パスワードを入力します";
			TEXT_Cancel = L"キャンセル";
			TEXT_Connect = L"接続";
			TEXT_Exit = L"終了";
			TEXT_Enter = L"入る";
			TEXT_Up = L"上";
			TEXT_Down = L"下";
			TEXT_Left = L"左";
			TEXT_Right = L"右";
			break;
			*/
			
		//TODO:
		//	CONF_LANG_SIMP_CHINESE,
		//	CONF_LANG_TRAD_CHINESE,
		//	CONF_LANG_KOREAN
	}
}