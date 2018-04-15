#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct Deleter
  {
    Device<claws::no_delete> device;

    template<class T>
    void operator()(T const &obj) const
    {
      if (device)
	device.destroy(obj);
    }
  };
};
