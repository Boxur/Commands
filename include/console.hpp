#pragma once
#include <cmath>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>

namespace cli {

/**
 *  @brief Console represents the customisable console.
 *
 *  Console sets up raw mode for the console,
 *  manages settings, autocomplete when typing commands,
 *  and executing stored commands.
 */
class Console {
public:
  class Node {
    friend class Console;

  private:
    std::unordered_map<std::string, std::shared_ptr<Node>> children;

  public:
    /**
     * @brief A function to call when executing a command
     */
    std::function<void(const std::vector<std::string> &)> function = nullptr;

  public:
    /**
     *  @brief Used to add a new command.
     *
     *  Can be chained togehter to add more complex commands.
     *
     *  ```cpp
     *  Console console;
     *
     *  //create a command "command set histSize";
     *  console["console"]["set"]["histSize"].function = [](){};
     *  ```
     *
     *  @param key Command name
     */
    Console::Node &operator[](const std::string &key) {
      std::shared_ptr<Node> &ret = children[key];
      if (!ret)
        ret = std::make_shared<Node>();
      return *ret;
    }
  };

  /**
   * @brief Manages if the main loop should stop.
   *
   * Set to true if you want to stop the console from running.
   */
  bool stop = false;
  /**
   * @brief Console settings
   */
  struct {
    /**
     * @brief Sets the style of autocomplete.
     *
     * When true creates a new line when autocompleating Cisco-style.
     * When false keeps the autocomplete in the same line like in a terminal.
     */
    bool newLineAutoComplete = true;
    /**
     * @brief Sets the size of history.
     *
     * History can be accessed with Up/Down arrow keys.
     */
    int histSize = 10;
  } settings;

private:
  std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
  std::string buf = "";
  std::deque<std::string> hist;
  int histIndex = -1;

public:
  /**
   *  @brief Used to add a new command.
   *
   *  Can be chained togehter with Console::Node::operator[] to
   *  add more complex commands.
   *
   *  ```cpp
   *  Console console;
   *
   *  //create a command "command set histSize";
   *  console["console"]["set"]["histSize"].function = [](){};
   *  ```
   *
   *  @param key Command name
   */
  Node &operator[](const std::string &key);

  /**
   * @brief Starts the custom console.
   *
   * Sets up raw mode for custom key functionality and begins the main loop.\
   */
  void Run();

  /**
   * @brief Starts the custom console and executes commands from a file.
   *
   * Like Run() but also executes commands from a file.
   * Useful when a script file is wanted.
   *
   * ```cpp
   * if (argc == 1)
   *   console.Run();
   * else
   *   console.Run(argv[1]);
   *
   * ```
   */
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
