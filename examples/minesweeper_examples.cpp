
#include <algorithm> // std::reverse, std::iter_swap
#include <chrono>    // std::chrono
#include <fstream>   // std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cout
#include <iterator>  // std::vector<int>::iterator
#include <sstream>   // std::ostringstream

#include <minesweeper/game.h>
#include <minesweeper/i_random.h>
#include <minesweeper/random.h>

// Used for showing off the Minesweeper library and maybe writing some preliminary tests

// function declarations:
void preliminaryTests();
void examples();
void exampleOfSimplestSetup();
void examplesOfRandomControllingSetup();
void usageExamples();
void serialisationTest();
void serialiseGameToFile(const minesweeper::Game& gameToPrint, const std::string& gameSerPath,
                         const std::string& gamePrintPath);
void gamePrinter(std::ostream& outStream, minesweeper::Game const* const gameToPrint);
void playGround1();
void playGround2();
void playGround3();

// (This can be used as a playground to see and use this library in action.)
void preliminaryTests() {

    // You can write some preliminary tests here

    serialisationTest();

    // playGround1();
    // playGround2();
    // playGround3();
}

// this method contains examples with some documentation
void examples() {

    // You can write some examples here

    exampleOfSimplestSetup();

    examplesOfRandomControllingSetup();

    usageExamples();
}

int main() {

    preliminaryTests();

    examples();

    return 0;
}

// +----------------------------------+
// |  Examples / Usage Instructions:  |
// +----------------------------------+

void exampleOfSimplestSetup() {

    // start by initialising instance of 'Random' class
    // (can be initialised to stack or heap)
    minesweeper::Random myRandom;

    // pass grid size (10x10), number of mines(20) and pointer to 'Random'(&myRandom)
    // (can be initialised to stack or heap)
    minesweeper::Game myGame(10, 20, &myRandom);

    // you can also seperately specify width and height of the grid:
    // pass grid height (9), grid width (12), number of mines(22) and pointer to 'Random'(&myRandom)
    minesweeper::Game myGame2(9, 12, 22, &myRandom);

    // alternatively you can initialise 'Random' or 'Game' to heap:
    minesweeper::IRandom* myRandomPtr = new minesweeper::Random();
    minesweeper::Game* myGamePtr = new minesweeper::Game(10, 20, myRandomPtr);

    delete myGamePtr;
    delete myRandomPtr;

    // or use better C++ 'std::unique_ptr' alternative to this: (takes care of garbage collection)

    auto myRandomUniquePtr = std::make_unique<minesweeper::Random>();
    // std::unique_ptr<minesweeper::IRandom> myRandomUniquePtr = std::make_unique<minesweeper::Random>();

    auto myGameUniquePtr = std::make_unique<minesweeper::Game>(10, 20, myRandomUniquePtr.get());
    // std::unique_ptr<minesweeper::Game> myGameUniquePtr
    //		= std::make_unique<minesweeper::Game>(10, 20, myRandomUniquePtr.get());

    // As of writing this, 'Game' does not ( and hopefully never will) delete the 'IRandom' object it has a pointer for
    // but beware that exposing the raw pointer from 'std::unique_ptr' is still dangerous.
    // So it might be advisable to use raw pointer for your 'Random' class instead.

    // instead off assigning 'IRandom' for each instance of 'Game',
    // you can assign a default 'Random' to all instances of 'Game' class
    // this gets used if it is not specifically set for an instance of 'Random' class
    minesweeper::Random myStaticRandom;

    minesweeper::Game::setDefaultRandom(&myStaticRandom);

    // game that uses static 'Random' class:
    minesweeper::Game myGameUsingStaticRandom(10, 20);

    // game that manually overrides static 'Random' class:
    minesweeper::Random myRandom2;
    minesweeper::Game myGameOverridingStaticRandom(10, 20, &myRandom2);

    // be free to use any of these ways to setup your Minesweeper 'Game'
    // just make sure that 'Random' variable is alive throughout your usage of the Minesweeper 'Game' :)
}

void examplesOfRandomControllingSetup() {

    // I recommend quickly reading through 'exampleOfSimplestSetup()' first to learn basic setup options.

    // 'Random' uses 'std::mt19937'
    // This is automatically seeded by 'std::random_device' when 'Random' gets contructed.

    // Instead of using provided 'Random' class, you can write your own class which implements abstract 'IRandom' class.
    // This allows you to take control of how you handle randomness.
    // This is especially useful if you need randomness anywhere else in your program, and you want to make sure that
    // 	this behaviour stays consistent throught your program. (Same RNG engine used everywhere, etc.)

    // As of writing this, the only method you need to implement from 'IRandom' is:
    // void shuffleVector(std::vector<int>& vec)

    // for example:
    class MyRandomGen : public minesweeper::IRandom {
      public:
        // just an example, shuffle properly in actual use
        void shuffleVector(std::vector<int>& vec) override { std::reverse(vec.begin(), vec.end()); }
    };

    MyRandomGen myRandomGen;
    auto myGame = std::make_unique<minesweeper::Game>(10, 20, &myRandomGen);
}

