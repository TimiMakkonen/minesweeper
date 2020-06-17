#include <stdexcept> // std::out_of_range
#include <vector> // std::vector
#include <algorithm> // std::max
#include <memory> // std::unique_ptr
#include <numeric> // std::iota
#include <assert.h> // assert
#include <iomanip> // std::setw
#include <sstream> // std::ostringstream
#include <chrono> // std::chrono

#include <nlohmann/json.hpp> // nlohmann::json

#include <minesweeper/game.h>
#include <minesweeper/cell.h>

namespace minesweeper {

	// initialisation of static fields:
	IRandom* Game::defaultRandom = nullptr;


	Game::Game() = default;

	Game::Game(int gridSize, int numOfMines, IRandom* random) : Game(gridSize, gridSize, numOfMines, random) {}

	Game::Game(int gridHeight, int gridWidth, int numOfMines, IRandom* random) 
			: gridHeight(this->verifyGridDimension(gridHeight)), // throws
				gridWidth(this->verifyGridDimension(gridWidth)), // throws
				numOfMines(this->verifyNumOfMines(numOfMines)), // throws
				random(random), 
				cells(this->initCells()) {}


	// required by to solve "error C2027: use of undefined type"
	// in short, std::unique_ptr requires destructor to be defined here
	// Check more info on this:
	// https://stackoverflow.com/questions/40383395/use-of-undefined-type-with-unique-ptr-to-forward-declared-class-and-defaulted
	// https://stackoverflow.com/questions/6012157/is-stdunique-ptrt-required-to-know-the-full-definition-of-t
	// https://stackoverflow.com/questions/13414652/forward-declaration-with-unique-ptr
	Game::~Game() = default;
	
	int Game::verifyGridDimension(int gridDimension) const {

		if (gridDimension < 0) {
			throw std::out_of_range("Game::verifyGridDimension(int gridDimension): "
									"Trying to create a grid with negative ("
									+ std::to_string(gridDimension) +
									") grid dimension.");
		}
		return gridDimension;
	}

	int Game::verifyNumOfMines(int numOfMines) const {

		int maxNumOfMinesForThisGrid = maxNumOfMines(this->gridHeight, this->gridWidth);
		if (numOfMines > maxNumOfMinesForThisGrid) {
			throw std::out_of_range("Game::verifyNumOfMines(int numOfMines): "
									"Trying to create a grid with too many ("
									+ std::to_string(numOfMines) +
									") mines.");
		} else if(numOfMines < minNumOfMines()) {
			throw std::out_of_range("Game::verifyNumOfMines(int numOfMines): "
									"Trying to create a grid with too few ("
									+ std::to_string(numOfMines) +
									") mines.");
		}
		return numOfMines;
	}


	int Game::verifyNumOfMarkedMines(int numOfMarkedMines) const {

		if (numOfMarkedMines > this->numOfMines) {
			throw std::out_of_range("Game::verifyNumOfMarkedMines(int numOfMarkedMines): "
									"Trying to create a grid with too many ("
									+ std::to_string(numOfMarkedMines) +
									") marked mines.");
		} else if(numOfMarkedMines < 0) {
			throw std::out_of_range("Game::verifyNumOfMarkedMines(int numOfMarkedMines): "
									"Trying to create a grid with too few ("
									+ std::to_string(numOfMarkedMines) +
									") marked mines.");
		}
		return numOfMarkedMines;
	}


	int Game::verifyNumOfWronglyMarkedCells(int numOfWronglyMarkedCells) const {
		
		int numOfMinelessCells = this->gridHeight * this->gridWidth - this->numOfMines;
		if (numOfWronglyMarkedCells > numOfMinelessCells) {
			throw std::out_of_range("Game::verifyNumOfWronglyMarkedCells(int numOfWronglyMarkedCells): "
									"Trying to create a grid with too many ("
									+ std::to_string(numOfWronglyMarkedCells) +
									") wrongly marked cells.");
		} else if(numOfWronglyMarkedCells < 0) {
			throw std::out_of_range("Game::verifyNumOfWronglyMarkedCells(int numOfWronglyMarkedCells): "
									"Trying to create a grid with too few ("
									+ std::to_string(numOfWronglyMarkedCells) +
									") wrongly marked cells.");
		}
		return numOfWronglyMarkedCells;
	}


