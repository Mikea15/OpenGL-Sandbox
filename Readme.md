## Modern OpenGL sandbox application.

This is a simple sandbox application that I've used to learn more about OpenGL, shaders, and some other subjects I am interested in. At this point, this app allows the creation of fast demo scenes where I can test stuff on.

Expect unoptimized code ( not the objective ), comments here and there, broken or half-baked solutions, duplicate code, stuff that I'm moving around and re-organizing with time.

---

### Things Implemented
- Deferred Rendering, Rendering to Texture, Shadow Mapping, IBL, Post Processing
- Async texture loading ( Producer / Consumer ) with multiple threads.
- ImGUI Integration
- LivePP Integration ( Trial over now )
- QuadTree / Octress Implementation
- Frustum Culling ( with and without Octrees )

### Things Planned, Started or Unfinished
- Material Definitions, Async Loading
- Scene Graph
- Occlusion and Portal Culling
- Simple Profiler ( w/ ImGUI Display )
- Entity Components and Serialization
- Others stuff I'll remember later

### Dependencies
- SDL2 2.0.8
- Glew 2.1.0
- GLM
- Assimp 4.1.0
- ImGUI
- stb_image
- nlohmannJson
