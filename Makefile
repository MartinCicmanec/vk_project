VULKAN_SDK_PATH = /usr/include/vulkan
VK_EXP_PATH = /usr/share/vulkan/explicit_layer.d

CPPFLAGS = -DGLFW_USE_WAYLAND="ON"
CXXFLAGS = -std=c++17 -O2 #-I $(VULKAN_SDK_PATH)
LDFLAGS = -lglfw -lvulkan -lxkbcommon #-ldl -lpthread #-L $(VULKAN_SDK_PATH)
# UNUSED LIBS -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: src/main.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -o VulkanTest src/main.cpp $(LDFLAGS)

.PHONY: test clean

glfw: src/test.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -o glfw src/test.cpp $(LDFLAGS)

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
	rm -f glfw