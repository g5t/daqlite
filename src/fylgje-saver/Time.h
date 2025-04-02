#include <string>
#include <ctime>
#include <chrono>

std::time_t now_to_time_t();
std::time_t string_to_time_t(const std::string & time_str);

std::string time_t_to_string(std::time_t time);

std::chrono::duration<long> duration_string_to_seconds(const std::string & duration_str);