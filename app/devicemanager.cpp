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

int DeviceManager::isDeviceValid(const char* value)
{
    (void)value;
    return true;
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
    vop_setDeviceName(current_devicename.toLatin1());
    copycmdset cpcmd = getCopyParameter();
    vop_copy(&cpcmd);
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
const char* DeviceManager::getCurrentDeviceURI()
{
    mutex.lock();
    current_devicename = selected_devicename;
    mutex.unlock();

    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(current_devicename.toLatin1() ,NULL ,num_dests ,dests);
    if(!dest)
        return NULL;
    return  cupsGetOption("device-uri", dest->num_options, dest->options);
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
            if(isDeviceValid(value))
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
            }
       }
   }
   if(!devices.count())
   {
       qDebug()<<"there is no printer";
       return -1;
   }

   if(!selected && -1 != default_printer)
   {
           selected_printer =default_printer;
           selected = true;
           qDebug()<<"select the default printer";

   }else{
       qDebug()<<"the selected printer is founded";
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

int DeviceManager::device_cmd(const char* device_name ,char* buffer ,int len)
{
    int err = 0;

    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(device_name ,NULL ,num_dests ,dests);
    if(!dest)
        return -1;

    //debug display all printer options
    qDebug()<<dest->name;
    cups_option_t* option;
    int j;
    for(j=0,option=dest->options ;j < dest->num_options ;j++,option++)
    {
        qDebug()<<option->name<<option->value;
    }

    cupsFreeDests(num_dests ,dests);
    (void)buffer;
    (void)len;
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
