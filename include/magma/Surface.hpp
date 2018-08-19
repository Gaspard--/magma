#pragma once

#include <claws/utils/handle_types.hpp>

namespace magma
{
  namespace impl
  {
    class Surface
    {
    public:
      vk::SurfaceKHR vkSurface;

      Surface() = default;

      Surface(vk::SurfaceKHR vkSurface) noexcept
	: vkSurface(vkSurface)
      {}

      ~Surface() = default;

      bool isQueueFamilySuitable(vk::PhysicalDevice physicalDevice, uint32_t queueIndex) const
      {
	return physicalDevice.getSurfaceSupportKHR(queueIndex, vkSurface);
      }
    };

    class SurfaceDeleter
    {
      vk::Instance instance;

    public:
      SurfaceDeleter(vk::Instance instance) noexcept
	: instance(instance)
      {
      }

      void operator()(Surface surface)
      {
	if (instance)
	  instance.destroySurfaceKHR(surface.vkSurface);
      }
    };
  }
  
  template<class Deleter = impl::SurfaceDeleter>
  using Surface = claws::handle<impl::Surface, Deleter>;

  inline Surface<> makeSurface(magma::Instance const &instance, vk::SurfaceKHR vkSurface)
  {
    return Surface<>{{instance.vkInstance}, vkSurface};
  }
}
