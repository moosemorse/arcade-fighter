# Arcade Fighter

A 2D arcade-style fighting game built in **C** using the **SDL2** library.

![Arcade Fighter Gameplay](TODO: INSERT PICTURE)
*Current build: Featuring dynamic hitboxes (red/green) and a moody, atmospheric battlefield.*

---

## 🕹 Controls
Grab a friend and battle it out on a single keyboard (or custom controllers if you have the time! - see more info below).

| Action | **Player 1** | **Player 2** |
| :--- | :--- | :--- |
| **Move** | `W` `A` `S` `D` | `←` `→` `↑` `↓` |
| **Jump** | `5` | `N` |
| **Attack** | `6` | `M` |

---

### 🛠 Installation
To simplify the process, the project currently includes local libraries. In the future, this could be handled by an automated installation script to keep the repository lightweight.

1. **Clone the repository.**
2. **Navigate to the source directory:**  ```cd src/```
3. **Compile the project**:  ```make```

### Running the game
Launch the executable from the src/ directory:  ```./main```

[!!] **WSL2 USERS**: If the game crashes on startup (specifically an AddressSanitizer SEGV), run the program using the following command: ```LIBGL_ALWAYS_SOFTWARE=1 ./main```
This problem likely arises due to WSL2's hardware acceleration bridge for Windows GPU drivers and how it conflicts with the memory sanitisers used during development.
So, when the app is run in WSL2, the code is in Linux but the GPU is in windows and ASan gets confused by the Windows Intel driver hence crashing.

### Hardware (Custom controllers)
The game wasn't just built for keyboards, it was actually designed to be played on a Raspberry Pi (RPi) using custom-built arcade hardware.
[TODO: INSERT PICTURE]
The physical controllers were constructed using:
- Arcade buttons and joysticks (bought from ebay)
- Recycled boxes and wiring
- An ADS1115 Board (Analog-to-Digital Converter) to bridge the joystick and the RPi

A rough idea of how the pipeline works is the following:
1. The joystick and buttons were mounted into the box and wired to the ADS board.
2. The ADS board was connected to the corresponding GPIO pins on the RPi via the I2C interface
3. The input API: To translate physical movements into game actions, I developed an interface using the ```pigpio``` and ```libads1115.a``` library. This system:
- Reads the raw digital/analog signals from the hardware (manage I2C communication for the joystick and monitor GPIO pins for button presses)
- Converts those signals into a usable playerInput struct e.g. how much the player is moving to the right or left, if they clicked attack button etc.
- Passes that struct into the game engine every tick

*Note on Project version*: Due to a version recovery from an older backup, some of the specific "Input API" code is currently being restored. The project currently uses the keyboard as the primary input method while I try and recover the original code.

### Project Status & TODOs
As I mentioned above, due to some problems recovering the newest version of this project there are some features missing. In the meantime I will leave these as todo's and note down other things I could do if I get around to it:
- **Game over loop**: Re-implement the end of game screen and the "Press Start to Replay" logic
- **Audio**: Fully restore the SDL_Mixer implementation to bring back the background music and combat sound effects
- **Refactoring**
- **Installation process**: Create a setup script to pull in necessary SDL2 dependencies rather than storing them on repo
- **Controller calibration**: Re-implement custom controller API
