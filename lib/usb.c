/*
 * testlibusb.c
 *
 *  Test suite program
 */
#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
//#include <usb.h>
#include <errno.h>
//#include <cups/cups.h>
#include <strings.h>
#include <sys/time.h>

//#include "usb.h"


/* some systems do not have newest memcpy@@GLIBC_2.14 - stay with old good one */
#ifdef __x86_64__
asm (".symver memcpy, memcpy@GLIBC_2.2.5");
#elif __i386__
asm (".symver memcpy, memcpy@GLIBC_2.0");
#endif

void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

#ifdef __x86_64__
asm (".symver __fdelt_chk, memcpy@GLIBC_2.2.5");
#elif __i386__
asm (".symver __fdelt_chk, memcpy@GLIBC_2.0");
#endif
//__asm__(".symver __fdelt_chk,__fdelt_chk@GLIBC_2.2.5");
void *__wrap_fdelt_chk(void *dest, const void *src, size_t n)
{
    return fdelt_chk(dest, src, n);
}

//#define ALTO_DEBUG 1

int verbose = 0;
libusb_device *g_device = NULL;
libusb_device_handle *g_dev_h = NULL;
static libusb_context *usb_ctx;
char g_open_flag = 0;
static int g_interface = 0;
//int first_run = 0;
//int last_status = 0;


#define devPID	"0456"
#define devVID	"05ca"

int search_char_idx(const char *str, int c)
{
	int i = 0;
	if(index(str, c) == NULL)
		return 0;
	while(str[i] != c)
	{
		i++;
	}
	return i;
	
}

int check_number(const char *str)
{
	int len = strlen(str);
	if(len == 0)
		return 0;
	int i;
	for(i = 0; i < len; i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			return 0;
		}
	}
	return 1;

}

int fetch_interface_value(const char *str, char *if_str)
{
	int len = strlen(str);
	int i,j;
	j=0;
	char num[5];
	for(i = 0; i < len; i++)
	{
		if(str[i] >= '0' && str[i] <= '9')
		{
			num[j] = str[i];
			num[j + 1] = '\0';
			j++;
		}
	}
	if(j)
	{
		memcpy(if_str, num, strlen(num));
		if_str[strlen(num)] = '\0';
		return atoi(num);
	}
	return -1;

}

