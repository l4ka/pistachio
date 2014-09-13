/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/



#include <l4io.h>

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: USB drive control */
//#include "atadrive.h"	/* Example: ATA drive control */
//#include "sdcard.h"		/* Example: MMC/SDC contorl */

#include "ff.h"

#include <mindrvr.h> //ATA driver

/* Definitions of physical drive number for each media */
#define ATA		0
#define MMC		1
#define USB		2

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

	printf("[diskio] : Inside disk_initialize(%x)\n", pdrv);

	switch (pdrv) {
	case ATA :
		//result = ATA_disk_initialize();

		result = reg_reset(0);
		printf("[diskio] : Found %d PATA devices\n", reg_config());

		// translate the reslut code here
		printf("[diskio] : PATA device 0 reset status: %d \n", result);

		return stat;

	//case MMC :
		//result = MMC_disk_initialize();

		// translate the reslut code here

		//return stat;

	//case USB :
		//result = USB_disk_initialize();

		// translate the reslut code here

		//return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

	printf("[diskio] : Inside disk_status(%x)\n", pdrv);

	switch (pdrv) {
	case ATA :
		//result = ATA_disk_status();
		result = int_ata_status; //Porbably doesn't work
		// translate the reslut code here

		return stat;

	case MMC :
		//result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case USB :
		//result = USB_disk_status();

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	int result;

printf("\n[diskio] : Inside disk_read(%d, BUFF, %d, %d)\n", pdrv, sector, count);

	switch (pdrv) {
	case ATA :
		// translate the arguments here
/* 
extern int reg_pio_data_in_lba28( unsigned char dev, unsigned char cmd,
                                  unsigned int fr, unsigned int sc,
                                  unsigned long lba,
                                  unsigned char * bufAddr,
                                  long numSect, int multiCnt );

Intuition says that this should be:
reg_pio_data_in_lba28(CB_DH_DEV0,CMD_READ_SECTORS, 0x00, count, sector, buff, count, count);
*/

		//result = ATA_disk_read(buff, sector, count);
		result = reg_pio_data_in_lba28(CB_DH_DEV0,CMD_READ_SECTORS, 0x00, count, sector, buff, count, count);
		printf("\n\n[diskio] : Buffer contained : %s\n\n", buff);
		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here

		//result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

		//result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		//result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	//case MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		//return res;

	//case USB :
		// translate the arguments here

		//result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		//return res;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
 DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// pre-process here

		//result = ATA_disk_ioctl(cmd, buff);

		// post-process here

		return res;

	//case MMC :
		// pre-process here

		//result = MMC_disk_ioctl(cmd, buff);

		// post-process here

		//return res;

	//case USB :
		// pre-process here

		//result = USB_disk_ioctl(cmd, buff)

		// post-process here

		//return res;
	}
	return RES_PARERR;
}
#endif


#ifdef __cplusplus
}
#endif
