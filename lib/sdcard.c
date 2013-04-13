#include "sdcard.h"

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
	if ( status == FR_OK ) sdcard_close();
	return status; //return the result of opening the file
}



/**
* Name : gpx_write_header
*
* Description: write header portion of KML file
*
* Author(s): Charles Wolf
*
* @param: filename - name of file to write header to
*
* @return: none
**/

int gpx_write_header( char * filename )
{
		char header_a[] = "<?xml version=\"1.0\"?>\n<gpx version = \"0.6\" creator = \"GPS WOLF\">\n<trk>\n<name>Path</name>\n<number>1</number>\n<trkseg>\n";
		f_write(&logFile, &header_a[0], strlen(header_a), &bytesWritten);
		return 1;

}




/**
* Name : kml_write_header (inactive)
*
* Description: write header portion of KML file
*
* Author(s): Charles Wolf
*
* @param: filename - name of file to write header to
*
* @return: none


int kml_write_header( char * filename )
{
		char header_a[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://www.opengis.net/kml/2.2\">";
		char header_b[] = "<Document><Style id=\"l_c\"><LineStyle><color>7f00ffff</color><width>3</width></LineStyle></Style>";
		char header_c[] = "<Placemark><styleUrl>#l_c</styleUrl><LineString><coordinates>";
		f_write(&logFile, &header_a[0], strlen(header_a), &bytesWritten);
		f_write(&logFile, &header_b[0], strlen(header_b), &bytesWritten);
		f_write(&logFile, &header_c[0], strlen(header_c), &bytesWritten);
		f_write(&logFile, "\n", 1, &bytesWritten);
		return 1;

}
**/


/**
* Name : gpx_write_footer
*
* Description: write header portion of KML file
*
* Author(s): Charles Wolf
*
* @param: filename - name of file to write footer to
*
* @return: none
**/

int gpx_write_footer( char * filename )
{
	char footer[] = "</trkseg></trk></gpx>";
	f_write(&logFile, &footer[0], strlen(footer), &bytesWritten);
	return 1;
}



/**
* Name : kml_write_footer  (inactive)
*
* Description: write header portion of KML file
*
* Author(s): Charles Wolf
*
* @param: filename - name of file to write footer to
*
* @return: none


int kml_write_footer( char * filename )
{
	char footer[] = "</coordinates></LineString></Placemark></Document></kml>";
	f_write(&logFile, &footer[0], strlen(footer), &bytesWritten);
	return 1;
}
**/



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
	while ( sd_check_file( filename ) == FR_OK )
	{
		i++;
		sprintf( filename, "/path%d.gpx", i );
	}
	//create next path file
	sdcard_open( filename );
	gpx_write_header( filename );
	gpx_write_footer( filename );
	f_close(&logFile);
	return i;	
}
