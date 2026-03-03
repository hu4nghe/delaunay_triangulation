#include "vulkan_renderer.h"

#include <shaderc/shaderc.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const char* VERT_SHADER_GLSL = R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        layout(location = 1) in vec3 inColor;
        layout(location = 0) out vec3 fragColor;
        layout(set = 0, binding = 0) uniform UBO { mat4 mvp; } ubo;
        void main() {
            gl_Position = ubo.mvp * vec4(inPos, 1.0);
            gl_PointSize = 4.0;
            fragColor = inColor;
        }
)";

    const char* FRAG_SHADER_GLSL = R"(
        #version 450
        layout(location = 0) in vec3 fragColor;
        layout(location = 0) out vec4 outColor;
        void main() {
            outColor = vec4(fragColor, 1.0);
        }
        )";
} // namespace

auto VulkanRenderer::Vertex::bindingDescription() -> VkVertexInputBindingDescription
{
    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.stride    = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding;
}

auto VulkanRenderer::Vertex::attributeDescriptions() -> std::array<
    VkVertexInputAttributeDescription,
    2>
{
    std::array<VkVertexInputAttributeDescription, 2> attributes{};

    attributes[0].binding  = 0;
    attributes[0].location = 0;
    attributes[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[0].offset   = offsetof(Vertex, position);

    attributes[1].binding  = 0;
    attributes[1].location = 1;
    attributes[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset   = offsetof(Vertex, color);

    return attributes;
}

auto VulkanRenderer::QueueFamilyIndices::isComplete() const -> bool
{
    return graphics_family.has_value() && present_family.has_value();
}

VulkanRenderer::~VulkanRenderer()
{
    cleanup();
}

void VulkanRenderer::initialize(GLFWwindow* window)
{
    window_ = window;
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipelines();
    createFramebuffers();
    createCommandPool();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanRenderer::cleanup()
{
    if (device_ == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle(device_);

    destroyMeshBuffers();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (uniform_buffers_mapped_[i] != nullptr)
            vkUnmapMemory(device_, uniform_buffers_memory_[i]);
        if (uniform_buffers_[i] != VK_NULL_HANDLE)
            vkDestroyBuffer(device_, uniform_buffers_[i], nullptr);
        if (uniform_buffers_memory_[i] != VK_NULL_HANDLE)
            vkFreeMemory(device_, uniform_buffers_memory_[i], nullptr);

        if (image_available_semaphores_[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        if (render_finished_semaphores_[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
        if (in_flight_fences_[i] != VK_NULL_HANDLE)
            vkDestroyFence(device_, in_flight_fences_[i], nullptr);
    }

    if (descriptor_pool_ != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);

    cleanupSwapChain();

    if (descriptor_set_layout_ != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);
    if (command_pool_ != VK_NULL_HANDLE)
        vkDestroyCommandPool(device_, command_pool_, nullptr);
    if (device_ != VK_NULL_HANDLE) vkDestroyDevice(device_, nullptr);
    if (surface_ != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_ != VK_NULL_HANDLE) vkDestroyInstance(instance_, nullptr);

    instance_ = VK_NULL_HANDLE;
    device_   = VK_NULL_HANDLE;
}

void VulkanRenderer::setMesh(const SceneBridge::MeshData& mesh)
{
    std::vector<Vertex> vertices;
    vertices.reserve(mesh.positions.size());

    for (const auto& p : mesh.positions)
    {
        const float t = (p.z + 1.0f) * 0.5f;
        vertices.push_back({p, {0.2f + 0.8f * t, 0.7f - 0.4f * t, 1.0f - 0.6f * t}});
    }

    vkDeviceWaitIdle(device_);
    destroyMeshBuffers();

    if (vertices.empty() || mesh.indices.empty())
    {
        index_count_      = 0;
        line_index_count_ = 0;
        vertex_count_     = 0;
        return;
    }

    std::set<std::pair<std::uint32_t, std::uint32_t>> unique_edges;
    unique_edges.clear();
    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
    {
        const std::uint32_t tri[3] = {
            mesh.indices[i],
            mesh.indices[i + 1],
            mesh.indices[i + 2]};
        for (int e = 0; e < 3; ++e)
        {
            std::uint32_t a = tri[e];
            std::uint32_t b = tri[(e + 1) % 3];
            if (a > b) std::swap(a, b);
            unique_edges.insert({a, b});
        }
    }

    std::vector<std::uint32_t> line_indices;
    line_indices.reserve(unique_edges.size() * 2);
    for (const auto& edge : unique_edges)
    {
        line_indices.push_back(edge.first);
        line_indices.push_back(edge.second);
    }

    createVertexBuffer(vertices);
    createIndexBuffer(mesh.indices);
    createLineIndexBuffer(line_indices);
    index_count_      = static_cast<std::uint32_t>(mesh.indices.size());
    line_index_count_ = static_cast<std::uint32_t>(line_indices.size());
    vertex_count_     = static_cast<std::uint32_t>(vertices.size());
}

void VulkanRenderer::draw(const glm::mat4& mvp)
{
    vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    std::uint32_t image_index = 0;
    VkResult      result      = vkAcquireNextImageKHR(
        device_,
        swap_chain_,
        UINT64_MAX,
        image_available_semaphores_[current_frame_],
        VK_NULL_HANDLE,
        &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image");

    updateUniformBuffer(current_frame_, mvp);

    vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);
    vkResetCommandBuffer(command_buffers_[current_frame_], 0);

    recordCommandBuffer(command_buffers_[current_frame_], image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[]      = {image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount     = 1;
    submit_info.pWaitSemaphores        = wait_semaphores;
    submit_info.pWaitDstStageMask      = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command_buffers_[current_frame_];

    VkSemaphore signal_semaphores[]  = {render_finished_semaphores_[current_frame_]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    if (vkQueueSubmit(
            graphics_queue_,
            1,
            &submit_info,
            in_flight_fences_[current_frame_]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer");

    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swap_chain_;
    present_info.pImageIndices      = &image_index;

    result = vkQueuePresentKHR(present_queue_, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebuffer_resized_)
    {
        framebuffer_resized_ = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image");
    }

    current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::setFramebufferResized()
{
    framebuffer_resized_ = true;
}

auto VulkanRenderer::aspectRatio() const -> float
{
    if (swap_chain_extent_.height == 0) return 1.0f;
    return static_cast<float>(swap_chain_extent_.width) /
           static_cast<float>(swap_chain_extent_.height);
}

void VulkanRenderer::createInstance()
{
    VkApplicationInfo app_info{};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Delaunay Vulkan Viewer";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "No Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    uint32_t     glfw_extension_count = 0;
    const char** glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    VkInstanceCreateInfo create_info{};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance");
}

void VulkanRenderer::createSurface()
{
    if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface");
}

void VulkanRenderer::pickPhysicalDevice()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    if (device_count == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support");

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physical_device_ = device;
            break;
        }
    }

    if (physical_device_ == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU");
}

void VulkanRenderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physical_device_);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<std::uint32_t>              unique_queue_families = {
        indices.graphics_family.value(),
        indices.present_family.value()};

    float queue_priority = 1.0f;
    for (std::uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount       = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
        static_cast<std::uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos       = queue_create_infos.data();
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device");

    vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
    vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}

void VulkanRenderer::createSwapChain()
{
    SwapChainSupportDetails swap_chain_support = querySwapChainSupport(physical_device_);

    VkSurfaceFormatKHR surface_format =
        chooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode =
        chooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 &&
        image_count > swap_chain_support.capabilities.maxImageCount)
        image_count = swap_chain_support.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = surface_;
    create_info.minImageCount    = image_count;
    create_info.imageFormat      = surface_format.format;
    create_info.imageColorSpace  = surface_format.colorSpace;
    create_info.imageExtent      = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices                = findQueueFamilies(physical_device_);
    uint32_t           queue_family_indices[] = {
        indices.graphics_family.value(),
        indices.present_family.value()};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices   = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform   = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = present_mode;
    create_info.clipped        = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain");

    vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr);
    swap_chain_images_.resize(image_count);
    vkGetSwapchainImagesKHR(
        device_,
        swap_chain_,
        &image_count,
        swap_chain_images_.data());

    swap_chain_image_format_ = surface_format.format;
    swap_chain_extent_       = extent;
}

void VulkanRenderer::createImageViews()
{
    swap_chain_image_views_.resize(swap_chain_images_.size());

    for (size_t i = 0; i < swap_chain_images_.size(); ++i)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image        = swap_chain_images_[i];
        create_info.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format       = swap_chain_image_format_;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(
                device_,
                &create_info,
                nullptr,
                &swap_chain_image_views_[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create image views");
    }
}

void VulkanRenderer::createRenderPass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format         = swap_chain_image_format_;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments    = &color_attachment;
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create render pass");
}

void VulkanRenderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding         = 0;
    ubo_layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings    = &ubo_layout_binding;

    if (vkCreateDescriptorSetLayout(
            device_,
            &layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout");
}

void VulkanRenderer::createGraphicsPipelines()
{
    auto vert_code = compileShader(VERT_SHADER_GLSL, VK_SHADER_STAGE_VERTEX_BIT);
    auto frag_code = compileShader(FRAG_SHADER_GLSL, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderModule vert_shader_module = createShaderModule(vert_code);
    VkShaderModule frag_shader_module = createShaderModule(frag_code);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_info,
        frag_shader_stage_info};

    auto binding_description    = Vertex::bindingDescription();
    auto attribute_descriptions = Vertex::attributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions    = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth   = 1.0f;
    rasterizer.cullMode    = VK_CULL_MODE_NONE;
    rasterizer.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments    = &color_blend_attachment;

    std::array<VkDynamicState, 2> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates    = dynamic_states.data();

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts    = &descriptor_set_layout_;

    if (vkCreatePipelineLayout(
            device_,
            &pipeline_layout_info,
            nullptr,
            &pipeline_layout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount          = 2;
    pipeline_info.pStages             = shader_stages;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = &dynamic_state;
    pipeline_info.layout              = pipeline_layout_;
    pipeline_info.renderPass          = render_pass_;
    pipeline_info.subpass             = 0;

    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    if (vkCreateGraphicsPipelines(
            device_,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &mesh_pipeline_) != VK_SUCCESS)
        throw std::runtime_error("failed to create mesh pipeline");

    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    if (vkCreateGraphicsPipelines(
            device_,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &wire_pipeline_) != VK_SUCCESS)
        throw std::runtime_error("failed to create wireframe pipeline");

    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    if (vkCreateGraphicsPipelines(
            device_,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &point_pipeline_) != VK_SUCCESS)
        throw std::runtime_error("failed to create point pipeline");

    vkDestroyShaderModule(device_, frag_shader_module, nullptr);
    vkDestroyShaderModule(device_, vert_shader_module, nullptr);
}

void VulkanRenderer::createFramebuffers()
{
    swap_chain_framebuffers_.resize(swap_chain_image_views_.size());

    for (size_t i = 0; i < swap_chain_image_views_.size(); ++i)
    {
        VkImageView attachments[] = {swap_chain_image_views_[i]};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass_;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments;
        framebuffer_info.width           = swap_chain_extent_.width;
        framebuffer_info.height          = swap_chain_extent_.height;
        framebuffer_info.layers          = 1;

        if (vkCreateFramebuffer(
                device_,
                &framebuffer_info,
                nullptr,
                &swap_chain_framebuffers_[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer");
    }
}

void VulkanRenderer::createCommandPool()
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(physical_device_);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool");
}

void VulkanRenderer::createUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniform_buffers_[i],
            uniform_buffers_memory_[i]);

        vkMapMemory(
            device_,
            uniform_buffers_memory_[i],
            0,
            buffer_size,
            0,
            &uniform_buffers_mapped_[i]);
    }
}

void VulkanRenderer::createDescriptorPool()
{
    VkDescriptorPoolSize pool_size{};
    pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes    = &pool_size;
    pool_info.maxSets       = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");
}

void VulkanRenderer::createDescriptorSets()
{
    std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts{};
    layouts.fill(descriptor_set_layout_);

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = descriptor_pool_;
    alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts        = layouts.data();

    if (vkAllocateDescriptorSets(device_, &alloc_info, descriptor_sets_.data()) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet          = descriptor_sets_[i];
        descriptor_write.dstBinding      = 0;
        descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo     = &buffer_info;

        vkUpdateDescriptorSets(device_, 1, &descriptor_write, 0, nullptr);
    }
}

void VulkanRenderer::createCommandBuffers()
{
    command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool_;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");
}

void VulkanRenderer::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(
                device_,
                &semaphore_info,
                nullptr,
                &image_available_semaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(
                device_,
                &semaphore_info,
                nullptr,
                &render_finished_semaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]) !=
                VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects");
        }
    }
}

void VulkanRenderer::recreateSwapChain()
{
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window_, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_);

    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipelines();
    createFramebuffers();
}

void VulkanRenderer::cleanupSwapChain()
{
    for (auto framebuffer : swap_chain_framebuffers_)
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    swap_chain_framebuffers_.clear();

    if (mesh_pipeline_ != VK_NULL_HANDLE)
        vkDestroyPipeline(device_, mesh_pipeline_, nullptr);
    mesh_pipeline_ = VK_NULL_HANDLE;

    if (wire_pipeline_ != VK_NULL_HANDLE)
        vkDestroyPipeline(device_, wire_pipeline_, nullptr);
    wire_pipeline_ = VK_NULL_HANDLE;

    if (point_pipeline_ != VK_NULL_HANDLE)
        vkDestroyPipeline(device_, point_pipeline_, nullptr);
    point_pipeline_ = VK_NULL_HANDLE;

    if (pipeline_layout_ != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
    pipeline_layout_ = VK_NULL_HANDLE;

    if (render_pass_ != VK_NULL_HANDLE)
        vkDestroyRenderPass(device_, render_pass_, nullptr);
    render_pass_ = VK_NULL_HANDLE;

    for (auto image_view : swap_chain_image_views_)
        vkDestroyImageView(device_, image_view, nullptr);
    swap_chain_image_views_.clear();

    if (swap_chain_ != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
    swap_chain_ = VK_NULL_HANDLE;
}

void VulkanRenderer::updateUniformBuffer(
    std::uint32_t    frame,
    const glm::mat4& mvp)
{
    UniformBufferObject ubo{};
    ubo.mvp = mvp;
    std::memcpy(uniform_buffers_mapped_[frame], &ubo, sizeof(ubo));
}

void VulkanRenderer::recordCommandBuffer(
    VkCommandBuffer command_buffer,
    std::uint32_t   image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer");

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = render_pass_;
    render_pass_info.framebuffer       = swap_chain_framebuffers_[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swap_chain_extent_;

    VkClearValue clear_color         = {{{0.05f, 0.05f, 0.08f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues    = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(swap_chain_extent_.width);
    viewport.height   = static_cast<float>(swap_chain_extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent_;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    if (index_count_ > 0)
    {
        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mesh_pipeline_);
        VkBuffer     vertex_buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[]        = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            0,
            1,
            &descriptor_sets_[current_frame_],
            0,
            nullptr);
        vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);

        if (line_index_count_ > 0)
        {
            vkCmdBindPipeline(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                wire_pipeline_);
            vkCmdBindIndexBuffer(
                command_buffer,
                line_index_buffer_,
                0,
                VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(command_buffer, line_index_count_, 1, 0, 0, 0);
        }

        if (vertex_count_ > 0)
        {
            vkCmdBindPipeline(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                point_pipeline_);
            vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
        }
    }

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer");
}

void VulkanRenderer::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

    createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertex_buffer_,
        vertex_buffer_memory_);

    void* data{};
    vkMapMemory(device_, vertex_buffer_memory_, 0, buffer_size, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<std::size_t>(buffer_size));
    vkUnmapMemory(device_, vertex_buffer_memory_);
}

void VulkanRenderer::createIndexBuffer(const std::vector<std::uint32_t>& indices)
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        index_buffer_,
        index_buffer_memory_);

    void* data{};
    vkMapMemory(device_, index_buffer_memory_, 0, buffer_size, 0, &data);
    std::memcpy(data, indices.data(), static_cast<std::size_t>(buffer_size));
    vkUnmapMemory(device_, index_buffer_memory_);
}

void VulkanRenderer::createLineIndexBuffer(const std::vector<std::uint32_t>& indices)
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        line_index_buffer_,
        line_index_buffer_memory_);

    void* data{};
    vkMapMemory(device_, line_index_buffer_memory_, 0, buffer_size, 0, &data);
    std::memcpy(data, indices.data(), static_cast<std::size_t>(buffer_size));
    vkUnmapMemory(device_, line_index_buffer_memory_);
}

void VulkanRenderer::destroyMeshBuffers()
{
    if (vertex_buffer_ != VK_NULL_HANDLE)
        vkDestroyBuffer(device_, vertex_buffer_, nullptr);
    if (vertex_buffer_memory_ != VK_NULL_HANDLE)
        vkFreeMemory(device_, vertex_buffer_memory_, nullptr);
    if (index_buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, index_buffer_, nullptr);
    if (index_buffer_memory_ != VK_NULL_HANDLE)
        vkFreeMemory(device_, index_buffer_memory_, nullptr);
    if (line_index_buffer_ != VK_NULL_HANDLE)
        vkDestroyBuffer(device_, line_index_buffer_, nullptr);
    if (line_index_buffer_memory_ != VK_NULL_HANDLE)
        vkFreeMemory(device_, line_index_buffer_memory_, nullptr);

    vertex_buffer_            = VK_NULL_HANDLE;
    vertex_buffer_memory_     = VK_NULL_HANDLE;
    index_buffer_             = VK_NULL_HANDLE;
    index_buffer_memory_      = VK_NULL_HANDLE;
    line_index_buffer_        = VK_NULL_HANDLE;
    line_index_buffer_memory_ = VK_NULL_HANDLE;
    line_index_count_         = 0;
    vertex_count_             = 0;
}

void VulkanRenderer::createBuffer(
    VkDeviceSize          size,
    VkBufferUsageFlags    usage,
    VkMemoryPropertyFlags properties,
    VkBuffer&             buffer,
    VkDeviceMemory&       buffer_memory)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer");

    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(device_, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex =
        findMemoryType(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory");

    vkBindBufferMemory(device_, buffer, buffer_memory, 0);
}

auto VulkanRenderer::findMemoryType(
    std::uint32_t         type_filter,
    VkMemoryPropertyFlags properties) const -> std::uint32_t
{
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

    for (std::uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i)
    {
        const bool type_match = (type_filter & (1u << i)) != 0;
        const bool property_match =
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties;
        if (type_match && property_match) return i;
    }

    throw std::runtime_error("failed to find suitable memory type");
}

auto VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) const
    -> SwapChainSupportDetails
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface_,
            &format_count,
            details.formats.data());
    }

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        surface_,
        &present_mode_count,
        nullptr);
    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            &present_mode_count,
            details.present_modes.data());
    }

    return details;
}

