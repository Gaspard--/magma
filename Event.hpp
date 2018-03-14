#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct EventDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::Event const &event) const
    {
      if (device)
        device.destroyEvent(event);
    }
  };

  template<class Deleter = EventDeleter>
  using Event = claws::Handle<vk::Event, Deleter>;

  inline auto DeviceImpl::createEvent(void) const
  {
    Event<>(EventDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createEvent({}));
  }
};
