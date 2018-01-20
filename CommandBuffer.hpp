#pragma once

#include "claws/ContextfulContainer.hpp"
#include "vulkan/vulkan.hpp"
#include "magma/Device.hpp"
#include "magma/Framebuffer.hpp"
#include "magma/RenderPass.hpp"
#include "magma/Pipeline.hpp"

namespace magma
{
  struct CommandBufferGroupDeleter
  {
    Device<claws::NoDelete> device;
    vk::CommandPool commandPool;

    template<class ContiguousContainer>
    constexpr void operator()(ContiguousContainer const &container) const
    {
      if (!container.empty())
        device.freeCommandBuffers(commandPool, container.size(), container.data());
    }
  };

  template<class Type>
  using VectorAlias = std::vector<Type>;

  template<class Type>
  using VectorAlias = std::vector<Type>;

  template<class CommandBufferType, class Deleter = CommandBufferGroupDeleter, template <class Type> typename ContiguousContainer = VectorAlias>
  using CommandBufferGroup = claws::GroupHandle<CommandBufferType, ContiguousContainer<vk::CommandBuffer>, Deleter>;

  class CommandBuffer
  {
  protected:
    vk::CommandBuffer commandBuffer;

  public:
    CommandBuffer(vk::CommandBuffer commandBuffer)
      : commandBuffer(commandBuffer)
    {
    }

    void end() const
    {
      commandBuffer.end();
    }

    void reset(vk::CommandBufferResetFlags flags) const
    {
      commandBuffer.reset(flags);
    }
  };

  class SecondaryCommandBuffer : public CommandBuffer
  {
    void begin(vk::CommandBufferUsageFlags flags,
	       vk::CommandBufferInheritanceInfo const &pInheritanceInfo) const
    {
      commandBuffer.begin(vk::CommandBufferBeginInfo{flags, &pInheritanceInfo});
    }
  };

  class PrimaryCommandBuffer;
  
  struct RenderPassExecLock
  {
  protected:
    vk::CommandBuffer commandBuffer;

    friend class PrimaryCommandBuffer;

    RenderPassExecLock(vk::CommandBuffer commandBuffer)
      : commandBuffer(commandBuffer)
    {}
  public:
    RenderPassExecLock()
      : commandBuffer(nullptr)
    {}

    RenderPassExecLock(RenderPassExecLock const &) = delete;

    RenderPassExecLock(RenderPassExecLock &&other)
      : commandBuffer(other.commandBuffer)
    {
      other.commandBuffer = nullptr;
    }

    auto &operator=(RenderPassExecLock other)
    {
      using std::swap;

      swap(commandBuffer, other.commandBuffer);
      return *this;
    }

    void next(vk::SubpassContents contents) const
    {
      commandBuffer.nextSubpass(contents);
    }

    ~RenderPassExecLock()
    {
      commandBuffer.endRenderPass();
    }
  };

  class PrimaryCommandBuffer : public CommandBuffer
  {
    void begin(vk::CommandBufferUsageFlags flags) const
    {
      commandBuffer.begin(vk::CommandBufferBeginInfo{flags, nullptr});
    }

    void execBuffers(CommandBufferGroup<SecondaryCommandBuffer> const &secondaryCommands) const
    {
      commandBuffer.executeCommands(static_cast<std::vector<vk::CommandBuffer> const &>(secondaryCommands));
    }

    auto beginRenderPass(RenderPass<claws::NoDelete> renderpass, Framebuffer<claws::NoDelete> framebuffer, vk::Rect2D renderArea, std::vector<vk::ClearValue> const &clearValues, vk::SubpassContents contents) const
    {
      commandBuffer.beginRenderPass({renderpass, framebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()}, contents);

      return RenderPassExecLock{commandBuffer};
    }

    auto bindGraphicsPipeline(Pipeline<claws::NoDelete> pipeline) const
    {
      commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }
  };

  class CommandPoolImpl : public vk::CommandPool
  {
  protected:
    Device<claws::NoDelete> device;

    ~CommandPoolImpl() = default;
  public:
    CommandPoolImpl()
      : vk::CommandPool(nullptr)
      , device(nullptr)
    {
    }

    CommandPoolImpl(Device<claws::NoDelete> device, vk::CommandPool commandPool)
      : vk::CommandPool(commandPool)
      , device(device)
    {
    }

    void reset(vk::CommandPoolResetFlags flags)
    {
      device.resetCommandPool(*this, flags);
    }

    auto allocatePrimaryCommandBuffers(uint32_t commandBufferCount)
    {
      vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::ePrimary, commandBufferCount};

      return CommandBufferGroup<PrimaryCommandBuffer>{{device, *this}, device.allocateCommandBuffers(info)};
    }

    auto allocateSecondaryCommandBuffers(uint32_t commandBufferCount)
    {
      vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::eSecondary, commandBufferCount};

      return CommandBufferGroup<SecondaryCommandBuffer>{{device, *this}, device.allocateCommandBuffers(info)};
    }

    void swap(CommandPoolImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::CommandPool &>(*this), static_cast<vk::CommandPool &>(other));
      swap(device, other.device);
    }

    struct CommandPoolDeleter
    {
      friend CommandPoolImpl;

      void operator()(CommandPoolImpl const &commandPool) const
      {
	if (commandPool)
	  commandPool.device.destroyCommandPool(commandPool);
      }
    };
  };

  void swap(CommandPoolImpl &lh, CommandPoolImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = CommandPoolImpl::CommandPoolDeleter>
  using CommandPool = claws::Handle<CommandPoolImpl, Deleter>;

  inline auto DeviceImpl::createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex) const
  {
    vk::CommandPoolCreateInfo createInfo{flags, queueFamilyIndex};

    return CommandPool<>{magma::Device<claws::NoDelete>(*this), vk::Device::createCommandPool(createInfo)};
  }
};
