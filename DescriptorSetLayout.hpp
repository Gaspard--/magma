#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorSetLayoutDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::DescriptorSetLayout const &descriptorSetLayout) const
    {
      if (device)
	device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
  };
  
  template<class Deleter = DescriptorSetLayoutDeleter>
  using DescriptorSetLayout = claws::Handle<vk::DescriptorSetLayout, Deleter>;

  inline auto DeviceImpl::createDescriptorSetLayout(vk::DescriptorSetLayoutCreateFlags flags, std::vector<vk::DescriptorSetLayoutBinding> const &bindings) const
  {
    return DescriptorSetLayout<>(DescriptorSetLayoutDeleter{magma::Device<claws::NoDelete>(*this)},
				 vk::Device::createDescriptorSetLayout({flags,
				       static_cast<uint32_t>(bindings.size()),
				       bindings.data()}));
  }
};
