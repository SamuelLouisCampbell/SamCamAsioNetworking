#pragma once
#include "NetCommon.h"

namespace netcommon
{
	template <typename T>

	//the message header
	struct message_header
	{
		T id{};
		uint32_t size = 0;
	};

	template<typename T>
	struct message
	{
		message_header<T> header{};
		std::vector<uint8_t> body;

		//Get size of message
		size_t size() const
		{
			//return sizeof(message_header<T>) + body.size();
			return body.size();
		}
		//overload so message can be pumped into cout 
		friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
		{
			os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
			return os;
		}
		//Push any POD like data into the message buffer
		template<typename DataType>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			//check data is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into message vector");

			//Cache current size of vector. this is the point we insert new data. 
			size_t i = msg.body.size();
			
			//resize vector to receive new data type
			msg.body.resize(msg.body.size() + sizeof(DataType));

			//copy the data into newly allocated vector space
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			//recalculate message size for header
			msg.header.size = uint32_t(msg.size());

			//return target message so it can be chained
			return msg;
		}

		//retrieve messages;
		template<typename DataType>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			//check data is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from message vector");

			//Cache location at end of vector where data will be pulled from 
			size_t i = msg.body.size() - sizeof(DataType);

			//copy data into user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
			
			//shrink vector to remove read bytes and reset end position.
			msg.body.resize(i);

			//recalculate message size for header
			msg.header.size = uint32_t(msg.size());

			//return target message so it can be chained
			return msg;
		}

	};

	//forward delcare connection
	template<typename T>
	class connection;

	template<typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;
		//string maker
		friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}