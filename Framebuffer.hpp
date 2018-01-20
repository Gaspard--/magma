#pragma once

#include "magma/Device.hpp"
#include "magma/RenderPass.hpp"

namespace magma
{
  struct FramebufferDeleter
  {
    Device<NoDelete> device;

    void operator()(vk::Framebuffer const &framebuffer) const
    {
      if (framebuffer)
        device.destroyFramebuffer(static_cast<vk::Framebuffer>(framebuffer));
    }
  };

  inline auto DeviceImpl::framebufferDestructor() const noexcept
  {
    return FramebufferDeleter{magma::Device<NoDelete>{*this}};
  }

  template<class Deleter = FramebufferDeleter>
  using Framebuffer = Handle<vk::Framebuffer, FramebufferDeleter>;

  inline auto DeviceImpl::createFramebuffer(RenderPass<NoDelete> renderPass, std::vector<vk::ImageView> const &attachements, uint32_t width, uint32_t height, uint32_t layers) const
  {
    return magma::Framebuffer<>(framebufferDestructor(), vk::Device::createFramebuffer({{}, renderPass, static_cast<uint32_t>(attachements.size()), attachements.data(), width, height, layers}));
  }
};
