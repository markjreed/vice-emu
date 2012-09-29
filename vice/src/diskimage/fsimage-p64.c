/*
 * fsimage-p64.c
 *
 * Written by
 *  Benjamin 'BeRo' Rosseaux <benjamin@rosseaux.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "fsimage-p64.h"
#include "fsimage.h"
#include "cbmdos.h"
#include "gcr.h"
#include "log.h"
#include "lib.h"
#include "types.h"
#include "util.h"
#include "p64.h"

static log_t fsimage_p64_log = LOG_ERR;

/*-----------------------------------------------------------------------*/
/* Intial P64 buffer setup.  */

int fsimage_read_p64_image(disk_image_t *image)
{
    TP64MemoryStream P64MemoryStreamInstance;
    PP64Image P64Image = (void*)image->p64;
    int lSize, rc;
    void *buffer;

    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    fseek(fsimage->fd, 0, SEEK_END);
    lSize = ftell(fsimage->fd);
    fseek(fsimage->fd, 0, SEEK_SET);
    buffer = (char*)lib_malloc(sizeof(char) * lSize);
    if (fread(buffer, 1, lSize, fsimage->fd) < 1) {
        lib_free(buffer);
        log_error(fsimage_p64_log, "Could not read P64 disk image.");
        return -1;
    }

    /*num_tracks = image->tracks;*/

    P64MemoryStreamCreate(&P64MemoryStreamInstance);
    P64MemoryStreamWrite(&P64MemoryStreamInstance,buffer,lSize);
    P64MemoryStreamSeek(&P64MemoryStreamInstance,0);
    if (P64ImageReadFromStream(P64Image,&P64MemoryStreamInstance)) {
        rc = 0;
    } else {
        rc = -1;
        log_error(fsimage_p64_log, "Could not read P64 disk image stream.");
    }
    P64MemoryStreamDestroy(&P64MemoryStreamInstance);

    lib_free(buffer);

    return rc;
}

int fsimage_write_p64_image(disk_image_t *image)
{
    TP64MemoryStream P64MemoryStreamInstance;
    PP64Image P64Image = (void*)image->p64;
    int rc;

    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    P64MemoryStreamCreate(&P64MemoryStreamInstance);
    P64MemoryStreamClear(&P64MemoryStreamInstance);
    if (P64ImageWriteToStream(P64Image,&P64MemoryStreamInstance)) {
        fseek(fsimage->fd, 0, SEEK_SET);
        if (fwrite(P64MemoryStreamInstance.Data, P64MemoryStreamInstance.Size, 1, fsimage->fd) < 1) {
                rc = -1;
                log_error(fsimage_p64_log, "Could not write P64 disk image.");
        } else {
                fflush(fsimage->fd);
                rc = 0;
        }
    } else {
        rc = -1;
        log_error(fsimage_p64_log, "Could not write P64 disk image stream.");
    }
    P64MemoryStreamDestroy(&P64MemoryStreamInstance);

    return rc;
}

/*-----------------------------------------------------------------------*/
/* Read an entire P64 track from the disk image.  */

int fsimage_p64_read_half_track(disk_image_t *image, unsigned int half_track,
                                BYTE *gcr_data, int *gcr_track_size)
{
    PP64Image P64Image = (void*)image->p64;
    unsigned int track;

    if (P64Image == NULL) {
        log_error(fsimage_p64_log, "P64 image not loaded.");
        return -1;
    }

    if (half_track > 84) {
        log_error(fsimage_p64_log, "Half track %i out of bounds.  Cannot read P64 track.", half_track);
        return -1;
    }

    memset(gcr_data, 0xff, 6250);

    track = half_track / 2;

    *gcr_track_size = (P64PulseStreamConvertToGCRWithLogic(&P64Image->PulseStreams[half_track], (void*)gcr_data, NUM_MAX_MEM_BYTES_TRACK, disk_image_speed_map_1541((track > 0) ? (track - 1) : 0)) + 7) >> 3;

    if (*gcr_track_size < 1) {
        *gcr_track_size = 6520;
    }

    return 0;
}

