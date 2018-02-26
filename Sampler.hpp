#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct SamplerDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::Sampler const &sampler) const
    {
      if (device)
	device.destroySampler(sampler);
    }
  };
  
  template<class Deleter = SamplerDeleter>
  using Sampler = claws::Handle<vk::Sampler, Deleter>;

  inline auto DeviceImpl::createSampler(vk::Filter magFilter, vk::Filter minFilter, vk::Sampler::MipmapMode mipmapMode, vk::Sampler::AddressMode adressModeU, vk::Sampler::AddressMode adressModeV, vk::Sampler::AddressMode adressModeW, float mipLoadBias, vk::Bool32 anisotropyEnable, float maxAnisotropy, vk::Bool32 compareEnable, vk::CompareOp compareOp, float minLod, float maxLod, vk::BorderColor borderColor, vk::Bool32 unnormalizedCoordinates) const
  {
    return Sampler<>(SamplerDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createSampler({0,
	    magFilter, minFilter, mipmapMode, adressModeU, adressModeV, adressModeW,  mipLoadBias, anisotropyEnable,
	    maxAnisotropy, compareEnable, compareOp, minLod, maxLod, borderColor, unnormalizedCoordinates
	    }));
  }
};
