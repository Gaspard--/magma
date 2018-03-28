#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct EventDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::Event const &event) const
    {
      if (device)
        device.destroyEvent(event);
    }
  };

  template<class Deleter = EventDeleter>
  using Event = claws::Handle<vk::Event, Deleter>;

  inline auto impl::Device::createEvent(void) const
  {
    Event<>(EventDeleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createEvent({}));
  }
};
