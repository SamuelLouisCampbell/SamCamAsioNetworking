#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(2048);

void GrabSomeData(asio::ip::tcp::socket& s)
{
	s.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "\n\nRead " << length << " bytes\n\n";
				for (size_t i = 0; i < length; i++)
				{
					std::cout << vBuffer[i];
				}
				GrabSomeData(s);
			}
		}
	);
}

int main()
{
	//Error code storage etc
	asio::error_code ec;

	//A context (or space) for ASIO to work within
	asio::io_context context;

	//Assign some non work for the context to keep it running
	asio::io_context::work idleWork(context);

	//Start the context
	std::thread thrContext = std::thread([&]() {context.run(); });

	//Get the address and port for somewhere we wish to connect to include the error store just in case.
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec), 80);

	//Create socket, the context will deliver the implementation.
	asio::ip::tcp::socket socket(context);

	//Tell socket to try & connect
	socket.connect(endpoint, ec);

	//Basic Error Check.
	if (!ec)
		std::cout << "connected!" << std::endl;
	else
		std::cout << "Failed to connect to addr:\n" << ec.message() << std::endl;

	if (socket.is_open())
	{
		GrabSomeData(socket);

		std::string sRequest = "GET /index.html HTTP/1.1\r\n"
							   "Host: example.com\r\n"
							   "Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);		
	}

	std::cin.get();
	return 0;
}