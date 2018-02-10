#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DeviceMemoryDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::DeviceMemory const &fence) const
    {
      if (device)
	device.freeMemory(fence);
    }
  };
  
  template<class Deleter = DeviceMemoryDeleter>
  using DeviceMemory = claws::Handle<vk::DeviceMemory, Deleter>;

  inline auto DeviceImpl::createDeviceMemory(vk::DeviceSize size, uint32_t typeIndex) const
  {
    return DeviceMemory<>(DeviceMemoryDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::allocateMemory({size, typeIndex}));
  }

  inline auto DeviceImpl::selectAndCreateDeviceMemory(vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::MemoryPropertyFlags memoryFlags, uint32_t memoryTypeIndexMask) const
  {
    auto const memProperties(physicalDevice.getMemoryProperties());

    for (uint32_t i(0u); i < memProperties.memoryTypeCount; ++i)
      {
	auto const &type(memProperties.memoryTypes[i]);

	if (((memoryTypeIndexMask >> i) & 1u) && (type.propertyFlags & memoryFlags) && memProperties.memoryHeaps[type.heapIndex].size >= size)
	  return createDeviceMemory(size, i);
      }
    throw std::runtime_error("Couldn't find proper memory type");
  }
};
