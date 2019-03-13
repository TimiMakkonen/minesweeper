# Basic Minesweeper game on console

The purpose of this program is to see/show/test if I can program a minesweeper game on my own.

---

### Note:

I have made sure not to look up codes of any other minesweeper programs prior making this program. Hence this program and the methods used in it to make it work are all mostly 'original'.
Throughout making this program, I have only googled some methods, good practices, algorithms and syntax to make things work the way I want them to.

---

### Version History:


#### Version 4:

* Continued with improvements on the input system.
* Increased maximum size of the grid up to 99x99. (I do not recommend grids larger than 50x50.)
* Program now shows your last grid spot input.
* Made function 'TryInputToCoordinates' mostly future proof.
	* can handle grid sizes up to 675 (overkill tbh)
	* able to give error messages and decide when new input is needed.
* Improved games structure for all the different possibilities.
* Added and made HELP menu optional. (show up in the beginning and later upon request)
* Improved instructions and other text output.
* Added ability to play again.
* Made sure that you cannot lose on the first turn.
	* Creation of the grid has been moved to take into account players first choice, making sure player will make some progress.

#### Version 3:

* Slightly improved readability of the code.
* Started to deal with proper handling of wrong inputs.
* Added initial win screen and win condition, if you manage to mark all mines.
* Ability to unmark grid spots by marking them again (in case you made a mistake).
* Initial options/special input menu implemented.


#### Version 2:

* Fixed some visual mistakes and made the game look prettier 
* Added automatic mine check for squares around, when choosing a grid location with no surrounding mines.
* Fixes input for 2 digit numbers.
* Initial losing screen.
* More convenient input system.
* \+ Background work to make next additions/fixes easier to implement.

#### Version 1:


Initial version of the game. Mostly in working condition.

---

### Fixes and features left to consider/implement:

* Increasing current maximum gridsize from 99x99? Madness?
* Separating column and row size to allow 'non-square' grids.
* Making sure incorrect inputs are less likely to cause bugs.
* Making the game look prettier.
* Better losing screen.
* Improve Win screen and winning conditions.
* Improve HELP menu.
* Edit input system overall.
* Add an option to add mines by percentage, not by quantity.

