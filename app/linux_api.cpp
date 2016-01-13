#include <sys/statfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "log.h"
#define MAX_SPACE 100

static QString log_file;
static QString tmp_file;
const char* lock_scan_file = "/tmp/.alto_lock";

bool device_no_space(const char* path)
{
    struct statfs tmp_buf;
    unsigned int t_space_size = MAX_SPACE;
    if(!statfs(path ,&tmp_buf)){
        t_space_size = (unsigned int)(tmp_buf.f_bavail * tmp_buf.f_bsize >> 10);
    }
//    C_LOG("device space:%d K" ,t_space_size);
    return t_space_size < MAX_SPACE;
}

bool scanner_locked()
{
    bool locked = false;
    int altolock = open(lock_scan_file ,O_WRONLY|O_CREAT ,0666);
    fchmod(altolock ,0666);
    if(altolock < 0){
        locked = true;
        _Q_LOG("can not open lock file");
    }else{
        int ret = flock(altolock ,LOCK_EX | LOCK_NB);
        close(altolock);
        locked = !!ret;
    }
    return locked;
}

extern "C"{
/* some systems do not have newest memcpy@@GLIBC_2.14 - stay with old good one */
#ifdef __x86_64__
asm (".symver memcpy, memcpy@GLIBC_2.2.5");
#elif __i386__
asm (".symver memcpy, memcpy@GLIBC_2.0");
#endif

//void *memcpy(void* ,const void* ,size_t);
void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

}

#include <QMutex>
#include <QFile>
#include <QTextStream>
static QMutex mutex;
static QString getStringFromShell(const QString& cmd ,int mode = 0);
static QString getStringFromShell(const QString& cmd ,int mode)
{
    QMutexLocker locker(&mutex);
    QString str;
    QString _cmd(cmd);
    _cmd += ">";
    _cmd += tmp_file;
//    _cmd += "&&chmod 666 ";
//    _cmd += tmp_file;
//    _cmd += " 2>>";
//    _cmd += log_file;
    if(!system(_cmd.toLatin1())){
        QFile fl(tmp_file);
        if(fl.open(QFile::ReadOnly)){
            QTextStream in(&fl);
            if(mode)
                str = in.readAll();
            else
                str = in.readLine();
            fl.close();
            fl.remove();
        }
    }
    return str;
}

bool is_running(const QString& program_dir ,const QString& program_name)
{
    QString cmd("file /proc/*/exe | grep -c '");
    cmd += program_dir;
    cmd += "/\\<";
    cmd += program_name;
    cmd +="\\>' ";
    QString result = getStringFromShell(cmd);
    _Q_LOG("shell cmd:" + cmd);
    _Q_LOG("result:" + result);
    if(result.isEmpty())
        return false;
    return !!result.compare("1");
}

//#include <cups/cups.h>
QString get_device_model(const QString& devicename)
{
//    const char* value;
//    cups_dest_t *dests;
//    int num_dests;
//    num_dests = cupsGetDests(&dests);
//    cups_dest_t* dest;
//    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
//    if(!dest)
//        return VopDevice::Device_invalid;
//    value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
//    QString makeAndModel(value);
//    cupsFreeDests(num_dests ,dests);
//    return value;
    if(devicename.isEmpty())
        return QString();

    QString str("LANG=en lpstat -l -p ");
    str += devicename;
    str += " 2>>";
    str += log_file;
    str += " |awk '/Interface/{printf $NF}' ";

    QString filename;
    filename = getStringFromShell(str);
    if(filename.isEmpty())
        return QString();
    QFile fn(filename);
    if(!fn.exists()){
        _Q_LOG("err:device " +devicename + " ppd file \"" + filename +"\" is not exist");
        return QString();
    }
    str = QString("awk -F\\\" '/\\*NickName/{print $2}'  ");
    str += filename;
    str += " 2>>";
    str += log_file;

    QString makeAndModel;
    makeAndModel = getStringFromShell(str);
    return makeAndModel;
}

QString get_default_printer()
{
    QString default_printer_name;
    QString str("LANG=en lpstat -d 2>>");
    str += log_file;
    str += "|awk 'NR==1{print $NF}' ";

    default_printer_name = getStringFromShell(str);
    return default_printer_name;
}

