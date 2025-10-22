# Pikranji - The Kanji Picross

-----

## 🎯 Game Objective

**Pikranji** is a version of the popular logic game Picross (also known as Nonogram or Griddlers), where the objective is to reveal a hidden image on a grid.

In this game, the hidden image is always a **Kanji** (Japanese character), and its meaning is displayed just below the grid to provide a hint.

To win, you must correctly fill in the cells of the 15x15 grid, following the numerical clues provided on the sides and at the top.

This game primarily has an educational objective, which is to learn the thousands of existing Kanji characters in a gamified way. Feel free to add those that are not included using the generator.

-----

## 🔍 How to Play

The board is composed of a central grid and two clue areas:

* **Side Clues (Left):** Indicate the number and size of the blocks of filled (black) cells in each **row**.
* **Top Clues (Top):** Indicate the number and size of the blocks of filled (black) cells in each **column**.

**Example:**
If a row clue is `[3 5 1]`, this means that row contains:

1.  A block of 3 filled cells.
2.  At least one empty/marked cell.
3.  A block of 5 filled cells.
4.  At least one empty/marked cell.
5.  A block of 1 filled cell.

### 🛠️ Interaction Modes (Tools)

You can switch between two tools using the **`⬛` (Fill)** and **`❌` (Mark)** buttons:

| Tool | Icon | Main Use |
| :--- | :--- | :--- |
| **Fill** | `⬛` | Marks a cell as filled (black color). |
| **Mark** | `❌` | Marks a cell with an **`×`** (to indicate it should remain empty). |

### 🖱️ Mouse Interaction

* **Quick Click:** Toggles the cell's state according to the selected tool.
* **Drag:** Holds the mouse button down and drags. The first cell clicked defines the action for all subsequent cells.

### 📱 Touch Interaction (Mobile)

* **Quick Tap (Tap):** Activates toggle mode on the cell, according to the selected tool.
* **Drag (Drag/Swipe):** The immediate movement after the tap activates drag mode to fill/mark multiple cells.
* **Long Press (\~0.5s):** Works like a quick tap (toggle), useful for ensuring a slower tap is registered.

### Control Buttons

* **Restart:** Clears the current grid and returns to the initial state, keeping the same Kanji.
* **Solve:** Reveals the correct solution (can be used as a last resort!).
* **Next Kanji:** Loads a new puzzle and a new random Kanji.

-----

## ✅ Completed Clues

When the filled cells in a row or column exactly match the clue numbers for that row/column, the numerical clues will be crossed out, indicating that the row/column is **completed**.

**The game ends and you win** when all rows and columns are completed and the Kanji image is fully revealed.

-----

## 🚀 Adding New Puzzles (Kanji)

You can easily add new challenges to the game by directly editing the **`puzzles.json`** file. A great, free, and open-source text editor for this task is **Vim**. This file is an *Array* of JSON objects, where each object represents a complete puzzle. (you can also use the included generator)

### Puzzle Structure

Each puzzle object must follow the format below and be appended to the main array:

| Field | Type | Description |
| :--- | :--- | :--- |
| `"kanji"` | String | The Japanese character that will be displayed. |
| `"meaning"` | String | The meaning of the Kanji. |
| `"grid"` | Array (15x15) | The solution grid. Must be a 15 by 15 matrix, where **`1`** represents a filled cell (black) and **`0`** represents an empty cell. |

### JSON Code Example

```json
{
                "kanji": "田",
                "meaning": "Rice Field",
                "grid": [
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                      	[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        	        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
          	        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
          	        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
          	        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
          	        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
          	        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
  	          ]
}
