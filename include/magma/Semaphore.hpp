#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using Semaphore = claws::handle<vk::Semaphore, Deleter>;

  inline auto impl::Device::createSemaphore() const
  {
    return Semaphore<>(Deleter{magma::Device<claws::no_delete>(*this)},
		       vk::Device::createSemaphore({}));
  }
};
