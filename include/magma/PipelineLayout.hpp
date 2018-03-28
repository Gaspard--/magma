#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct PipelineLayoutDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::PipelineLayout const &fence) const
    {
      if (device)
        device.destroyPipelineLayout(fence);
    }
  };

  template<class Deleter = PipelineLayoutDeleter>
  using PipelineLayout = claws::handle<vk::PipelineLayout, Deleter>;

  inline auto impl::Device::createPipelineLayout(vk::PipelineLayoutCreateFlags flags,
                                                 std::vector<vk::DescriptorSetLayout> const &setLayouts,
                                                 std::vector<vk::PushConstantRange> const &pushConstantRanges) const
  {
    return PipelineLayout<>(PipelineLayoutDeleter{magma::Device<claws::no_delete>(*this)},
                            vk::Device::createPipelineLayout({flags,
                                                              static_cast<uint32_t>(setLayouts.size()),
                                                              setLayouts.data(),
                                                              static_cast<uint32_t>(pushConstantRanges.size()),
                                                              pushConstantRanges.data()}));
  }
};
