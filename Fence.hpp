#pragma once

#include <vector>

#include "magma/Device.hpp"
#include "magma/DeviceBasedHandle.hpp"

namespace magma
{
  class FenceImpl : protected DeviceBasedHandleImpl<vk::Fence>
  {
  protected:
    ~FenceImpl() = default;

  public:
    FenceImpl() = default;

    FenceImpl(Device<NoDelete> device, vk::FenceCreateFlags flags)
      : DeviceBasedHandleImpl<vk::Fence>(vk::Fence(device.createFence(flags)), device)
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

  template<class Deleter = FenceImpl::Deleter>
  using Fence = Handle<FenceImpl, Deleter>;
};
