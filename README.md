# Elysium

Elysium is a personal passion project of mine. The long-term goal is to make a fully functional powerful 3D game engine, with support for physically-based 3d rendering, multiple rendering APIs(Vulkan, OpenGl, DirectX etc), 2D and 3D physics, AI, 2D and 3D Audio, Scripting behaviour and fully-featured editor. Current goal is to have complete 2D worflow setup.

## Getting Started
1. Clone this repository
2. Open the `Scripts/` directory and run the appropriate `Setup` script to generate projects files. You can edit the setup script to change the type of project that is generated - out of the box it is set to Visual Studio 2022 for Windows.

## Current Features
- [X] Entity Drag and Drop
- [X] Entity Inspector and Attach Components to Entities. 
- [X] Save and load Entity component data. 
- [X] Make a Play Test Scene, where the created level and its systems(collision, animation, etc) can be tested.
- [X] World view/camera to make bigger levels
- [X] basic Asset Management system
- [X] Entity Parent-Child relationship

### Physics Features
- [X] Polygon Collider
- [X] Circle collider
- [X] Box collider
- [X] SAT for narrow phase detection
- [X] Basic 2D Rigidbody physics
- [X] Hinge Joint

  
### Features currently in development
- [ ] Basic Scripting Support
- [ ] Prefabs
- [ ] Editor Improvements

### Features to come
- [ ] Complete 2D rendering using Vulkan and/or OpenGL (currently all graphics are being rendered using SFMl, plan is to get rid of that completely)
- [ ] Complete Scripting Capabilities

