#pragma once

#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::Fence>>
  using Fence = claws::handle<vk::Fence, Deleter>;

  inline auto impl::Device::createFence(vk::FenceCreateFlags flags) const
  {
    return Fence<>(Deleter<vk::Fence>{magma::Device<claws::no_delete>(*this)}, vk::Device::createFence({flags}));
  }
};
