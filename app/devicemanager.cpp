#include "devicemanager.h"

#include<QDebug>
#include<QStringList>
#include"vop_device.h"

DeviceManager::DeviceManager(QObject *parent) : QObject(parent),
  device(new VopDevice)
{
    devices.clear();
}

DeviceManager::~DeviceManager()
{
    delete device;
}

void DeviceManager::slots_device_status()
{
    static int status = 0;
    if(status) status = 0;
    else status = 1;
//    qDebug()<<"Get device status:"<<status;
    emit signals_device_status(status);
}

void DeviceManager::slots_copy()
{
    mutex.lock();
    current_devicename = selected_devicename;
    mutex.unlock();
    if(current_devicename == QString())
        return;
    copycmdset cpcmd = getCopyParameter();
    vop_copy(&cpcmd ,sizeof(cpcmd));
}

void DeviceManager::setCopyParameter(copycmdset* p)
{
    QMutexLocker locker(&mutex);
    device->getVopCopy()->setCopyParameter(p);
}

copycmdset DeviceManager::getCopyParameter()
{
    QMutexLocker locker(&mutex);
    return device->getVopCopy()->getCopyParameter();
}

void DeviceManager::setDefaultCopyParameter()
{
    QMutexLocker locker(&mutex);
    device->getVopCopy()->set_parameter_default();
}

void DeviceManager::selectDevice(int selected_device)
{
    QMutexLocker locker(&mutex);
    if(devices.count())
    {
        selected_devicename = devices.at(selected_device);
    }
    else
    {
        selected_devicename = QString();
    }
}

QString DeviceManager::getDeviceURI(const QString& devicename)
{
    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
    if(!dest)
        return NULL;
    QString device_uri =  cupsGetOption("device-uri", dest->num_options, dest->options);
    cupsFreeDests(num_dests ,dests);
    return device_uri;
}

QString DeviceManager::getCurrentDeviceURI()
{
    mutex.lock();
    current_devicename = selected_devicename;
    mutex.unlock();

    return getDeviceURI(current_devicename);
}

//#include<QPrinterInfo>
int DeviceManager::getDeviceList(QStringList& printerInfo)
{
#if 0

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    printerNames =QPrinterInfo::availablePrinterNames();
#else
    QList<QPrinterInfo> printerInfoList = QPrinterInfo::availablePrinters();
    for(int i = 0 ; i < printerInfoList.count() ;i++)
    {
        printerNames.append(printerInfoList[i].printerName());
    }
#endif//(QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))

#endif

    int selected_printer = 0;
    int default_printer = -1;
    cups_dest_t *dests;
    int num_dests;
//clear the list
    devices.clear();

    //get new list
    num_dests = cupsGetDests(&dests);

    cups_dest_t *dest;
    int i;
    const char *value;
    bool selected = false;
   for (i = num_dests, dest = dests; i > 0; i --, dest ++)
   {

       if (dest->instance == NULL)
       {
            value = cupsGetOption("printer-info", dest->num_options, dest->options);
            if(VopDevice::isValidDevice(value))
            {
                devices << dest->name;
                printerInfo << value;

                if(dest->is_default)
                {
                    default_printer = devices.indexOf(dest->name);
                }
                if(( !QString::compare(selected_devicename  ,dest->name)))
                {
                    selected_printer = devices.indexOf(dest->name);
                    selected = true;
                }

                //debug display all printer options
                qDebug()<<dest->name;
               cups_option_t* option;
                int j;
               for(j=0,option=dest->options ;j < dest->num_options ;j++,option++)
                {
                    qDebug()<<option->name<<option->value;
                }
               qDebug()<<"";
            }
       }
   }
   if(!devices.count())
   {
       qDebug()<<"there is no printer";
       return -1;
   }

   if(selected)
       qDebug()<<"the selected printer is founded";
   else if(-1 != default_printer)
   {
           selected_printer =default_printer;
           selected = true;
           qDebug()<<"select the default printer";

   }

   if(!selected)
   {
       qDebug()<<"the printer isn't selected,select the first printer";
   }
   //select the device
   selectDevice(selected_printer);   
   cupsFreeDests(num_dests, dests);
   return selected_printer;
}

QString DeviceManager::current_devicename = QString();
int DeviceManager::device_writeThenRead(const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    if(!current_devicename.count())
        return -1;
    int err = 0;
    QString device_uri = getDeviceURI(current_devicename);
    qDebug()<<"device-uri"<<device_uri;
    err = VopDevice::writeThenRead(device_uri.toLatin1() ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    return err;
}

#include<cups/sidechannel.h>
#include<cups/backend.h>
#include <string.h>
int device_cmd(char* buffer ,int len)
{
    int result = 0;
    (void)buffer;
    (void)len;
    return result ;

    char data[2049];
    int datalen = 2048;
    cups_sc_status_t status =
    cupsSideChannelDoRequest(CUPS_SC_CMD_GET_DEVICE_ID ,data ,&datalen ,1.0);
    if(status == CUPS_SC_STATUS_OK && datalen > 0)
        data[datalen] = '\0';
    else
        data[0]='\0';
    char* s = data;
    qDebug()<<"device id"<<s;
}
#if 1
void exec_backend(char **argv)		/* I - Command-line arguments */
{
  const char	*resolved_uri,		/* Resolved device URI */
        *cups_serverbin;	/* Location of programs */
  char		scheme[1024],		/* Scheme from URI */
        *ptr,			/* Pointer into scheme */
        filename[1024];		/* Backend filename */


 /*
  * Resolve the device URI...
  */

  while ((resolved_uri = cupsBackendDeviceURI(argv)) == NULL)
  {
//    _cupsLangPrintFilter(stderr, "INFO", _("Unable to locate printer."));
    qDebug()<<"Unable to locate printer.";
    sleep(10);

    if (getenv("CLASS") != NULL)
      exit(CUPS_BACKEND_FAILED);
  }

 /*
  * Extract the scheme from the URI...
  */

//  strlcpy(scheme, resolved_uri, sizeof(scheme));
  if ((ptr = strchr(scheme, ':')) != NULL)
    *ptr = '\0';

 /*
  * Get the filename of the backend...
  */

  if ((cups_serverbin = getenv("CUPS_SERVERBIN")) == NULL)
//    cups_serverbin = CUPS_SERVERBIN;

  snprintf(filename, sizeof(filename), "%s/backend/%s", cups_serverbin, scheme);

 /*
  * Overwrite the device URI and run the new backend...
  */

  setenv("DEVICE_URI", resolved_uri, 1);

  argv[0] = (char *)resolved_uri;

  fprintf(stderr, "DEBUG: Executing backend \"%s\"...\n", filename);

  execv(filename, argv);

//  fprintf(stderr, "ERROR: Unable to execute backend \"%s\": %s\n", filename,
//          strerror(errno));
//  exit(CUPS_BACKEND_STOP);
}
#endif
