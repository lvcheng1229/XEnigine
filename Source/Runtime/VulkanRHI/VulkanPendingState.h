#pragma once
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"

class XVulkanPendingGfxState
{
public:
	void SetGfxPipeline(XVulkanRHIGraphicsPipelineState* InGfxPipeline);
	inline void Bind(VkCommandBuffer CmdBuffer)
	{
		CurrentPipeline->Bind(CmdBuffer);
	}

	void SetStreamSource(uint32 StreamIndex, VkBuffer VertexBuffer, uint32 Offset)
	{
		PendingStreams[StreamIndex].Stream = VertexBuffer;
		PendingStreams[StreamIndex].BufferOffset = Offset;
		bDirtyVertexStreams = true;
	}

	inline void UpdateDynamicStates(XVulkanCmdBuffer* Cmd)
	{
		vkCmdSetViewport(Cmd->GetHandle(), 0, 1, &Viewport);
		vkCmdSetScissor(Cmd->GetHandle(), 0, 1, &Scissor);
	}

	void PrepareForDraw(XVulkanCmdBuffer* CmdBuffer);

	void SetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)
	{
		Viewport = {};
		Viewport.x = MinX;
		Viewport.y = MinY;
		Viewport.width = MaxX - MinX;
		Viewport.height = MaxY - MinY;
		Viewport.minDepth = MinZ;
		if (MinZ == MaxZ)
		{
			Viewport.maxDepth = MinZ + 1.0f;
		}
		else
		{
			Viewport.maxDepth = MaxZ;
		}

		Scissor = {};
		Scissor.offset.x = MinX;
		Scissor.offset.y = MinY;
		Scissor.extent.width = (uint32)(MaxX - MinX);
		Scissor.extent.height = (uint32)(MaxY - MinY);
	}
private:
	struct XVertexStream
	{
		XVertexStream() :
			Stream(VK_NULL_HANDLE),
			BufferOffset(0)
		{
		}

		VkBuffer Stream;
		uint32 BufferOffset;
	};

	XVertexStream PendingStreams[17];
	bool bDirtyVertexStreams;

	VkViewport Viewport;
	VkRect2D Scissor;

	XVulkanRHIGraphicsPipelineState* CurrentPipeline;
};