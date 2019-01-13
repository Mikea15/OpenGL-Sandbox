# Sandbox
Modern OpenGL sandbox application.
---
This is a simple sandbox application that I've used to learn more about OpenGL, shaders, and some other subjects I am interested in. At this point, this app allows the creation of fast demo scenes where I can test stuff on.

Things I've played arround with:
- Async texture loading ( Producer / Consumer ) with multiple threads.
- Frustum Occlusion Culling
- QuadTree / Octress Implementation
- Frustum Occlusion Culling with Octree Acceleration
- Deferred Rendering, Rendering to Texture, Shadow Mapping.
- Post Processing FX
- Image Based Lighting

### Planned, Started or Unfinished
Some things I still want to explore and build upon
- Implement Material Definitions, Async Loading
- Scene Graph
- Portal Culling

### Dependencies
- SDL2 2.0.8
- Glew 2.1.0
- GLM
- Assimp 4.1.0
- ImGUI
- std_image
- nlohmannJson