int URI_convert(const char* uri, char* new_uri)
{
        char vid[8], pid[8], serial[256], if_str[8];
	char uri_con[1024];
	int inf = 0;
	if(uri == NULL || new_uri == NULL)
	{
#ifdef ALTO_DEBUG
		printf("URI_convert: URI is NULL\n");
#endif
		return 0;
	}

/*	if(strstr(uri, "hal://") == NULL)
	{
		printf("URI is not hal://\n");
		return -1;
	}
*/
	
	char * uri_tmp = strstr(uri, "usb_device_");
	if(uri_tmp != NULL)
	{
		uri_tmp +=11;
	}
	else
	{
#ifdef ALTO_DEBUG
		printf("URI_convert: URI not contain \"usb_device_\"\n");
#endif
		return 0;
	}
	int index =0;
	index = search_char_idx (uri_tmp, '_');
	if(index == 0)
	{
#ifdef ALTO_DEBUG
		printf("URI_convert: Not fetch the Vid from URI\n");
#endif
		return 0;
	}
	memcpy(vid, uri_tmp, index);
	vid[index] = '\0';
	if((strlen(vid) == 3 && (!strcmp(vid, "5ca"))) || (strlen(vid) == 4 && (!strcmp(vid, "05ca"))))
	{
		uri_tmp += (index + 1);
		index = search_char_idx (uri_tmp, '_');
		if(index == 0)
		{
#ifdef ALTO_DEBUG
			printf("URI_convert: Not fetch the Pid from URI\n");
#endif
			return 0;
		}
		memcpy(pid, uri_tmp, index);
		pid[index] = '\0';
		
		memset(uri_con, 0, sizeof(uri_con));
		strcpy(uri_con, "usb://RICOH/SP%20");
		if((strlen(pid) == 3 && (!strcmp(pid, "454"))) || (strlen(pid) == 4 && (!strcmp(pid, "0454"))))
			strcat(uri_con, "150SU");
		else if((strlen(pid) == 3 && (!strcmp(pid, "456"))) || (strlen(pid) == 4 && (!strcmp(pid, "0456"))))
			strcat(uri_con, "150SUw");
		else if((strlen(pid) == 3 && (!strcmp(pid, "455"))) || (strlen(pid) == 4 && (!strcmp(pid, "0455"))))
			strcat(uri_con, "150w");
                // for support  151SUw and 151w(Ricom AirPrint)
                else if((strlen(pid) == 3 && (!strncasecmp(pid, "45c", 3))) || (strlen(pid) == 4 && (!strncasecmp(pid, "045c", 4))))
                        strcat(uri_con, "151SUw");
                else if((strlen(pid) == 3 && (!strncasecmp(pid, "45d", 3))) || (strlen(pid) == 4 && (!strncasecmp(pid, "045d", 4))))
                        strcat(uri_con, "151w");
		else
		{
#ifdef ALTO_DEBUG
			printf("URI_convert: Fetch the Pid[%s] from URI is invalid\n", pid);
#endif
			return 0;
		}

		uri_tmp += (index + 1);
		index = search_char_idx (uri_tmp, '_');
		if(index == 0)
		{
#ifdef ALTO_DEBUG
			printf("URI_convert: Not fetch the serial from URI\n");
#endif
			//return 0;
		}
		memcpy(serial, uri_tmp, index);
		serial[index] = '\0';

		if(!check_number(serial))
		{
#ifdef ALTO_DEBUG
			printf("URI_convert: Fetch the serial from URI is invalid\n");
#endif
		}
		else
		{
			uri_tmp += (index + 1);
			index = search_char_idx (uri_tmp, '_');
			if(index == 0)
			{
#ifdef ALTO_DEBUG
					printf("URI_convert: Not fetch the interface from URI\n");
#endif
					//return 0;
			}
			//char tmp[index +1];
			memcpy(if_str, uri_tmp, index);
			inf = fetch_interface_value(if_str, if_str);
			if(inf >= 0)
			{
				strcat(uri_con, "?serial=");
				strcat(uri_con, serial);
				strcat(uri_con, "&interface=");
				strcat(uri_con, if_str);
			}
		}
		memcpy(new_uri, uri_con, strlen(uri_con));
		new_uri[strlen(uri_con)] = '\0';
	}
	else
	{
#ifdef ALTO_DEBUG
		printf("URI_convert: Fetch the Vid[%s] from URI is invalid\n", vid);
#endif	
		return 0;
	}
        return 1;
}

static char *
alto_libusb_strerror (int errcode)
{
  /* Error codes & descriptions from the libusb-1.0 documentation */

  switch (errcode)
    {
      case LIBUSB_SUCCESS:
	return "Success (no error)";

      case LIBUSB_ERROR_IO:
	return "Input/output error";

      case LIBUSB_ERROR_INVALID_PARAM:
	return "Invalid parameter";

      case LIBUSB_ERROR_ACCESS:
	return "Access denied (insufficient permissions)";

      case LIBUSB_ERROR_NO_DEVICE:
	return "No such device (it may have been disconnected)";

      case LIBUSB_ERROR_NOT_FOUND:
	return "Entity not found";

      case LIBUSB_ERROR_BUSY:
	return "Resource busy";

      case LIBUSB_ERROR_TIMEOUT:
	return "Operation timed out";

      case LIBUSB_ERROR_OVERFLOW:
	return "Overflow";

      case LIBUSB_ERROR_PIPE:
	return "Pipe error";

      case LIBUSB_ERROR_INTERRUPTED:
	return "System call interrupted (perhaps due to signal)";

      case LIBUSB_ERROR_NO_MEM:
	return "Insufficient memory";

      case LIBUSB_ERROR_NOT_SUPPORTED:
	return "Operation not supported or unimplemented on this platform";

      case LIBUSB_ERROR_OTHER:
	return "Other error";

      default:
	return "Unknown libusb-1.0 error code";
    }

  return "Unknown libusb-1.0 error code";
}

