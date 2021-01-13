#pragma once
#include "NetCommon.h"
#include "NetQueue.h"
#include "NetMessage.h"
#include "NetConnection.h"
#include "NetServer.h"

namespace netcommon
{
	template<typename T>
	class ClientInterface
	{
	public:
		ClientInterface()
			:
			m_socket(m_context)
		{

		}
		virtual ~ClientInterface()
		{
			Disconnect();
		}

		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				//Resolve Hostname 
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type m_endpoints = resolver.resolve(host, std::to_string(port));

				//create connection
				m_connection = std::make_unique<connection<T>>(
					connection<T>::owner::client,
					m_context,
					asio::ip::tcp::socket(m_context), Q_MessagesIn);

				//Connect if resolve OK
				m_connection->ConnectToServer(m_endpoints);

				//Sart context thread
				thrContxt = std::thread([this]() {m_context.run(); });
				
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";
				return false;
			}
			return true;
		}
		void Disconnect()
		{
			if (IsConnected())
			{
				m_connection->Disconnect();
			}
			//stop asio context
			m_context.stop();
			//stop thread
			if (thrContxt.joinable())
				thrContxt.join();

			//release unique pointer
			m_connection.release();

		}
		bool IsConnected() const
		{
			if (m_connection)
				return m_connection->IsConnected();
			else
				return false;
		}
		NetQueue<owned_message<T>>& IncomingMessages()
		{
			return Q_MessagesIn;
		}
		std::string GetConnectionInfo() const
		{
			return m_connection->GetInformation();
		}

	protected:
		//asio context handles transfer
		asio::io_context m_context;
		//thread required for asio context
		std::thread thrContxt;
		//physical socket connected to server
		asio::ip::tcp::socket m_socket;
		//client has a single instance of a "connection" object, which handles data transfer
		std::unique_ptr<connection<T>> m_connection;


	private:
		//threadsafe queue of incoming messages from server
		NetQueue<owned_message<T>> Q_MessagesIn;

		//Get info for terminals etc
		std::ostringstream oss;
	};

}