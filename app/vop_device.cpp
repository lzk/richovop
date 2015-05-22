#include <cups/cups.h>
#include<QDebug>
#include<QStringList>
#include<cups/sidechannel.h>

int device_cmd(char* buffer ,int len)
{
    int result = 0;
    return result ;
}

QStringList get_dests()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
//    cups_dest_t *dest = cupsGetDest("name", NULL, num_dests, dests);
    /* do something with dest */
    QStringList str;
    QStringList list;
    for(int i = 0; i < num_dests ;i++)
    {
        str <<dests[i].name;
        for(int j=0 ;j < dests[i].num_options ;j++)
        {
            list<<dests[i].options[j].name<<dests[i].options[j].value<<"\n";
        }
        qDebug()<<dests[i].name<<list;
        list.clear();
    }

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

    cupsFreeDests(num_dests, dests);
    return str;
}
