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

### Animation System  
- [X] Add and play simple 2d sprite sheet based Animation
- [ ] Animation state Transitions
- [ ] Multiple animation tracks: position, scale, etc
- [ ] Event support
- [ ] multiple animation layers support
- [ ] Enhanced editing tools which make creating Animations easier

### Scripting System
- [X] Integrate Mono
- [X] Manipulate Transform of entities through c# scripts
- [X] Ability to get, add and remove components through c# scripts
- [X] Texture and Sprite Renderer in c# API
- [ ] Extension of Parent-Child Relation in c# API
- [ ] Rigidbody interface and control physics through c# scripts

### Features to come
- [ ] Prefabs
- [ ] Upgraded Physics engine capabilities, with proper contact management, contact listeners, broadphase detection, raycast etc...
