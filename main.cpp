//
// git-select
//
// Copyright (C) 2025 Daher Alfawares
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//


#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct raw_mode_t {
    termios orig;
    raw_mode_t() {
        tcgetattr(STDIN_FILENO, &orig);
        termios raw = orig;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    ~raw_mode_t() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    }
};

int get_terminal_height() {
    winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

int get_terminal_width() {
    winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

struct Branch {
    std::string name;
    std::string line;
    bool active = false;
};

std::vector<Branch> get_branches(int &current_index) {
    std::vector<Branch> branches;
    FILE* pipe = popen("git branch -v --no-color", "r");
    if (!pipe) {
        std::cerr << "Not a git repository.\n";
        exit(1);
    }
    char buffer[1024];
    int index = 0;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\n\r") + 1);
        if (line.size() < 3) continue;
        Branch b;
        b.active = (line[0] == '*');
        size_t name_start = 2;
        size_t name_end = line.find(' ', name_start);
        b.name = line.substr(name_start, name_end - name_start);
        b.line = line;
        if (b.active) current_index = index;
        branches.push_back(b);
        index++;
    }
    pclose(pipe);
    return branches;
}

void switch_branch(const std::string &branch_name) {
    std::string cmd = "git switch " + branch_name;
    system(cmd.c_str());
}

void render_table(const std::vector<Branch> &branches, int selected_index, int scroll_offset, int &lines_rendered) {
    int width = get_terminal_width();
    int height = get_terminal_height() - 2;
    int count = std::min(height, (int)branches.size() - scroll_offset);

    for (int i = 0; i < lines_rendered; ++i)
        std::cout << "\033[F";

    std::cout << "Select git branch (↑/↓ j/k, Enter to switch, q to quit)\n";

    for (int i = 0; i < count; ++i) {
        int idx = i + scroll_offset;
        std::string line = branches[idx].line;
        if ((int)line.size() > width) line.resize(width - 1);

        if (idx == selected_index && branches[idx].active)
            std::cout << "\033[7;32m"; // invert + green
        else if (idx == selected_index)
            std::cout << "\033[7m";    // invert
        else if (branches[idx].active)
            std::cout << "\033[32m";   // green

        std::cout << line << "\033[0m\033[K\n";
    }

    lines_rendered = count + 1;
    std::cout << std::flush;
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--version") {
        std::cout << "git-select v1.0.0\n";
        std::cout << "An interactive git branch selector.\n";
        std::cout << "https://github.com/da0x/git-select\n";
        return 0;
    }

    int selected_index = 0;
    auto branches = get_branches(selected_index);
    if (branches.empty()) {
        std::cerr << "No branches found.\n";
        return 1;
    }

    raw_mode_t raw;
    std::cout << "\033[?25l";

    int scroll_offset = 0;
    int lines_rendered = 0;
    render_table(branches, selected_index, scroll_offset, lines_rendered);

    int term_height = get_terminal_height() - 2;

    while (true) {
        char c = getchar();
        if (c == 27) {
            getchar();
            char dir = getchar();
            if (dir == 'A' && selected_index > 0) selected_index--;
            if (dir == 'B' && selected_index < (int)branches.size() - 1) selected_index++;
        }
        else if (c == 'j' && selected_index < (int)branches.size() - 1) selected_index++;
        else if (c == 'k' && selected_index > 0) selected_index--;
        else if (c == '\n') {
            std::cout << "\033[?25h";
            switch_branch(branches[selected_index].name);
            break;
        }
        else if (c == 'q') {
            std::cout << "\033[?25h";
            break;
        }

        if (selected_index < scroll_offset)
            scroll_offset = selected_index;
        else if (selected_index >= scroll_offset + term_height)
            scroll_offset = selected_index - term_height + 1;

        render_table(branches, selected_index, scroll_offset, lines_rendered);
    }

    std::cout << "\033[?25h\n";
    return 0;
}

