#ifndef LINUX_API
#define LINUX_API

#include <QString>

bool device_no_space(const char* path);
bool scanner_locked();
bool is_running(const QString& program_dir ,const QString& program_name);
bool isRunning(const QString& serverName);
QString get_device_model(const QString& devicename);
QString get_default_printer();
QStringList get_printers();
QString get_device_uri(const QString& devicename);
bool get_printer_jobs(const QString& devicename);
QString get_printer_status(const QString& devicename);
bool region_paper_is_A4();

#endif // LINUX_API

