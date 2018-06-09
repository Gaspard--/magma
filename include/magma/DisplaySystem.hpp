#pragma once

#include <memory>
#include <fstream>
#include <iostream>
#include <mutex>

#include "magma/VulkanHandler.hpp"
#include "magma/Device.hpp"
#include "magma/Swapchain.hpp"
#include "magma/CommandBuffer.hpp"
#include "magma/Semaphore.hpp"
#include "magma/ImageView.hpp"
#include "magma/CreateInfo.hpp"

namespace magma {
  ///
  /// A class that helps with swapchain-related data segmentation
  ///
  /// @tparam UserData will ba available on swapchain creation
  /// @tparam SwachainUserData will be recreated everytime the swapchain gets recreated
  /// @tparam FrameUserData will be (re)created for each swapchain image
  ///
  /// `UserData` must have a member `UserData::getExtent()` returning a value convertible to vk::Extent2D
  template<class UserData, class SwapchainUserData, class FrameUserData>
  class DisplaySystem
  {
  private:
    struct FrameData
    {
      magma::ImageView<> swapchainImageView;
      FrameUserData userData;

      FrameData(magma::Device<claws::no_delete> device, magma::Swapchain<claws::no_delete> swapchain, UserData &userData, SwapchainUserData &swapchainUserData, vk::Image swapchainImage)
	: swapchainImageView(device.createImageView({},
						    swapchainImage,
						    vk::ImageViewType::e2D,
						    swapchain.getFormat(),
						    {vk::ComponentSwizzle::eIdentity,
							vk::ComponentSwizzle::eIdentity,
							vk::ComponentSwizzle::eIdentity,
							vk::ComponentSwizzle::eIdentity},
						    {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}))
	, userData(device, swapchain, userData, swapchainUserData, swapchainImageView)
      {
      }	
    };

    vk::PhysicalDevice physicalDevice;
    magma::Surface<claws::no_delete> surface;
    magma::Device<claws::no_delete> device;
    vk::Queue queue;
  public:
    UserData userData;
  private:
    magma::Swapchain<> swapchain;
  public:
    SwapchainUserData swapchainUserData;
  private:
    std::vector<FrameData> frames;

    template<class T, class = void>
    struct CallExtentOrReturnDefault
    {
      vk::Extent2D operator()(T const &)
      {
	return vk::Extent2D{0xFFFFFFFF, 0xFFFFFFFF};
      }
    };

    template<class T>
    struct CallExtentOrReturnDefault<T, decltype(std::declval<T>().getExtent(), void(0))>
    {
      vk::Extent2D operator()(T const &t)
      {
	return t.getExtent();
      }
    };


  public:
    DisplaySystem(DisplaySystem const &) = delete;
    DisplaySystem(DisplaySystem &&) = delete;

    template<class... T>
    explicit DisplaySystem(vk::PhysicalDevice physicalDevice,
			   magma::Surface<claws::no_delete> surface,
			   magma::Device<claws::no_delete> device,
			   vk::Queue queue,
			   uint32_t queueFamilyIndex,
			   T &&... userDataParams)
      : physicalDevice(physicalDevice)
      , surface(surface)
      , device(device)
      , queue(queue)
      , userData(device, physicalDevice, queueFamilyIndex, std::forward<T>(userDataParams)...)
    {
      recreateSwapchain();
    }

    void recreateSwapchain()
    {
      swapchain = magma::Swapchain<>(surface, device, physicalDevice, swapchain, CallExtentOrReturnDefault<UserData>{}(userData));
      device.waitIdle();
      auto const &swapchainImages(swapchain.getImages());
      swapchainUserData = SwapchainUserData(device, swapchain, userData, static_cast<uint32_t>(swapchainImages.size()));

      frames.clear();
      frames.reserve(swapchainImages.size());

      for (uint32_t i(0u); i < swapchainImages.size(); ++i)
	{
	  static_assert(std::is_same_v<decltype((userData)), UserData &> && !std::is_const_v<UserData>);

	  frames.emplace_back(device, swapchain, userData, swapchainUserData, swapchainImages[i]);
	}
    }

    std::pair<uint32_t, FrameUserData &>  getImage(magma::Semaphore<claws::no_delete> toBeSignaled)
    {
    retry:
      auto[result, index] = swapchain.getImageIndex(toBeSignaled, ~0ul, nullptr);

      switch (result)
	{
	case vk::Result::eSuccess:
	case vk::Result::eSuboptimalKHR:
	  break;
	case vk::Result::eErrorOutOfDateKHR:
	  recreateSwapchain();
	  goto retry;
	default:
	  throw std::runtime_error("Problem occurred during swap chain image acquisition!");
	};
      return {index, frames[index].userData};
    }

    void presentImage(magma::Semaphore<claws::no_delete> wait, uint32_t index)
    {
      VkPresentInfoKHR presentInfoRaw(magma::StructBuilder<vk::PresentInfoKHR>::make
				      (magma::asListRef(wait), magma::asListRef(swapchain.raw()), &index, nullptr));
      switch (vk::Result(vkQueuePresentKHR(queue, &presentInfoRaw)))
	{
	case vk::Result::eSuccess:
	  break;
	case vk::Result::eSuboptimalKHR:
	case vk::Result::eErrorOutOfDateKHR:
	  recreateSwapchain();
	  break;
	default:
	  throw std::runtime_error("Problem occurred during image presentation!");
	}
    }

    magma::Swapchain<claws::no_delete> getSwapchain()
    {
      return swapchain;
    }

    ~DisplaySystem()
    {
      device.waitIdle();
    }
  };
}
