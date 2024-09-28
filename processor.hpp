#include <fstream>
#include <map>
#include <iostream>
#include "terminal.hpp"
#include "bricks.hpp"
#include "lexer.hpp"

class Processor {
    std::shared_ptr<C64Terminal> terminal;
    Bricks vars;
    int pointer = -1;
    std::map<int, std::u32string> commands;
    bool is_running = false;
public:
    Processor(std::shared_ptr<C64Terminal> terminal) {
        this->terminal = terminal;
    }

    void clear_memory() {
        commands.clear();
    }

    void add_command(std::u32string command) {
        int index;
        size_t space_pos = command.find_first_of(' ');

        std::u32string indexStr = command.substr(0, space_pos);
        if (!isDigit(indexStr)) {
            *terminal << "INVALID INDEX: " << indexStr << std::endl;
            return;
        }
        index = u32stoi(indexStr);
        if (indexStr != command) {
            while (command[space_pos++] == ' ') continue;
            commands[index] = command.substr(space_pos - 1);
        }
        else commands.erase(index);
    }

    void print_error(std::u32string text) {
        terminal->setTextColor(16);
        *terminal << "ERROR ON LINE " << pointer << std::endl;
        *terminal << text << std::endl;
        is_running = false;
    }

    std::u32string format_string(std::u32string str) {
        auto start_it = str.begin();
        auto end_it = str.rbegin();
        if (str.size() == 1) {
            this->print_error(U"BAD STRING: " + str);
            return U"";
        }
        if (*start_it != L'"' || *end_it != L'"') {
            start_it++; end_it++;
            if (contains(L'"', std::u32string(start_it, end_it.base()))) {
                this->print_error(U"BAD STRING: " + str);
                return U"";
            }
        }
        start_it++;
        end_it++;
        return std::u32string(start_it, end_it.base());
    }

    void print_commands() {
        for (auto& kvp : commands) {
            *terminal << kvp.first << ' ' << kvp.second << std::endl;
            terminal->render();
        }
        if (!commands.empty())
            *terminal << "READY" << std::endl;
        else
            *terminal << "<MEMORY EMPTY>" << std::endl;
    }

    void run_command(std::u32string command) {
        size_t space_pos = command.find_first_of(' ');

        std::u32string command_name = command.substr(0, space_pos);
        std::u32string command_params = U"";

        if (contains(U' ', command)) {
            while (command[space_pos++] == ' ') continue;
            command_params = command.substr(space_pos - 1);
        }
        if (command_name == U"CLS") {
            terminal->clearScreen();
        }
        else if (command_name == U"PRINT") {
            if (command_params != U"")
                command_params = this->format_string(command_params);
            if (is_running) {
                *terminal << command_params << std::endl;
                terminal->render();
            }
        }
        else if (command_name == U"GOTO") {
            if (command_params == U"") {
                this->print_error(U"NO INDEX GIVEN IN GOTO");
            }
            else if (isDigit(command_params)) {
                this->print_error(U"NOT A NUMBER IN GOTO");
            }
            else pointer = u32stoi(command_params);
        }
        else if (command_name == U"COLOR") {
            int arg = 16;
            if (isDigit(command_params)) arg = u32stoi(command_params);
            terminal->setTextColor(arg);
        }
        else if (command_name == U"END") {
            is_running = false;
        }
        else if (command_name == U"POINT") {}
        else {
            this->print_error(U"UNKNOWN COMMAND: " + command_name);
            is_running = false;
        }
    }

    void run_commands() {
        is_running = true;
        pointer = -1;
        while (is_running) {
            if (terminal->escPressed()) {
                terminal->setTextColor(16);
                *terminal << "== INTERRUPTED BY USER ==" << std::endl;
                goto stop;
            }
            auto it = commands.lower_bound(pointer);
            if (it != commands.end()) {
                pointer = it->first;
                this->run_command(commands[pointer++]);
            }
            else is_running = false;
        }
        terminal->setTextColor(16);
        *terminal << "== EXECUTED SUCCESSFULLY ==" << std::endl;
        stop:
        is_running = false;
    }

    void run_single_command(std::u32string command) {
        pointer = 10;
        is_running = true;
        this->run_command(command);
        terminal->setTextColor(16);
        is_running = false;
    }

    void save_program(std::u32string program) {
        std::string fp = u32string_to_string(u32strip(program)) + ".ANTIGRAVIC";
        std::ofstream Fout(fp);
        if (Fout) {
            for (int i = 0; i < 9999; i++) {
                if (!commands[i].empty()) {
                    Fout << i+1 << ' ' << u32string_to_string(commands[i]) << "\n";
                }
            }
            *terminal << "SAVED!" << std::endl;
        }
        else *terminal << "CANT OPEN FILE" << std::endl;
        Fout.close();
    }

    void load_program(std::u32string program) {
        std::string fp = u32string_to_string(u32strip(program)) + ".ANTIGRAVIC";
        std::ifstream Fin(fp);
        int lines = 0;
        if (Fin) {
            this->clear_memory();
            std::string s;
            while (!Fin.eof()) {
                getline(Fin, s);
                if (s != "") {
                    this->add_command(string_to_u32string(s));
                    lines++;
                }
            }
            Fin.close();
            terminal->loadingScreen(lines);
        }
        else {
            Fin.close();
            *terminal << "NOT FOUND" << std::endl;
        }
    }

    void process_input(std::u32string input) {
        terminal->showCursor = false;
        size_t space_pos = input.find_first_of(' ');

        std::u32string command_name = input.substr(0, space_pos);
        std::u32string command_params = U"";

        if (contains(U' ', input)) {
            while (input[space_pos++] == U' ') continue;
            command_params = input.substr(space_pos - 1);
        }

        if (u32strip(input) == U"") {}
        else if (input == U"LIST")
            this->print_commands();
        else if (command_name == U"SAVE") {
            if (command_params == U"")
                *terminal << U"NO FILENAME PROVIDED." << std::endl;
            else {
                this->save_program(command_params);
            }
        }
        else if (command_name == U"LOAD") {
            if (command_params == U"")
                *terminal << U"NO FILENAME PROVIDED." << std::endl;
            else {
                this->load_program(command_params);
            }
        }
        else if (command_name == U"CLEAR") {
            this->clear_memory();
            *terminal << U"MEMORY CLEARED" << std::endl;
        }
        else if (command_name == U"RUN") {
            if (!commands.empty())
                this->run_commands();
            else
                *terminal << U"<MEMORY EMPTY>" << std::endl;
        }
        else if (starts_with_number(input))
            this->add_command(input);
        else
            this->run_single_command(input);
        terminal->showCursor = true;
    }

    void mainloop() {
        while (true) {
            this->process_input(terminal->input());
        }
    }
};
