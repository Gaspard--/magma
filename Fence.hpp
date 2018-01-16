#pragma once

#include <vector>

#include "magma/Device.hpp"

namespace magma
{
  class FenceImpl : protected vk::Fence
  {
  protected:
    Device<NoDelete> device;

    ~FenceImpl() = default;

  public:
    FenceImpl()
      : vk::Fence(nullptr)
      , device(nullptr)
    {
    }

    FenceImpl(Device<NoDelete> device, vk::FenceCreateFlags flags)
      : vk::Fence(device.createFence(flags))
      , device(device)
    {
    }

    auto getStatus() const
    {
      return device.getFenceStatus(static_cast<vk::Fence>(*this));
    }

    void reset() const
    {
      device.resetFences(1, this);
    }

    void swap(FenceImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::Fence &>(*this), static_cast<vk::Fence &>(other));
      swap(device, other.device);
    }

    struct Deleter
    {
      friend class FenceImpl;

      void operator()(FenceImpl const &fence) const
      {
	if (fence.device)
	  fence.device.destroyFence(static_cast<vk::Fence>(fence));
      }
    };
  };

  inline void swap(FenceImpl &lh, FenceImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = FenceImpl::Deleter>
  using Fence = Handle<FenceImpl, Deleter>;
};