#include <QStringList>
//#include<QPrinterInfo>
QStringList get_printers()
{
#if 0
#if 0
    QList<QPrinterInfo> printerInfoList = QPrinterInfo::availablePrinters();
    for(int i = 0 ; i < printerInfoList.count() ;i++)
    {
        devices << printerInfoList[i].printerName();
        printerNames << printerInfoList[i].printerName();
        if(printerInfoList[i].isDefault()){
            default_printer = devices.indexOf(printerInfoList[i].printerName());
        }
        if(( !QString::compare(selected_devicename  ,printerInfoList[i].printerName()))){
            selected_printer = devices.indexOf(printerInfoList[i].printerName());
            selected = true;
        }
    }
#else
    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    _Q_LOG("cupsDests:" +QString().sprintf("%d" ,num_dests));
    cups_dest_t *dest;
    int i;
    const char *value;
   for (i = num_dests, dest = dests; i > 0; i --, dest ++)
   {
       if (dest->instance == NULL){
            value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
           if(VopDevice::getDeviceModel(value)){
                devices << dest->name;
                printerNames << dest->name;
//                if(dest->is_default){
//                    default_printer = devices.indexOf(dest->name);
//                }
//                if(( !QString::compare(selected_devicename  ,dest->name))){
//                    selected_printer = devices.indexOf(dest->name);
//                    selected = true;
//                }
           }
       }
   }
//   cupsFreeDests(num_dests, dests);
#endif
#endif
    QStringList printers;
    QString print;
    QString str = QString("LANG=en lpstat -a 2>>");
    str += log_file;
    str +="|awk \'{print $1}\' ";

    print = getStringFromShell(str ,1);
    if(print.isEmpty()){
        return QStringList();
    }
    printers = print.split("\n");
    return printers;
}

QString get_device_uri(const QString& devicename)
{
    QString device_uri;

    //rhel5 not work
//    cups_dest_t *dests;
//    int num_dests;
//    num_dests = cupsGetDests(&dests);
//    cups_dest_t* dest;
//    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
//    if(!dest)
//        return NULL;
//    device_uri =  cupsGetOption("device-uri", dest->num_options, dest->options);
//    cupsFreeDests(num_dests ,dests);

    QString str("LANG=en lpstat -v ");
    str += devicename;
    str += " 2>>";
    str += log_file;
    str += " |awk 'NR==1{print $NF}'";

    device_uri = getStringFromShell(str);
    return device_uri;
}

bool get_printer_jobs(const QString& devicename)
{
    QString str("LANG=en lpstat -l -o ");
    str += devicename;
    str += " 2>>";
    str += log_file;
    str += "|grep -w ";
    str += devicename;
    QString printer_jobs = getStringFromShell(str);
    return !printer_jobs.isEmpty();
}

QString get_printer_status(const QString& devicename)
{
        QString str("LANG=en lpstat -p ");
        str += devicename;
        str += " 2>>";
        str += log_file;
        str += "|awk 'NR==1{printf $4}' ";
        QString printer_status;
        printer_status = getStringFromShell(str);
        return printer_status;
}

#if 1
#include <QApplication>
#include <QLocalSocket>
 #include <QLocalServer>
QLocalServer* m_localServer;
bool isRunning(const QString& serverName)
{
    bool running = true;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected()) {
        _Q_LOG(socket.errorString());
        if(QFile::exists(serverName))
            QFile::remove(serverName);

        m_localServer = new QLocalServer(qApp);
        if (!m_localServer->listen(serverName)) {
            if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
                    && QFile::exists(serverName)) { //make sure listening success
                QFile::remove(serverName);
                m_localServer->listen(serverName);
            }
        }
        if(!system(("chmod a+w " + serverName+ " 2>/dev/null").toLatin1())){
            ;
        }
        running = false;
    }
    return running;
}
#endif

#include <QTime>
#include <QThread>
#include<QTextStream>
#include "../version.h"
void init_log_file()
{
    log_file = "/tmp/AltoVOP.log";
    tmp_file = "/tmp/tmp-" + QDateTime::currentDateTime().toString("yyMMdd-HHmmsszzz");
}

void init_log()
{
    int ret;    
    log_file = "/tmp/AltoVOP-" + QDateTime::currentDateTime().toString("yyMMdd-HHmmsszzz") + ".log";
    QString str = "touch " + log_file;
//    str = "echo \"----------------AltoVOP debug log------------------\" > " + log_file;
//    ret = system(str.toLatin1());
    QFile file(log_file);
    if(file.exists())
        file.remove();
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QTextStream out(&file);
        QString str;
        out << str.sprintf("----------------AltoVOP %s  %s  ------------------" ,vop_version ,copy_right) << endl;
        file.close();
    }
    str = "chmod 666 " + log_file + " 2>>" + log_file;
    ret = system(str.toLatin1());
    str = "rm /tmp/AltoVOP.log 2>>" + log_file;
    ret = system(str.toLatin1());
    str = "ln -s " + log_file + " /tmp/AltoVOP.log" " 2>>" + log_file;
    ret = system(str.toLatin1());
    str = "uname -a >> " +log_file;
    ret = system(str.toLatin1());
    str = "cat /etc/issue >> " +log_file;
    ret = system(str.toLatin1());
    if(ret){;}
}

static QMutex mutex_write_log_file;
void logout(const QString& msg)
{
    QMutexLocker locker(&mutex_write_log_file);

    QFile file(log_file);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz:")
               <<"                        " <<msg << endl;
//            <<QThread::currentThreadId()<<"                        " <<msg << endl;
        file.close();
    }
}

bool region_paper_is_A4()
{
    return true;
    QString str("locale -k LC_PAPER  ");
    str += " 2>>";
    str += log_file;
    str += "|awk '{if($1==\"height=297\" || $1==\"width=210\"){print \"A4\"}}' ";
    QString result;
    result = getStringFromShell(str);
    return !result.compare("A4");
}
