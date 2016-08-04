// RelayManager.h

#ifndef _RELAYMANAGER_h
#define _RELAYMANAGER_h
#include "arduino.h"
#include "WebServer.h"

class RelayManager : public IWebNotifications
{
private:
	int _pin;
	int _state;

 public:
	explicit RelayManager(int pin);
	virtual void Set(int value);
	int State() const { return _state; }

	void OnCommand(const std::string & commandName, int commandId) override = 0;
	void OnConnected(ConnectionStatus status, IPAddress ipAddress) override {}
	void OnDisconnected(ConnectionStatus status) override {}
	void OnError(ConnectionStatus status) override {}
	virtual void Loop() {}
};

typedef std::shared_ptr<RelayManager> RelayManagerPtr_t;
#endif
