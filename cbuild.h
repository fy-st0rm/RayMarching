#pragma once

#include <fstream>
#include <numeric>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>


#define RECORD_FILE "records.txt"

#define LOG(...) \
	printf("[LOG]: ");\
	printf(__VA_ARGS__);\
	printf("\n")

template <typename TP>
static std::time_t to_time_t(TP tp) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(
		tp - TP::clock::now()
		+ system_clock::now()
	);
	return system_clock::to_time_t(sctp);
}

typedef std::unordered_map<std::string, std::time_t> FileRecords;

void cbuild_rebuild(int argc, char** argv);
std::string vec_join(const std::vector<std::string>& vec, const std::string& prefix = "");
bool replace(std::string& str, const std::string& from, const std::string& to);
bool contains(const std::string& src, const std::string& to_find);
void read_file_records(FileRecords& records);

typedef struct {
	std::string dir, cmd, file;
} CompileCommand;

class CBuild {
public:
	CBuild (std::string cc);
	~CBuild();

	CBuild& set_file_records(FileRecords* records);
	CBuild& out(std::string out_dir, std::string out_file);
	CBuild& flags(std::vector<std::string> flags);
	CBuild& objs(std::vector<std::string> objs);
	CBuild& src(std::vector<std::string> src);
	CBuild& inc_paths(std::vector<std::string> inc_paths);
	CBuild& lib_paths(std::vector<std::string> lib_paths);
	CBuild& libs(std::vector<std::string> libs);
	CBuild& pop_objs(std::vector<std::string> objs);
	CBuild& compile();
	CBuild& build();
	CBuild& build_static_lib();
	CBuild& generate_compile_cmds();
	CBuild& run(int argc = 0, char** argv = NULL);
	CBuild& clean();

private:
	bool compile_single(const std::string& src);

private:
	std::string record_filename = "record.txt";
	std::string m_cc, m_out_dir, m_out_file;
	std::vector<std::string> m_flags, m_src, m_objs, m_inc_paths, m_lib_paths, m_libs;
	std::vector<CompileCommand> m_cmp_cmds;
	FileRecords* m_file_records = nullptr;
};


#ifdef CBUILD_IMPLEMENTATION

void cbuild_rebuild(int argc, char** argv) {
#ifdef _WIN32
	std::time_t bin_t = to_time_t(std::filesystem::last_write_time("cbuild.exe"));
#elif defined(__linux__)
	std::time_t bin_t = to_time_t(std::filesystem::last_write_time("cbuild"));
#endif
	std::time_t src_t = to_time_t(std::filesystem::last_write_time("cbuild.cpp"));

	if (src_t > bin_t) {
		LOG("Rebuilding cbuild.cpp");
		std::string cmd = "g++ -o cbuild cbuild.cpp";
		LOG(cmd.c_str());

		int status = std::system(cmd.c_str());
		if (status) {
			LOG("Rebuilding failed.");
			exit(1);
		}

		std::vector<std::string> args(argv, argv + argc);
		std::string rerun = "./cbuild " + vec_join(args);

		status = std::system(rerun.c_str());
		exit(status);
	}
}

