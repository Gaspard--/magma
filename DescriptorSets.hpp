#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorSetsDeleter
  {
    Device<claws::NoDelete> device;
    vk::DescriptorPool pool;

    void operator()(std::vector<vk::DescriptorSet> const &descriptorSets) const
    {
      if (device)
	device.freeDescriptorSets(pool, descriptorSets);
    }
  };

  template<class Deleter = DescriptorSetsDeleter>
  using DescriptorSets = claws::Handle<std::vector<vk::DescriptorSet>, Deleter>;

  inline auto DeviceImpl::createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> const &setLayout) const
  {
    return DescriptorSets<>(DescriptorSetsDeleter{magma::Device<claws::NoDelete>(*this), descriptorPool},
			    vk::Device::allocateDescriptorSets({descriptorPool,
				  static_cast<uint32_t>(setLayout.size()),
				  setLayout.data()}));
  }
  
};
