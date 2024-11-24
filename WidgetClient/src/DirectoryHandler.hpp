#include <vector>
#include <string>

class DirectoryHandler
{
public:
    DirectoryHandler();
    bool validateAndLoadDirectory(const std::string& path);
    std::vector<std::tuple<std::string, std::string>> loadDirectory(const std::string& path);
private:
    std::string curPath_;
};
