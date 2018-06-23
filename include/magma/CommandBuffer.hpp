#pragma once

#include "claws/contextful_container.hpp"

#include "vulkan/vulkan.hpp"

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"
#include "magma/Framebuffer.hpp"
#include "magma/RenderPass.hpp"
#include "magma/Pipeline.hpp"

namespace magma
{
  struct CommandBufferGroupDeleter
  {
    Device<claws::no_delete> device;
    vk::CommandPool commandPool;

    template<class ContiguousContainer>
    constexpr void operator()(ContiguousContainer const &container) const
    {
      if (!container.empty())
        device.freeCommandBuffers(commandPool, static_cast<uint32_t>(container.size()), container.data());
    }
  };

  template<class Type>
  using VectorAlias = std::vector<Type>;

  template<class Type>
  using VectorAlias = std::vector<Type>;

  template<class CommandBufferType, class Deleter = CommandBufferGroupDeleter, template<class Type> typename ContiguousContainer = VectorAlias>
  using CommandBufferGroup = claws::group_handle<CommandBufferType, ContiguousContainer<vk::CommandBuffer>, Deleter>;

  class CommandBuffer : protected vk::CommandBuffer
  {
  protected:
  public:
    CommandBuffer(vk::CommandBuffer commandBuffer)
      : vk::CommandBuffer(commandBuffer)
    {}

    auto &raw()
    {
      return static_cast<vk::CommandBuffer &>(*this);
    }

    void end() const
    {
      vk::CommandBuffer::end();
    }

    void reset(vk::CommandBufferResetFlags flags) const
    {
      vk::CommandBuffer::reset(flags);
    }

    using vk::CommandBuffer::pipelineBarrier;

    using vk::CommandBuffer::bindVertexBuffers;
    using vk::CommandBuffer::bindIndexBuffer;

    using vk::CommandBuffer::setEvent;
    using vk::CommandBuffer::resetEvent;
    using vk::CommandBuffer::waitEvents;

    template<class Container>
    void pushConstants(claws::handle<vk::PipelineLayout, claws::no_delete> pipelineLayout,
                       vk::ShaderStageFlags shaderStages,
                       uint32_t elemOffset,
                       Container const &data)
    {
      constexpr uint32_t elemSize(sizeof(decltype(*data.data())));
      pushConstants(pipelineLayout, shaderStages, elemOffset * elemSize, static_cast<uint32_t>(elemSize * data.size()), data.data());
    }

    using vk::CommandBuffer::pushConstants;
  };

  class SecondaryCommandBuffer : public CommandBuffer
  {
  public:
    using CommandBuffer::CommandBuffer;

    void begin(vk::CommandBufferUsageFlags flags, vk::CommandBufferInheritanceInfo const &pInheritanceInfo) const
    {
      vk::CommandBuffer::begin(vk::CommandBufferBeginInfo{flags, &pInheritanceInfo});
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

    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const
    {
      commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t firstIndex, uint32_t firstInstance) const
    {
      commandBuffer.drawIndexed(indexCount, instanceCount, vertexOffset, firstIndex, firstInstance);
    }

    void bindGraphicsPipeline(Pipeline<claws::no_delete> pipeline) const
    {
      commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    ~RenderPassExecLock()
    {
      commandBuffer.endRenderPass();
    }
  };

  class PrimaryCommandBuffer : public CommandBuffer
  {
  public:
    using CommandBuffer::CommandBuffer;

    void begin(vk::CommandBufferUsageFlags flags) const
    {
      vk::CommandBuffer::begin(vk::CommandBufferBeginInfo{flags, nullptr});
    }

    void execBuffers(CommandBufferGroup<SecondaryCommandBuffer> const &secondaryCommands) const
    {
      vk::CommandBuffer::executeCommands(static_cast<std::vector<vk::CommandBuffer> const &>(secondaryCommands));
    }

    auto beginRenderPass(RenderPass<claws::no_delete> renderpass,
                         Framebuffer<claws::no_delete> framebuffer,
                         vk::Rect2D renderArea,
                         std::vector<vk::ClearValue> const &clearValues,
                         vk::SubpassContents contents) const
    {
      vk::CommandBuffer::beginRenderPass({renderpass, framebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()}, contents);

      return RenderPassExecLock{*this};
    }
  };

  namespace impl
  {
    class CommandPool : public vk::CommandPool
    {
    protected:
      magma::Device<claws::no_delete> device;

      ~CommandPool() = default;

    public:
      CommandPool()
        : vk::CommandPool(nullptr)
        , device{}
      {}

      CommandPool(magma::Device<claws::no_delete> device, vk::CommandPool commandPool)
        : vk::CommandPool(commandPool)
        , device(device)
      {}

      void reset(vk::CommandPoolResetFlags flags) const
      {
        device.resetCommandPool(*this, flags);
      }

      auto allocatePrimaryCommandBuffers(uint32_t commandBufferCount) const
      {
        vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::ePrimary, commandBufferCount};

        return CommandBufferGroup<PrimaryCommandBuffer>{{device, *this}, device.allocateCommandBuffers(info)};
      }

      auto allocateSecondaryCommandBuffers(uint32_t commandBufferCount) const
      {
        vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::eSecondary, commandBufferCount};

        return CommandBufferGroup<SecondaryCommandBuffer>{{device, *this}, device.allocateCommandBuffers(info)};
      }

      void swap(CommandPool &other)
      {
        using std::swap;

        swap(static_cast<vk::CommandPool &>(*this), static_cast<vk::CommandPool &>(other));
        swap(device, other.device);
      }

      struct Deleter
      {
	void operator()(impl::CommandPool &&commandPool) const
	{
	  magma::Deleter{commandPool.device}(commandPool);
	}
      };
    };

    inline void swap(CommandPool &lh, CommandPool &rh)
    {
      lh.swap(rh);
    }

  }

  template<class Deleter = impl::CommandPool::Deleter>
  using CommandPool = claws::handle<impl::CommandPool, Deleter>;

  inline auto impl::Device::createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex) const
  {
    vk::CommandPoolCreateInfo createInfo{flags, queueFamilyIndex};

    return magma::CommandPool<>{{}, magma::Device<claws::no_delete>(*this), vk::Device::createCommandPool(createInfo)};
  }
};
