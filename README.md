# magma
#### A vulkan library

### Goals

Library Features:
 - Core Vulkan for rendering
 - None intrusive Swapchain support
 - Device and Format selection

Library Style:
 - Header only
 - Proper RAII support, at the cost of minor performance and memory
 - No `virtual` classe inheritence or function pointers
 - `std::variants` are to be avoided unless they bring sizable usability benefits (none in sight yet)
 - Exceptions, not return codes
 - `constexpr` all the things that make sense to be constexpr

### Dependencies

Uses claws library. https://github.com/Gaspard--/claws


## Usage

Magma expects to be able to access itself with `magma/*.hpp` includes.
Magma expects to be able to access claws with `claws/*.hpp` includes.
