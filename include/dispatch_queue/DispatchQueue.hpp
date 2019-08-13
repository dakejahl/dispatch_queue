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

#include <condition_variable>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>
#include <atomic>

typedef std::function<void(void)> fp_t;

class DispatchQueue
{
public:
	DispatchQueue(std::string name, size_t thread_count = 1);
	~DispatchQueue();

	void dispatch(const fp_t& callback);
	void schedule_on_interval(const fp_t& callback, const unsigned interval_ms);
	bool empty(void)
	{
		std::unique_lock<std::mutex> lock(_lock);
		return _queue.empty();
	};

private:
	void dispatch_thread_handler(void);
	void join_timer_threads(void);
	void join_worker_threads(void);

	std::string _name;
	std::queue<fp_t> _queue;
	std::vector<std::thread> _worker_threads;
	std::vector<std::thread> _timer_threads;
	std::mutex _lock;
	std::condition_variable _cv;

	std::atomic<bool> _should_exit {};
};