	int Game::verifyNumOfVisibleCells(int numOfVisibleCells) const {

		if (numOfVisibleCells > this->gridHeight * this->gridWidth) {
			throw std::out_of_range("Game::verifyNumOfVisibleCells(int numOfVisibleCells): "
									"Trying to create a grid with too many ("
									+ std::to_string(numOfVisibleCells) +
									") visible cells.");
		} else if(numOfVisibleCells < 0) {
			throw std::out_of_range("Game::verifyNumOfVisibleCells(int numOfVisibleCells): "
									"Trying to create a grid with too few ("
									+ std::to_string(numOfVisibleCells) +
									") visible cells.");
		}
		return numOfVisibleCells;
	}

	// consider combining with Game::resizeCells()
	std::vector< std::vector< std::unique_ptr<Cell> > > Game::initCells() {

		std::vector< std::vector< std::unique_ptr<Cell> > > initTempCells;
		initTempCells.reserve(this->gridHeight);
		for (int i = 0; i < this->gridHeight; ++i) {
			std::vector< std::unique_ptr<Cell> > tempVecOfCells;
			tempVecOfCells.reserve(this->gridWidth);
			for (int j = 0; j < this->gridWidth; ++j) {
				tempVecOfCells.emplace_back(std::make_unique<Cell>());
			}
			initTempCells.emplace_back(std::move(tempVecOfCells));
		}
		return initTempCells;
	}


	// consider combining with Game::initCells()
	void Game::resizeCells() {

		this->cells.reserve(this->gridHeight);
		this->cells.resize(this->gridHeight);
		for (auto& cellRow : cells) {

			cellRow.reserve(this->gridWidth);
			if (cellRow.size() < this->gridWidth) {
				for (size_t i = cellRow.size(); i < this->gridWidth; ++i) {
					cellRow.emplace_back(std::make_unique<Cell>());
				}
			}
			cellRow.resize(this->gridWidth);
		}
	}



	void Game::createMinesAndNums(const int initChosenX, const int initChosenY) {

		std::vector<int> mineSpots(this->gridWidth * this->gridHeight);
		this->chooseRandomMineCells(mineSpots, initChosenX, initChosenY);

		int X, Y;
		for (int i = 0; i < this->numOfMines; ++i) {

			X = mineSpots[i] % this->gridWidth;
			Y = mineSpots[i] / this->gridWidth;

			this->createMine(X, Y);

			this->incrNumsAroundMine(X, Y);
		}
		
		this->minesHaveBeenSet = true;
	}



