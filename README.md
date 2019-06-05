# FERI-SudokuDLX

[Dancing Links (DLX)](https://en.wikipedia.org/wiki/Dancing_Links) Sudoku solver at Artificial Intelligence (slo. Umetna Inteligenca).

Solves Sudoku using dancing links technique, an efficient implementation of [Algorithm X](https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X), as suggested by Donald Knuth. Supports all NxN sizes where N is a perfect square (4x4, 9x9, 16x16 ...).

Thanks to [KarlHajal/DLX-Sudoku-Solver](https://github.com/KarlHajal/DLX-Sudoku-Solver) for a high-performance C-like implementation that served as the most important reference for this implementation. Additional thanks to [Jonathan Chu's Java Sudoku Solver and paper](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/sudoku.paper.html) for another important reference.

### Features

- Sudoku Solver using Dancing Links Algorithm
- Sudoku Grids NxN _(N is perfect square)_
  - Manual Input _(non-validated - by design for DLX error testing)_
  - Import Dotted String Format _(size-validated only)_
    - `53.2..4...` _(length: N*N)_
  - Test Cases (9x9 and 16x16) _(in-code on start)_
  - Benchmark _(build & search)_

### Setup

**Dependencies:**
- [Qt](https://www.qt.io/)

### Resources

- [KarlHajal/DLX-Sudoku-Solver (reference C++ implementation)](https://github.com/KarlHajal/DLX-Sudoku-Solver)
- [Dancing Links, _Donald E. Knuth_, Stanford University paper](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/0011047.pdf)
- [A Sudoku Solver in Java implementing Knuth’s Dancing Links Algorithm](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/sudoku.paper.html)
- [Zendoku puzzle generation](http://garethrees.org/2007/06/10/zendoku-generation/)

**Test Cases:**
- [Sudopedia: Test Cases](http://sudopedia.enjoysudoku.com/Test_Cases.html)
- [Wikipedia: Sudoku solving algorithms](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms)
- [Computing Magic Knight Tours: Sudoku](http://magictour.free.fr/sudoku.htm)
