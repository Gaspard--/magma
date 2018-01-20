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

Uses libclaws. https://github.com/Gaspard--/libclaws


## Usage

Magma expects to be able to access itself with `magma/*.hpp` includes.
Magma expects to be able to access libclaws with `claws/*.hpp` includes.
