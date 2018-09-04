#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct Deleter
  {
    vk::Device device;

    template<class T>
    void operator()(T const &obj) const
    {
      if (device)
	device.destroy(obj);
    }
  };
};