/*O - 0: free, 1 : busy*/
int get_device_status(int busno, int dev_addr)
{
    if(busno == libusb_get_bus_number(g_device) && dev_addr == libusb_get_device_address(g_device))
    {
//        printf ("get_device_status: Device is busy\n");
        return 1;
    }
//    printf ("get_device_status: Device is free\n");
    return 0;
}

int /* O - 0 on success, -1 on error */
get_device_id(char *buffer, /* I - String buffer */
size_t bufsize) /* I - Number of bytes in buffer */
{

	int config;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config0;
    int result;

    result = libusb_get_configuration (g_dev_h, &config);
    if (result < 0)
	{
		printf ("get_device_id: Could not get configuration for device (err %d)\n", result);
		return 0;
	}

      if (config == 0)
	{
		printf ("get_device_id: Device not configured?\n");
		return 0;
	}

	result = libusb_get_device_descriptor (g_device, &desc);
	if (result < 0)
	{
		printf ("get_device_id: Could not get device descriptor for device  (err %d)\n", result);
		return 0;
	}

	result = libusb_get_config_descriptor (g_device, 0, &config0);
	if (result < 0)
	{
		printf ("get_device_id: Could not get config[0] descriptor for device (err %d)\n", result);
		return 0;
	}

      /* Set the configuration */
	if (desc.bNumConfigurations > 1)
	{
#ifdef ALTO_DEBUG
		printf ("get_device_id: More than one configuration (%d), choosing first config (%d)\n", desc.bNumConfigurations, config0->bConfigurationValue);
#endif
	}
	result = libusb_set_configuration (g_dev_h, config0->bConfigurationValue);

	libusb_free_config_descriptor (config0);

	if (result < 0)
	{
		int status = 0;
#ifdef ALTO_DEBUG
	  	printf ("get_device_id: libusb complained: %s\n", alto_libusb_strerror (result));
#endif
		if (result == LIBUSB_ERROR_ACCESS)
	    {
			printf ("get_device_id: Make sure you run as root or set appropriate permissions\n");
	      	return status = 0;
	    }
	  	else if (result == LIBUSB_ERROR_BUSY)
	    {
#ifdef ALTO_DEBUG
	    	printf ("get_device_id: Maybe the kernel scanner driver or usblp claims the interface? Ignoring this error...\n");
#endif
	    }

	}

	/* Check whether a kernel driver is attached to interface #0. If so, we'll
	 * need to detach it.
	 */

	int ret = libusb_kernel_driver_active(g_dev_h, g_interface);

	if (ret == 1) { //un-mount usbhid driver
					//HID會被Linux掛上usbhid的driver造成我們無法存取，先把它detach掉
		ret = libusb_detach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
		if(!ret)
			printf("get_device_id: usb_detach_kernel_driver_np : success\n");
#endif
	}
#ifdef ALTO_DEBUG
	else
	{
		printf("get_device_id: Nokernel driver is active \n");
	}
#endif


    /* Claim the interface */
    result = libusb_claim_interface (g_dev_h, g_interface);
    if (result < 0)
	{
	  int status = 0;

	  printf ("get_device_id: libusb complained: %s\n", alto_libusb_strerror (result));
	  if (result == LIBUSB_ERROR_ACCESS)
	  {
	      printf ("get_device_id: Make sure you run as root or set appropriate permissions\n");
	      status = 0;
	  }
	  else if (result == LIBUSB_ERROR_BUSY)
	  {
	      printf ("get_device_id: Maybe the kernel scanner driver claims the scanner's interface?\n");
	      status = 0;
	   }

	  return status;
	}

	if (libusb_control_transfer(g_dev_h, (0x01 << 5) | 0x80 | 0x01, 0x00, 0x01,
			(g_interface ==1? ((1 << 8) | 0) : 0), (unsigned char *) buffer, bufsize, 5000) < 0) 
	{
		printf("get_device_id: get 1284 fail\n");
		*buffer = '\0';

		result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
		if (result != 0) {
			libusb_attach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
			printf("get_device_id: Cannot Release Interface: %d \"%s\" \n", result, strerror(-result));
#endif
			return 0;
		}

	}


	int length; /* Length of device ID */
	/*
	 * Extract the length of the device ID string from the first two
	 * bytes.  The 1284 spec says the length is stored MSB first...
	 */

	length = (((unsigned) buffer[0] & 255) << 8) + ((unsigned) buffer[1] & 255);

	/*
	 * Check to see if the length is larger than our buffer; first
	 * assume that the vendor incorrectly implemented the 1284 spec,
	 * and then limit the length to the size of our buffer...
	 */

	if (length > (bufsize - 2))
		length = (((unsigned) buffer[1] & 255) << 8)
				+ ((unsigned) buffer[0] & 255);

	if (length > (bufsize - 2))
		length = bufsize - 2;

	/*
	 * Copy the device ID text to the beginning of the buffer and
	 * nul-terminate.
	 */

	memmove(buffer, buffer + 2, length);
        buffer[length] = 0;

	result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
	if (result != 0) {
		libusb_attach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
		printf("Cannot Release Interface: %d \"%s\" \n", result, strerror(-result));
#endif
		return 0;
	}//*/
	libusb_attach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
			printf("get_device_id: (%d): %s\n", length, buffer);
#endif	
	return 1;
}

