#pragma once
#include "NetCommon.h"
#include "NetQueue.h"
#include "NetMessage.h"
#include "NetConnection.h"

namespace netcommon
{
	template<typename T>
	class NetServer
	{
	public:
		NetServer(uint16_t port)
			:
			asioAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{}
		~NetServer()
		{
			Stop();
		}
		bool Start()
		{
			try
			{
				WaitForClientConnection();
				thrContext = std::thread([this]() {asioContext.run(); });
			}
			catch(std::exception& e)
			{
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}
			std::cout << "[SERVER] Started!\n";
			return true;
		}
		void Stop()
		{
			asioContext.stop();
			if (thrContext.joinable())
				thrContext.join();
			std::cout << "[SERVER] Stopped!\n";

		}
		//ASYNC
		void WaitForClientConnection()
		{
			asioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
						std::shared_ptr<connection<T>> newconn =
							std::make_shared<connection<T>>(connection<T>::owner::server,
								asioContext, std::move(socket), Q_messagesIn);

						//ability for user to deny connection
						if (OnClientConnect(newconn))
						{
							//connection allowed, so add to container.
							deqConnections.push_back(std::move(newconn));
							//assign UUID to connection.
							deqConnections.back()->ConnectToClient(this, nIDCounter++);
							std::cout << "[" << deqConnections.back()->GetUUID() << "] Connection Approoproo\n";
						}
						else
						{
							std::cout << "[SERVER] Connection denied.\n";
						}
					}
					else
					{
						//Error accepting new connection
						std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
					}

					//Prime the asio context with more work 
					WaitForClientConnection();
				});
		}
		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				OnClientDisconnect(client);
				client.reset();
				deqConnections.erase(
					std::remove(deqConnections.begin(), deqConnections.end(), client), deqConnections.end());
			}
		}
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> ignore = nullptr)
		{
			bool invalidClientExists = false;

			for (auto& client : deqConnections)
			{
				if (client && client->IsConnected())
				{
					if(client != ignore)
						client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					invalidClientExists = true;
				}
				if(invalidClientExists)
					std::remove(deqConnections.begin(), deqConnections.end(), nullptr), deqConnections.end();
			}
		}

		void Update(size_t nMaxMessages = -1, bool bWait = false)
		{
			//We do not want server to occupy 100% of CPU core.
			if (bWait) Q_messagesIn.wait();

			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !Q_messagesIn.empty())
			{
				//grab and remove front message
				auto msg = Q_messagesIn.pop_front();

				//pass to message handler
				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

	protected:
		//called on client connected, can veto by returning false
		virtual bool OnClientConnect(std::shared_ptr<connection<T>> connection)
		{
			return false;
		}
		// called when client appears to have disconneceted
		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> connection)
		{

		}
		// called when message arrives
		virtual void OnMessage(std::shared_ptr<connection<T>> connection, const message<T>& msg)
		{

		}

	public:
		virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
		{

		}

	protected:
		//thread safe message q
		NetQueue<owned_message<T>> Q_messagesIn;

		//containter of active valid connections
		std::deque<std::shared_ptr<connection<T>>> deqConnections;

		//asio - order here is important
		asio::io_context asioContext;
		std::thread thrContext;
		asio::ip::tcp::acceptor asioAcceptor;

		// Clients given UID
		uint32_t nIDCounter = 10000;

	private:
	};

};