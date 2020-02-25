# TerrainGenerator
A simple program that takes a heightmap and generates an interactable and lightable terrain that can also be permanently deformed and destroyed.

![](Resources/Gameplay2.gif)

#### - Project Status: [Completed]

### Built with
  * C++
  * [OpenGL](https://www.opengl.org/)
  * [EGC-Framework](https://github.com/UPB-Graphics/Framework-EGC)
  * [stb_image](https://github.com/nothings/stb)
  
### More examples

![](Gameplay1.gif)

#### Lighting
The lighting is based on the Phong reflection model, this brings out every creek and valley and makes it easier to spot the normals when deforming the terrain.

The terrain is generated from a heightmap and scaled up afterwards to adequate resolution.

The terrain can be **deformed** by throwing projectiles at it. They are thrown in the direction the third person camera is looking, and create a crater radially from the point of impact, with a set Area Of Effect.

The normals are recalculated after each impact to ensure corect lighting inside the craters.

## Usage
Move the camera freely with `W` `A` `S` `D` `Q` `E` `RClick` `LCtrl` and shoot projectiles with `P`, switch to birds eye view with `Space`.
## Completion Date
2019, December 7
