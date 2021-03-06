#ifndef VDF_LIB_H
#define VDF_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "helper.h"

struct filter_tbl;

/* Common interface for video library */
/*
typedef enum {
	DRM_MODULE_XILINX,
	DRM_MODULE_XYLON,
	DRM_MODULE_NONE = -1,
} drm_module;
*/

/* FIXME: remove global varaible */
//extern drm_module module_g;

typedef enum {
	VIDEO_CTRL_OFF,
	VIDEO_CTRL_ON,
	VIDEO_CTRL_AUTO
} video_ctrl;

typedef enum {
	CAPTURE,
	DISPLAY,
	PROCESS_IN,
	PROCESS_OUT,
	NUM_EVENTS
} pipeline_event;

enum vlib_vsrc_class {
	VLIB_VCLASS_VIVID,
	VLIB_VCLASS_UVC,
	VLIB_VCLASS_TPG,
	VLIB_VCLASS_HDMII,
	VLIB_VCLASS_CSI,
	VLIB_VCLASS_FILE,
};

#include "filter.h"

struct vlib_config {
	size_t vsrc;
	unsigned int type;
	size_t mode;
};

#include <linux/videodev2.h>
#include "common.h"

struct vlib_config_data {
	struct filter_tbl *ft; 			/* filter table */
	int width_in;						/* input width */
	int height_in;						/* input height */
	uint32_t fmt_in;					/* input pixel format */
	unsigned int flags;					/* flags */
	unsigned int dri_card_id;			/* dri card number */
	int width_out;						/* output width */
	int height_out;						/* output height */
	uint32_t fmt_out;					/* output pixel format */
	struct v4l2_fract fps;				/* frames per second */
	const char *vcap_file_fn;			/* filename for file source */
	struct vlib_plane plane;
	size_t vrefresh;					/* vertical refresh rate */
	const char *drm_background;			/* path to background image */
	size_t buffer_cnt;					/* number of frame buffers */
};

#define VLIB_CFG_FLAG_PR_ENABLE				BIT(0) /* enable partial reconfiguration */
#define VLIB_CFG_FLAG_MULTI_INSTANCE	BIT(1) /* enable multi-instance mode */

/**
 * Error codes. Most vlib functions return 0 on success or one of these
 * codes on failure.
 * User can call vlib_error_name() to retrieve a string representation of an
 * error code or vlib_strerror() to get an end-user suitable description of
 * an error code.
*/

/* Total number of error codes in enum vlib_error */
#define VLIB_ERROR_COUNT 6

typedef enum {
	VLIB_SUCCESS = 0,
	VLIB_ERROR_INTERNAL = -1,
	VLIB_ERROR_CAPTURE = -2,
	VLIB_ERROR_INVALID_PARAM = -3,
	VLIB_ERROR_FILE_IO = -4,
	VLIB_ERROR_NOT_SUPPORTED = -5,
	VLIB_ERROR_NO_MEM = -6,
	VLIB_ERROR_OTHER = -99
} vlib_error;

/* Character-array to store string-representation of the error-codes */
extern char vlib_errstr[];

/**
 *  Log message levels.
 *  - VLIB_LOG_LEVEL_NONE (0)
 *  - VLIB_LOG_LEVEL_ERROR (1)
 *  - VLIB_LOG_LEVEL_WARNING (2)
 *  - VLIB_LOG_LEVEL_INFO (3)
 *  - VLIB_LOG_LEVEL_DEBUG (4)
 *  All the messages are printed on stderr.
 */
typedef enum {
	VLIB_LOG_LEVEL_NONE = 0,
	VLIB_LOG_LEVEL_ERROR,
	VLIB_LOG_LEVEL_WARNING,
	VLIB_LOG_LEVEL_INFO,
	VLIB_LOG_LEVEL_DEBUG,
} vlib_log_level;

struct video_resolution {
	unsigned int height;
	unsigned int width;
	unsigned int stride;
};

/* The following is used to silence warnings for unused variables */
#define UNUSED(var)		do { (void)(var); } while(0)

/* video source helper functions */
struct vlib_vdev;

const char *vlib_video_src_get_display_text(const struct vlib_vdev *vsrc);
const struct vlib_vdev *vlib_video_src_get(size_t id);
size_t vlib_video_src_cnt_get(void);

static inline const char *vlib_video_src_get_display_text_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	return vlib_video_src_get_display_text(v);
}

/* drm helper functions */
int vlib_drm_set_layer0_state(int);
int vlib_drm_set_layer0_transparency(int);
int vlib_drm_set_layer0_position(int, int);
int vlib_drm_try_mode(unsigned int dri_card_id, int width, int height,
						size_t *vrefresh);

/* video resolution functions */
int vlib_get_active_height(void);
int vlib_get_active_width(void);

/* init/uinit functions */
int vlib_init(struct vlib_config_data *cfg);
int vlib_uninit(void);

/* video pipeline control functions */
int vlib_pipeline_stop(void);
int vlib_change_mode(struct vlib_config *config);

/* set event-log function */
int vlib_set_event_log(int state);
/* Query pipeline events */
float vlib_get_event_cnt(pipeline_event event);

/* return the string representation of the error code */
const char *vlib_error_name(vlib_error error_code);
/* return user-readable description of the error-code */
char *vlib_strerror(void);

#ifdef __cplusplus
}
#endif

#endif /* VDF_LIB_H */
