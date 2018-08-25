#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using Event = claws::handle<vk::Event, Deleter>;

  inline auto impl::Device::createEvent(void) const
  {
    Event<>(Deleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createEvent({}));
  }
};
