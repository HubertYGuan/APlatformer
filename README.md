# APlatformer (Placeholder Name)

**Currently still being prototyped**
A platformer/FPS made in Unreal Engine 5.2. 
> "Media" is an orphan branch with media for the repo

![Level 2 Preview](https://raw.githubusercontent.com/dorioto/APlatformer/Media/Media/HighresScreenshot_2023.12.04-12.22.04.png?token=GHSAT0AAAAAACIUE25NHAOQCG7MLZLB6O5AZLOXORA)
## Features
> Mostly implemented in C++ without plugins

### First Person Movement System
- Inspired by certain games by Respawn Entertainment
- Enhanced Input
- *Movement mechanics get unlocked by collecting items or being in a certain area*
- **Climbing**
	- You can climb up on any sufficiently steep surface for a pretty long time (if you have the item)
	- You are allowed to climb for a certain amount of time (non-consecutively)
	- This climbing timer is reset via another timer or landing
	- You can hang indefinitely if climbing close to a ledge
- **Sliding**
	- Lowers ground friction and boosts you forward depending on ground slope
	- Boost has a cooldown
- **Wall Jumping**
	- If you can slide and climb, you can wall jump after jumping while sliding
	- You typically have to wait a little bit on the wall before jumping
- Sprinting and double jumping (hold for higher double jumps)
- Coyote time and jump buffering
- **Lurch**
	- Allows for quick movement redirection by pressing movement keys after a jump
	- Comes at the cost of total speed (more sharp = more speed loss)
- **Supergliding**
	- Press crouch and then jump in quick succession at the end of a mantle to boost forward
- **Wall Running *(WIP)***
### Levels
- 2 tutorial levels to unlock and get used to movement and game mechanics
- (to be added) Further levels with enemies and objectives
- (to be added sooner) a "gauntlet" parkour course to try to compete as fast as possible
### Gun(s) (soon)
### Menu
- **Saving and loading system**
	- auto-saves when entering a new level
- Options (coming in $$
\tan(\frac{1}{2}(\int_0^\infty t^{-1/2}e^{-t}dt\,)^2)
$$days)

## Assets
- Epic Games Quixel Megascans
- Epic Games Starter Content
- Modular Building Set by PurePolygons
- Free models from TurboSquid
- UINav 3.0 by Goncas Mage

> P.S. this is a solo project for now, please don't fork
