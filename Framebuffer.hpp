#pragma once

#include "magma/Device.hpp"

namespace magma
{
  class FramebufferImpl : protected vk::Framebuffer
  {
  protected:
    Device<NoDelete> device;

    ~FramebufferImpl() = default;

  public:
    FramebufferImpl()
      : vk::Framebuffer(nullptr)
      , device(nullptr)
    {
    }
    
    FramebufferImpl(RenderPass<NoDelete> renderPass, std::vector<vk::ImageView> const attachements, uint32_t width, uint32_t height, uint32_t layers)
      : vk::Framebuffer(device.createFramebuffer({{}, renderPass.raw(), static_cast<uint32_t>(attachements.size()), attachements.data(), width, height, layers}))
    {
    }

    auto raw() const
    {
      return static_cast<vk::Framebuffer>(*this);
    }

    void swap(FramebufferImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::Framebuffer &>(*this), static_cast<vk::Framebuffer &>(other));
      swap(device, other.device);
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

  inline void swap(FramebufferImpl &lh, FramebufferImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = FramebufferImpl::Deleter>
  using Framebuffer = Handle<FramebufferImpl, Deleter>;
};
