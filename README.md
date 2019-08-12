The dispatch_queue is built as a library. Include it in your project like:

#### CMake
`target_link_libraries(app mylib1)`

#### Source file
`#include <dispatch_queue/DispatchQueue.hpp>`

**_References:_**  
https://embeddedartistry.com/blog/2017/2/1/c11-implementing-a-dispatch-queue-using-stdfunction
https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/dispatch.cpp