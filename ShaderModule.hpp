#pragma once
#include <climits>

#include "magma/Device.hpp"

namespace magma
{
  inline static auto inputStreamToUint32Vect(std::istream &input)
  {
    std::vector<uint32_t> out{};
    constexpr uint32_t MAX_SHIFT(sizeof(uint32_t) * CHAR_BIT);
    uint32_t shift(0u);

    for (auto it = std::istreambuf_iterator(input); it.equal({}); ++it)
      {
	if (!(shift & (MAX_SHIFT - 1u)))
	  out.emplace_back(0u);
	out.back() += static_cast<uint32_t>(*it) << shift;
	(shift += CHAR_BIT) &= (MAX_SHIFT - 1u);
      }
    return out;
  }

  struct ShaderModuleDestructor
  {
    magma::Device<claws::NoDelete> device;

    void operator()(vk::ShaderModule const &shaderModule) const
    {
      if (shaderModule)
	device.destroyShaderModule(shaderModule);
    }
  };

  template<class Deleter = ShaderModuleDestructor>
  using ShaderModule = claws::Handle<vk::ShaderModule, Deleter>;

  template<class Container>
  inline auto DeviceImpl::createShaderModule(Container const &code) const
  {
    return ShaderModule<>(std::move(ShaderModuleDestructor{magma::Device<claws::NoDelete>(*this)}), vk::Device::createShaderModule(vk::ShaderModuleCreateInfo{{}, code.size() * sizeof(uint32_t), code.data()}));
  }

  inline auto DeviceImpl::createShaderModule(std::istream &input) const
  {
    return createShaderModule(inputStreamToUint32Vect(input));
  }
};
