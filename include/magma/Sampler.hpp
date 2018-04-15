#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using Sampler = claws::handle<vk::Sampler, Deleter>;

  inline auto impl::Device::createSampler(vk::Filter magFilter,
                                          vk::Filter minFilter,
                                          vk::SamplerMipmapMode mipmapMode,
                                          vk::SamplerAddressMode adressModeU,
                                          vk::SamplerAddressMode adressModeV,
                                          vk::SamplerAddressMode adressModeW,
                                          float mipLoadBias,
                                          vk::Bool32 anisotropyEnable,
                                          float maxAnisotropy,
                                          vk::Bool32 compareEnable,
                                          vk::CompareOp compareOp,
                                          float minLod,
                                          float maxLod,
                                          vk::BorderColor borderColor,
                                          vk::Bool32 unnormalizedCoordinates) const
  {
    return Sampler<>(Deleter{magma::Device<claws::no_delete>(*this)},
                     vk::Device::createSampler({{},
                                                magFilter,
                                                minFilter,
                                                mipmapMode,
                                                adressModeU,
                                                adressModeV,
                                                adressModeW,
                                                mipLoadBias,
                                                anisotropyEnable,
                                                maxAnisotropy,
                                                compareEnable,
                                                compareOp,
                                                minLod,
                                                maxLod,
                                                borderColor,
                                                unnormalizedCoordinates}));
  }
};
