#pragma once
#include "NetCommon.h"
#include "NetQueue.h"
#include "NetMessage.h"
#include <random>

namespace netcommon
{
	//forward declare 
	template<typename T>
	class NetServer;

	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		enum class owner
		{
			server,
			client
		};

		connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, NetQueue<owned_message<T>>& qIn)
			:
			m_context(asioContext),
			m_socket(std::move(socket)),
			m_qMessagesIn(qIn)
		{
			m_nOwnerType = parent;

			if (m_nOwnerType == owner::server)
			{
				//Generate a random number to send to the client
				std::random_device rd;
				std::mt19937 rng(rd());
				std::uniform_int_distribution<int>dist (0, std::numeric_limits<int>::max()-1);
				m_nHandshakeOut = dist(rng);

				//Pre calculate answer for client to respond with
				m_nHandshakeCheck = Scramble(m_nHandshakeOut);
			}
			else
			{
				uint64_t m_nHandshakeOut = 0;
				uint64_t m_nHandshakeIn = 0;
			}

		}
		virtual ~connection()
		{}
	private:
		//ASYNC - Prime context to read header
		void ReadHeader()
		{
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_msgTemporaryIn.header.size > 0)
						{
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << UUID << "] Read Header Fail.\n";
						m_socket.close();
					}
				});
		}
		void ReadBody()
		{
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						AddToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << UUID << "] Read Body Fail.\n";
						m_socket.close();
					}

				});
		}
		void WriteHeader()
		{
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_qMessagesOut.front().body.size() > 0)
						{
							WriteBody();
						}
						else
						{
							m_qMessagesOut.pop_front();
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						std::cout << "[" << UUID << "] Write Header Fail.\n";
						m_socket.close();
					}

				});
		}
		void WriteBody()
		{
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						m_qMessagesOut.pop_front();

						if (!m_qMessagesOut.empty())
						{
							WriteHeader();
						}
					}
						
					else
					{
						std::cout << "[" << UUID << "] Write Body Fail.\n";
						m_socket.close();
					}
				});
		}
		void AddToIncomingMessageQueue()
		{
			if (m_nOwnerType == owner::server)
			{
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				
			}
			else
			{
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
			}
			ReadHeader();
		}

	public:

		//get information for terminals etc.
		std::string GetInformation()
		{
			std::stringstream ss;
			ss << m_socket.local_endpoint();
			return ss.str();
		}
		void ConnectToClient(netcommon::NetServer<T>* server, uint32_t uuid = 0)
		{
			if (m_nOwnerType == owner::server)
			{
				if (m_socket.is_open())
				{
					UUID = uuid;
					//ReadHeader();
					//client has atttempted to connect, handshake must be validated
					WriteValidation();

					//next issue a task and wait asynchronously for precisely the validatio
					//data to be senback from the client
					ReadValidation(server);
				}
			}
		}
		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_nOwnerType == owner::client)
			{
				asio::async_connect(m_socket, endpoints, [this](std::error_code ec,
					asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							//ReadHeader();

							//First thing to do is send packet to be validated
							ReadValidation();

						}
					});
			}
		}
		void Disconnect()
		{
			if (IsConnected())
				asio::post(m_context, [this]() {m_socket.close(); });
		}
		bool IsConnected() const
		{
			return m_socket.is_open();
		}
		void Send(const message<T>& msg)
		{
			asio::post(m_context, [this, msg]()
				{
					bool bWritingMessage = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(msg);
					if (!bWritingMessage)
						WriteHeader();
				});
		}
		uint32_t GetUUID() const
		{
			return UUID;
		}

		//"Encrypt" Data
		uint64_t Scramble(uint64_t nInput)
		{
			uint64_t out = nInput ^ 0xF00DBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DED00D23051988;
			//return nInput;
		}

		void WriteValidation()
		{
			asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_nOwnerType == owner::client)
							ReadHeader();
					}
					else
					{
						m_socket.close();
					}
				});
		}
		void ReadValidation(netcommon::NetServer<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_nOwnerType == owner::server)
						{
							if (m_nHandshakeIn == m_nHandshakeCheck)
							{
								//client has responded with correct solution so allow it to connect.
								std::cout << "Client Validated" << std::endl;
								server->OnClientValidated(this->shared_from_this());

								//Sit waiting to recieve data now. 
								ReadHeader();
							}
							else
							{
								std::cout << "Client Disconnected (Fail Validation)" << std::endl;
								m_socket.close();
							}
						}
						else
						{
							//connection is a client so solve puzzle;
							m_nHandshakeOut = Scramble(m_nHandshakeIn);

							//Write back the result;
							WriteValidation();
						}
					}
					else
					{
						std::cout << "Client Disconnected (Read Validation)" << std::endl;
						m_socket.close();
					}
				});
		}

	protected:
		// each connection has a unique socket to the remote
		asio::ip::tcp::socket m_socket;

		// Context is shared with whole ASIO instance
		asio::io_context& m_context;

		// Q holds all messages to be sent to remote
		NetQueue<message<T>> m_qMessagesOut;

		// Q holds all messages which have been recieved from the remote
		NetQueue<owned_message<T>>& m_qMessagesIn;

		//owner describes how the connection behaves.(well.. some of it)
		owner m_nOwnerType = owner::server;
		uint32_t UUID = 0;
		message<T> m_msgTemporaryIn;

		//Handshake validation
		uint64_t m_nHandshakeOut = 0;
		uint64_t m_nHandshakeIn = 0;
		uint64_t m_nHandshakeCheck = 0;

		//return information
		


	};

}