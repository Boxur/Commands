#pragma once
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>

namespace cli
{

class Console
{
  private:
    struct Node
    {
        std::unordered_map<std::string, std::shared_ptr<Node>> children;
        std::function<void(const std::vector<std::string> &)> function = nullptr;

        Console::Node &operator[](const std::string &key)
        {
            std::shared_ptr<Node> &ret = children[key];
            if (!ret)
                ret = std::make_shared<Node>();
            return *ret;
        }
    };

    struct Set
    {
        bool newLineAutoComplete = true;
        int histSize = 10;
    };

  public:
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
    bool stop = false;
    Set settings;

  private:
    std::string buf = "";
    std::deque<std::string> hist;
    int histIndex = -1;

  public:
    Node &operator[](const std::string &key);

    void Run();

    void Run(std::string filename);

  private:
    void SetRawMode(bool enable);

    std::optional<char> ReadChar();

    bool Input_available(int fd, int timeout_ms);

    void HandleEscapeChar();

    void AutoComplete();

    void SaveToHist();

    void ExecuteCommand();

    int HandleInput(char ch);

    bool VerifyHelper(const std::shared_ptr<Node> &node);

    bool Verify();
};
} // namespace cli