int /* O - 0 on success, -1 on error */
USBRead(char *buffer, /* I - String buffer */
size_t bufsize) /* I - Number of bytes in buffer */
{

	int config;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config0;
    int result;

    result = libusb_get_configuration (g_dev_h, &config);
    if (result < 0)
	{
		printf ("USBRead: Could not get configuration for device (err %d)\n", result);
		return 0;
	}

      if (config == 0)
	{
		printf ("USBRead: Device not configured?\n");
		return 0;
	}

	result = libusb_get_device_descriptor (g_device, &desc);
	if (result < 0)
	{
		printf ("USBRead: Could not get device descriptor for device  (err %d)\n", result);
		return 0;
	}

	result = libusb_get_config_descriptor (g_device, 0, &config0);
	if (result < 0)
	{
		printf ("USBRead: Could not get config[0] descriptor for device (err %d)\n", result);
		return 0;
	}

      /* Set the configuration */
	if (desc.bNumConfigurations > 1)
	{
#ifdef ALTO_DEBUG
		printf ("USBRead: More than one configuration (%d), choosing first config (%d)\n", desc.bNumConfigurations, config0->bConfigurationValue);
#endif
	}
	result = libusb_set_configuration (g_dev_h, config0->bConfigurationValue);

	libusb_free_config_descriptor (config0);

	if (result < 0)
	{
		int status = 0;
#ifdef ALTO_DEBUG
	  	printf ("USBRead: libusb complained: %s\n", alto_libusb_strerror (result));
#endif
		if (result == LIBUSB_ERROR_ACCESS)
	    {
			printf ("USBRead: Make sure you run as root or set appropriate permissions\n");
	      	return status = 0;
	    }
	  	else if (result == LIBUSB_ERROR_BUSY)
	    {
#ifdef ALTO_DEBUG
	    	printf ("USBRead: Maybe the kernel scanner driver or usblp claims the interface? Ignoring this error...\n");
#endif
	    }

	}

	/* Check whether a kernel driver is attached to interface #0. If so, we'll
	 * need to detach it.
	 */

	int ret = libusb_kernel_driver_active(g_dev_h, g_interface);

	if (ret == 1) { //un-mount usbhid driver
					//HID會被Linux掛上usbhid的driver造成我們無法存取，先把它detach掉
		ret = libusb_detach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
		if(!ret)
			printf("USBRead: usb_detach_kernel_driver_np : success\n");
#endif
	}
#ifdef ALTO_DEBUG
	else
	{
		printf("USBRead: No	kernel driver is active \n");
	}

#endif

    /* Claim the interface */
    result = libusb_claim_interface (g_dev_h, g_interface);
    if (result < 0)
	{
	  int status = 0;

	  printf ("USBRead: libusb complained: %s\n", alto_libusb_strerror (result));
	  if (result == LIBUSB_ERROR_ACCESS)
	  {
	      printf ("USBRead: Make sure you run as root or set appropriate permissions\n");
	      status = 0;
	  }
	  else if (result == LIBUSB_ERROR_BUSY)
	  {
	      printf ("USBRead: Maybe the kernel scanner driver claims the scanner's interface?\n");
	      status = 0;
	   }

	  return status;
	}


	int doneByte; //for return values


	doneByte = libusb_control_transfer(g_dev_h, 0xc1, 0x00, 0x00, (g_interface ==1? 0x0100 : 0),
			(unsigned char *) buffer, bufsize, 5000);

	if (result < 0) {
		printf("USBRead: Control read failed\n");
		*buffer = '\0';

		result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
		if (result != 0) {
#ifdef ALTO_DEBUG
			printf("USBRead: Cannot Release Interface %d \"%s\" \n", result, strerror(-result));
#endif
			libusb_attach_kernel_driver(g_dev_h, g_interface);
			return -1;
		}

		return doneByte;
	}


	result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
	if (result != 0) {
#ifdef ALTO_DEBUG
		printf("USBRead: Cannot Release Interface : %d \"%s\" \n", result, strerror(-result));
#endif
		libusb_attach_kernel_driver(g_dev_h, g_interface);
		return -1;
	}

	libusb_attach_kernel_driver(g_dev_h, g_interface);
	return doneByte;
}

