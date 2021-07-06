#include <iostream>
#include <string>
#include <tchar.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

std::string get_environment_variable(const std::string& environment_variable_name)
{
    char* buffer = nullptr;
    size_t buffer_size = 0;
    if (const auto errno_value = _dupenv_s(&buffer, &buffer_size, environment_variable_name.c_str());
        errno_value != 0)
    {
        throw std::runtime_error("_dupenv_s() failed: " + std::to_string(errno_value));
    }

    if (buffer == nullptr)
    {
        throw std::runtime_error("buffer was nullptr");
    }

    std::string environment_variable = buffer;
    free(buffer);
    return environment_variable;
}

void terminate_process(const DWORD process_id, const UINT exit_code)
{
	const auto process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
    if (process_handle == nullptr)
    {
        throw std::runtime_error("OpenProcess() failed");
    }

	if(const auto result = TerminateProcess(process_handle, exit_code); !result)
	{
        CloseHandle(process_handle);
        throw std::runtime_error("TerminateProcess() failed");
	}
	
    CloseHandle(process_handle);
}

void terminate_processes(const std::wstring &process_name)
{
    PROCESSENTRY32 pe32{};

    // Take a snapshot of all processes in the system.
	const auto process_snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (process_snapshot_handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("CreateToolhelp32Snapshot() failed");
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(process_snapshot_handle, &pe32))
    {
        CloseHandle(process_snapshot_handle);  // clean the snapshot object
        throw std::runtime_error("Process32First() failed");
    }

    // Now walk the snapshot of processes 
    do
    {
	    if (std::wstring(pe32.szExeFile) == process_name)
        {
            terminate_process(pe32.th32ProcessID, 1);
            std::cout << pe32.th32ProcessID << " successfully terminated" << std::endl;
        }
    } while (Process32Next(process_snapshot_handle, &pe32));

    CloseHandle(process_snapshot_handle);
}

// For static linking support
#pragma comment(lib, "ws2_32")
#pragma comment (lib, "Wldap32")
#pragma comment (lib, "crypt32")

std::string download_file_contents(const std::string& server_url)
{
    const curlpp::options::Url url(server_url);
    curlpp::Easy request;
    request.setOpt(url);
    request.setOpt(new curlpp::options::FollowLocation(true));
	
    std::ostringstream output_stream;
    const curlpp::options::WriteStream write_stream(&output_stream);
    request.setOpt(write_stream);
    request.perform();

    return output_stream.str();
}

void write_string_to_file(const std::filesystem::path& file_path, const std::string& file_contents)
{
    std::ofstream file_writer(file_path.string(), std::ios_base::binary);
    if (!file_writer.is_open())
    {
        throw std::runtime_error("Could not open file "
            + file_path.string() + " for writing");
    }
    file_writer << file_contents;
}

int main()
{
    try
    {
        SetConsoleOutputCP(1252); // Allow the copyright symbol to be displayed correctly
        std::cout << "Discord Installation Failed Fixer v1.0 \xa9 2021 BullyWiiPlaza Productions" << std::endl;
        Sleep(2000);
        std::cout << "Terminating running Discord instances..." << std::endl;
        terminate_processes(L"Discord.exe");
        Sleep(1000);
    	
        std::cout << "Deleting local appdata leftovers..." << std::endl;
        const std::filesystem::path local_app_data_path = get_environment_variable("localappdata");
        const auto local_app_data_discord_file_path = local_app_data_path / "Discord";
    	if (is_directory(local_app_data_discord_file_path))
        {
			remove_all(local_app_data_discord_file_path);
        } else
        {
            std::cout << "Directory did not exist..." << std::endl;
        }

        std::cout << "Deleting appdata leftovers..." << std::endl;
        const std::filesystem::path app_data_path = get_environment_variable("appdata");
        const auto app_data_discord_file_path = app_data_path / "discord";
        if (is_directory(local_app_data_discord_file_path))
        {
	        remove_all(app_data_discord_file_path);
        } else
        {
            std::cout << "Directory did not exist..." << std::endl;
        }

        char response;
        while (true)
        {
            std::cout << "Do you want to download the latest Discord setup? [y/n]" << std::endl;
            std::string read_line;
            std::getline(std::cin, read_line);

            if (read_line.size() != 1)
            {
                continue;
            }
        	
            response = read_line.at(0);
            if (response != 'y' && response != 'n')
            {
                continue;
            }

            break;
        }

    	if (response == 'y')
    	{
            std::cout << "Downloading fresh Discord setup..." << std::endl;
            const std::filesystem::path downloads_folder = get_environment_variable("USERPROFILE");
            const auto downloaded_discord_file_path = downloads_folder / "Downloads" / "DiscordSetup.exe";
            const std::string discord_download_url = "https://discord.com/api/downloads/distributions/app/installers/latest?channel=stable&platform=win&arch=x86";
            const auto downloaded_file_contents = download_file_contents(discord_download_url);
            std::cout << "Writing Discord setup to " << downloaded_discord_file_path << "..." << std::endl;
            write_string_to_file(downloaded_discord_file_path, downloaded_file_contents);
            std::cout << "Running fresh Discord setup..." << std::endl;
            if (const auto shell_execute_result = ShellExecuteA(nullptr, "open", downloaded_discord_file_path.string().c_str(),
                                                               nullptr, nullptr, SW_SHOWDEFAULT);
                reinterpret_cast<int>(shell_execute_result) <= 32)
    		{
                throw std::runtime_error("Opening Discord setup failed");
    		}
    	}

        std::cout << "All operations completed successfully!" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
    	
        getchar();
    }
	catch (std::exception &exception)
	{
        std::cerr << "An unhandled exception occurred: " << exception.what() << std::endl;
	}
	
    return EXIT_SUCCESS;
}