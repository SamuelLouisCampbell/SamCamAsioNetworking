#include "NetCommonInclude.h"
#include <NetClient.h>

enum class CustomMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomClient : public netcommon::ClientInterface<CustomMsgType>
{

public:
	void PingServer()
	{
		netcommon::message<CustomMsgType> msg;
		msg.header.id = CustomMsgType::ServerPing;

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		msg << now;
		m_connection->Send(msg);
	}
	void SendHello()
	{
		netcommon::message<CustomMsgType> msg;
		msg.header.id = CustomMsgType::ServerMessage;
		msg << "Hello from : " << m_connection->GetInformation().c_str();
		m_connection->Send(msg);
	}

private:
};

int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);


	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "Sending Hello Message..." << std::endl;
		c.PingServer();
		//c.SendHello();
		if (c.IsConnected())
		{

			if (!c.IncomingMessages().empty())
			{
				auto msg = c.IncomingMessages().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgType::ServerAccept:
				{
					// Server has responded to a ping request				
					std::cout << "Server Accepted Connection\n";
				}
				break;
				case CustomMsgType::ServerPing:
				{
					std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point then;
					msg >> then;
					std::cout << "Ping: " << std::chrono::duration<double>(now - then).count() << "\n";
				}
				break;
				case CustomMsgType::ServerMessage:
				{
					std::cout << msg << std::endl;
				}
				break;
				default:
					break;
				}
			}

		}
	}

		
	
	std::cin.get();
	return 0;
}