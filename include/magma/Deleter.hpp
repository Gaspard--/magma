#pragma once

#include "magma/Device.hpp"

namespace magma
{
  template<class T>
  struct Deleter
  {
    Device<claws::no_delete> device;

    void operator()(T const &obj) const
    {
      if (device)
	device.destroy(obj);
    }
  };
};
