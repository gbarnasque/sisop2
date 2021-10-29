#include "Terminal.hpp"

Terminal::Terminal() {
    struct winsize ws;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws);
    this->_cols = ws.ws_col;
    this->_rows = ws.ws_row;
    this->_lastRow = ws.ws_row;
    this->_maxRow = ws.ws_row-2;
    this->clearScreen();
    this->printSeparator();

    sem_init(&_terminalSempahore, 1, 1);
}

void Terminal::setMaxRow(int maxRow) {
    this->_maxRow = maxRow;
}

void Terminal::clearScreen() {
    system("clear");
    //printf("\033[H\033[J");      
}

void Terminal::gotoRow(int row) {
    //std::cout << "Linha1: " << row << endl;
    gotoXY(row, 1);
}

void Terminal::gotoRowAndUpdate(int* line) {
    //std::cout << "Linha2: " << *line << endl;
    gotoXY(*line, 1);
    if(*line != _maxRow) {
        *line += 1;
    }
}

void Terminal::gotoXY(int row, int col) {
    printf("\033[%d;%dH", (row), (col));
}

void Terminal::gotoEnd() {
    gotoXY(_lastRow, 1);
}

void Terminal::clearCommandFromTerminal() {
    sem_wait(&_terminalSempahore);
    std::string clearStr(_cols, ' ');
    gotoXY(_rows-2, 1);
    std::cout << clearStr << std::endl;
    printSeparator();
    sem_post(&_terminalSempahore);
}

void Terminal::printSeparator() {
    gotoXY(_rows-1,1);
    std::string separator(_cols, '=');
    std::cout << separator << std::endl;
    gotoEnd();
}

void Terminal::print(string s, PrintType printType, int* line, string prefix, Color colorCode, bool bold) {

    switch (printType)
    {
        case PrintType::SUCCESS:
            StringUtils::printSuccess(s);
            break;
        case PrintType::INFO:
            StringUtils::printInfo(s);
            break;
        case PrintType::DANGER:
            StringUtils::printDanger(s);
            break;
        case PrintType::WARNING:
            StringUtils::printWarning(s);
            break;
        case PrintType::LINE:
            StringUtils::print(s, colorCode, bold);
            break;
        case PrintType::BOLD:
            break;
        case PrintType::PRINT_WITH_COLOR:
            break;
        case PrintType::PRINT_WITH_PREFIX:
            break;
        case PrintType::PRINT_WITH_PREFIX_COLOR:
            break;
        default:
            break;
    }
}


void Terminal::printInfo(string s, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printInfo(s);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printWarning(string s, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printWarning(s);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printDanger(string s, int* line) {
    //std:: cout << "Danger: " << *line << std::endl;
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printDanger(s);
    gotoEnd();
    //*line += 1;
    //std:: cout << "Danger Final: " << *line << std::endl;
    sem_post(&_terminalSempahore);
}
void Terminal::printSuccess(string s, int* line) {
    //std:: cout << "Success: " << *line << std::endl;
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printSuccess(s);
    gotoEnd();
    //*line++;
    //std:: cout << "Success Final: " << *line << std::endl;
    sem_post(&_terminalSempahore);
}
void Terminal::printLine(string s, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printLine(s);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printLine(const char* s, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printLine(s);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::print(string s, Color colorCode, bool bold, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::print(s, colorCode, bold);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printBold(string s, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printBold(s);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printWithPrefix(string s, string prefix, Color colorCode, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printWithPrefix(s, prefix, colorCode);
    gotoEnd();
    sem_post(&_terminalSempahore);
}
void Terminal::printWithRandomPrefixColor(string s, string prefix, int* line) {
    sem_wait(&_terminalSempahore);
    gotoRowAndUpdate(line);
    StringUtils::printWithRandomPrefixColor(s, prefix);
    gotoEnd();
    sem_post(&_terminalSempahore);
}