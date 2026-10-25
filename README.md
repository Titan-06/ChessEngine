# Menso — C Chess Engine

Menso is a chess engine written in C, built to explore chess search algorithms and measure their impact on performance and playing strength.

## Features

- **Board representation:** 0x88
- **Search:** Negamax with Alpha-Beta Pruning
- **Move ordering:** MVV-LVA
- **Tactical search:** Quiescence Search

## To use it:

The repository contains two folders: `Engine_and_Game` for playing against Menso directly in the terminal, and `uci_engine` containing the UCI-compliant version.

**Branches:** Older engine versions are available through the repository's branches for `uci_engine`. The `Engine_and_Game` folder remains at the original v1 version **in the branches**. The main branch is in v4 version for both the folders.

## Playing Strength

Menso played 400 games against Stockfish at Elo limits of 1400, 1600, 1800, and 2000, using a fixed depth(piles) of 5. It scored **50% against the Stockfish 1800 configuration**.

The matches used `openings.pgn` to provide opening positions.

## Limitations

- No iterative deepening, time management, or built-in threefold-repetition detection.
- No internal handling of the 50-move rule or insufficient-material draws.
- Simple material and piece-square-table evaluation, with limited move ordering.
- Quiescence search limited to two plies.

## Analysis
- [Long-form analysis article](https://titan-06.github.io/blog/analysis-chess-engine/) — A detailed account of the development process, design decisions, experiments, and lessons learned.
- [`analysis.md`](analysis.md) — A condensed formal version of the long form article focusing only on analysis.