void usageExamples() {

    // Here we are going to present basic usage of the minesweeper library.
    // For more detailed public interface, check 'game.h' file. (include/minesweeper/game.h as of writing this)

    // Let us start with a basic setup of our Minesweeper game:
    // (check exampleOfSimplestSetup() and examplesOfRandomControllingSetup() methods for more information on this)

    minesweeper::Random myRandom;
    // Minsweeper game with: grid height (10), grid width (12), number of mines(22) and pointer to 'Random'(&myRandom)
    auto myGame = std::make_unique<minesweeper::Game>(10, 12, 22, &myRandom);

    // A minesweeper game is automatically initialised into an empty game of specialised size.
    // However, the game does not generate mines on the grid before the first move has been made.
    // This is to make sure that the player cannot lose straight away.

    // The minesweeper grid uses common programming coordinate system with:
    //			top left corner (0, 0)
    //			top right corner (gridWidth - 1, 0)
    //			bottom left corner (0, gridHeight - 1)
    //			bottom right corner (gridWidth - 1, gridHeight - 1)

    // When coordinates are needed as an argument to methods,
    // 	x-coordinate is always given first, followed by y-coordinate

    // For reference, specific spots on the grid are known as 'Cell's.

    // Creation of mines can be done manually (mostly for legacy reasons) by calling:
    // 		myGame->createMinesAndNums(5, 6);
    // but you can (and should) just ignore this.
    // This method gets automatically called when you check/click coordinates.

    // Player can check/click coordinates:
    myGame->checkInputCoordinates(5, 6);

    // Player can mark coordinates:
    myGame->markInputCoordinates(3, 6);

// We can do a number of checks: (to assist with visualisation of game etc.)
// Small macro to print method names and values:
#define PRINT_FUNC_RESULT(func) std::cout << #func << ": " << (func) << std::endl

    PRINT_FUNC_RESULT(myGame->playerHasWon());
    PRINT_FUNC_RESULT(myGame->playerHasLost());
    PRINT_FUNC_RESULT(myGame->isCellVisible(5, 6));
    PRINT_FUNC_RESULT(myGame->doesCellHaveMine(1, 3));
    PRINT_FUNC_RESULT(myGame->isCellMarked(3, 6));
    PRINT_FUNC_RESULT(myGame->numOfMinesAroundCell(4, 6));

#undef PRINT_FUNC_RESULT

    // All of these are quite self-explanatory.
    // There is quite a few improvements and additions on the way.
    // Again, for more info, check 'game.h' file. (include/minesweeper/game.h as of writing this.
}

// +-----------------------------------+
// |  Testing / Playground Functions:  |
// +-----------------------------------+

void serialisationTest() {

    // testing speed of serialisation

    minesweeper::Random myRandom;
    minesweeper::Game myGame(80, 200, &myRandom);
    myGame.checkInputCoordinates(5, 6);

    using clock = std::chrono::system_clock;
    using sec = std::chrono::duration<double>;
    // for milliseconds, use using ms = std::chrono::duration<double, std::milli>;

    auto before = clock::now();

    std::stringstream strStream;
    myGame.serialise(strStream);
    // myGame.serialise(std::cout);

    sec duration = clock::now() - before;

    std::cout << "It took " << duration.count() << "s to serialise." << std::endl;
    std::cout << "NOTE: std::cout (printing on console) takes a long time!" << std::endl;

    std::cout << "---------------------------------------------------------" << std::endl;

    // testing speed of deserialisation

    before = clock::now();

    myGame.deserialise(strStream);

    duration = clock::now() - before;

    std::cout << "It took " << duration.count() << "s to deserialise." << std::endl;
}

// used to output a game serialisation and print data in files
void playGround1() {

    // implementation of 'IRandom' class which allows determining minespots
    // (until the method of doing this gets changed,
    //   minespots are the first 'numOfMines' elements of the vector)
    class PutIntsToFrontRandom : public minesweeper::IRandom {
      public:
        // predetermined shuffle
        // puts class member ints to front of vector parameter
        // (at this moment first numbers of vector are the mines)
        void shuffleVector(std::vector<int>& vec) override {
            auto toIt = vec.begin();
            for (const auto& num : intsToPutInFront) {

                auto fromIt = std::find(vec.begin(), vec.end(), num);

                std::iter_swap(toIt, fromIt);
                ++toIt;
            }
        }

        explicit PutIntsToFrontRandom(std::vector<int> intsToPutInFront)
            : intsToPutInFront(std::move(intsToPutInFront)) {}

      private:
        const std::vector<int> intsToPutInFront;
    };

    PutIntsToFrontRandom myDeterminedRandom({35, 37, 41, 31, 3, 9, 56, 52, 65, 69, 76, 66, 39, 7, 13, 29});
    // setting up game with predetermined random
    minesweeper::Game myGame(10, 8, 16, &myDeterminedRandom);
    myGame.checkInputCoordinates(4, 7);
    serialiseGameToFile(myGame, "__game_serialisation1.json", "__game_print1.txt");
}

