#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::Event>>
  using Event = claws::handle<vk::Event, Deleter>;

  inline auto impl::Device::createEvent(void) const
  {
    Event<>(Deleter<vk::Event>{magma::Device<claws::no_delete>(*this)}, vk::Device::createEvent({}));
  }
};
