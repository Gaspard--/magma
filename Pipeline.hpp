#pragma once

#include "magma/DeviceBasedHandle.hpp"

namespace magma
{
  // typedef struct VkGraphicsPipelineCreateInfo {
  //     VkStructureType                                  sType;
  //     const void*                                      pNext;
  //     VkPipelineCreateFlags                            flags;
  //     uint32_t                                         stageCount;
  //     const VkPipelineShaderStageCreateInfo*           pStages;
  //     const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
  //     const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
  //     const VkPipelineTessellationStateCreateInfo*     pTessellationState;
  //     const VkPipelineViewportStateCreateInfo*         pViewportState;
  //     const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
  //     const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
  //     const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
  //     const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
  //     const VkPipelineDynamicStateCreateInfo*          pDynamicState;
  //     VkPipelineLayout                                 layout;
  //     VkRenderPass                                     renderPass;
  //     uint32_t                                         subpass;
  //     VkPipeline                                       basePipelineHandle;
  //     int32_t                                          basePipelineIndex;
  // } VkGraphicsPipelineCreateInfo;

  // required:
  // - pVertexInputState is a pointer to an instance of the VkPipelineVertexInputStateCreateInfo structure.
  // - pInputAssemblyState is a pointer to an instance of the VkPipelineInputAssemblyStateCreateInfo structure which determines input assembly behavior, as described in Drawing Commands.
  // - pRasterizationState is a pointer to an instance of the VkPipelineRasterizationStateCreateInfo structure.

  // tesselation:
  // - pTessellationState is a pointer to an instance of the VkPipelineTessellationStateCreateInfo structure, and is ignored if the pipeline does not include a tessellation control shader stage and tessellation evaluation shader stage.

  // rasterisation
  // - pViewportState is a pointer to an instance of the VkPipelineViewportStateCreateInfo structure, and is ignored if the pipeline has rasterization disabled.
  // - pMultisampleState is a pointer to an instance of the VkPipelineMultisampleStateCreateInfo, and is ignored if the pipeline has rasterization disabled.
  // rasterisation & depth stencil
  // - pDepthStencilState is a pointer to an instance of the VkPipelineDepthStencilStateCreateInfo structure, and is ignored if the pipeline has rasterization disabled or if the subpass of the render pass the pipeline is created against does not use a depth/stencil attachment.
  // rasterisation & color attachements
  // - pColorBlendState is a pointer to an instance of the VkPipelineColorBlendStateCreateInfo structure, and is ignored if the pipeline has rasterization disabled or if the subpass of the render pass the pipeline is created against does not use any color attachments.


  class GraphicsPipelineConfig : public vk::GraphicsPipelineCreateInfo
  {
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
  public:
    GraphicsPipelineConfig(vk::PipelineCreateFlagBits flags,
		   std::vector<vk::PipelineShaderStageCreateInfo> &&stages,
		   vk::PipelineVertexInputStateCreateInfo const &vertexInputState,
		   vk::PipelineInputAssemblyStateCreateInfo const &inputAssemblyState,
		   vk::PipelineRasterizationStateCreateInfo const &rasterizationState,
		   vk::PipelineLayout const &layout,
		   RenderPass<NoDelete> renderPass,
		   uint32_t subpass)
      : vk::GraphicsPipelineCreateInfo(flags,
				       static_cast<uint32_t>(stages.size()),
				       stages.data(),
				       &vertexInputState,
				       &inputAssemblyState,
				       nullptr,
				       nullptr,
				       &rasterizationState,
				       nullptr,
				       nullptr,
				       nullptr,
				       nullptr,
				       layout,
				       renderPass.raw(),
				       subpass,
				       nullptr,
				       -1)
      , stages(std::move(stages))
    {
    }

    void addRasteringInfo(vk::PipelineViewportStateCreateInfo const &viewportState, vk::PipelineMultisampleStateCreateInfo const &multisampleState)
    {
      this->pViewportState = &viewportState;
      this->pMultisampleState = &multisampleState;
    }

    void addRasteringDepthStencilInfo(vk::PipelineViewportStateCreateInfo const &viewportState, vk::PipelineMultisampleStateCreateInfo const &multisampleState, vk::PipelineDepthStencilStateCreateInfo const &depthStencilState)
    {
      this->pDepthStencilState = &depthStencilState;
       addRasteringInfo(viewportState, multisampleState);
    }

    void addRasteringColorAttachementDepthStencilInfo(vk::PipelineViewportStateCreateInfo const &viewportState, vk::PipelineMultisampleStateCreateInfo const &multisampleState, vk::PipelineDepthStencilStateCreateInfo const &depthStencilState, vk::PipelineColorBlendStateCreateInfo const &colorBlendState)
    {
      this->pColorBlendState = &colorBlendState;
      addRasteringDepthStencilInfo(viewportState, multisampleState, depthStencilState);
    }

    void addRasteringColorAttachementInfo(vk::PipelineViewportStateCreateInfo const &viewportState, vk::PipelineMultisampleStateCreateInfo const &multisampleState, vk::PipelineColorBlendStateCreateInfo const &colorBlendState)
    {
      this->pColorBlendState = &colorBlendState;
      addRasteringInfo(viewportState, multisampleState);
    }

    template<class Container>
    void addTesselationInfo(vk::PipelineTessellationStateCreateInfo const &tesselationState, Container const &tesselationStages)
    {
      stages.insert(stages.end(), tesselationStages.begin(), tesselationStages.end());
      stageCount = static_cast<uint32_t>(stages.size());
      pStages = stages.data();
      this->pTessellationState = tesselationState;
    }
  };
  
  class PipelineImpl : protected DeviceBasedHandleImpl<vk::Pipeline>
  {
  protected:
    ~PipelineImpl() = default;

  public:
    PipelineImpl() = default;

    // Note: use above `GraphicPipelineConfig` if you want to have an easier time configurating the pipeline.
    PipelineImpl(Device<NoDelete> device, vk::PipelineCache cache, vk::GraphicsPipelineCreateInfo const &info)
      : DeviceBasedHandleImpl(vk::Pipeline(device.createGraphicsPipeline(cache, info)), device)
    {
    }

    auto raw() const noexcept
    {
      return static_cast<vk::Pipeline>(*this);
    }

    struct Deleter
    {
      friend class PipelineImpl;

      void operator()(PipelineImpl const &pipeline) const
      {
	if (pipeline.device)
	  pipeline.device.destroyPipeline(pipeline);
      }
    };
  };

  template<class Deleter = PipelineImpl::Deleter>
  using Pipeline = Handle<PipelineImpl, Deleter>;
};