int /* O - 0 on success, -1 on error */
USBWrite(char *buffer, /* I - String buffer */
size_t bufsize)/* I - Number of bytes in buffer */
{

	int config;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config0;
    int result;

    result = libusb_get_configuration (g_dev_h, &config);
    if (result < 0)
	{
		printf ("USBWrite: Could not get configuration for device (err %d)\n", result);
		return 0;
	}

      if (config == 0)
	{
		printf ("USBWrite: Device not configured?\n");
		return 0;
	}

	result = libusb_get_device_descriptor (g_device, &desc);
	if (result < 0)
	{
		printf ("USBWrite: Could not get device descriptor for device  (err %d)\n", result);
		return 0;
	}

	result = libusb_get_config_descriptor (g_device, 0, &config0);
	if (result < 0)
	{
		printf ("USBWrite: Could not get config[0] descriptor for device (err %d)\n", result);
		return 0;
	}

      /* Set the configuration */
	if (desc.bNumConfigurations > 1)
	{
#ifdef ALTO_DEBUG
		printf ("USBWrite: More than one configuration (%d), choosing first config (%d)\n", desc.bNumConfigurations, config0->bConfigurationValue);
#endif
	}
	result = libusb_set_configuration (g_dev_h, config0->bConfigurationValue);

	libusb_free_config_descriptor (config0);

	if (result < 0)
	{
		int status = 0;
#ifdef ALTO_DEBUG
	  	printf ("USBWrite: libusb complained: %s\n", alto_libusb_strerror (result));
#endif
		if (result == LIBUSB_ERROR_ACCESS)
	    {
			printf ("USBWrite: Make sure you run as root or set appropriate permissions\n");
	      	return status = 0;
	    }
	  	else if (result == LIBUSB_ERROR_BUSY)
	    {
#ifdef ALTO_DEBUG
	    	printf ("USBWrite: Maybe the kernel scanner driver or usblp claims the interface? Ignoring this error...\n");
#endif
	    }

	}


	/* Check whether a kernel driver is attached to interface #0. If so, we'll
	 * need to detach it.
	 */

	int ret = libusb_kernel_driver_active(g_dev_h, g_interface);

	if (ret == 1) { //un-mount usbhid driver
					//HID會被Linux掛上usbhid的driver造成我們無法存取，先把它detach掉
		ret = libusb_detach_kernel_driver(g_dev_h, g_interface);
#ifdef ALTO_DEBUG
		if(!ret)
			printf("USBWrite: usb_detach_kernel_driver_np : success\n");
#endif
	}
#ifdef ALTO_DEBUG
	else
	{
		printf("USBWrite: No kernel driver is active \n");
	}

#endif

    /* Claim the interface */
    result = libusb_claim_interface (g_dev_h, g_interface);
    if (result < 0)
	{
	  int status = 0;

	  printf ("USBWrite: libusb complained: %s\n", alto_libusb_strerror (result));
	  if (result == LIBUSB_ERROR_ACCESS)
	  {
	      printf ("USBWrite: Make sure you run as root or set appropriate permissions\n");
	      status = 0;
	  }
	  else if (result == LIBUSB_ERROR_BUSY)
	  {
	      printf ("USBWrite: Maybe the kernel scanner driver claims the scanner's interface?\n");
	      status = 0;
	   }

	  return status;
	}

	int doneByte; //for return values


	doneByte = libusb_control_transfer(g_dev_h, 0x41, 0x4D, 0x3C2B, (g_interface ==1? 0x0100 : 0),
			(unsigned char *) buffer, bufsize, 5000);

	if (doneByte < 0) {
		printf("USBWrite: Control write failed\n");
		*buffer = '\0';

		result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
		if (result != 0) {
#ifdef ALTO_DEBUG
			printf("USBWrite: Cannot Release Interface: %d \"%s\" \n", result, strerror(-result));
#endif
			libusb_attach_kernel_driver(g_dev_h, g_interface);
			return -1;
		}

		return doneByte;
	}

	result = libusb_release_interface(g_dev_h, g_interface); //release the claimed interface
	if (result != 0) {
#ifdef ALTO_DEBUG
		printf("USBWrite: Cannot Release Interface: %d \"%s\" \n", result, strerror(-result));
#endif
		libusb_attach_kernel_driver(g_dev_h, g_interface);
		return -1;
	}
	libusb_attach_kernel_driver(g_dev_h, g_interface);
	return doneByte;
}

