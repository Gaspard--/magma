#pragma once

#include "magma/Device.hpp"
#include "magma/RenderPass.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::Framebuffer>>
  using Framebuffer = claws::handle<vk::Framebuffer, Deleter>;

  inline auto impl::Device::createFramebuffer(
    RenderPass<claws::no_delete> renderPass, std::vector<vk::ImageView> const &attachements, uint32_t width, uint32_t height, uint32_t layers) const
  {
    return magma::Framebuffer<>(Deleter<vk::Framebuffer>{
	magma::Device<claws::no_delete>(*this)},
      vk::Device::createFramebuffer({{}, renderPass,
					   static_cast<uint32_t>(attachements.size()),
					   attachements.data(), width, height, layers}));
  }
};