static int fsimage_p64_read_track(disk_image_t *image, unsigned int track,
                           BYTE *gcr_data, int *gcr_track_size)
{
    PP64Image P64Image = (void*)image->p64;

    if (P64Image == NULL) {
        log_error(fsimage_p64_log, "P64 image not loaded.");
        return -1;
    }

    if (track > 42) {
        log_error(fsimage_p64_log, "Track %i out of bounds.  Cannot read P64 track.", track);
        return -1;
    }

    memset(gcr_data, 0xff, 6250);

    *gcr_track_size = (P64PulseStreamConvertToGCRWithLogic(&P64Image->PulseStreams[track << 1], (void*)gcr_data, NUM_MAX_MEM_BYTES_TRACK, disk_image_speed_map_1541((track > 0) ? (track - 1) : 0)) + 7) >> 3;

    if (*gcr_track_size < 1) {
        *gcr_track_size = 6520;
    }

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Write an entire P64 track to the disk image.  */

int fsimage_p64_write_half_track(disk_image_t *image, unsigned int half_track,
                                 int gcr_track_size, BYTE *gcr_track_start_ptr)
{
    PP64Image P64Image = (void*)image->p64;

    if (P64Image == NULL) {
        log_error(fsimage_p64_log, "P64 image not loaded.");
        return -1;
    }

    if (half_track > 84) {
        log_error(fsimage_p64_log, "Half track %i out of bounds.  Cannot write P64 track.", half_track);
        return -1;
    }
    if (gcr_track_start_ptr == NULL) {
        return 0;
    }

    P64PulseStreamConvertFromGCR(&P64Image->PulseStreams[half_track], (void*)gcr_track_start_ptr, gcr_track_size << 3);

    return fsimage_write_p64_image(image);
}

static int fsimage_p64_write_track(disk_image_t *image, unsigned int track,
                            int gcr_track_size, BYTE *gcr_track_start_ptr)
{
    PP64Image P64Image = (void*)image->p64;

    if (P64Image == NULL) {
        log_error(fsimage_p64_log, "P64 image not loaded.");
        return -1;
    }

    if (track > 42) {
        log_error(fsimage_p64_log, "Track %i out of bounds.  Cannot write P64 track.", track);
        return -1;
    }

    P64PulseStreamConvertFromGCR(&P64Image->PulseStreams[track << 1], (void*)gcr_track_start_ptr, gcr_track_size << 3);

    return fsimage_write_p64_image(image);
}

/*-----------------------------------------------------------------------*/
/* Read a sector from the P64 disk image.  */

int fsimage_p64_read_sector(disk_image_t *image, BYTE *buf,
                               unsigned int track, unsigned int sector)
{
    BYTE gcr_data[NUM_MAX_MEM_BYTES_TRACK];
    int gcr_track_size;
    fdc_err_t rf;
    disk_track_t raw;

    if (track > 42) {
        log_error(fsimage_p64_log, "Track %i out of bounds.  Cannot read P64 track.", track);
        return CBMDOS_IPE_ILLEGAL_TRACK_OR_SECTOR;
    }

    if (fsimage_p64_read_track(image, track, gcr_data, &gcr_track_size) < 0) {
        return CBMDOS_IPE_NOT_READY;
    }
    raw.data = gcr_data;
    raw.size = gcr_track_size;

    rf = gcr_read_sector(&raw, buf, sector);
    if (rf != CBMDOS_FDC_ERR_OK) {
        log_error(fsimage_p64_log, "Cannot find track: %i sector: %i within P64 image.", track, sector);
        switch (rf) {
        case CBMDOS_FDC_ERR_HEADER:
            return CBMDOS_IPE_READ_ERROR_BNF;   /* 20 */
        case CBMDOS_FDC_ERR_SYNC:
            return CBMDOS_IPE_READ_ERROR_SYNC;  /* 21 */
        case CBMDOS_FDC_ERR_NOBLOCK:
            return CBMDOS_IPE_READ_ERROR_DATA;  /* 22 */
        case CBMDOS_FDC_ERR_DCHECK:
            return CBMDOS_IPE_READ_ERROR_CHK;   /* 23 */ 
        case CBMDOS_FDC_ERR_VERIFY:
            return CBMDOS_IPE_WRITE_ERROR_VER;  /* 25 */
        case CBMDOS_FDC_ERR_WPROT:
            return CBMDOS_IPE_WRITE_PROTECT_ON; /* 26 */
        case CBMDOS_FDC_ERR_HCHECK:
            return CBMDOS_IPE_READ_ERROR_BCHK;  /* 27 */
        case CBMDOS_FDC_ERR_BLENGTH:
            return CBMDOS_IPE_WRITE_ERROR_BIG;  /* 28 */
        case CBMDOS_FDC_ERR_ID:
            return CBMDOS_IPE_DISK_ID_MISMATCH; /* 29 */
        case CBMDOS_FDC_ERR_DRIVE:
            return CBMDOS_IPE_NOT_READY;        /* 74 */
        case CBMDOS_FDC_ERR_DECODE:
            return CBMDOS_IPE_READ_ERROR_GCR;   /* 24 */
        default:
            return CBMDOS_IPE_NOT_READY;
        }
    }
    return CBMDOS_IPE_OK;
}


/*-----------------------------------------------------------------------*/
/* Write a sector to the P64 disk image.  */

int fsimage_p64_write_sector(disk_image_t *image, BYTE *buf,
                                unsigned int track, unsigned int sector)
{
    unsigned int max_track_length = NUM_MAX_MEM_BYTES_TRACK;
    BYTE *gcr_data;
    int gcr_track_size;
    disk_track_t raw;

    if (track > 42) {
        log_error(fsimage_p64_log, "Track %i out of bounds.  Cannot write P64 sector", track);
        return -1;
    }

    gcr_data = (BYTE*) lib_malloc(max_track_length);

    if (fsimage_p64_read_track(image, track, gcr_data, &gcr_track_size) < 0) {
        log_error(fsimage_p64_log, "Cannot read track %i from P64 image.", track);
        lib_free(gcr_data);
        return -1;
    }
    raw.data = gcr_data;
    raw.size = gcr_track_size;

    if (gcr_write_sector(&raw, buf, sector) != CBMDOS_FDC_ERR_OK) {
        log_error(fsimage_p64_log, "Could not find track %i sector %i in disk image", track, sector);
        lib_free(gcr_data);
        return -1;
    }

    if (fsimage_p64_write_track(image, track, raw.size, raw.data) < 0) {
        log_error(fsimage_p64_log, "Failed writing track %i to disk image.", track);
        lib_free(gcr_data);
        return -1;
    }

    lib_free(gcr_data);

    return 0;

}

/*-----------------------------------------------------------------------*/

void fsimage_p64_init(void)
{
    fsimage_p64_log = log_open("Filesystem Image P64");
}

