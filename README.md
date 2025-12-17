# git select

Interactive git branch selector in the terminal.

- Highlight the currently active branch in green.
- Navigate with **arrows** or **j/k** keys.
- Press **Enter** to checkout the selected branch.
- Press **q** to quit.

---

## Installation

### Build

```
make
```

### Install

```
sudo make install
```

This installs the `git-select` binary to `/usr/local/bin/`.

---

## Usage

Run in any git repository:

```
git select
```

The program will:

1. Display a prompt and the branch table (`git branch -v` output).
2. Highlight the currently active branch.
3. Allow navigation with arrows or `j/k`.
4. Checkout the selected branch with Enter.
5. Quit with `q`.

---

## Example Output

Here is an example of how `git select` looks in the terminal:

```
$ git select
Select git branch (↑/↓ j/k, Enter to checkout, q to quit)
  1-initial-setup                         a1b2c3d   initialize project structure
  2-api-refactor                          b2c3d4e   refactor request handling
  3-ui-polish                             c3d4e5f   adjust spacing and colors
  experiment-fast-build                   d4e5f6a   test alternative build flags
➜ main                                   e5f6a7b   merge feature branches
  release-2025-01                         f6a7b8c   prepare release artifacts
  spike-performance                       a7b8c9d   measure cold start latency
```

- `➜` marks the **currently selected branch** (highlighted).  
- `main` is the **active branch** (green).  
- Navigate with **arrows** or **j/k**, press **Enter** to checkout, **q** to quit.  
- Lines are truncated to terminal width to prevent wrapping issues.

---

## Optional: Create a shorter alias

If you want even faster access, you can add an alias in your shell configuration (`~/.bashrc`, `~/.zshrc`, etc.):

```
# shorter alias for git select
alias br='git select'
```

Reload your shell:

```
source ~/.bashrc   # or ~/.zshrc
```

Then you can run:

```
br
```

---

## Dependencies

- Standard C++ compiler (`g++`)
- POSIX-compatible terminal (Linux, macOS)

No external libraries required.

