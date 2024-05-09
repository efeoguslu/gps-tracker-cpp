#include <iostream>
#include <fstream>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>
#include <ctime> 
#include <iomanip> 
#include <sys/time.h>
#include <sys/select.h>
#include <algorithm>
#include <wiringPi.h>

const int gpsLedPin{ 18 };

// Trim leading whitespace
std::string lstrip(const std::string& s) {
    auto it = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    return std::string(it, s.end());
}

// Trim trailing whitespace
std::string rstrip(const std::string& s) {
    auto it = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return std::string(s.begin(), it);
}

// Trim leading and trailing whitespace
std::string strip(const std::string& s) {
    return rstrip(lstrip(s));
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(s.substr(start));


    return tokens;
}

// Function to parse NMEA sentences
std::vector<std::string> parseNMEASentence(const std::string& sentence) {

    char delimiter = ',';
    std::vector<std::string> fields = split(strip(sentence), delimiter);

    return fields;
}

// Function to get the log filename
std::string getLogFilename() {
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    std::string path;
    std::ifstream file("/home/efeoguslu/Desktop/directory_path.txt");
    if (file.is_open()) {
        std::getline(file, path); // Read the path from the file
        file.close();
    } else {
        std::cerr << "Failed to open directory path file." << std::endl;
        return "";
    }

    std::ostringstream oss;
    oss << path << "/CPP_NMEA_" << std::put_time(local_time, "%Y-%m-%d_%H-%M-%S") << ".nmea";
    return oss.str();
}

// Function to blink the LED a specific number of times
void blinkLED(int ledPin, int blinkCount) {
    const int blinkDurationMs = 200; // Each blink lasts 100 ms
    for (int i = 0; i < blinkCount; ++i) {
        digitalWrite(ledPin, HIGH); // Turn the LED on
        std::this_thread::sleep_for(std::chrono::milliseconds(blinkDurationMs)); // Wait for the blink duration
        digitalWrite(ledPin, LOW); // Turn the LED off
        std::this_thread::sleep_for(std::chrono::milliseconds(blinkDurationMs)); // Wait for the blink duration
    }
}

static int count{ 0 };

int main(){

    // Initialize wiringPi and allow the use of BCM pin numbering
    wiringPiSetupGpio();
    pinMode(gpsLedPin, OUTPUT);

    blinkLED(gpsLedPin, 3);

    std::string port = "/dev/ttyUSB0";
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (fd == -1) {
        std::cerr << "Error opening serial port: " << port << std::endl;
        return 1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &options);

    char buf[256];

    std::ofstream logFile; // Declare the log file stream
    std::string logFilename = getLogFilename(); // Get the log filename

    // Open the log file in append mode
    logFile.open(logFilename, std::ios::app);

    // Warm-up loop to discard initial garbage values
    for (int i = 0; i < 10; ++i) {
        read(fd, buf, sizeof(buf) - 1);
    }
    
    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        
        if (n > 0) {

            buf[n] = '\0';

            std::vector<std::string> parsedSentence = parseNMEASentence(buf);

            // Check if the sentence is valid and log it
            if (parsedSentence[0] == "$GPRMC") {
                std::string validity = parsedSentence[2];
                std::string speed = parsedSentence[7];
                
                struct timeval tv;
                gettimeofday(&tv, nullptr);
                tm* ltm = localtime(&tv.tv_sec);
                char timeBuf[256];
                strftime(timeBuf, sizeof(timeBuf), "[%Y-%m-%d %H:%M:%S.", ltm);
                std::string header(timeBuf);
                header += std::to_string(tv.tv_usec) + "] ";

                std::ostringstream timestamp;
            
                timestamp << header;

                logFile << timestamp.str() << buf << std::endl;

                std::cout << count++ << " ";

                // Control the LED based on validity
                if (validity == "A") {
                    // Turn on the LED
                    // Note: GPIO control requires additional setup and libraries
                    digitalWrite(gpsLedPin, HIGH);
                    std::cout << "LED ON. Validity: " << validity << " Speed:" << speed << std::endl;
                } 
                
                else if (validity == "V") {
                    // Turn off the LED
                    // Note: GPIO control requires additional setup and libraries
                    digitalWrite(gpsLedPin, LOW);
                    std::cout << "LED OFF. Validity: " << validity << " Speed:" << speed << std::endl;
                }
            }
        }
    }

    close(fd);
    logFile.close();
    return 0;
}
