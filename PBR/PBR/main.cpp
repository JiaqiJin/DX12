#include <iostream>
/*
* https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12
*
PSO
When geometry is submitted in the GPU to drawn, there a wide range of hardware setting that
determine how the input data is interpreted and rendered. those setting call Graphic pipeline state
and include common setting such as the rasterizer, blend state ...
The most os the graphic state are setting by PSO.(pipeline state objs)
https://docs.microsoft.com/en-us/windows/win32/direct3d12/managing-graphics-pipeline-state-in-direct3d-12
Root signature
Descriptor table are logically an array of descriptors[Descriptor table[offset][seize]] -> Descriptor heap[Descriptor 0...n].
Root signature define data(resource) that shader access. defines what types of resources are bound to the graphics pipeline.
Contain Root constant, Root descriptor, and Descripto table.
Resources descriptor and descriptor heap a resources binding.
A descriptro small block of data describe an obj in GPU.
2 stage, shader register mapped to descripto in descriptor heap. The descriptor(SRV, UAV, CBV or Sampler)
reference the resource in GPU memory
http://diligentgraphics.com/diligent-engine/architecture/d3d12/managing-descriptor-heaps/
资源绑定
其中CBV _SRV_UAV和Sampler类型的Descriptor Heap可以创建为Shader可见的。Shader可见的意思就是Shader执行时可以访问
当我们提交一个绘制命令的时候，我们需要绑定资源到渲染管线上，这样GPU才能获取到资源。
但是，GPU资源不是直接被绑定的，而是通过一种引用的方式来绑定。这个资源描述符（descriptor）就是用来引用资源的工具。
*/
int main()
{
	return 0;
}