#include <string>
#include <vector>
std::string NFD_OpenFile_Cpp(const std::string& fileFilters,const std::string& path); //Returns the file name with path or empty string on cancel/error
std::vector<std::string> NFD_OpenMultiple_Cpp(const std::string& fileFilters,const std::string& path); //Returns a vector of file names with paths or empty vector on error/cancel/error
std::string NFD_SaveFile_Cpp(const std::string& fileFilters,const std::string& path); //Returns the file name with path or empty string on cancel/error