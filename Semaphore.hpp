#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct SemaphoreDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::Semaphore const &fence) const
    {
      if (device)
        device.destroySemaphore(fence);
    }
  };

  template<class Deleter = SemaphoreDeleter>
  using Semaphore = claws::handle<vk::Semaphore, Deleter>;

  inline auto DeviceImpl::createSemaphore() const
  {
    return Semaphore<>(SemaphoreDeleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createSemaphore({}));
  }
};
