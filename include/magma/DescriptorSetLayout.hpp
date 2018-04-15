#pragma once


#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using DescriptorSetLayout = claws::handle<vk::DescriptorSetLayout, Deleter>;

  inline auto impl::Device::createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> const &bindings) const
  {
    return DescriptorSetLayout<>(Deleter{magma::Device<claws::no_delete>(*this)},
				 vk::Device::createDescriptorSetLayout({{},
				       static_cast<uint32_t>(bindings.size()),
					 bindings.data()}));
  }
};
