#include "common.h"
#include "vulkan_shader_object.h"
#include "vulkan_utils.h"
#include "shaders/vulkan_shaders.h"

namespace moth::gfx::vulkan {
    namespace {
        // The quad vertex shader's push constant block (xyScale/xyOffset), see
        // Graphics::PushConstants. Must match drawing_shader.vert.
        constexpr uint32_t kVertexPushConstantSize = 2 * 2 * sizeof(float);

        VkDescriptorSet AllocateDescriptorSet(SurfaceContext& context, VkDescriptorSetLayout layout) {            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = context.GetVkDescriptorPool();
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;
            CHECK_VK_RESULT(vkAllocateDescriptorSets(context.GetVkDevice(), &allocInfo, &descriptorSet));
            return descriptorSet;
        }
    }

    VulkanShader::~VulkanShader() {
        // Free the channel descriptor sets while the pool they were allocated
        // from is still alive (the factory cache is flushed before the device
        // and pools are torn down).
        if (!m_descriptorSets.empty()) {
            std::vector<VkDescriptorSet> sets;
            sets.reserve(m_descriptorSets.size());
            for (auto const& [key, set] : m_descriptorSets) {
                (void)key;
                sets.push_back(set);
            }
            vkFreeDescriptorSets(m_context.GetVkDevice(), m_context.GetVkDescriptorPool(),
                                 static_cast<std::uint32_t>(sets.size()), sets.data());
        }
    }

    std::shared_ptr<VulkanShader> VulkanShader::Create(SurfaceContext& context, std::vector<std::uint32_t> const& fragSpv) {
        auto shader = std::shared_ptr<VulkanShader>(new VulkanShader(context));

        shader->m_shader = ShaderBuilder(context.GetVkDevice(), context.GetVkDescriptorPool())
                              .AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, kVertexPushConstantSize)
                              .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .AddStage(VK_SHADER_STAGE_VERTEX_BIT, "main", drawing_shader_vert_spv, drawing_shader_vert_spv_len)
                              .AddStage(VK_SHADER_STAGE_FRAGMENT_BIT, "main",
                                        reinterpret_cast<unsigned char const*>(fragSpv.data()), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                        fragSpv.size() * sizeof(std::uint32_t))
                              .Build();

        shader->m_builtinsBuffer = std::make_unique<Buffer>(
            context, sizeof(Builtins),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // A 1x1 white texture used for unbound channels so the descriptor set
        // always has a valid sampler to bind.
        static unsigned char const kWhitePixel[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        shader->m_defaultImage = Texture::FromRGBA(context, 1, 1, kWhitePixel);

        return shader;
    }

    VkDescriptorSet VulkanShader::GetDescriptorSet(std::array<std::shared_ptr<Texture>, 4> const& channels) {
        using Key = std::array<std::pair<std::uint32_t, VkSampler>, 4>;
        Key key{};
        for (std::size_t i = 0; i < 4; ++i) {
            Texture* texture = channels[i] ? channels[i].get() : m_defaultImage.get();
            key[i] = { texture->GetId(), texture->GetVkSampler() };
        }

        auto const it = m_descriptorSets.find(key);
        if (it != m_descriptorSets.end()) {
            return it->second;
        }

        VkDescriptorSet const descriptorSet = AllocateDescriptorSet(m_context, m_shader->m_descriptorSetLayout.Get());
        VkDevice const device = m_context.GetVkDevice();

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_builtinsBuffer->GetVKBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Builtins);

        VkWriteDescriptorSet writes[5]{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = descriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].pBufferInfo = &bufferInfo;

        VkDescriptorImageInfo imageInfos[4]{};
        for (std::size_t i = 0; i < 4; ++i) {
            Texture* texture = channels[i] ? channels[i].get() : m_defaultImage.get();
            imageInfos[i].sampler = texture->GetVkSampler();
            imageInfos[i].imageView = texture->GetVkView();
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            writes[i + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i + 1].dstSet = descriptorSet;
            writes[i + 1].dstBinding = static_cast<uint32_t>(i + 1);
            writes[i + 1].descriptorCount = 1;
            writes[i + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i + 1].pImageInfo = &imageInfos[i];
        }

        vkUpdateDescriptorSets(device, 5, writes, 0, nullptr);
        m_descriptorSets.emplace(std::move(key), descriptorSet);
        return descriptorSet;
    }
}
