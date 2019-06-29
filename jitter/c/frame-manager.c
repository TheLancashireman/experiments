/* frame-manager.c - source code for a frame manager
 *
 * Each frame starts with the FrameStart task.
 * The  FrameStart task does some housekeeping and error recording/recovery.
 * Then each task in the frame is chained in sequence.
 * The last task in each fram id the FrameEnd task, which increments the next_frame index and clears
 * the running flag. Finally, it terminaes and drops back to whatever got interrupted.
 *
 * (c) David Haworth
*/
#define DV_ASM	0
#include <dv-config.h>
#include <davroska.h>
#include <frame-manager.h>

#define FM_MAXJOBS		16
#define FM_MAXFRAMES	16

dv_id_t fm_frameStart, fm_frameEnd;	/* Task IDs */

struct job_s
{
	dv_id_t task;
};

struct frame_s
{
	struct job_s jobs[FM_MAXJOBS];
	dv_qty_t n_jobs;
	dv_qty_t n_overruns;
};

struct framemanager_s
{
	struct frame_s frames[FM_MAXFRAMES];
	dv_id_t current_frame;
	dv_id_t next_frame;
	dv_id_t current_job;
	int running;
	dv_id_t max_frame;
	dv_qty_t n_overruns;
};

struct framemanager_s framemanager;

void main_FrameStart(void);
void main_FrameEnd(void);

/* fm_CreateTasks() - create the fm_frameStart and fm_frameEnd tasks
 *
 * To be called in the davroska callout_addtasks() function
*/
void fm_CreateTasks(void)
{
	fm_frameStart = dv_addtask("fm_FrameStart", &main_FrameStart, 4, 1);
	fm_frameEnd = dv_addtask("fm_FrameEnd", &main_FrameEnd, 4, 1);
}

/* fm_Init() - initialise the frame manager
*/
void fm_Init(void)
{
}

/* fm_AddTask() - add a task to the frame manager
*/
void fm_AddTask(dv_id_t frame, dv_id_t task)
{
	if ( frame >= FM_MAXFRAMES )
	{
		/* Report error here */
		return;
	}

	struct frame_s *fr = &framemanager.frames[frame];

	if ( fr->n_jobs >= FM_MAXJOBS )
	{
		/* Report error here */
		return;
	}

	struct job_s *job = &fr->jobs[fr->n_jobs];
	fr->n_jobs++;
	job->task = task;
}

void fm_TaskStart(void)
{
}

void fm_TaskEnd(void)
{
	framemanager.current_job++;
	dv_chaintask(framemanager.frames[framemanager.current_frame].jobs[framemanager.current_job].task);
}

/* main_FrameStart() - main function for the FrameStart task
*/
void main_FrameStart(void)
{
	if ( framemanager.running )
	{
		/* Handle deadline volation
		*/
		framemanager.n_overruns++;
		framemanager.frames[framemanager.current_frame].n_overruns++;

		/* Need to determine what the next frame is going to be.
		 * If next_frame == current_frame we didn't get to the end
		*/

		/* Need to terminate the running task here
		*/
	}

	/* Go to next frame
	*/
	framemanager.current_frame = framemanager.next_frame;
	framemanager.current_job = 0;
	framemanager.running = 0;

	dv_chaintask(framemanager.frames[framemanager.current_frame].jobs[0].task);
}

/* main_FrameEnd() - main function for the FrameEnd task
*/
void main_FrameEnd(void)
{
	if ( framemanager.next_frame < framemanager.max_frame )
	{
		framemanager.next_frame++;
	}
	else
	{
		framemanager.next_frame = 0;
	}
	framemanager.running = 0;

	/* Fall back to idle loop
	*/
	dv_terminatetask();
}