int print_device(libusb_device *dev, int level, char *host, char *resource) 
{
	libusb_device_handle *udev;
	struct libusb_device_descriptor desc;
//	struct libusb_config_descriptor *config0;
	unsigned short vid, pid;
	unsigned char busno, address;
//	int config;
	char open_dev_flag = 0;

//	unsigned char description[256];
	char string[256];
	int ret;
	char devhost[1024], devProduct[256], devserialNumber[1024];

#ifdef ALTO_DEBUG
		printf ("print_device: start\n");
#endif	
    ret = libusb_open (dev, &udev);
    if (ret < 0)
	{
#ifdef ALTO_DEBUG
		printf ("print_device: skipping device 0x%04x/0x%04x at %03d:%03d: cannot open: %s\n",
	       vid, pid, busno, address, alto_libusb_strerror (ret));
#endif		
		open_dev_flag = 0;
		g_open_flag = 0;
		return 0;
	}
	else 
	{
		open_dev_flag = 1;
		ret = libusb_get_device_descriptor (dev, &desc);
		if (ret < 0)
		{
#ifdef ALTO_DEBUG 
	  		printf ("print_device: could not get device descriptor for device at %03d:%03d (err %d)\n", busno, address, ret);
#endif
			libusb_close (udev);
			g_open_flag = 0;
			return 0;
		}
		else
		{
			if (desc.iManufacturer) 
			{
	
				ret = libusb_get_string_descriptor_ascii(udev, desc.iManufacturer, (unsigned char *)string, sizeof(string));	
				if (ret > 0)
				{	
					snprintf(devhost, sizeof(devhost), "%s", string);
				}
				else
				{	
					snprintf(devhost, sizeof(devhost), "%04X", desc.idVendor);
				}
			} 
			else
			{	
				snprintf(devhost, sizeof(devhost), "%04X", desc.idVendor);
			}
			if (desc.iProduct) 
			{
				ret = libusb_get_string_descriptor_ascii(udev, desc.iProduct, (unsigned char *)string, sizeof(string));
				if (ret > 0)
				{
					snprintf(devProduct, sizeof(devProduct), "%s", string);
				}
				else
				{
					snprintf(devProduct, sizeof(devProduct), "%04X", desc.iProduct);
				}
			}
			else
			{
				snprintf(devProduct, sizeof(devProduct), "%04X", desc.iProduct);

			}
			if (desc.iSerialNumber)
			{

				ret = libusb_get_string_descriptor_ascii(udev, desc.iSerialNumber, (unsigned char *)devserialNumber, sizeof(devserialNumber));
				if (ret > 0)
					printf("%.*s  - Serial Number: %s\n", level * 2,
						"                    ", devserialNumber);
			}
			printf(" devhost [%s] devProduct [%s] devserialNumber [%s] \n", devhost, devProduct, devserialNumber);
		}
	}


	if (strstr(resource, "serial") == NULL || strlen(devserialNumber) == 0) {
		if ((strcmp(devhost, host) == 0 && strstr(resource, devProduct) != NULL)
				|| ((strcasecmp(devhost, devVID) == 0
						&& strcasecmp(devProduct, devPID) == 0))) {
			//libusb_set_debug (usb_ctx, 3);;
			g_dev_h = udev;
			g_device = dev;

			if(strstr(resource, "SP 150SU") != NULL)
			{
				g_interface = 1;
			}
			else if(strstr(resource, "SP 150") != NULL)
			{
				g_interface = 0;
			}
                        else if(strstr(resource, "SP 151SU") != NULL)
                        {
                                g_interface = 1;
                        }
                        else if(strstr(resource, "SP 151") != NULL)
                        {
                                g_interface = 0;
                        }
			else
			{
				g_interface = 1;
			}
			g_open_flag = 1;
#ifdef ALTO_DEBUG
			printf("%s:interface=%d\n", resource, g_interface);
#endif
			return 1;
		}

	} else {

		if ((strcmp(devhost, host) == 0 && strstr(resource, devProduct) != NULL
				&& strstr(resource, devserialNumber) != NULL)
				|| (strcasecmp(devhost, devVID) == 0
						&& strcasecmp(devProduct, devPID) == 0
						&& strstr(resource, devserialNumber) != NULL)) {
			//libusb_set_debug (usb_ctx, 3); 
			g_dev_h = udev;
			g_device = dev;

                        if(strstr(resource, "SP 150SU") != NULL)
                        {
                                g_interface = 1;
                        }
                        else if(strstr(resource, "SP 150") != NULL)
                        {
                                g_interface = 0;
                        }
                        else if(strstr(resource, "SP 151SU") != NULL)
                        {
                                g_interface = 1;
                        }
                        else if(strstr(resource, "SP 151") != NULL)
                        {
                                g_interface = 0;
                        }
                        else
                        {
                                g_interface = 1;
                        }
#ifdef ALTO_DEBUG
                        printf("interface=%d\n", g_interface);
#endif
			g_open_flag = 1;
			return 1;
		}
	}

	if (open_dev_flag)
	{
		libusb_close (udev);
		open_dev_flag = 0;
	}
	g_open_flag = 0;
	return 0;

}

