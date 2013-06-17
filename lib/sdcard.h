#ifndef _sdcard_header_
#define _sdcard_hader_

//Macros for setting, clearing and toogleing bits.
#define SET_BIT(PORT, BITNUM) ((PORT) |= (1<<(BITNUM)))
#define CLEAR_BIT(PORT, BITNUM) ((PORT) &= ~(1<<(BITNUM)))
#define TOGGLE_BIT(PORT, BITNUM) ((PORT) ^= (1<<(BITNUM)))

int init_sdcard( uint8_t drive );
int sdcard_open( char * filename );
void sdcard_close(void);
int sd_check_file( char * filename );
int gpx_write_progmem( PGM_P loc );
int sd_new_pathfile( char * filename );

#endif