	void Game::chooseRandomMineCells(std::vector<int>& mineSpots, const int initChosenX, const int initChosenY) const {

		// to populate mineSpots vector with values: 0, 1, 2, ..., gridWidth*gridHeight - 1
		std::iota(mineSpots.begin(), mineSpots.end(), 0);

		// to shuffle this vector
		this->randomizeMineVector(mineSpots);

		// to remove bad gridspots (those on and around chosen initial spot)
		mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
			(initChosenY * this->gridWidth + initChosenX)), mineSpots.end());
		if (initChosenX > 0) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				(initChosenY * this->gridWidth + initChosenX - 1)), mineSpots.end());	//left
		}
		if (initChosenX < this->gridWidth - 1) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				(initChosenY * this->gridWidth + initChosenX + 1)), mineSpots.end());	// right
		}
		if (initChosenY > 0) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY - 1) * this->gridWidth + initChosenX)), mineSpots.end());	// top
		}
		if (initChosenY < this->gridHeight - 1) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY + 1) * this->gridWidth + initChosenX)), mineSpots.end());	// bottom
		}
		if (initChosenY > 0 && initChosenX > 0) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY - 1) * this->gridWidth + initChosenX - 1)), mineSpots.end());	// top left
		}
		if (initChosenY > 0 && initChosenX < this->gridWidth - 1) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY - 1) * this->gridWidth + initChosenX + 1)), mineSpots.end());	// top right
		}
		if (initChosenY < this->gridHeight - 1 && initChosenX > 0) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY + 1) * this->gridWidth + initChosenX - 1)), mineSpots.end());	// bottom left
		}
		if (initChosenY < this->gridHeight - 1 && initChosenX < this->gridWidth - 1) {
			mineSpots.erase(std::remove(mineSpots.begin(), mineSpots.end(),
				((initChosenY + 1) * this->gridWidth + initChosenX + 1)), mineSpots.end());	// bottom right
		}
	}

	

	void Game::randomizeMineVector(std::vector<int>& mineSpots) const {
		
		if (this->random != nullptr) {
			this->random->shuffleVector(mineSpots);
		} else {
			if (Game::defaultRandom != nullptr) {
				Game::defaultRandom->shuffleVector(mineSpots);
			} else {
				throw std::invalid_argument("Game::randomizeMineVector(std::vector<int>& mineSpots): "
									 "Neither field 'random' nor static field 'defaultRandom' is initialised.");
			}
		}
	}


	void Game::createMine(const int X, const int Y) {

		assert(X >= 0 && Y >= 0 && X < this->gridWidth && Y < this->gridHeight);
		this->cells[Y][X]->putMine();
	}



	void Game::incrNumsAroundMine(const int X, const int Y) {

		assert(X >= 0 && Y >= 0 && X < this->gridWidth && Y < this->gridHeight);
		if (X > 0) {
			this->cells[Y][X - 1]->incrNumOfMinesAround();	//left
		}
		if (X < this->gridWidth - 1) {
			this->cells[Y][X + 1]->incrNumOfMinesAround();	// right
		}
		if (Y > 0) {
			this->cells[Y - 1][X]->incrNumOfMinesAround();	// top
		}
		if (Y < this->gridHeight - 1) {
			this->cells[Y + 1][X]->incrNumOfMinesAround();	// bottom
		}
		if (Y > 0 && X > 0) {
			this->cells[Y - 1][X - 1]->incrNumOfMinesAround();	// top left
		}
		if (Y > 0 && X < this->gridWidth - 1) {
			this->cells[Y - 1][X + 1]->incrNumOfMinesAround();	// top right
		}
		if (Y < this->gridHeight - 1 && X > 0) {
			this->cells[Y + 1][X - 1]->incrNumOfMinesAround();	// bottom left
		}
		if (Y < this->gridHeight - 1 && X < this->gridWidth - 1) {
			this->cells[Y + 1][X + 1]->incrNumOfMinesAround();	// bottom right
		}
	}


	// to mark (or unmark) given coordinates, and keeping track of marked and wrongly marked mines
	void Game::markInputCoordinates(const int X, const int Y) {

		if (X < 0 || Y < 0 || X >= this->gridWidth || Y >= this->gridHeight) {
			throw std::out_of_range("Game::markInputCoordinates(const int X, const int Y): "
									"Trying to mark cell outside grid.");
		}

		if (!minesHaveBeenSet) {
			throw std::invalid_argument("Game::markInputCoordinates(const int X, const int Y): "
										"Trying to mark a cell before mines hava been initialised. " 
										"(Initialise mines by calling: "
										"createMinesAndNums(const int initChosenX, const int initChosenY) "
										"or checkInputCoordinates(const int X, const int Y).)");
		}


		if (this->isCellMarked(X, Y)) {
			this->unmarkCell(X, Y);

			if (this->doesCellHaveMine(X, Y)) {
				--(this->numOfMarkedMines);
			}
			else {
				--(this->numOfWronglyMarkedCells);
			}
		}
		else {
			this->markCell(X, Y);

			if (this->doesCellHaveMine(X, Y)) {
				++(this->numOfMarkedMines);
			}
			else {
				++(this->numOfWronglyMarkedCells);
			}
		}
	}



	bool Game::allMinesMarked() const {

		if (this->numOfMines == this->numOfMarkedMines) {
			return true;
		}
		else {
			return false;
		}
	}

	bool Game::noNonMinesMarked() const {
		if (this->numOfWronglyMarkedCells == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	bool Game::allNonMinesVisible() const {

		if (this->numOfVisibleCells + this->numOfMines == this->gridWidth * this->gridHeight && !this->_checkedMine) {
			return true;
		}
		else {
			return false;
		}
	}

	bool Game::playerHasWon() const {

		bool playerWon = false;

		if (!this->playerHasLost()) {
			if (this->allNonMinesVisible()) {
				playerWon = true;
			} else if (this->allMinesMarked() && this->noNonMinesMarked()) {
				playerWon = true;
			}
		}
		return playerWon;
	}

	bool Game::playerHasLost() const {

		if (this->checkedMine()) {
			return true;
		}
		else {
			return false;
		}
	}

	bool Game::checkedMine() const {
		return this->_checkedMine;
	}

	bool Game::isCellVisible(const int X, const int Y) const {

		return this->cells[Y][X]->isVisible();
	}

	bool Game::doesCellHaveMine(const int X, const int Y) const {

		return this->cells[Y][X]->hasMine();
	}

	bool Game::isCellMarked(const int X, const int Y) const {

		return this->cells[Y][X]->isMarked();
	}

	int Game::numOfMinesAroundCell(const int X, const int Y) const {

		return this->cells[Y][X]->numOfMinesAround();
	}

	void Game::makeCellVisible(const int X, const int Y) {
		this->cells[Y][X]->makeVisible();
	}

	void Game::markCell(const int X, const int Y) {
		this->cells[Y][X]->markCell();
	}

	void Game::unmarkCell(const int X, const int Y) {
		this->cells[Y][X]->unmarkCell();
	}

	// to check user given coordinates, and make it visible
	void Game::checkInputCoordinates(const int X, const int Y) {

		if (X < 0 || Y < 0 || X >= this->gridWidth || Y >= this->gridHeight) {
			throw std::out_of_range("Game::checkInputCoordinates(const int X, const int Y): "
									"Trying to check cell outside grid.");
		}
		
		if (!minesHaveBeenSet) {
			this->createMinesAndNums(X, Y);
		}

		if (!(this->isCellVisible(X, Y)) && !(this->isCellMarked(X, Y))) {
			this->makeCellVisible(X, Y);
			++(this->numOfVisibleCells);

			if (this->doesCellHaveMine(X, Y)) {
				this->_checkedMine = true;
			}
			else if (this->numOfMinesAroundCell(X, Y) == 0) {
				this->checkAroundCoordinate(X, Y);
			}
		}
	}

	
	void Game::checkAroundCoordinate(const int X, const int Y) {

		assert(X >= 0 && Y >= 0 && X < this->gridWidth && Y < this->gridHeight);
		if (X > 0) {
			this->checkInputCoordinates(X - 1, Y);		// left
		}
		if (X < this->gridWidth - 1) {
			this->checkInputCoordinates(X + 1, Y);		// right
		}
		if (Y > 0) {
			this->checkInputCoordinates(X, Y - 1);		// top
		}
		if (Y < this->gridHeight - 1) {
			this->checkInputCoordinates(X, Y + 1);		// bottom
		}
		if (X > 0 && Y > 0) {
			this->checkInputCoordinates(X - 1, Y - 1);	// top left
		}
		if (Y > 0 && X < this->gridWidth - 1) {
			this->checkInputCoordinates(X + 1, Y - 1);	// top right
		}
		if (Y < this->gridHeight - 1 && X > 0) {
			this->checkInputCoordinates(X - 1, Y + 1);	// bottom left
		}
		if (X < this->gridWidth - 1 && Y < this->gridHeight - 1) {
			this->checkInputCoordinates(X + 1, Y + 1);	// bottom right
		}
	}


	int Game::getGridHeight() const {

		return this->gridHeight;
	}


	int Game::getGridWidth() const {

		return this->gridWidth;
	}


	int Game::getNumOfMines() const {

		return this->numOfMines;
	}


	std::ostream& Game::serialise(std::ostream& outStream) const {

		nlohmann::json j;

		// magic and version information:
		j["magic"] = "Timi's Minesweeper";
		j["version"] = "1.0";

		// current game fields:
		j["currentGame"]["gridHeight"] = this->gridHeight;
		j["currentGame"]["gridWidth"] = this->gridWidth;
		j["currentGame"]["numOfMines"] = this->numOfMines;
		j["currentGame"]["numOfMarkedMines"] = this->numOfMarkedMines;
		j["currentGame"]["numOfWronglyMarkedCells"] = this->numOfWronglyMarkedCells;
		j["currentGame"]["numOfVisibleCells"] = this->numOfVisibleCells;
		j["currentGame"]["_checkedMine"] = this->_checkedMine;
		j["currentGame"]["minesHaveBeenSet"] = this->minesHaveBeenSet;

		// cell data:
		if (this->gridHeight != 0 && this->gridWidth != 0) {
			for (int y = 0; y < this->gridHeight; ++y) {
				
				nlohmann::json jRowObject;
				jRowObject["rowNumber"] = y;

				for (int x = 0; x < this->gridWidth; ++x) {

					nlohmann::json jCellObject;
					jCellObject["columnNumber"] = x;

					//std::stringstream outCellStream;
					//cells[y][x]->serialise(outCellStream);
					//jCellObject["cell"] = nlohmann::json::parse(outCellStream.str());
					jCellObject["cell"] = cells[y][x]->serialise();

					jRowObject["rowCells"].push_back(jCellObject);
				}

				j["currentGame"]["cells"].push_back(jRowObject);
			}
		}

		outStream << std::setw(4) << j << std::endl;

		return outStream;
	}


	std::istream& Game::deserialise(std::istream& inStream) {

		try {
		
		nlohmann::json j;
		inStream >> j;

		if (j.at("magic") == "Timi's Minesweeper") {
			if (j.at("version") == "1.0") {

				// current game fields:
				this->gridHeight = this->verifyGridDimension(j.at("currentGame").at("gridHeight"));
				this->gridWidth = this->verifyGridDimension(j.at("currentGame").at("gridWidth"));
				this->numOfMines = this->verifyNumOfMines(j.at("currentGame").at("numOfMines"));
				this->numOfMarkedMines = this->verifyNumOfMarkedMines(j.at("currentGame").at("numOfMarkedMines"));
				this->numOfWronglyMarkedCells = 
					this->verifyNumOfWronglyMarkedCells(j.at("currentGame").at("numOfWronglyMarkedCells"));
				this->numOfVisibleCells = this->verifyNumOfVisibleCells(j.at("currentGame").at("numOfVisibleCells"));
				this->_checkedMine = j.at("currentGame").at("_checkedMine");
				this->minesHaveBeenSet = j.at("currentGame").at("minesHaveBeenSet");

				// resize cells/grid to accept their data
				this->resizeCells();

				// cell data:
				if (this->gridHeight != 0 && this->gridWidth != 0) {
					for (auto& jRowObject : j.at("currentGame").at("cells")) {
						
						const int y = jRowObject.at("rowNumber");

						for (auto& jCellObject : jRowObject.at("rowCells")) {

							const int x = jCellObject.at("columnNumber");

							//std::stringstream inCellStream;
							//inCellStream << jCellObject.at("cell");
							cells[y][x]->deserialise(jCellObject.at("cell"));
						}

					}
					int x = 1;
				}

			}
		}

		} catch (nlohmann::json::parse_error& ex) {
			throw std::invalid_argument("Game::deserialise(std::istream& inStream): "
								  "Argument is not valid JSON.\n\t"
								  + std::string(ex.what()));
		} catch (nlohmann::json::type_error& ex) {
			throw std::invalid_argument("Game::deserialise(std::istream& inStream): "
								  "Argument is not a JSON object.\n\t"
								  + std::string(ex.what()));
		} catch (nlohmann::json::out_of_range& ex) {
			throw std::invalid_argument("Game::deserialise(std::istream& inStream): "
								  "Argument key does not exist.\n\t"
								  + std::string(ex.what()));
		}

		return inStream;
	}


	// static method
	void Game::setDefaultRandom(IRandom* defaultRandom) {
		Game::defaultRandom = defaultRandom;
	}

	// static method
	int Game::maxNumOfMines(int gridH, int gridW) {

		if(gridH < 0 || gridW < 0) {
			throw std::out_of_range("Game::maxNumOfMines(int gridH, int gridW): "
									"Trying to check maximum number of mines for a negative sized grid.");
		}
		
		return std::max(gridH * gridW - 9, 0);
	}


	// static method
	int Game::minNumOfMines() {

		return 0;
	}


	// static method
	int Game::minNumOfMines(int gridH, int gridW) {

		if(gridH < 0 || gridW < 0) {
			throw std::out_of_range("Game::minNumOfMines(int gridH, int gridW): "
									"Trying to check minimum number of mines for a negative sized grid.");
		}

		return minNumOfMines();
	}
}