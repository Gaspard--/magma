# magma
#### A vulkan library

### Goals

Library Features:
 - Core Vulkan for rendering
 - None intrusive Swapchain support
 - Device and Format selection

Library Style:
 - Proper RAII support, at the cost of minor performance and memory
 - No `virtual` classe inheritence or function pointers
 - `std::variants` are to be avoided unless they bring sizable usability benefits (none in sight yet)
 - Exceptions, not return codes
 - `constexpr` all the things that make sense to be constexpr

### Dependencies

Uses an unamed utils library. Give me some time to create that repo. Currently utils are stored here: https://github.com/Gaspard--/Wasted-Prophecies/tree/master/include/util
