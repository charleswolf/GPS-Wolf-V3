#include "sdcard.h"
#include <string.h>
#include <avr/pgmspace.h> 


#include <avr/pgmspace.h> 
 
const char header_a[] PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gpx creator=\"GPSWOLF\" version=\"1.1\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\" xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\">\n<trk>\n<name>Path</name>\n<number>1</number>\n<trkseg>\n";
const char footer_a[] PROGMEM = "</trkseg></trk></gpx>";
const char header_b[] PROGMEM = "<extensions>\n<gpxtpx:TrackPointExtension>\n<gpxtpx:hr>";
const char footer_b[] PROGMEM = "</gpxtpx:hr>\n</gpxtpx:TrackPointExtension>\n</extensions>";

/**CHANGE LOG
 * -----------------------
 * 10-13-2011 changed from .kml to .gpx xml filetype
 * -----------------------
 * 7-19-2011 added function for creating new pathfile 
 * -----------------------
 * 7-17-2011 updated functions to accept filename
 * -----------------------  
 * INITIAL RELEASE
 * -----------------------
*/



/**
* Name : init_sdcard
*
* Description: initialize the SD card and mount the filesystem
*
* Author(s): Charles Wolf
*
* @param: uint8_t drive - which drive to initialize ( usually 0 )
*
* @return: int 
**/
int init_sdcard( uint8_t drive )
{
	int status = 1; // 1 == no errors
	DSTATUS driveStatus = disk_initialize(drive);
	if ((driveStatus & STA_NODISK) || (driveStatus & STA_NOINIT)) //check for initialization errors
	{
		status = 0; //error initialization failed
	}
	else{ //only mount filesystem if initialization was sucessful
		if( f_mount(drive, &FileSystemObject) != FR_OK ) 
		{
			//flag error
			status = 0; //error mounting failed
		}
	}
	return status; 
}


/**
* Name : sdcard_open
*
* Description: open a file on the sd card
*
* Author(s): Charles Wolf
*
* @param: char * filename - name of file to open
*
* @return: int 
**/
int sdcard_open( char * filename )
{
	if(f_open(&logFile, filename , FA_READ | FA_WRITE | FA_OPEN_ALWAYS)!= FR_OK) {
		//flag error
		return 0;
	}
	else return 1;
}

/**
* Name : sdcard_close
*
* Description: open a file on the sd card
*
* Author(s): Charles Wolf
*
* @param: none
*
* @return: none
**/
void sdcard_close()
{
	//close file
	f_close(&logFile);
}


/**
* Name : sd_check_file
*
* Description: try opening a file to check if it exists.  function can 
* 	also be used to check if sd card is present
*
* Author(s): Charles Wolf
*
* @param: none
*
* @return:	function returns an int with the returned value from fopen()
**/
int sd_check_file( char * filename )
{
	int status = 0;
	//try to open an existing file for reading
	status =  f_open(&logFile, filename , FA_OPEN_EXISTING | FA_READ );
	//if sucessful, close the file.  
	if ( status == FR_OK ) f_close(&logFile);
	return status; //return the result of opening the file
}



/**
* Name : gpx_write_progmem
*
* Description: write to a GPX file from a location in program memory
*
* Author(s): Charles Wolf
*
* @param: loc - location in program memory
*
* @return: none
**/

int gpx_write_progmem( PGM_P loc )
{
    char c;
    while ((c = pgm_read_byte(loc++)) != 0) 
    {
		f_putc (c, &logFile);
		//f_write(&logFile, &c, 1, &bytesWritten);
	}
	return 1;

}



/**
* Name : sd_new_pathfile
*
* Description: examine the files on the SD card to determine the new 
* 	path name.  File path names be GPS_PATH_#.gpx  where # is an intiger
*
* Author(s): Charles Wolf
*
* @param: filename - a characterstring large enough to contain the new
* 	path file name.
*
* @return:	function returns an int with the number of pathfiles.  The
*	created file path name is returned in the pointer "filename"
**/
int sd_new_pathfile( char * filename )
{
	int i = 0;
	
	sprintf( filename, "/path%d.gpx",  i );
	//determine the next path file by checking for old path files
	while ( f_open(&logFile, filename , FA_OPEN_EXISTING | FA_READ ) == FR_OK )
	{
		f_close(&logFile);
		i++;
		sprintf( filename, "/path%d.gpx", i );
	}
	//create next path file
	sdcard_open( filename );
	gpx_write_progmem(header_a );
	f_sync(&logFile); //sync buffer to file
	gpx_write_progmem(footer_a );
	f_close(&logFile);
	return i;	
}

