# Thread Plus
A Library dedicated to the development of Multithreading Utils Post C++23 Standard. This uses all the
expected C++23 features such as formatting, expected, optionals. 

# What is Include ?
# 1. thread_plus::channel::Channel
`thread_plus::channel::Channel` implements multithreaded communication by allowing sending an object of a specific type into a message queue and receiving on the other end. To guarantee data ownership, internally std::unique_ptr is used. 
Example : 
```cpp
import thread_plus;

... 

struct BigObject {
  /*
    Some object with a lot of things. 
  */
};
auto channel = thread_plus::channel::Channel<BigObject>{};
channel.send(BigObject{});

```
