/* frame-manager.h - header file for a frame manager
 *
 * (c) David Haworth
*/
#ifndef frame_manager_h
#define frame_manager_h	1

#define DV_ASM  0
#include <davroska.h>

extern void fm_CreateTasks(void);
extern void fm_Init(void);
extern void fm_AddTask(dv_id_t frame, dv_id_t task);
extern void fm_TaskStart(void);
extern void fm_TaskEnd(void);
extern void fm_StartFrame(void);

#endif
