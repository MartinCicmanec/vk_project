VULKAN_SDK_PATH = /usr/include/vulkan
VK_EXP_PATH = /usr/share/vulkan/explicit_layer.d

CFLAGS = -std=c++17 -O2 -I $(VULKAN_SDK_PATH)
LDFLAGS = -L $(VULKAN_SDK_PATH) -lglfw -lvulkan -ldl -lpthread
# UNUSED LIBS -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: src/main.cpp
	g++ $(CFLAGS) -o VulkanTest src/main.cpp $(LDFLAGS)

.PHONY: test clean

glfw: src/test.cpp
	g++ $(CFLAGS) -o glfw src/test.cpp $(LDFLAGS)

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest