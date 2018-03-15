#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct FenceDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::Fence const &fence) const
    {
      if (device)
        device.destroyFence(fence);
    }
  };

  template<class Deleter = FenceDeleter>
  using Fence = claws::handle<vk::Fence, Deleter>;

  inline auto DeviceImpl::createFence(vk::FenceCreateFlags flags) const
  {
    return Fence<>(FenceDeleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createFence({flags}));
  }
};
