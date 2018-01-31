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

  inline auto DeviceImpl::selectAndCreateDeviceMemory(vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::MemoryPropertyFlagBits memoryFlags) const
  {
    auto const memProperties(physicalDevice.getMemoryProperties());
    
    auto it(std::find_if(memProperties.memoryTypes, memProperties.memoryTypes + memProperties.memoryTypeCount, [&memProperties, size, memoryFlags](auto const &type)
			 {
			   return (type.propertyFlags & memoryFlags) && memProperties.memoryHeaps[type.heapIndex].size >= size;
			 }));
    if (it == memProperties.memoryTypes + memProperties.memoryTypeCount)
      throw std::runtime_error("Couldn't find proper memory type");
    return createDeviceMemory(size, it - memProperties.memoryTypes);
  }
};
