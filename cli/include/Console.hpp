#pragma once
#include <termios.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>
#include <deque>

namespace cli
{
  struct Node
  {
    std::unordered_map<std::string,std::shared_ptr<Node>> children;
    std::function<void(const std::vector<std::string>&)> function = nullptr;

    Node& operator[](const std::string& key)
    {
      std::shared_ptr<Node>& ret = children[key];

      if(!ret)
        ret = std::make_shared<Node>();

      return *ret;
    }
  };

  class Console
  {
    public:
    std::unordered_map<std::string,std::shared_ptr<Node>> nodes;
    bool stop = false;
    std::string buf= "";
    std::deque<std::string> hist;
    int histIndex=-1;
    struct Set
    {
      bool newLineAutoComplete = true;
      int histSize = 10;
    };
    Set settings;

    Node& operator[](const std::string& key);
  
    void SetRawMode(bool enable);

    std::optional<char> ReadChar();

    bool Input_available(int fd, int timeout_ms);

    void HandleEscapeChar();

    void AutoComplete();

    void SaveToHist();

    void ExecuteCommand();

    int HandleInput(char ch);

    bool VerifyHelper(const std::shared_ptr<Node>& node);

    bool Verify();

    void Run();

    void Run(std::string filename);
  };
}
