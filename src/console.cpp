#include "console.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace cli {
Console::Node &Console::operator[](const std::string &key) {
  std::shared_ptr<Node> &ret = nodes[key];

  if (!ret)
    ret = std::make_shared<Node>();

  return *ret;
}

void Console::SetRawMode_(bool enable) {
  static struct termios oldt, newt;
  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
}

std::optional<char> Console::ReadChar_() {
  char c;
  if (read(STDIN_FILENO, &c, 1))
    return c;
  return {};
}

bool Console::Input_available_(int fd, int timeout_ms) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv{0, timeout_ms * 1000};
  return select(fd + 1, &fds, NULL, NULL, &tv) > 0;
}

void Console::HandleEscapeChar_() {
  if (!Input_available_(STDIN_FILENO, 10)) {
    write(STDIN_FILENO, "\r\n", 2);
    stop = true;
    return;
  }
  char seq[2];
  read(STDIN_FILENO, seq, 2);
  if (seq[0] != '[')
    return;
  bool arrows = false;
  if (seq[1] == 'A' || seq[1] == 'B') {
    if (hist.size() == 0)
      return;
    arrows = true;
    write(STDIN_FILENO, "\r\033[K>", 5);
  }
  if (seq[1] == 'A') {
    if (histIndex < (settings.histSize - 1) &&
        histIndex < ((int)hist.size() - 1))
      histIndex++;
  } else if (seq[1] == 'B') {
    if (histIndex >= 0)
      histIndex--;
  }
  buf = "";
  if (arrows && histIndex != -1) {
    write(STDIN_FILENO, hist[histIndex].c_str(), hist[histIndex].size());
    buf = hist[histIndex];
  }
}

void Console::AutoComplete_() {
  std::istringstream ss(buf);
  std::vector<std::string> values;
  std::string value;
  auto *node = &nodes;
  while (getline(ss, value, ' ')) {
    if (value.size() > 0)
      values.push_back(value);
  }
  if (values.size() == 0)
    return;
  std::string result = "";
  std::string best = "";
  bool sure;
  for (int i = 0; i < values.size() && node->size() != 0; i++) {
    sure = true;
    for (auto &it : *node) {
      if (values[i] != it.first.substr(0, values[i].size()))
        continue;
      if (best == "")
        best = it.first;
      for (int j = 0; j < it.first.size() && j < best.size(); j++) {
        if (best[j] != it.first[j]) {
          best = best.substr(0, j);
          sure = false;
        } else if (j == it.first.size() - 1) {
          auto s = best.substr(0, j + 1);
          if (s != best)
            sure = false;
          best = s;
        }
      }
    }
    result += best;
    if (sure) {
      result += " ";
      node = &((*node)[best]->children);
    }
    best = "";
  }
  if (result.size() <= buf.size())
    return;

  buf = result;
  if (!settings.newLineAutoComplete) {
    write(STDIN_FILENO, "\r>", 2);
    write(STDIN_FILENO, result.c_str(), result.size());
  } else {
    write(STDIN_FILENO, "\r\n>", 3);
    write(STDIN_FILENO, result.c_str(), result.size());
  }
}

void Console::SaveToHist_() {
  histIndex = -1;
  if (buf == "" || (hist.size() != 0 && buf == hist[0]))
    return;

  if (hist.size() < settings.histSize)
    hist.push_front(buf);
  else {
    while (hist.size() >= settings.histSize && hist.size() > 0)
      hist.pop_back();
    hist.push_front(buf);
  }
}

void Console::ExecuteCommand_() {
  SaveToHist_();
  std::istringstream ss(buf);
  std::vector<std::string> values;
  std::string value;
  while (getline(ss, value, ' ')) {
    if (value.size() > 0)
      values.push_back(value);
  }
  buf = "";
  write(STDIN_FILENO, "\r\n", 2);
  if (values.size() < 1)
    return;
  if (nodes.find(values[0]) == nodes.end()) {
    write(STDIN_FILENO, "Unknown command: \r\n", 19);
    write(STDIN_FILENO, values[0].c_str(), values[0].size());
    write(STDIN_FILENO, "\r\n", 2);
    return;
  }
  std::shared_ptr<Node> node = nodes[values[0]];
  int i;
  for (i = 1; i < values.size() && node->function == nullptr; i++) {
    if (node->children.find(values[i]) == node->children.end()) {
      write(STDIN_FILENO, "Incorrect command usage\r\n", 26);
      return;
    }
    node = node->children[values[i]];
  }
  if (node->function == nullptr) {
    write(STDIN_FILENO, "Incorrect command usage\r\n", 26);
    return;
  }
  std::vector<std::string> arguments(values.begin() + i, values.end());
  node->function(arguments);
}

int Console::HandleInput_(char ch) {
  std::string s;
  switch (ch) {
  case 27:
    HandleEscapeChar_();
    break;
  case 8:
  case 127:
    if (buf.size() > 0) {
      buf.pop_back();
      write(STDIN_FILENO, "\b \b", 3);
    }
    break;
  case '\t':
    AutoComplete_();
    break;
  case 10:
    ExecuteCommand_();
    if (!stop)
      write(STDIN_FILENO, ">", 1);

    break;
  default:
    buf.push_back(ch);
    write(STDIN_FILENO, &ch, 1);
    break;
  }
  return 0;
}

bool Console::VerifyHelper_(const std::shared_ptr<Node> &node) {
  if (node->function != nullptr) {
    if (node->children.size() != 0)
      return false;
    return true;
  }
  if (node->children.size() == 0)
    return false;
  for (auto &it : node->children)
    if (VerifyHelper_(it.second) == false)
      return false;
  return true;
}

bool Console::Verify_() {
  for (auto &it : nodes)
    if (VerifyHelper_(it.second) == false)
      return false;
  return true;
}

void Console::Run() {
  if (!Verify_()) {
    write(STDIN_FILENO, "invalid console", 15);
    return;
  }
  SetRawMode_(true);
  write(STDIN_FILENO, ">", 1);
  std::optional<char> ch;
  while (!stop) {
    ch = ReadChar_();
    if (ch.has_value())
      if (HandleInput_(ch.value()) == 1)
        break;
  }
  SetRawMode_(false);
}

void Console::Run(std::string filename) {
  if (!Verify_()) {
    write(STDIN_FILENO, "invalid console", 15);
    return;
  }
  std::ifstream file(filename);
  if (!file) {
    write(STDIN_FILENO, "invalid file", 12);
    return;
  }
  SetRawMode_(true);
  write(STDIN_FILENO, ">", 1);
  char c;
  bool start = true;
  bool skip = false;
  while (file.get(c) && !stop) {
    switch (c) {
    case ' ':
      if (!start && !skip)
        HandleInput_(c);
      break;
    case '\n':
      if (!start && !skip)
        HandleInput_((char)10);
      start = true;
      skip = false;
      break;
    case '#':
      skip = true;
      break;
    default:
      start = false;
      if (!skip)
        HandleInput_(c);
      break;
    }
  }
  SetRawMode_(false);
  if (!stop)
    Run();
}
} // namespace cli
