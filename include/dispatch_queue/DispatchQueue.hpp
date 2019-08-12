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

class DispatchQueue
{
	typedef std::function<void(void)> fp_t;

public:
	DispatchQueue(std::string name, size_t thread_count = 1);
	~DispatchQueue();

	void dispatch(const fp_t& callback);
	bool empty(void) { return _queue.empty(); };

private:
	void dispatch_thread_handler(void);

	std::string _name;
	std::queue<fp_t> _queue;
	std::vector<std::thread> _threads;
	std::mutex _lock;
	std::condition_variable _cv;

	bool _should_exit = false;
};