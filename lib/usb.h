#ifndef __myUSB_H__
#define __myUSB_H__
//Protocol limitations: you have to open device before do any r/w or get device id ops,
//                      And close it to release system resource.


int openPrinter(char* printerURI);
void closePrinter(void);
int USBWrite(char *buffer, size_t bufsize);
int USBRead(char *buffer, size_t bufsize);
int get_device_id(char *buffer, size_t bufsize);


#endif /* __USB_H__ */
