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
	// Acquire lock to prevent race condition with the thread handler
	std::unique_lock<std::mutex> lock(_lock);
	_should_exit = true;
	lock.unlock();

	_cv.notify_all();

	// Wait for threads to finish their work before we exit
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

// Adds a callback item to the dispatch queue
void DispatchQueue::dispatch(const fp_t& callback)
{
	std::unique_lock<std::mutex> lock(_lock);
	_queue.push(callback);

	// Manual unlocking is done before notifying, to avoid waking up
	// the waiting thread only to block again (see notify_one for details)
	lock.unlock();
	_cv.notify_all();
}

void DispatchQueue::schedule_on_interval(const fp_t& callback, const unsigned interval_ms)
{
	// Kick off a thread, put it to sleep, and then have it call the callback
	auto thread = std::thread(&DispatchQueue::timer_callback_dispatch, this, callback, interval_ms);

	// We want to keep track of our timer threads so we can kill them properly
	_timer_threads.push_back(std::move(thread));
}

// TODO: figure out how to make this a lambda... would be more succint
void DispatchQueue::timer_callback_dispatch(const fp_t& callback, const unsigned interval_ms)
{
		// The entire purpose of this function is to run in a thread and only wake up to schedule 
		// the work item in the DispatchQueue.
		do
		{
			dispatch(callback);
			std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
		} while (!_should_exit);
}

void DispatchQueue::dispatch_thread_handler(void)
{
	// Construction acquires the lock
	std::unique_lock<std::mutex> lock(_lock);

	do
	{
		// Wait until we have data
		_cv.wait(lock, [this]
			{ return (!_queue.empty() || _should_exit); }
		);

		// After wait, we own the lock
		if (!_queue.empty() && !_should_exit)
		{
			// Find first ready item
			auto callback = std::move(_queue.front());
			_queue.pop();

			// TODO: add a check to see if the queue is empty, and if so, signal 
			// that we are out of useful work to do. Alternatively, run background
			// tasks here :)

			lock.unlock();

			callback();

			// It's worth noting that condition variable's wait function requires a lock. If 
			// the thread is waiting for data, it will release the mutex and only re-lock when 
			// notified and ready to run. This is why we lock at the end of the while loop.
			lock.lock();
		}
	} while (!_should_exit);
}
