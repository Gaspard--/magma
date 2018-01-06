# magma
#### A vulkan library

### Goals

Library Features:
 - Core Vulkan for rendering
 - None intrusive Swapchain support with glfw
 - Device and Format selection

Library Style:
 - Proper RAII support, at the cost of minor performance and memory
 - No virtual classes or function pointers, virutal inheritence is out
 - `std::variants` are to be avoided unless they bring huge uability benefits (none in sight yet)
 - Exceptions, not return codes
 - `constexpr` all the things that make sense to be constexpr

### Dependencies

Uses an unamed utils library. Give me some time to create that repo
