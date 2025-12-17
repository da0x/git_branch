#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

// ------------------- Raw mode -------------------
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

// ------------------- Terminal width -------------------
int get_terminal_width() {
    struct winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

// ------------------- Git helpers -------------------
std::vector<std::string> get_branch_names(int &current_index) {
    std::vector<std::string> names;
    FILE* pipe = popen("git branch --no-color", "r");
    if (!pipe) {
        std::cerr << "Not a git repository.\n";
        exit(1);
    }
    char buffer[512];
    int index = 0;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\n\r")+1);
        if (line.empty()) continue;

        bool active = line[0] == '*';
        std::string name = line.substr(2);
        if (active) current_index = index;
        names.push_back(name);
        index++;
    }
    pclose(pipe);
    return names;
}

std::vector<std::string> get_table_lines() {
    std::vector<std::string> table;
    FILE* pipe = popen("git branch -v --no-color", "r");
    if (!pipe) return table;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\n\r")+1);
        if (!line.empty()) table.push_back(line);
    }
    pclose(pipe);
    return table;
}

void checkout_branch(const std::string &branch_name) {
    std::string cmd = "git checkout " + branch_name;
    system(cmd.c_str());
}

// ------------------- Render table -------------------
int prev_lines_drawn = 0;

void render_table(const std::vector<std::string> &branch_names,
                  const std::vector<std::string> &table_lines,
                  int selected_index) {
    int term_width = get_terminal_width();

    for (int i = 0; i < prev_lines_drawn; ++i) std::cout << "\033[F";
    prev_lines_drawn = branch_names.size();

    for (size_t i = 0; i < branch_names.size(); ++i) {
        std::string branch = branch_names[i];
        std::string line;

        for (const auto &tbl : table_lines) {
            if (tbl.find(branch) != std::string::npos) {
                line = tbl;
                break;
            }
        }

        if ((int)line.size() > term_width) line = line.substr(0, term_width - 1);

        bool is_active = !line.empty() && line[0] == '*';
        if (i == selected_index)
            std::cout << "\033[7m"; // invert
        else if (is_active)
            std::cout << "\033[32m"; // green

        std::cout << line << "\033[0m\033[K" << std::endl;
    }
    std::cout << std::flush;
}

// ------------------- Main -------------------
int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--version") {
        std::cout << "git-select v1.0.0\n";
        std::cout << "An interactive git branch selector.\n";
        std::cout << "https://github.com/da0x/git-select\n";
        return 0;
    }

    int selected_index = 0;
    auto branch_names = get_branch_names(selected_index);
    auto table_lines = get_table_lines();
    if (branch_names.empty()) {
        std::cerr << "No branches found.\n";
        return 1;
    }

    raw_mode_t raw;
    std::cout << "\033[?25l"; // hide cursor
    auto restore_cursor = [](){ std::cout << "\033[?25h"; };

    std::cout << "Select git branch (↑/↓ j/k, Enter to checkout, q to quit)\n";
    render_table(branch_names, table_lines, selected_index);

    while (true) {
        char c = getchar();
        if (c == 27) { // arrow keys
            char c2 = getchar();
            char c3 = getchar();
            if (c2=='[') {
                if (c3=='A' && selected_index>0) selected_index--;
                if (c3=='B' && selected_index<(int)branch_names.size()-1) selected_index++;
            }
        } else if (c=='j' && selected_index<(int)branch_names.size()-1) selected_index++;
        else if (c=='k' && selected_index>0) selected_index--;
        else if (c=='\n') { restore_cursor(); checkout_branch(branch_names[selected_index]); break; }
        else if (c=='q') { restore_cursor(); break; }

        render_table(branch_names, table_lines, selected_index);
    }

    restore_cursor();
    std::cout << std::endl;
    return 0;
}

