#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorSetLayoutDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::DescriptorSetLayout const &descriptorSetLayout) const
    {
      if (device)
        device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
  };

  template<class Deleter = DescriptorSetLayoutDeleter>
  using DescriptorSetLayout = claws::handle<vk::DescriptorSetLayout, Deleter>;

  inline auto DeviceImpl::createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> const &bindings) const
  {
    return DescriptorSetLayout<>(DescriptorSetLayoutDeleter{magma::Device<claws::no_delete>(*this)},
                                 vk::Device::createDescriptorSetLayout({{}, static_cast<uint32_t>(bindings.size()), bindings.data()}));
  }
};
