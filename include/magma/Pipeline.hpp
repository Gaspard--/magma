#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"
#include "magma/PipelineLayout.hpp"

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
                           PipelineLayout<claws::no_delete> const &layout,
                           RenderPass<claws::no_delete> renderPass,
                           uint32_t subpass)
      : vk::GraphicsPipelineCreateInfo(flags,
                                       static_cast<uint32_t>(stages.size()),
                                       nullptr,
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
                                       renderPass,
                                       subpass,
                                       nullptr,
                                       -1)
      , stages(std::move(stages))
    {
      this->pStages = this->stages.data();
    }

    void addRasteringInfo(vk::PipelineViewportStateCreateInfo const &viewportState, vk::PipelineMultisampleStateCreateInfo const &multisampleState)
    {
      this->pViewportState = &viewportState;
      this->pMultisampleState = &multisampleState;
    }

    void addRasteringDepthStencilInfo(vk::PipelineViewportStateCreateInfo const &viewportState,
                                      vk::PipelineMultisampleStateCreateInfo const &multisampleState,
                                      vk::PipelineDepthStencilStateCreateInfo const &depthStencilState)
    {
      this->pDepthStencilState = &depthStencilState;
      addRasteringInfo(viewportState, multisampleState);
    }

    void addRasteringColorAttachementDepthStencilInfo(vk::PipelineViewportStateCreateInfo const &viewportState,
                                                      vk::PipelineMultisampleStateCreateInfo const &multisampleState,
                                                      vk::PipelineDepthStencilStateCreateInfo const &depthStencilState,
                                                      vk::PipelineColorBlendStateCreateInfo const &colorBlendState)
    {
      this->pColorBlendState = &colorBlendState;
      addRasteringDepthStencilInfo(viewportState, multisampleState, depthStencilState);
    }

    void addRasteringColorAttachementInfo(vk::PipelineViewportStateCreateInfo const &viewportState,
                                          vk::PipelineMultisampleStateCreateInfo const &multisampleState,
                                          vk::PipelineColorBlendStateCreateInfo const &colorBlendState)
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
      this->pTessellationState = &tesselationState;
    }
  };

  template<class Deleter = Deleter<vk::Pipeline>>
  using Pipeline = claws::handle<vk::Pipeline, Deleter>;

  inline auto impl::Device::createPipeline(vk::GraphicsPipelineCreateInfo const &createInfo) const
  {
    return Pipeline<>(Deleter<vk::Pipeline>{magma::Device<claws::no_delete>(*this)}, vk::Device::createGraphicsPipeline(nullptr, createInfo));
  }
};
