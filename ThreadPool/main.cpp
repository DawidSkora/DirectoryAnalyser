#include "ThreadPool.h"
#include <filesystem>
#include <iostream>
#include <thread>

std::mutex dirMutex;
std::mutex fileMutex;
int dir = 0;
int file = 0;
bool TPdone = false;
bool pathExist = true;

void dairectoryCounter()
{
	std::unique_lock<std::mutex> lock{ dirMutex };
	++dir;
}

void filesCounter()
{
	std::unique_lock<std::mutex> lock{ fileMutex };
	++file;
}

void checkDir(const std::filesystem::path& p, ThreadPool& TP)
{
	for (auto it = std::filesystem::directory_iterator(p); it != std::filesystem::directory_iterator(); ++it)
	{
		std::filesystem::file_status s = it->symlink_status();
		std::filesystem::path _p = *it;
		switch (s.type())
		{
			case std::filesystem::file_type::regular:
				filesCounter();
				break;
			case std::filesystem::file_type::directory:
				dairectoryCounter();
				std::function<void(const std::filesystem::path&, ThreadPool&)> f_check = checkDir;
				TP.feedQueue(f_check, _p);
				break;
		}
	}
}

int main()
{
	{
		std::string apath = "../Sandbox";
		std::cout << "Enter directory path: " << std::endl;
		std::cin >> apath;
		std::filesystem::path pathToDir(apath);

		if (std::filesystem::exists(pathToDir))
		{
			ThreadPool TP;
			checkDir(pathToDir, TP);
		}
		else
		{
			pathExist = false;
		}
	}

	while (pathExist)
	{
		if (TPdone)
		{
			std::cout << "Directories: " << dir << " files: " << file << std::endl;
			break;
		}
	}

    return 42;
}