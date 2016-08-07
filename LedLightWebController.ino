#include <Arduino.h>
#include "OnOffRelayManager.h"
#include "PulseRelayManager.h"
#include "Util.h"
#include "WiFiManager.h"
#include "WebServer.h"
#include "Logger.h"
#include "MementaryPushButtonManager.h"
#include "TogglePushButtonManager.h"
#include <memory>
#include "Configuration.h"

using namespace std;

//Board configuration
int pushButton = 5;
int redLed = 4;
int greenLed = 15;
int relay = 2;

class WebCommand : public IWebCommand, public enable_shared_from_this<WebCommand>
{
private:
	static int s_id;
	const string _menuEnrty;
	const string _commandName;
	const string _resultHtml;
	const int _id = ++s_id;
	weak_ptr<WebServer> _webServer;

public:
	WebCommand(string menuEntry, string commandName, WebServerPtr_t webServer) : _menuEnrty(menuEntry), _commandName(commandName),
		_resultHtml(string("Processing ") + _commandName + " Command"), _webServer(webServer)
	{
	}

	void Register()
	{
		_webServer.lock()->RegisterCommand(shared_from_this());
	}

	const string& MenuEntry() const override
	{
		return _menuEnrty;
	}

	const string& Name() const override
	{
		return _commandName;
	}

	const string& ResultHTML() const override
	{
		return _resultHtml;
	}
	const string& TriggerUrl() const override
	{
		return _commandName;
	}

	const int Id() const override
	{
		return _id;
	}
};

int WebCommand::s_id = 0;

void SwitchRelayState(int state);
void Reset();

LoggerPtr_t logger;
WiFiManagerPtr_t wifiManager;
WebServerPtr_t server;
RelayManagerPtr_t relayManager;
PushButtonManagerPtr_t pushButtonManager;

void setup()
{
	logger = make_shared<Logger>(redLed, greenLed);
	wifiManager = make_shared<WiFiManager>(SSID, password);
	server = make_shared<WebServer>(wifiManager, 80, appKey);
	server->SetWebSiteHeader(string(webSiteHeader));
	server->Register(logger);

#ifdef PULSE_COMMAND
	pushButtonManager = make_shared<MementaryPushButtonManager>(pushButton, &SwitchRelayState, &Reset);
	relayManager = make_shared<PulseRelayManager>(relay, 1000, [=](const string &message) { logger->WriteMessage(message); });
	make_shared<WebCommand>(pulseMenuEntry, "Activate", server)->Register();
#else
	pushButtonManager = make_shared<TogglePushButtonManager>(pushButton, &SwitchRelayState, &Reset);
	relayManager = make_shared<OnOffRelayManager>(relay, [=](const string &message) { logger->WriteMessage(message); });
	make_shared<WebCommand>(turnOnMenuEntry, "On", server)->Register();
	make_shared<WebCommand>(turnOffMenuEntry, "Off", server)->Register();
#endif

	server->Register(relayManager);

	logger->TestLeds();
}

void loop()
{
	wifiManager->Loop();
	server->Loop(relayManager->State());
	logger->Loop();
	pushButtonManager->Loop();
	relayManager->Loop();
}

void SwitchRelayState(int state)
{
	relayManager->Set(state);
}

void Reset()
{
	Util::software_Reboot();
}