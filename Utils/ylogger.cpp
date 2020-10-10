






#include <map>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <glog/logging.h>
#include <glog/log_severity.h>

#include "ylogger.h"

std::map<int, std::string> registerLoggerModuleName()
{
    std::map<int, std::string> module_name;
    module_name.insert(std::pair<int, std::string>(Module_Ui, "Ui"));
    module_name.insert(std::pair<int, std::string>(Module_Utils, "Utils"));
    module_name.insert(std::pair<int, std::string>(Module_VideoIo, "VideoIo"));
    module_name.insert(std::pair<int, std::string>(Module_Algorithm, "Algorithm"));
    return module_name;
}
static std::map<int, std::string> g_YLoggerModuleName = registerLoggerModuleName();

int YLogger::LogFileCount = 20;
std::string YLogger::LogDir = strcat(getcwd(nullptr, 512), "/log");
std::string YLogger::LogConfigFilePath = strcat(getcwd(nullptr, 512), "/pm.conf");
YLoggerModule YLogger::LogModule = Module_All;
YLoggerSeverity YLogger::LogSeverity = LoggerSeverity_INFO;


YLogger::YLogger(const char* file, int line, int module, int severity) : m_dummy(nullptr)
{
    m_file   = file;
    m_line   = line;
    m_module = module;
    m_severity = severity;
    m_logger = nullptr;
}

YLogger::~YLogger() {
    if (nullptr != m_logger) {
        delete m_logger;
    }
}

std::ostream& YLogger::log()
{
    auto module = YLogger::LogModule;
    auto severity = YLogger::LogSeverity;
    if (!(m_module & module) || m_severity < severity)
    {
        return m_dummy;
    }

    int google_severity = m_severity + google::GLOG_INFO - LoggerSeverity_INFO;
    google_severity = (google_severity >= google::GLOG_INFO ? google_severity : google::GLOG_INFO);
    m_logger = new google::LogMessage(m_file, m_line, google_severity);

    return m_logger->stream() << g_YLoggerModuleName.at(module) << ": ";
}


std::map<std::string, YLoggerModule> registerLoggerNameModule()
{
    std::map<std::string, YLoggerModule> module_name;
    module_name.insert(std::pair<std::string, YLoggerModule>("Ui", Module_Ui));
    module_name.insert(std::pair<std::string, YLoggerModule>("Utils", Module_Utils));
    module_name.insert(std::pair<std::string, YLoggerModule>("VideoIo", Module_VideoIo));
    module_name.insert(std::pair<std::string, YLoggerModule>("Algorithm", Module_Algorithm));
    module_name.insert(std::pair<std::string, YLoggerModule>("All", Module_All));
    return module_name;
}
static std::map<std::string, YLoggerModule> g_YLoggerNameModule = registerLoggerNameModule();

std::map<std::string, YLoggerSeverity> registerLoggerNameSeverity()
{
    std::map<std::string, YLoggerSeverity> module_severity;
    module_severity.insert(std::pair<std::string, YLoggerSeverity>("DEBUG", LoggerSeverity_DEBUG));
    module_severity.insert(std::pair<std::string, YLoggerSeverity>("INFO", LoggerSeverity_INFO));
    module_severity.insert(std::pair<std::string, YLoggerSeverity>("WARNING", LoggerSeverity_WARNING));
    module_severity.insert(std::pair<std::string, YLoggerSeverity>("ERROR", LoggerSeverity_ERROR));
    module_severity.insert(std::pair<std::string, YLoggerSeverity>("FATAL", LoggerSeverity_FATAL));
    return module_severity;
}
static std::map<std::string, YLoggerSeverity> g_YLoggerNameSeverity = registerLoggerNameSeverity();

void readLogConfig()
{
    std::ifstream in(YLogger::LogConfigFilePath);
    if(!in.is_open())
    {
        return;
    }

    char buffer[256];
    char key[256];
    char value[256];
    while(!in.eof())
    {
        in.getline(buffer, 255);
        if('#' == buffer[0]) {
            continue;
        }

        int key_index = 0;
        int value_index = 0;
        bool value_start = false;
        for(int i = 0; i < 256; i++) {
            if('\n' == buffer[i] || '\0' == buffer[i]) {
                break;
            }

            if('=' == buffer[i]) {
                value_start = true;
                continue;
            } else if(' ' == buffer[i]) {
                continue;
            }

            if(value_start) {
                key[key_index++] = buffer[i];
            } else {
                value[value_index++] = buffer[i];
            }
        }
        key[key_index] = '\0';
        value[value_index] = '\0';

        if(0 == strcmp(key, "LogModule")) {
            if(g_YLoggerNameModule.find(key) != g_YLoggerNameModule.end()) {
                YLogger::LogModule = g_YLoggerNameModule.at(key);
            }
        } else if(0 == strcmp(key, "LogSeverity")) {
            if(g_YLoggerNameSeverity.find(key) != g_YLoggerNameSeverity.end()) {
                YLogger::LogSeverity = g_YLoggerNameSeverity.at(key);
            }
        } else if(0 == strcmp(key, "LogDir")) {
            YLogger::LogDir.assign(value);
        } else if(0 == strcmp(key, "LogFileCount")) {
            YLogger::LogFileCount = atoi(value);
        } else if(0 == strcmp(key, "LogConfigFilePath")) {
            YLogger::LogConfigFilePath.assign(value);
        }
    }

    in.close();
}

bool initLoggerModule(const char *log_dir, const char *config_path)
{
    int mkdir_ret = 0;
    if(nullptr == log_dir) {
        mkdir_ret = mkdir(YLogger::LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    } else {
        YLogger::LogDir.assign(log_dir);
        mkdir_ret = mkdir(log_dir, S_IRWXU | S_IRWXG | S_IROTH);
    }

    if(0 != mkdir_ret) {
        return false;
    }

    if(nullptr != config_path) {
        YLogger::LogConfigFilePath.assign(config_path);
    }

    readLogConfig();
    google::InitGoogleLogging("Y");
    google::SetStderrLogging(google::GLOG_INFO);
    fLS::FLAGS_log_dir = YLogger::LogDir;
    fLI::FLAGS_logbufsecs = 0;
    fLI::FLAGS_max_log_size = 20;

    /* TODO: 开启后台线程，控制日志文件数量 */

    return true;
}
