#pragma once
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>

namespace cli {

class Console {
private:
  struct Node {
    std::unordered_map<std::string, std::shared_ptr<Node>> children;
    std::function<void(const std::vector<std::string> &)> function = nullptr;

    Console::Node &operator[](const std::string &key) {
      std::shared_ptr<Node> &ret = children[key];
      if (!ret)
        ret = std::make_shared<Node>();
      return *ret;
    }
  };

  struct Set {
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
  void SetRawMode_(bool enable);

  std::optional<char> ReadChar_();

  bool Input_available_(int fd, int timeout_ms);

  void HandleEscapeChar_();

  void AutoComplete_();

  void SaveToHist_();

  void ExecuteCommand_();

  int HandleInput_(char ch);

  bool VerifyHelper_(const std::shared_ptr<Node> &node);

  bool Verify_();

  void SetupDefaultCommands_();
};
} // namespace cli
