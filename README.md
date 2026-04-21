# Tomb of the Mask - Pang-All-In

**Repository:** [https://github.com/Soul02G/Tomb-of-the-Task]

**Gameplay Showcase:** [Link yt]

## Team Members

* Sergio Garcia Pérez: @soul02G
* Jordi Fernández Prat: @jordifdezzz
* Sergio Andrés Llanos Orozco: @Serinio28
* Andrea Vasco Peiró: @Andreavsc30
* Vladimir Solovevñ: @Undermancer

## Description
This project is a recreation of the original video game *Tomb of the Mask*, wich is an arcade-style labyrinth game where the difficulty increases with each level.
The gameplay revolves around a dash mechanic that allows the player to move from wall to wall.Players must collect coins and stars, outsmart deadly traps like spike walls,
 and dodge enemies to reach the finish line.

## How to Play
The objective is to navigate the labyrinth and reach the finish line of each level. You move by dashing in a straight line until you hit a wall. 
Beware of touching any elements that cause the character to die.

**Controls:**

* **Movement (Dash):** `W` `A` `S` `D` / `Arrow Keys` (The character moves from wall to wall without stopping )
* **Settings:** `M` 
* **Change settings:** `Enter`
* **Select level:** `Space`

## Features Implemented

* **Labyrinths:** Creation of the labyrinth for each level with increasing difficulty, accessible via a level selector interface.

* **Movement & Mechanics:**

    * **Dash:** Mechanics allowing the player to move from wall to wall.
    * **Camera Tracking:** The camera is strictly centered on the character at all times, regardless of movement.
    * **Rebound:** Special squares that redirect the player towards the square it is facing upon touch.
    * **Trail:** Character trail design that goes along with the movement.
    * **Economy:** Coin and star creation, detection upon touch, and collection into a counter.
    * **UX & UI:** Functional configuration menu with sliders, pause menu, game data saving, a screen upon completion, and a credit screen.

* **Enemies:**

    * **Bats:** Follow a linear, rectilinear movement and cause automatic game loss on contact.
