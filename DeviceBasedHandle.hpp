#pragma once

#include "magma/Device.hpp"

namespace magma
{
  template<class Data>
  class DeviceBasedHandleImpl : protected Data
  {
  protected:
    Device<NoDelete> device;

    ~DeviceBasedHandleImpl<Data>() = default;

  public:
    constexpr DeviceBasedHandleImpl<Data>() noexcept
      : Data(nullptr)
      , device(nullptr)
    {
    }

    DeviceBasedHandleImpl<Data>(Data data, Device<NoDelete> device)
      : Data(data)
      , device(device)
    {
    }

    constexpr void swap(DeviceBasedHandleImpl<Data> &other) noexcept
    {
      using std::swap;

      swap(static_cast<Data &>(*this), static_cast<Data &>(other));
      swap(device, other.device);
    }
  };

  template<class Data>
  constexpr inline void swap(DeviceBasedHandleImpl<Data> &lh, DeviceBasedHandleImpl<Data> &rh) noexcept
  {
    lh.swap(rh);
  }
};
