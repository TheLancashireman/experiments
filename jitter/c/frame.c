/* frame.c - source code for a frame driver
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
#include <frame.h>

#define FR_NJOBS	16
#define FR_NFRAMES	16

dv_id_t FrameStart, FrameEnd;	/* Task IDs */

struct frame_s
{
	dv_id_t jobs[FR_NJOBS];
	dv_qty_t n_jobs;
	dv_qty_t n_overruns;
};

struct framedriver_s
{
	struct frame_s frames[FR_NFRAMES];
	dv_id_t current_frame;
	dv_id_t next_frame;
	dv_id_t current_job;
	int running;
	dv_id_t max_frame;
	dv_qty_t n_overruns;
};

struct framedriver_s framedriver;

/* FrameCreateTasks() - create the FrameStart and FrameEnd tasks
*/
void FrameCreateTasks(void)
{
}

/* FrameInit() - initialise the frame driver
*/
void FrameInit(void)
{
}

/* FrameInit() - initialise the frame driver
*/
void FrameAddTask(int frame)
{
}

void FrameChain(void)
{
	frame_driver.current_job++;
	dv_chaintask(frame_driver.frames[frame_driver.current_frame].jobs[frame_driver.current_job]);
}

/* main_FrameStart() - main function for the FrameStart task
*/
void main_FrameStart(void)
{
	if ( framedriver.running )
	{
		/* Handle deadline volation
		*/
		framedriver.n_overruns++;
		framedriver.frames[current_frame].n_overruns++;

		/* Need to determine what the next frame is going to be.
		 * If next_frame == current_frame we didn't get to the end
		*/

		/* Need to terminate the running task here
		*/
	}

	/* Go to next frame
	*/
	frame_driver.current_frame = frame_driver.next_frame;
	frame_driver.current_job = 0;
	frame_driver.running = 0;

	dv_chaintask(framedriver.frames[current_frame].jobs[0]);
}

/* main_FrameEnd() - main function for the FrameEnd task
*/
void main_FrameEnd(void)
{
	if ( framedriver.next_frame < frame_driver.max_frame )
	{
		framedriver.next_frame++;
	}
	else
	{
		framedriver.next_frame = 0;
	}
	framedriver.running = 0;

	/* Fall back to idle loop
	*/
}

#endif
