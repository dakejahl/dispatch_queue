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

#include <iostream>
#include <memory>

int main()
{
	// Test it
	std::cout << "<<<<<<<<< TESTING dispatch_queue >>>>>>>>>>\n\n";

	unsigned shared_counter = 0;

	auto work_item_1 = [&shared_counter]
	{
		// Do something with a shared resource
		shared_counter++;
		printf("Item 1:  %d\n", shared_counter);
	};

	auto work_item_2 = [&shared_counter]
	{
		// Do something with a shared resource
		shared_counter++;
		printf("Item 2:  %d\n", shared_counter);
	};

	auto work_item_3 = [&shared_counter]
	{
		// Do something with a shared resource
		shared_counter++;
		printf("Item 3:  %d\n\n", shared_counter);
	};

	// Single shared serial queue
	auto q = std::make_shared<DispatchQueue>("test_shared_resource");

	// Schedule our stuff on intervals using shared resource
	q->schedule_on_interval(work_item_1, 500);
	q->schedule_on_interval(work_item_2, 1000);
	q->schedule_on_interval(work_item_3, 2000);

	// Wait for all work to finish
	while (shared_counter < 10)
	{
		// busy wait uses entire core -- use some arbitrary polling time
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	printf("main() exiting\n\n");

	return 0;
}