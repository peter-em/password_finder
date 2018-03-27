#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

/*
* Linux version of find_password.cpp.
* It uses unistd.h and termios.h libraries,
* instead of conio.h library, found i.e. in windows mingw compiler.
* Also enter and backspace characters, have different values 
* (10 and 127 respectively) used on linux terminal.
* Windows version can be found at github.com/peter-em
*/

// Function used to read text files independently of their encoding.
// Windows (CR LF), Linux (LF) or Mac (CR) encoded files,
// will not affect this program behavior.
std::istream& getline_safe(std::istream& is, std::string& str) {

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    str.clear();
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    while (true) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return is;
            case '\r':
                if (sb->sgetc() == '\n') {
                    sb->sbumpc();
                }
                return is;
            case EOF:
                // Also handle the case when the last line has no line ending
                if (str.empty()) {
                    is.setstate(std::ios::eofbit);
                }
                return is;
            default:
                str.push_back((char)c);
        }
    }
}

// Linux implementation of getch() from conio.h
char getch(){
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    return buf;
}

bool read_answer () {
    std::string answer;
    getline(std::cin, answer);
    // Transform user answer to all lower letters
    transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
    return answer == "y" || answer == "yes";
}

// Check if file provided as program argument exist
inline bool file_exist(const std::string& name) {
    return access(name.c_str(), F_OK) != -1;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "ERROR: no input file provided!" << std::endl;
        return 0;
    }

    std::string filename = argv[1];
    if (!file_exist(filename)) {
        std::cout << "ERROR: file '" << filename << "' not found!" << std::endl;
        return 0;
    }
    std::ifstream in_file;
    in_file.open (filename, std::ios::in);

    std::cout << "Show password characters? (y/n): ";
    // When true, inserted characters will be displayed
    // When false, no characters will be displayed while typing
    bool show_password = read_answer();

    std::cout << "Please type your password: ";
    std::string password = "";
    int c = 0;
    while (c != 10) {   // Loop until 'Enter' is pressed
        c = getch();

        if (c == 127) { // Delete last character when 'Backspace' is pressed
            if (password.size() > 0)
                password.pop_back();

            if (show_password)
                std::cout << "\b \b";
        } else {
            password.push_back((char) c);
            if (show_password)
                std::cout << (char) c;
        }
    }
    password.pop_back();

    std::string str;
    int count_records = 0;
    bool is_found = false;
    while (getline_safe(in_file, str)) {
        count_records++;
        if (str == password) {
            is_found = true;
            break;
        }
    }
    in_file.close();

    std::cout << "Number of records checked: " << count_records << std::endl;
    std::cout << "This file DOES" << (is_found ? " " : " NOT ")
         << "contain such password." << std::endl;

    return 0;
}
