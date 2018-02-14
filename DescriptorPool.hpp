#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorPoolDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::DescriptorPool const &descriptorPool) const
    {
      if (device)
	device.destroyDescriptorPool(descriptorPool);
    }
  };

  template<class Deleter = DescriptorPoolDeleter>
  using DescriptorPool = claws::Handle<vk::DescriptorPool, Deleter>;

  inline auto DeviceImpl::createDescriptorPool(std::uint32_t maxSets, std::vector<vk::DescriptorPoolSize> const &size) const
  {
    return DescriptorPool<>(DescriptorPoolDeleter{magma::Device<claws::NoDelete>(*this)},
			    vk::Device::createDescriptorPool({vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				  maxSets,
				  static_cast<uint32_t>(size.size()),
				  size.data()}));
  }
};
