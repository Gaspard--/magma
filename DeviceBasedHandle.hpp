#pragma once

#include <vector>
#include <unordered_map>

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

    DeviceBasedHandleImpl<Data>(Data &&data, Device<NoDelete> device)
      : Data(std::move(data))
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

  // template<class Data>
  // class DeviceBasedGroupeHandleImpl
  // {
  // protected:
  //   std::unordered_map<Device<NoDelete>, std::vector<Data>> data;

  // public:
  //   DeviceBasedGroupeHandleImpl<Data>() = default;
  //   DeviceBasedGroupeHandleImpl<Data>(DeviceBasedGroupeHandleImpl<Data> &&) = default;

  //   template<class It>
  //   DeviceBasedGroupeHandleImpl<Data>(It begin, It end)
  //   {
  //     insert(begin, end);
  //   }

  //   template<class Func>
  //   void doFunc(Func &&func) const
  //   {
  //     for (std::pair<Device<NoDelete>, std::vector<Data>> const &pair : data)
  // 	func(pair.first, pair.second);
  //   }

  //   template<class It>
  //   void insert(It begin, It end)
  //   {
  //     for (auto it(begin); it != end; ++it)
  // 	data[it->device].push_back(static_cast<Data>(*it));
  //   }

  //   auto operator[](Device<NoDelete> device, std::size_t index) const
  //   {
  //     return data[device][index];
  //   }
  // };
};
