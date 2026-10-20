#ifndef ENGINE_H
#define ENGINE_H

#include "defs.h"
#include "game.h"

void generate_ai_move(int depth, int playing_or_testing);

void calculate_eval_scores();

#endif