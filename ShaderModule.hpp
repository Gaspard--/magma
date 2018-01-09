#pragma once
#include <climits>

namespace magma
{
  class ShaderModuleImpl : protected vk::ShaderModule
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
    Device<NoDelete> device;

    ~ShaderModuleImpl() = default;
  public:
    ShaderModuleImpl()
      : vk::ShaderModule(nullptr)
      , device(nullptr)
    {
    }

    template<class Container>
    ShaderModuleImpl(Device<NoDelete> device, Container const &code)
      : vk::ShaderModule(device.createShaderModule(vk::ShaderModuleCreateInfo{{}, code.size(), code.data()}))
      , device(device)
    {
    }

    ShaderModuleImpl(Device<NoDelete> device, std::istream &input)
      : ShaderModuleImpl(device, inputStreamToUint32Vect(input))
    {
    }

    void swap(ShaderModuleImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::ShaderModule &>(*this), static_cast<vk::ShaderModule &>(other));
      swap(device, other.device);
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

  inline void swap(ShaderModuleImpl &lh, ShaderModuleImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = ShaderModuleImpl::Deleter>
  using ShaderModule = Handle<ShaderModuleImpl, Deleter>;
};
