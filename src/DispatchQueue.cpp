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
	: _name (name)
	, _threads(thread_count)
{
	printf("Creating dispatch queue:  %s\n[%zu] thread(s) used\n\n", name.c_str(), thread_count);

	for (auto& t : _threads)
	{
		t = std::thread(&DispatchQueue::dispatch_thread_handler, this);
	}
}

DispatchQueue::~DispatchQueue()
{
	// acquire lock to prevent race condition with the thread handler
	std::unique_lock<std::mutex> lock(_lock);
	_should_exit = true;
	lock.unlock();

	_cv.notify_all();

	// Wait for threads to finish their work before we exit
	for (auto& t : _threads)
	{
		unsigned i = 0;
		if (t.joinable())
		{
			printf("Joining thread %u\n", i++);
			t.join();
		}
	}

	printf("\nDispatchQueue destroyed\n");
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

// This dispatch thread handler does...
void DispatchQueue::dispatch_thread_handler(void)
{
	// construction acquires the lock
	std::unique_lock<std::mutex> lock(_lock);

	do
	{
		// Wait until we have data
		_cv.wait(lock, [this]
			{ return (!_queue.empty() || _should_exit); }
		);

		// after wait, we own the lock
		if (!_queue.empty() && !_should_exit)
		{
			auto callback = std::move(_queue.front());
			_queue.pop();

            // unlock now that we're done messing with the queue
            lock.unlock();

            callback();
			// It's worth noting that condition variable's wait function requires a lock. If 
			// the thread is waiting for data, it will release the mutex and only re-lock when 
			// notified and ready to run. This is why we lock at the end of the while loop.
            lock.lock();
		}
	} while (!_should_exit);

	// lock is automatically released at the end of the scope
}