auto VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) const
    -> QueueFamilyIndices
{
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queue_family_count,
        queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics_family = static_cast<std::uint32_t>(i);

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
        if (present_support) indices.present_family = static_cast<std::uint32_t>(i);

        if (indices.isComplete()) break;
        ++i;
    }

    return indices;
}

auto VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) const -> bool
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        available_extensions.data());

    std::set<std::string> required_extensions(
        DEVICE_EXTENSIONS.begin(),
        DEVICE_EXTENSIONS.end());
    for (const auto& extension : available_extensions)
        required_extensions.erase(extension.extensionName);

    bool swap_chain_adequate = false;
    if (required_extensions.empty())
    {
        SwapChainSupportDetails swap_chain_support = querySwapChainSupport(device);
        swap_chain_adequate = !swap_chain_support.formats.empty() &&
                              !swap_chain_support.present_modes.empty();
    }

    return indices.isComplete() && required_extensions.empty() && swap_chain_adequate;
}

auto VulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats) const -> VkSurfaceFormatKHR
{
    for (const auto& available_format : formats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }

    return formats[0];
}

auto VulkanRenderer::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes) const -> VkPresentModeKHR
{
    for (const auto& available_present_mode : present_modes)
    {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return available_present_mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

auto VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    -> VkExtent2D
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);

    VkExtent2D actual_extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)};

    actual_extent.width = std::clamp(
        actual_extent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(
        actual_extent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actual_extent;
}

auto VulkanRenderer::compileShader(
    const std::string&    source,
    VkShaderStageFlagBits stage) -> std::vector<std::uint32_t>
{
    shaderc_shader_kind shader_kind = shaderc_vertex_shader;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) shader_kind = shaderc_fragment_shader;

    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(
        shaderc_target_env_vulkan,
        shaderc_env_version_vulkan_1_0);

    auto result = compiler.CompileGlslToSpv(source, shader_kind, "shader.glsl", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        throw std::runtime_error(result.GetErrorMessage());

    return {result.cbegin(), result.cend()};
}

auto VulkanRenderer::createShaderModule(const std::vector<std::uint32_t>& code)
    -> VkShaderModule
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size() * sizeof(std::uint32_t);
    create_info.pCode    = code.data();

    VkShaderModule shader_module = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device_, &create_info, nullptr, &shader_module) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");

    return shader_module;
}
