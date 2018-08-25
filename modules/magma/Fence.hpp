#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using Fence = claws::handle<vk::Fence, Deleter>;

  inline auto impl::Device::createFence(vk::FenceCreateFlags flags) const
  {
    return Fence<>(Deleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createFence({flags}));
  }
};
