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

  inline auto DeviceImpl::createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> const &setLayout, std::vector<vk::WriteDescriptorSet> const &wDescriptorSet) const
  {
    auto descriptorSet = DescriptorSets<>(DescriptorSetsDeleter{magma::Device<claws::NoDelete>(*this), descriptorPool},
					  vk::Device::allocateDescriptorSets({descriptorPool,
						static_cast<uint32_t>(setLayout.size()),
						setLayout.data()}));
    vk::Device::updateDescriptorSets(static_cast<uint32_t>(wDescriptorSet.size()),
				    wDescriptorSet.data(),
				    0, nullptr);
    return descriptorSet;
  }
  
  inline auto DeviceImpl::createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> const &setLayout, std::vector<vk::CopyDescriptorSet> const &cDescriptorSet) const
  {
    auto descriptorSet = DescriptorSets<>(DescriptorSetsDeleter{magma::Device<claws::NoDelete>(*this), descriptorPool},
					  vk::Device::allocateDescriptorSets({descriptorPool,
						static_cast<uint32_t>(setLayout.size()),
						setLayout.data()}));
    vk::Device::updateDescriptorSets(0, nullptr,
				    static_cast<uint32_t>(cDescriptorSet.size()),
				    cDescriptorSet.data());
    return descriptorSet;
  }
  
};