void playGround2() {

    minesweeper::Random myRandom;
    minesweeper::Game myGame(4, 7, 10, &myRandom);
    myGame.checkInputCoordinates(4, 2);

    serialiseGameToFile(myGame, "__game_serialisation2.json", "__game_print2.txt");
}

void playGround3() {
    minesweeper::Random myRandom;
    minesweeper::Game myGame(10, 20, &myRandom);
    myGame.checkInputCoordinates(5, 5);

    std::cout << "1D output:" << std::endl;
    auto myVisualCells = myGame.visualise<std::vector<int>>();
    for (const auto& visualCell : myVisualCells) {
        std::cout << visualCell;
    }
    std::cout << std::endl;

    std::cout << "2D output:" << std::endl;
    auto my2DVisualCell = myGame.visualise<std::vector<std::vector<int>>>();
    for (const auto& visualCellRow : my2DVisualCell) {
        for (const auto& visualCell : visualCellRow) {
            std::cout << std::setw(2) << visualCell;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    serialiseGameToFile(myGame, "__game_serialisation3.json", "__game_print3.json");
}

// simple printout of the game (for debugging/inspection purposes)
// works only for column sizes of up to 26 (number of letters on the english alphabet)
void gamePrinter(std::ostream& outStream, minesweeper::Game const* const gameToPrint) {

    const char MINESYMBOL = '*';
    const char NO_MINES_AROUND_SYMBOL = '.';
    const char NOT_VISIBLE_SYMBOL = ' ';
    const char MARKED_SYMBOL = '#';

    outStream << "Here is the visibility print of the game:\n" << std::endl;

    // to print letters above the grid
    outStream << std::setw(5);
    for (int x = 0; x < gameToPrint->getGridWidth(); ++x) {
        outStream << ' ' << char(65 + x);
    }

    outStream << std::endl;

    for (int y = 0; y < gameToPrint->getGridHeight(); ++y) {
        outStream << std::setw(3) << y + 1 << '|';
        for (int x = 0; x < gameToPrint->getGridWidth(); ++x) {
            outStream << ' ';
            if (gameToPrint->isCellMarked(x, y)) {
                outStream << MARKED_SYMBOL;

            } else if (!gameToPrint->isCellVisible(x, y)) {
                outStream << NOT_VISIBLE_SYMBOL;

            } else if (gameToPrint->doesCellHaveMine(x, y)) {
                outStream << MINESYMBOL;

            } else if (gameToPrint->numOfMinesAroundCell(x, y) == 0) {
                outStream << NO_MINES_AROUND_SYMBOL;

            } else {
                outStream << gameToPrint->numOfMinesAroundCell(x, y);
            }
        }

        outStream << std::endl;
    }

    outStream << "\n\nHere is the solution print of the game:\n" << std::endl;

    // to print letters above solution grid
    outStream << std::setw(5);
    for (int x = 0; x < gameToPrint->getGridWidth(); ++x) {
        outStream << ' ' << char(65 + x);
    }

    outStream << std::endl;

    for (int y = 0; y < gameToPrint->getGridHeight(); ++y) {
        outStream << std::setw(3) << y + 1 << '|';
        for (int x = 0; x < gameToPrint->getGridWidth(); ++x) {
            if (gameToPrint->doesCellHaveMine(x, y)) {
                outStream << ' ' << MINESYMBOL;
            } else if (gameToPrint->numOfMinesAroundCell(x, y) == 0) {
                outStream << ' ' << NO_MINES_AROUND_SYMBOL;
            } else {
                outStream << ' ' << gameToPrint->numOfMinesAroundCell(x, y);
            }
        }

        outStream << std::endl;
    }

    outStream << std::endl;
}

void serialiseGameToFile(const minesweeper::Game& gameToPrint, const std::string& gameSerPath,
                         const std::string& gamePrintPath) {

    // print game serialisation on file
    std::ofstream outFile(gameSerPath);
    gameToPrint.serialise(outFile);

    // print game visualisation on file
    std::ofstream gamePrintFile(gamePrintPath);
    gamePrinter(gamePrintFile, &gameToPrint);
}