int openPrinter(char* printerURI)
{
	int rtn = 0;

	if (printerURI == NULL)
		return rtn;
#ifdef ALTO_DEBUG
	printf("openPrinter\n");
	printf("URI:%s\n", printerURI);
#endif
	char URI[1024];
	memset(URI, 0, sizeof(URI));
	if(strstr(printerURI, "hal://") != NULL)
	{
		if(!URI_convert(printerURI, URI))
		{
			return 0;
		}
	}
	else
	{
		memcpy(URI, printerURI, strlen(printerURI));
	}

	char scheme[32], username[256], host[1024], resource[1024];
	int port;
/*	httpSeparate2(printerURI, scheme, sizeof(scheme), username,
			sizeof(username), host, sizeof(host), &port, resource,
			sizeof(resource));*/
	httpSeparate2(URI, scheme, sizeof(scheme), username,
			sizeof(username), host, sizeof(host), &port, resource,
			sizeof(resource));
	if (strcasecmp(scheme, "usb") != 0)
		return 0;

	printf("host:%s\nresource:%s\n", host, resource);

	libusb_device **devlist;
	ssize_t ndev;

	libusb_device *dev;
//	libusb_device_handle *hdl;
	struct libusb_device_descriptor desc;
//	struct libusb_config_descriptor *config0;
	unsigned short vid, pid;
	unsigned char busno, address;
//	int config;
//	int interface;
	int ret, i;

	if (!usb_ctx)
    {
#ifdef ALTO_DEBUG
		printf("openPrinter: initializing libusb-1.0\n");
#endif
		ret = libusb_init (&usb_ctx);
		if (ret < 0)
		{
			printf("openPrinter: failed to initialize libusb-1.0, error %d\n", ret);
			return 0;
		}

		//libusb_set_debug (usb_ctx, 3);

	}
#ifdef ALTO_DEBUG
	printf ("openPrinter: Looking for libusb-1.0 devices\n");
#endif
	ndev = libusb_get_device_list (usb_ctx, &devlist);
	if (ndev < 0)
    {
		printf ("openPrinter: failed to get libusb-1.0 device list, error %d\n", (int) ndev);
		return 0;
	}
#ifdef ALTO_DEBUG
	printf ("openPrinter: Looking for %d devices\n", ndev);
#endif
	for (i = 0; i < ndev && rtn == 0; i++)
    {
//		char found = 0;

		dev = devlist[i];

		busno = libusb_get_bus_number (dev);
		address = libusb_get_device_address (dev);

		ret = libusb_get_device_descriptor (dev, &desc);
		if (ret < 0)
		{
#ifdef ALTO_DEBUG
			printf ("openPrinter: could not get device descriptor for device at %03d:%03d (err %d)\n", busno, address, ret);
#endif
			continue;
		}

		vid = desc.idVendor;
		pid = desc.idProduct;

		if ((vid != 0x05ca))
		{
#ifdef ALTO_DEBUG
			printf ("openPrinter: device 0x%04x/0x%04x at %03d:%03d is not RICOH\n", vid, pid, busno, address);
#endif
	  		continue;
		}

		rtn = print_device(dev, 0, host, resource);

     }

 	return rtn;
}