std::string vec_join(const std::vector<std::string>& vec, const std::string& prefix) {
	std::string out;
	for (auto e : vec) {
		out += prefix + e + " ";
	}
	return out;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

bool contains(const std::string& src, const std::string& to_find) {
	if (src.find(to_find) != std::string::npos)
		return true;
	return false;
}

void read_file_records(FileRecords& records) {
	std::ifstream infile(RECORD_FILE);
	if (!infile.is_open()) {
		LOG("Cannot find: %s\n", RECORD_FILE);
		return;
	}

	// Read from the file if exists
	std::string line;
		while (std::getline(infile, line)) {
			std::istringstream iss(line);
			std::string filepath;
			std::time_t timestamp;

			if (iss >> filepath >> timestamp) {
				records.insert({filepath, timestamp});
			} else {
				LOG("Invalid line format: %s", line.c_str());
			}
		}

	infile.close();
}

void save_file_records(FileRecords& records) {
	std::ofstream outfile(RECORD_FILE);
	if (!outfile.is_open()) {
		LOG("Failed to write in: %s", RECORD_FILE);
		return;
	}

	for (const auto& [path, timestamp] : records) {
		outfile << path << " " << timestamp << "\n";
	}

	outfile.close();
	LOG("Sucessfully dumped the file records");
}

CBuild::CBuild(std::string cc)
	: m_cc(cc) {
}

CBuild::~CBuild() {
}

CBuild& CBuild::set_file_records(FileRecords* records) {
	m_file_records = records;
	return *this;
}

CBuild& CBuild::out(std::string out_dir, std::string out_file) {
	if (out_dir.length()) {
		LOG("Generating dir: %s", out_dir.c_str());
		std::filesystem::create_directories(out_dir);
	}
	m_out_dir  = out_dir;
	m_out_file = out_file;
	return *this;
}

CBuild& CBuild::flags(std::vector<std::string> flags) {
	m_flags.insert(m_flags.end(), flags.begin(), flags.end());
	return *this;
}

CBuild& CBuild::objs(std::vector<std::string> objs) {
	m_objs.insert(m_objs.end(), objs.begin(), objs.end());
	return *this;
}

CBuild& CBuild::src(std::vector<std::string> src) {
	m_src.insert(m_src.end(), src.begin(), src.end());
	return *this;
}

CBuild& CBuild::inc_paths(std::vector<std::string> inc_paths) {
	m_inc_paths.insert(m_inc_paths.end(), inc_paths.begin(), inc_paths.end());
	return *this;
}

CBuild& CBuild::lib_paths(std::vector<std::string> lib_paths) {
	m_lib_paths.insert(m_lib_paths.end(), lib_paths.begin(), lib_paths.end());
	return *this;
}

CBuild& CBuild::libs(std::vector<std::string> libs) {
	m_libs.insert(m_libs.end(), libs.begin(), libs.end());
	return *this;
}

CBuild& CBuild::pop_objs(std::vector<std::string> objs) {
	for(int i = 0; i < objs.size(); i++) {
		auto iter = std::find(m_objs.begin(), m_objs.end(), objs[i]);
		if(iter != m_objs.end()) {
			m_objs.erase(iter);
		}
	}
	return *this;
}

CBuild& CBuild::generate_compile_cmds() {
	std::ofstream file("compile_commands.json");
	file << "[\n";

	for (auto cmd : m_cmp_cmds) {
		file << "\t{\n";
		file << "\t\t\"directory\": \"" + cmd.dir + "\",\n";
		file << "\t\t\"command\": \"" + cmd.cmd+ "\",\n";
		file << "\t\t\"file\": \"" + cmd.file+ "\"\n";
		file << "\t}";

		if (&cmd == &m_cmp_cmds.back()) {
			file << ",";
		}

		file << "\n";
	}

	file << "]\n";
	file.close();
	return *this;
}

CBuild& CBuild::compile() {
	for (auto src : m_src) {
		bool should_compile = true;

		// Checking if the recorded time and the updated time is same
		// If yes donot compile else compilation is needed
		if (m_file_records != nullptr) {
			if (m_file_records->find(src) != m_file_records->end()) {
				std::time_t updated_time = to_time_t(std::filesystem::last_write_time(src));
				std::time_t recorded_time = (*m_file_records)[src];

				if (updated_time == recorded_time) {
					should_compile = false;

					// If no compilation is needed then just add the obj file
					std::string src_name = std::filesystem::path(src).filename().string();
					std::string obj = "objs/" + src_name;
					if (contains(obj, ".cpp"))
						replace(obj, ".cpp", ".o");
					else if (contains(obj, ".c"));
						replace(obj, ".c", ".o");

					m_objs.push_back(obj);

				} else {
					should_compile = true;
				}
			}
		}

		if (should_compile) {
			if(!compile_single(src)) {
				LOG("Compilation failed at: %s", src.c_str());
				exit(1);
			}
		}
	}
	return *this;
}

CBuild& CBuild::build() {
	compile();

	std::string flags = vec_join(m_flags);
	std::string lib_paths = vec_join(m_lib_paths, "-L");
	std::string libs = vec_join(m_libs, "-l");
	std::string objs = vec_join(m_objs);

	std::string cmd = m_cc + " "
		+ flags
		+ " -o " + m_out_dir +
		(m_out_dir.length() ? "/" : "")
		+ m_out_file + " "
		+ objs + " "
		+ lib_paths + " "
		+ libs;
	LOG(cmd.c_str());

	int status = std::system(cmd.c_str());
	if (status) {
		LOG("Building failed");
		exit(1);
	}

	LOG("Sucessfully built: %s", m_out_file.c_str());

	return *this;
}

CBuild& CBuild::build_static_lib() {
	compile();

	std::string flags = vec_join(m_flags);
	std::string objs = vec_join(m_objs);
	std::string cmd = "ar rcs "
		+ m_out_dir
		+ (m_out_dir.length() ? "/" : "")
		+ m_out_file + " "
		+ objs;

	LOG(cmd.c_str());
	int status = std::system(cmd.c_str());
	if (status) {
		LOG("Building failed");
		exit(1);
	}

	LOG("Sucessfully built: %s", m_out_file.c_str());
	return *this;
}

CBuild& CBuild::run(int argc, char** argv) {
#ifdef _WIN32
  LOG("Not implemented for windows :c");
#elif defined(__linux__)
  std::string out = m_out_dir + (m_out_dir.length() ? "/" : "") + m_out_file;

  LOG("Running: %s", out.c_str());

  pid_t pid = fork();

  if (pid == 0) {
    // Child

    std::vector<char*> args;
    args.push_back(const_cast<char*>(out.c_str()));

    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);

    args.push_back(nullptr);

    execv(out.c_str(), args.data());

    perror("execv");
    _exit(1);
  }

  if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);

    LOG("Process exited with code %d", WEXITSTATUS(status));
  } else {
    LOG("Failed to fork");
  }
#endif

  return *this;
}

CBuild& CBuild::clean() {
	LOG("Cleaning...");
	for (auto obj : m_objs) {
		std::filesystem::remove(obj);
	}
	std::filesystem::remove("objs");
	return *this;
}

bool CBuild::compile_single(const std::string& src) {
	std::string flags = vec_join(m_flags);
	std::string inc_paths = vec_join(m_inc_paths, "-I");

	// Saving the updated time of the src
	if (m_file_records != nullptr) {
		std::time_t updated_time = to_time_t(std::filesystem::last_write_time(src));
		m_file_records->insert_or_assign(src, updated_time);
	}

	// Create objs directory
	std::filesystem::create_directories("objs");

	std::string src_name = std::filesystem::path(src).filename().string();

	std::string obj = "objs/" + src_name;
	if (contains(obj, ".cpp"))
		replace(obj, ".cpp", ".o");
	else if (contains(obj, ".c"));
		replace(obj, ".c", ".o");

	m_objs.push_back(obj);

	std::string dir = std::filesystem::current_path().string();
	std::string cmd = m_cc + " " + flags + " " + inc_paths + " -o " + obj + " -c " + src;
	std::string file = dir + "/" + src;
	m_cmp_cmds.push_back({dir, cmd, file});

	LOG(cmd.c_str());
	int status = std::system(cmd.c_str());
	return !status;
}

#endif
