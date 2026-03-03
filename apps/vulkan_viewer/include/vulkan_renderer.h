#pragma once

#include "scene_bridge.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class VulkanRenderer
{
    public:

    VulkanRenderer() = default;
    ~VulkanRenderer();

    void initialize(GLFWwindow* window);
    void cleanup();

    void setMesh(const SceneBridge::MeshData& mesh);
    void draw(const glm::mat4& mvp);

    void setFramebufferResized();
    auto aspectRatio() const -> float;

    private:

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        static auto bindingDescription() -> VkVertexInputBindingDescription;
        static auto attributeDescriptions() -> std::array<
            VkVertexInputAttributeDescription,
            2>;
    };

    struct UniformBufferObject
    {
        glm::mat4 mvp;
    };

    struct QueueFamilyIndices
    {
        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> present_family;

        auto isComplete() const -> bool;
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    GLFWwindow* window_{};

    VkInstance       instance_        = VK_NULL_HANDLE;
    VkSurfaceKHR     surface_         = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice         device_          = VK_NULL_HANDLE;
    VkQueue          graphics_queue_  = VK_NULL_HANDLE;
    VkQueue          present_queue_   = VK_NULL_HANDLE;

    VkSwapchainKHR             swap_chain_ = VK_NULL_HANDLE;
    std::vector<VkImage>       swap_chain_images_;
    VkFormat                   swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D                 swap_chain_extent_{};
    std::vector<VkImageView>   swap_chain_image_views_;
    std::vector<VkFramebuffer> swap_chain_framebuffers_;

    VkRenderPass          render_pass_           = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout      pipeline_layout_       = VK_NULL_HANDLE;
    VkPipeline            mesh_pipeline_         = VK_NULL_HANDLE;
    VkPipeline            wire_pipeline_         = VK_NULL_HANDLE;
    VkPipeline            point_pipeline_        = VK_NULL_HANDLE;

    VkCommandPool                command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;

    VkBuffer       vertex_buffer_            = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory_     = VK_NULL_HANDLE;
    VkBuffer       index_buffer_             = VK_NULL_HANDLE;
    VkDeviceMemory index_buffer_memory_      = VK_NULL_HANDLE;
    std::uint32_t  index_count_              = 0;
    VkBuffer       line_index_buffer_        = VK_NULL_HANDLE;
    VkDeviceMemory line_index_buffer_memory_ = VK_NULL_HANDLE;
    std::uint32_t  line_index_count_         = 0;
    std::uint32_t  vertex_count_             = 0;

    std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>       uniform_buffers_{};
    std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> uniform_buffers_memory_{};
    std::array<void*, MAX_FRAMES_IN_FLIGHT>          uniform_buffers_mapped_{};

    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptor_sets_{};

    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_available_semaphores_{};
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finished_semaphores_{};
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     in_flight_fences_{};

    std::uint32_t current_frame_       = 0;
    bool          framebuffer_resized_ = false;

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipelines();
    void createFramebuffers();
    void createCommandPool();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    void recreateSwapChain();
    void cleanupSwapChain();

    void updateUniformBuffer(
        std::uint32_t    frame,
        const glm::mat4& mvp);
    void recordCommandBuffer(
        VkCommandBuffer command_buffer,
        std::uint32_t   image_index);

    void createVertexBuffer(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<std::uint32_t>& indices);
    void createLineIndexBuffer(const std::vector<std::uint32_t>& indices);
    void destroyMeshBuffers();

    void createBuffer(
        VkDeviceSize          size,
        VkBufferUsageFlags    usage,
        VkMemoryPropertyFlags properties,
        VkBuffer&             buffer,
        VkDeviceMemory&       buffer_memory);

    auto findMemoryType(
        std::uint32_t         type_filter,
        VkMemoryPropertyFlags properties) const -> std::uint32_t;
    auto querySwapChainSupport(VkPhysicalDevice device) const
        -> SwapChainSupportDetails;
    auto findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices;
    auto isDeviceSuitable(VkPhysicalDevice device) const -> bool;

    auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
        const -> VkSurfaceFormatKHR;
    auto chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& present_modes) const
        -> VkPresentModeKHR;
    auto chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
        -> VkExtent2D;

    auto compileShader(
        const std::string&    source,
        VkShaderStageFlagBits stage) -> std::vector<std::uint32_t>;
    auto createShaderModule(const std::vector<std::uint32_t>& code)
        -> VkShaderModule;
};
