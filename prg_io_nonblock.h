/*
 * Filename: prg_io_nonblock.h
 * Date:     2019/12/25 14:20
 * Author:   Jan Faigl
 */

#ifndef __PRG_IO_NONBLOCK_H__
#define __PRG_IO_NONBLOCK_H__

/// ----------------------------------------------------------------------------
/// @brief io_open_read
/// 
/// @param fname  -- device name
/// 
/// @return 
/// ----------------------------------------------------------------------------
int io_open_read(const char *fname);

/// ----------------------------------------------------------------------------
/// @brief io_open_write
/// 
/// @param fname  -- device name
/// 
/// @return 
/// ----------------------------------------------------------------------------
int io_open_write(const char *fname);


int io_open_read(const char *fname);

/// ----------------------------------------------------------------------------
/// @brief io_close
/// 
/// @param fd 
/// 
/// @return 
/// ----------------------------------------------------------------------------
int io_close(int fd);

/// ----------------------------------------------------------------------------
/// @brief io_putc
/// 
/// @param fd 
/// @param c 
/// 
/// @return 
/// ----------------------------------------------------------------------------
int io_putc(int fd, char c);

/// ----------------------------------------------------------------------------
/// @brief io_getc
/// 
/// @param fd 
/// 
/// @return 
/// ----------------------------------------------------------------------------
int io_getc(int fd);

/// ----------------------------------------------------------------------------
/// @brief io_getc_timeout
/// 
/// @param fd 
/// 
/// @return -1 on error, 0 no byte ready within the timeout, 1 one byte read
/// ----------------------------------------------------------------------------
int io_getc_timeout(int fd, int timeout_ms, unsigned char *c);

#endif

/* end of prg_io_nonblock.h */
