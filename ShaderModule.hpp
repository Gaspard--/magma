#pragma once
#include <climits>

#include <numeric>
#include <iomanip>
#include "magma/Device.hpp"

namespace magma
{
  inline static auto inputStreamToUint32Vect(std::istream &input)
  {
    std::vector<uint32_t> out{};
    std::array<char, sizeof(uint32_t)> data;

    input.seekg(0, std::ios::end);
    out.reserve(input.tellg() / sizeof(uint32_t));
    input.seekg(0, std::ios::beg);
    while (input.read(data.data(), data.size()))
      out.emplace_back(std::accumulate(data.rbegin(), data.rend(), 0u, [](uint32_t last, char next) {
        return (last << CHAR_BIT) + static_cast<uint32_t>(static_cast<uint8_t>(next));
      }));
    return out;
  }

  struct ShaderModuleDestructor
  {
    magma::Device<claws::no_delete> device;

    void operator()(vk::ShaderModule const &shaderModule) const
    {
      if (shaderModule)
        device.destroyShaderModule(shaderModule);
    }
  };

  template<class Deleter = ShaderModuleDestructor>
  using ShaderModule = claws::handle<vk::ShaderModule, Deleter>;

  template<class Container>
  inline auto DeviceImpl::createShaderModule(Container const &code) const
  {
    return ShaderModule<>(ShaderModuleDestructor{magma::Device<claws::no_delete>(*this)},
                          vk::Device::createShaderModule(vk::ShaderModuleCreateInfo{{}, code.size() * sizeof(uint32_t), code.data()}));
  }

  inline auto DeviceImpl::createShaderModule(std::istream &input) const
  {
    auto code(inputStreamToUint32Vect(input));

    return createShaderModule(code);
  }
};
