#pragma once

#include "magma/DeviceBasedHandle.hpp"

namespace magma
{
  class FramebufferImpl : protected DeviceBasedHandleImpl<vk::Framebuffer>
  {
  protected:
    ~FramebufferImpl() = default;

  public:
    FramebufferImpl() = default;
    
    FramebufferImpl(Device<NoDelete> device, RenderPass<NoDelete> renderPass, std::vector<vk::ImageView> const attachements, uint32_t width, uint32_t height, uint32_t layers)
      : DeviceBasedHandleImpl<vk::Framebuffer>(device.createFramebuffer({{}, renderPass.raw(), static_cast<uint32_t>(attachements.size()), attachements.data(), width, height, layers}), device)
    {
    }

    auto raw() const noexcept
    {
      return static_cast<vk::Framebuffer>(*this);
    }

    struct Deleter
    {
      friend class FramebufferImpl;

      void operator()(FramebufferImpl const &framebuffer) const
      {
	if (framebuffer)
	  framebuffer.device.destroyFramebuffer(static_cast<vk::Framebuffer>(framebuffer));
      }
    };
  };

  template<class Deleter = FramebufferImpl::Deleter>
  using Framebuffer = Handle<FramebufferImpl, Deleter>;
};
