#pragma once
#include <climits>

namespace magma
{
  class ShaderModuleImpl : protected DeviceBasedHandleImpl<vk::ShaderModule>
  {
    static auto inputStreamToUint32Vect(std::istream &input)
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
  protected:
    ~ShaderModuleImpl() = default;

  public:
    ShaderModuleImpl() = default;

    template<class Container>
    ShaderModuleImpl(Device<NoDelete> device, Container const &code)
      : DeviceBasedHandleImpl<vk::ShaderModule>(device.createShaderModule(vk::ShaderModuleCreateInfo{{}, code.size() * sizeof(uint32_t), code.data()}), device)
    {
    }

    ShaderModuleImpl(Device<NoDelete> device, std::istream &input)
      : ShaderModuleImpl(device, inputStreamToUint32Vect(input))
    {
    }

    struct Deleter
    {
      friend class ShaderModuleImpl;
      void operator()(ShaderModuleImpl const &shaderModule)
      {
	if (shaderModule)
	  shaderModule.device.destroyShaderModule(static_cast<vk::ShaderModule>(shaderModule));
      }
    };
  };

  template<class Deleter = ShaderModuleImpl::Deleter>
  using ShaderModule = Handle<ShaderModuleImpl, Deleter>;
};
