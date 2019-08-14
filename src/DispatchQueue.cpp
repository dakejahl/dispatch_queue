// MIT License

// Copyright (c) 2019 Jacob Dahl

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <dispatch_queue/DispatchQueue.hpp>

DispatchQueue::DispatchQueue(std::string name, size_t thread_count)
	: _name(name)
	, _worker_threads(thread_count)
{
	printf("Creating dispatch queue:  %s\n[%zu] thread(s) used\n\n", name.c_str(), thread_count);

	for (auto& t : _worker_threads)
	{
		t = std::thread(&DispatchQueue::dispatch_thread_handler, this);
	}
}

DispatchQueue::~DispatchQueue()
{
	// Acquire lock to prevent race condition with the thread handler.
	std::unique_lock<std::mutex> lock(_lock);
	_should_exit = true;
	lock.unlock();

	_cv.notify_all();

	// Wait for threads to finish their work before we exit.
	join_timer_threads();
	join_worker_threads();

	printf("\nDispatchQueue destroyed\n");
}

void DispatchQueue::join_timer_threads()
{
	unsigned i = 0;
	for (auto& t : _timer_threads)
	{
		if (t.joinable())
		{
			printf("Joining timer thread %u\n", i++);
			t.join();
		}
	}
}

void DispatchQueue::join_worker_threads()
{
	unsigned i = 0;
	for (auto& t : _worker_threads)
	{
		if (t.joinable())
		{
			printf("Joining worker thread %u\n", i++);
			t.join();
		}
	}   
}

// Adds a callback item to the dispatch queue.
void DispatchQueue::dispatch(const WorkItem& callback)
{
	std::unique_lock<std::mutex> lock(_lock);
	_standard_queue.push(callback);

	// Manual unlocking is done before notifying, to avoid waking up
	// the waiting thread only to block again (see notify_one for details).
	lock.unlock();
	_cv.notify_all();
}

// Priority dispatch. Lower numbers are higher priority.
void DispatchQueue::dispatch(const PriorityWorkItem& callback)
{
	std::unique_lock<std::mutex> lock(_lock);

	auto it = _priority_queue.begin();
	for ( ; it != _priority_queue.end(); it++)
	{
		if (callback.priority < it->priority)
		{
			_priority_queue.insert(it, callback);
			break;
		}
	}
	// We reached the end of the list and didn't get to cut the line :(
	if (it == _priority_queue.end())
	{
		_priority_queue.push_back(callback);
	}

	// Manual unlocking is done before notifying, to avoid waking up
	// the waiting thread only to block again (see notify_one for details).
	lock.unlock();
	_cv.notify_all();
}

// TODO: use just a single thread with a TimedDispatcher class. This guy can just sleep until the
// next deadline for a work item that needs to be run. After dispatching an item to the queue, it
// will check which item in its own queue is up next, move its queue pointer to that item and then
// sleep for that time delta. On wakeup it will just call dispatch().
void DispatchQueue::schedule_on_interval(const WorkItem& callback, const unsigned interval_ms)
{
	auto timed_dispatch = [this](const WorkItem& callback, const unsigned interval_ms)
	{
		do
		{
			this->dispatch(callback);
			std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
		} while (!this->_should_exit);
	};

	auto thread = std::thread(timed_dispatch, callback, interval_ms);

	// We want to keep track of our timer threads so we can shut down properly.
	_timer_threads.push_back(std::move(thread));
}

void DispatchQueue::dispatch_thread_handler(void)
{
	// Construction acquires the lock
	std::unique_lock<std::mutex> lock(_lock);

	do
	{
		// Wait until we have data.
		// Condition_variable::wait(...) always evaluates predicate and
		// continues if true, and blocks if false.
		_cv.wait(lock, [this]
			{ return (!_standard_queue.empty() || !_priority_queue.empty() || _should_exit); }
		);

		// After wait, we own the lock.
		// We also know that the queue is not empty, as we just took the lock, checked
		// if empty, and then continued because it was not empty.
		if (!_should_exit)
		{
			fp_t work;

			// Check priority queue first
			if (!_priority_queue.empty())
			{
				PriorityWorkItem item = std::move(_priority_queue.front());
				_priority_queue.pop_front();
				work = item.work;
			}
			else
			{
				WorkItem item = std::move(_standard_queue.front());
				_standard_queue.pop();
				work = item.work;
			}

			// TODO: add a check to see if the queue is empty, and if so, signal 
			// that we are out of useful work to do. Alternatively, run background
			// tasks here :)

			lock.unlock();

			work();

			// It's worth noting that condition variable's wait function requires a lock. If 
			// the thread is waiting for data, it will release the mutex and only re-lock when 
			// notified and ready to run. This is why we lock at the end of the while loop.
			lock.lock();
		}
	} while (!_should_exit);
}
