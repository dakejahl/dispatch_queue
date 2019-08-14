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
#include <list>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>
#include <atomic>

typedef std::function<void(void)> fp_t;

struct WorkItem
{
	WorkItem(const fp_t work)
	: work(work)
	{}

	fp_t work;
	// rhianna would be proud^
};

struct PriorityWorkItem
{
	PriorityWorkItem(const fp_t work, const uint8_t priority)
	: work(work)
	, priority(priority)
	{}

	uint8_t priority;
	fp_t work;
};

struct TimedWorkItem
{
	WorkItem(const fp_t work, const unsigned interval_ms)
	: work(work)
	, timeout(interval_ms)
	{}

	fp_t work;
	unsigned timeout;
};


class DispatchQueue
{
public:
	DispatchQueue(const std::string name, size_t thread_count = 1);
	~DispatchQueue();

	void dispatch(const WorkItem& callback);
	void dispatch(const PriorityWorkItem& callback);
	void dispatch(const TimedWorkItem& callback);

	void schedule_on_interval(const WorkItem& callback, const unsigned interval_ms);
	bool empty(void)
	{
		std::unique_lock<std::mutex> lock(_lock);
		return _standard_queue.empty();
	};

private:
	void dispatch_thread_handler(void);
	void timed_dispatch_thread_handler(void);

	void join_timer_thread(void);
	void join_worker_threads(void);

	std::string _name;
	std::atomic<bool> _should_exit {};

	std::queue<WorkItem> _standard_queue;

	std::list<PriorityWorkItem> _priority_queue;
	std::list<TimedWorkItem> _timed_queue;

	std::vector<std::thread> _worker_threads;
	std::thread _timer_thread;

	std::mutex _lock;
	std::condition_variable _cv;
};
