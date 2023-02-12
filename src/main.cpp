#include "control/simstate.hpp"
#include <engine/game/core/game.hpp>
#include <iostream>

int main()
{
	game::Game game;
	game.run(std::make_unique<control::SimState>());
}