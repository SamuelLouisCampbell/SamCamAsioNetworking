#include <iostream>
#include <NetCommon.h>
#include <NetServer.h>

enum class CustomMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageServer,
};

class CustomServer : public netcommon::NetServer<CustomMsgType>
{
public:
	CustomServer(uint16_t nPort) : netcommon::NetServer<CustomMsgType>(nPort)
	{

	}
protected:
	virtual bool OnClientConnect(std::shared_ptr<netcommon::connection<CustomMsgType>> connection) override
	{
		netcommon::message<CustomMsgType> msg;
		msg.header.id = CustomMsgType::ServerAccept;
		connection->Send(msg);
		return true;
	}
	virtual void OnClientDisconnect(std::shared_ptr<netcommon::connection<CustomMsgType>> connection) override
	{
		std::cout << "Removing client [" << connection->GetUUID() << "]\n";
	}
	virtual void OnMessage(std::shared_ptr<netcommon::connection<CustomMsgType>> connection, const netcommon::message<CustomMsgType>& msg) override
	{
		switch (msg.header.id)
		{
		case CustomMsgType::ServerPing:
		{
			std::cout << "[" << connection->GetUUID() << "]: Server Ping\n";
			connection->Send(msg);
			break;
		}
		case CustomMsgType::MessageServer:
		{
			std::cout << "[" << connection->GetUUID() << "]: Has Sent a Message!\n";
			connection->Send(msg);
			for (int i = 0; i < msg.size(); i++)
			{
				std::cout << msg.body[i];
			}
			std::cout << std::endl;
			break;
		}
		default:
			break;
		}
	}

};

int main()
{
	CustomServer server(60000);
	server.Start();
	while (true)
	{
		server.Update(-1, false);
	}

	return 0;
}