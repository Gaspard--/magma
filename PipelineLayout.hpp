#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct PipelineLayoutDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::PipelineLayout const &fence) const
    {
      if (device)
	device.destroyPipelineLayout(fence);
    }
  };
  
  template<class Deleter = PipelineLayoutDeleter>
  using PipelineLayout = claws::Handle<vk::PipelineLayout, Deleter>;

  inline auto DeviceImpl::createPipelineLayout(vk::PipelineLayoutCreateFlags flags, std::vector<vk::DescriptorSetLayout> const &setLayouts, std::vector<vk::PushConstantRange> const &pushConstantRanges) const
  {
    return PipelineLayout<>(PipelineLayoutDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createPipelineLayout({flags,
	    static_cast<uint32_t>(setLayouts.size()), setLayouts.data(),
	    static_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data()
	    }));
  }
};
