






#ifndef YLOGGER_H
#define YLOGGER_H

#include <vector>
#include <iostream>

typedef enum enYLoggerModule {
    Module_Ui           = 0x00000001,
    Module_Utils        = 0x00000002,
    Module_VideoIo      = 0x00000004,
    Module_Algorithm    = 0x00000008,
    Module_All          = 0x0000ffff
} YLoggerModule;

typedef enum enYLoggerSeverity {
    LoggerSeverity_DEBUG     = 0x00000000,
    LoggerSeverity_INFO      = 0x00000001,
    LoggerSeverity_WARNING   = 0x00000002,
    LoggerSeverity_ERROR     = 0x00000003,
    LoggerSeverity_FATAL     = 0x00000004
} YLoggerSeverity;

#define YLOG(module, severity) YLogger(__FILE__, __LINE__, module, severity).log()

namespace google {
    class LogMessage;
}

class YLogger
{
public:
    YLogger(const char* file, int line, int module, int severity);
    ~YLogger();

    std::ostream& log();

    static int LogFileCount;
    static std::string LogDir;
    static std::string LogConfigFilePath;
    static YLoggerModule LogModule;
    static YLoggerSeverity LogSeverity;

private:
    const char* m_file;
    int m_line;
    int m_module;
    int m_severity;
    std::ostream m_dummy;
    google::LogMessage *m_logger;
};

/* supported options are list below
 * LogModule=All
 * LogSeverity=INFO
 * LogDir=/opt/log                  绝对路径，暂不支持相对路径
 * LogFileCount=20
 * LogConfigFilePath=/opt/pm.conf   绝对路径，暂不支持相对路径
 */
void readLogConfig();

bool initLoggerModule(const char* log_dir = nullptr, const char* config_path = nullptr);

#endif // YLOGGER_H