void closePrinter(void) {

	if (g_open_flag) 
	{
#ifdef ALTO_DEBUG
		printf("closePrinter\n");
#endif
		libusb_release_interface(g_dev_h, g_interface);
//		libusb_attach_kernel_driver(g_dev_h, g_interface);

		//usb_reset(g_dev);
		libusb_close (g_dev_h);
		g_dev_h = NULL;
	}
	return;
}

int main(int argc, char *argv[]) {
	int rtn = 0;
	char * printerURI = "usb://RICOH/SP%20150SUw?serial=987654321&interface=1";

	char scheme[32], username[255], host[2048], resource[2048];
	int port;
	httpSeparate2(printerURI, scheme, sizeof(scheme), username,
			sizeof(username), host, sizeof(host), &port, resource,
			sizeof(resource));
	if (strcasecmp(scheme, "usb") != 0)
		return rtn;

	printf("uri:%s\n", printerURI);
	printf("host:%s\nresource:%s\n", host, resource);

	libusb_device **devlist;
	ssize_t ndev;

	libusb_device *dev;
//	libusb_device_handle *hdl;

	int ret, i;

	if (!usb_ctx)
    {
		printf("Initializing libusb-1.0\n");
		ret = libusb_init (&usb_ctx);
		if (ret < 0)
		{
			printf("Failed to initialize libusb-1.0, error %d\n", ret);
			return 0;
		}

		//libusb_set_debug (usb_ctx, 3);

	}

	printf ("Looking for libusb-1.0 devices\n");

	ndev = libusb_get_device_list (usb_ctx, &devlist);
	if (ndev < 0)
    {
		printf ("Failed to get libusb-1.0 device list, error %d\n", (int) ndev);
		return 0;
	}

	for (i = 0; i < ndev && rtn == 0; i++)
    {
//		char found = 0;

		dev = devlist[i];

		rtn = print_device(dev, 0, host, resource);
     }

	closePrinter();
	return rtn;
}

