#pragma once
#include "NetCommon.h"

namespace netcommon
{
	template<typename T>
	class NetQueue
	{
	public:
		NetQueue() = default;
		NetQueue(const NetQueue<T>&) = delete;
		virtual ~NetQueue() { eraseQ(); }
		
		//Lock Q and get front item
		const T& front()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}
		//Lock Q and get back item
		const T& back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}
		//push onto back with lock
		void push_back(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));
		}
		//push onto front with lock
		void push_front(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));
		}
		//see if Q empty
		bool empty()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}
		//See n items in Q
		size_t count()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.size();
		}
		//erase Q
		void eraseQ()
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}
		//pop front but return item
		T pop_front()
		{
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.front());
			deqQueue.pop_front();
			return t;
		}
		//pop back but return item
		T pop_back()
		{
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.back());
			deqQueue.pop_back();
			return t;
		}
		void wait()
		{
			//if the queue is empty we wait here...
			while (empty())
			{
				//Suspend until a message arrive.
				std::unique_lock<std::mutex> ul(muxBlock);
				cvBlock.wait(ul);
			}
		}

	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;

		std::condition_variable cvBlock;
		std::mutex muxBlock;
	};
}