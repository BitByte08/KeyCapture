#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void set_raw_mode(bool enable) {
    static struct termios oldt, newt;
    if(enable){
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

std::vector<std::string> serial_port_reader() {
    std::vector<std::string> ports;
    DIR* dir = opendir("/dev");
    if(!dir) return ports;

    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr){
        std::string name = entry->d_name;
        if(name.find("ttyUSB") != std::string::npos ||
           name.find("ttyACM") != std::string::npos ||
           name.find("cu.usb") != std::string::npos)
            ports.push_back("/dev/" + name);
    }
    closedir(dir);
    return ports;
}

int main() {
    auto ports = serial_port_reader();
    if(ports.empty()){
        std::cout << "사용 가능한 시리얼 포트가 없습니다.\n";
        return 1;
    }

    std::cout << "사용 가능한 포트:\n";
    for(size_t i=0; i<ports.size(); ++i)
        std::cout << i+1 << ") " << ports[i] << "\n";

    int choice;
    std::cout << "선택: ";
    std::cin >> choice;
    if(choice < 1 || choice > (int)ports.size()){
        std::cout << "잘못된 선택\n";
        return 1;
    }

    const char* port = ports[choice-1].c_str();
    int fd = open(port, O_WRONLY | O_NOCTTY);
    if(fd < 0){
        std::cerr << port << " 열기 실패: " << strerror(errno) << "\n";
        return 1;
    }

    set_raw_mode(true);
    std::cout << "키 입력을 Arduino로 전송합니다. (q 입력 시 종료)\n";

    int c;
    while((c=getchar())!='q'){
        std::cout << "\r전송: " << (char)c << " (코드 " << c << ")   " << std::flush;
        write(fd, &c, 1);
    }

    set_raw_mode(false);
    close(fd);
    std::cout << "\n종료\n";
    return 0;
}
