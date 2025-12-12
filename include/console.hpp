#pragma once
#include <termios.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>
#include <deque>
#include <sstream>

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

struct Console
{
	std::unordered_map<std::string,std::shared_ptr<Node>> nodes;
	bool stop = false;
	std::string buf= "";
	std::deque<std::string> hist;
	int histIndex=-1;
	struct Set
	{
		bool newLineAutoComplete = true;
	};
	Set settings;

	Node& operator[](const std::string& key)
	{
		std::shared_ptr<Node>& ret = nodes[key];

		if(!ret)
			ret = std::make_shared<Node>();

		return *ret;
	}
};

void setRawMode(bool enable)
{
	static struct termios oldt,newt;
	if(enable)
	{
		tcgetattr(STDIN_FILENO,&oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON|ECHO);
		newt.c_cc[VMIN] = 1;
		newt.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO,TCSANOW,&newt);
	}else
	{
		tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
	}
}

std::optional<char> readChar()
{
	char c;
	if(read(STDIN_FILENO,&c,1))
		return c;
	return {};
}

bool input_available(int fd, int timeout_ms) 
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv{0, timeout_ms * 1000};
    return select(fd + 1, &fds, NULL, NULL, &tv) > 0;
}

void handleEscapeChar(Console& console)
{
	if (!input_available(STDIN_FILENO, 10))
	{
		write(STDIN_FILENO,"\r\n",2);
		console.stop = true;
		return;
	}
	char seq[2];
 		read(STDIN_FILENO, seq, 2);
	if(seq[0]!='[')
		return;
	bool arrows = false;
	if(seq[1]=='A'||seq[1]=='B')
	{
		if(console.hist.size()==0)
			return;
		arrows = true;
		write(STDIN_FILENO,"\r\033[K>",5);
	}
	if(seq[1]=='A')
	{
		//TODO: check if i actually need this <10 check
		if(console.histIndex<(int)(console.hist.size()-1))
			console.histIndex++;
	}
	else if(seq[1]=='B')
	{
		if(console.histIndex>=0)
			console.histIndex--;
	}
	console.buf="";
	if(arrows&&console.histIndex!=-1)
	{
		write(STDIN_FILENO,console.hist[console.histIndex].c_str(),console.hist[console.histIndex].size());
		console.buf = console.hist[console.histIndex];
	}
}

void autoComplete(Console& console)
{
	std::istringstream ss(console.buf);
	std::vector<std::string> values;
	std::string value;
	auto* nodes = &console.nodes;
	while(getline(ss,value,' '))
	{
		if(value.size()>0)
			values.push_back(value);
	}
	if(values.size()==0)
		return;
	std::string result = "";
	std::string best="";
	bool sure;
	for(int i=0;i<values.size()&&nodes->size() != 0;i++)
	{
		sure = true;
		for(auto& it : *nodes)
		{
			if(values[i]!=it.first.substr(0,values[i].size()))
				continue;
			if(best=="")
				best = it.first;
			for(int j=0;j<it.first.size()&&j<best.size();j++)
			{
				if(best[j]!=it.first[j])
				{
					best = best.substr(0,j);
					sure = false;
				}else if(j==it.first.size()-1)
				{
					auto s = best.substr(0,j+1);
					if(s!=best)
						sure = false;
					best = s;
				}
			}
		}
		result += best ;
		if(sure)
		{
			result += " ";
			nodes = &((*nodes)[best]->children);
		}
		best = "";
	}
	if(result.size() <= console.buf.size())
		return;

	console.buf = result;
	if(!console.settings.newLineAutoComplete)
	{
		write(STDIN_FILENO,"\r>",2);
		write(STDIN_FILENO,result.c_str(),result.size());
	}
	else
	{
		write(STDIN_FILENO,"\r\n>",3);
		write(STDIN_FILENO,result.c_str(),result.size());
	}
}

void saveToHist(Console& console)
{
	console.histIndex=-1;
	if(console.buf==""||console.buf==console.hist[0])
		return;
	
	if(console.hist.size()<10)
		console.hist.push_front(console.buf);
	else
	{
		console.hist.pop_back();
		console.hist.push_front(console.buf);
	}
}

void executeCommand(Console& console)
{
	saveToHist(console);
	std::istringstream ss(console.buf);
	std::vector<std::string> values;
	std::string value;
	while(getline(ss,value,' '))
	{
		if(value.size()>0)
			values.push_back(value);
	}
	console.buf = "";
	write(STDIN_FILENO,"\r\n",2);
	if(values.size()<1)
		return;
	if(console.nodes.find(values[0])==console.nodes.end())
	{
		write(STDIN_FILENO, "Unknown command: \r\n",19);
		write(STDIN_FILENO, values[0].c_str(),values[0].size());
		return;
	}
	std::shared_ptr<Node> node = console.nodes[values[0]];
	int i;
	for(i=1;i<values.size()&&node->function==nullptr;i++)
	{
		if(node->children.find(values[i])==node->children.end())
		{
			write(STDIN_FILENO,"Incorrect command usage\r\n",26);
			return;
		}
		node = node->children[values[i]];
	}
	if(node->function == nullptr)
	{
		write(STDIN_FILENO,"Incorrect command usage\r\n",26);
		return;
	}
	std::vector<std::string> arguments(values.begin()+i,values.end());
	node->function(arguments);
}

int handleInput(char ch,Console& console)
{
	std::string s;
	switch(ch)
	{
		case 27:
			handleEscapeChar(console);
			break;
		case 8:
		case 127:
			if(console.buf.size()>0)
			{
				console.buf.pop_back();
				write(STDIN_FILENO,"\b \b",3);
			}
			break;
		case '\t':
			autoComplete(console);
			break;
		case 10:			
			executeCommand(console);
			if(!console.stop)
				write(STDIN_FILENO,">",1);
			
			break;
		default:
			console.buf.push_back(ch);
			write(STDIN_FILENO,&ch,1);
		break;
	}
	return 0;
}

bool verifiHelper(const std::shared_ptr<Node>& node)
{
	if(node->function!=nullptr)
		if(node->children.size()!=0)
			return false;
		else
			return true;
	if(node->children.size()==0)
		return false;
	for(auto& it : node->children)
		if(verifiHelper(it.second)==false)
			return false;
	return true;
}

bool verifyConsole(const Console& console)
{
	for(auto& it : console.nodes)
		if(verifiHelper(it.second)==false)
			return false;
	return true;
}

void runConsole(Console& console)
{
	if(!verifyConsole(console))
	{
		write(STDIN_FILENO,"invalid console",15);
		return;
	}
	setRawMode(true);
	write(STDIN_FILENO,">",1);
  std::optional<char> ch;
	while(!console.stop)
	{
		ch = readChar();
		if(ch.has_value())
			if(handleInput(ch.value(),console)==1)
				break;
	}
	setRawMode(false);
}
