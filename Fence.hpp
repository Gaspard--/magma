#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct FenceDeleter
  {
    Device<NoDelete> device;

    void operator()(vk::Fence const &fence) const
    {
      if (device)
	device.destroyFence(fence);
    }
  };
  
  template<class Deleter = FenceDeleter>
  using Fence = Handle<vk::Fence, Deleter>;

  inline auto DeviceImpl::createFence(vk::FenceCreateFlags flags) const
  {
    return Fence<>(FenceDeleter{magma::Device<NoDelete>(*this)}, vk::Device::createFence(flags));
  }
};
