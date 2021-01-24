# PBRDx12

## Resource Binding
https://docs.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-flow-of-control
The key to resource binding in DirectX 12 are the concepts of a descriptor, descriptor tables, descriptor heaps, and a root signature.

Shader resources (such as textures, constant tables, images, buffers and so on) are not bound directly to the shader pipeline; instead, they are referenced through a descriptor. A descriptor is a small object that contains information about one resource.

Descriptors are grouped together to form descriptor tables. Each descriptor table stores information about one range of types of resource. There are many different types of resources. The most common resources are:

- Constant buffer views (CBVs)
- Unordered access views (UAVs)
- Shader resource views (SRVs)
- Samplers

The graphics and compute pipelines gain access to resources by referencing into descriptor tables by index.

Descriptor tables are stored in a descriptor heap. Descriptor heaps will ideally contain all the descriptors (in descriptor tables) for one or more frames to be rendered. All the resources will be stored in user mode heaps.

Another concept is that of a root signature. The root signature is a binding convention, defined by the application, that is used by shaders to locate the resources that they need access to. The root signature can store:

- Indexes to descriptor tables in a descriptor heap, where the layout of the descriptor table has been pre-defined.
- Constants, so apps can bind user-defined constants (known as root constants) directly to shaders without having to go through descriptors and descriptor tables.
- A very small number of descriptors directly inside the root signature, such as a constant buffer view (CBV) that changes per draw, thereby saving the application from needing to put those descriptors in a descriptor heap.
