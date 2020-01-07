#include <android/log.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "vulkan/vulkan.h"
#include "log.h"
#include "callstack.h"
#include "hookutils.h"

VkResult (*old_vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) = NULL;
VkResult mj_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LOGI("mj_vkCreateInstance");
    print_callstack();
    int re = 0;
    re = old_vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    return re;
}

VkResult (*old_vkQueueSubmit)(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) = NULL;
VkResult mj_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    LOGI("mj_vkQueueSubmit");
    return old_vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

void hookVulkanFun()
{
    hook((uint32_t) vkCreateInstance, (uint32_t)mj_vkCreateInstance, (uint32_t **) &old_vkCreateInstance);
    hook((uint32_t) vkQueueSubmit, (uint32_t)mj_vkQueueSubmit, (uint32_t **) &old_vkQueueSubmit);
}
