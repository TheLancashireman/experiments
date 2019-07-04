/* frame-manager.c - source code for a frame manager
 *
 * Each frame starts with the FrameStart task.
 * The FrameStart task does some housekeeping and error recording/recovery.
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
#include <dv-stdio.h>

#include TARGET_HDR

#define FM_MAXJOBS		16
#define FM_MAXFRAMES	16

/* For the experiment: print the results after this many rounds
*/
#define FM_NROUNDS		10

dv_id_t fm_frameStart, fm_frameEnd;	/* Task IDs */

struct timing_s
{
	dv_u64_t t_min;
	dv_u64_t t_max;
	dv_u64_t t_sum;
	unsigned n;
};

struct job_s
{
	dv_u64_t start_time;
	dv_u64_t end_time;
	dv_u64_t prev_start_time;
	dv_id_t task;
	struct timing_s latency;		/* From end of previous task to start of task */
	struct timing_s runtime;		/* From start of task to end of task */
	struct timing_s interval;		/* From previous start time to new start time */
};

struct frame_s
{
	struct job_s jobs[FM_MAXJOBS];
	dv_qty_t n_jobs;
	dv_qty_t n_overruns;
	dv_u64_t activation_time;
	dv_u64_t start_time;
	dv_u64_t prev_activation_time;
	dv_u64_t prev_start_time;
	int n_runs;
	struct timing_s act_interval;		/* From previous activation time to new activation time */
	struct timing_s start_interval;		/* From previous start time to new start time */
	struct timing_s latency;			/* From activation to start */
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
	dv_u64_t activation_time;
	dv_u64_t rounds;
};

struct framemanager_s framemanager;

void main_FrameStart(void);
void main_FrameEnd(void);
void fm_ComputeTimes(void);
void fm_PrintResults(void);

static inline void fm_InitTime(struct timing_s *ts)
{
	ts->t_min = 0xffffffffffffffff;
	ts->t_max = 0;
	ts->t_sum = 0;
	ts->n = 0;
}

static inline void fm_StoreTime(struct timing_s *ts, dv_u64_t t_from, dv_u64_t t_to)
{
	if ( t_from != 0 )
	{
		dv_u64_t diff = t_to - t_from;
		if ( ts->t_min > diff )	ts->t_min = diff;
		if ( ts->t_max < diff )	ts->t_max = diff;
		ts->t_sum += diff;
		ts->n++;
	}
}

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
	framemanager.n_overruns = 0;
	framemanager.max_frame = 0;
	framemanager.running = 0;
	framemanager.current_job = 0;
	framemanager.next_frame = 0;
	framemanager.current_frame = 0;
	framemanager.activation_time = 0;
	framemanager.rounds = 0;

	for (int f = 0; f < FM_MAXFRAMES; f++)
	{
		framemanager.frames[f].n_overruns = 0;
		framemanager.frames[f].n_jobs = 0;
		framemanager.frames[f].n_runs = 0;
		framemanager.frames[f].activation_time = 0;
		framemanager.frames[f].start_time = 0;
		framemanager.frames[f].prev_activation_time = 0;
		framemanager.frames[f].prev_start_time = 0;
		fm_InitTime(&framemanager.frames[f].act_interval);
		fm_InitTime(&framemanager.frames[f].start_interval);
		fm_InitTime(&framemanager.frames[f].latency);

		for ( int j = 0; j < FM_MAXJOBS; j++ )
		{
			framemanager.frames[f].jobs[j].task = fm_frameEnd;
			framemanager.frames[f].jobs[j].start_time = 0;
			framemanager.frames[f].jobs[j].end_time = 0;
			framemanager.frames[f].jobs[j].prev_start_time = 0;
			fm_InitTime(&framemanager.frames[f].jobs[j].latency);
			fm_InitTime(&framemanager.frames[f].jobs[j].runtime);
			fm_InitTime(&framemanager.frames[f].jobs[j].interval);
		}
	}
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

	/* Limit is (FM_MAXJOBS - 1) because the last "job" is always fm_FrameEnd
	*/
	if ( fr->n_jobs >= (FM_MAXJOBS - 1) )
	{
		/* Report error here */
		return;
	}

	/* Remember the highest frame
	*/
	if ( frame > framemanager.max_frame )
	{
		framemanager.max_frame = frame;
	}

	struct job_s *job = &fr->jobs[fr->n_jobs];
	fr->n_jobs++;
	job->task = task;
}

/* fm_StartFrame() - called by interrupt to start a new frame
 *
 * Record the activation time
 * Activates the fm_FrameStart task
*/
void fm_StartFrame(void)
{
#ifdef FM_NROUNDS
	/* For timing tests: stop activating after configured number of rounds
	*/
	if ( framemanager.rounds >= FM_NROUNDS )
		return;
#endif

	framemanager.activation_time = dv_readtime();
	dv_activatetask(fm_frameStart);
}

/* fm_TaskStart() - called at the start of every task
 *
 * Records the start time
*/
void fm_TaskStart(void)
{
	framemanager.frames[framemanager.current_frame].jobs[framemanager.current_job].start_time = dv_readtime();
}

/* fm_TaskEnd() - called at the end of every task
 *
 * Records the end time;
 * Chains the next task in the frame
*/
void fm_TaskEnd(void)
{
	framemanager.frames[framemanager.current_frame].jobs[framemanager.current_job].end_time = dv_readtime();
	framemanager.current_job++;
	dv_chaintask(framemanager.frames[framemanager.current_frame].jobs[framemanager.current_job].task);
}

/* main_FrameStart() - main function for the FrameStart task
 *
 * Note the start time
 * Check for deadline violation
 * Move to the next frame and initialise it for a new run
 * Record the activation time and start time for the frame
*/
void main_FrameStart(void)
{
	dv_u64_t start_time = dv_readtime();

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
	framemanager.frames[framemanager.current_frame].activation_time = framemanager.activation_time;
	framemanager.frames[framemanager.current_frame].start_time = start_time;

	dv_chaintask(framemanager.frames[framemanager.current_frame].jobs[0].task);
}

/* main_FrameEnd() - main function for the FrameEnd task
 *
 * Calculate the next frame
 * Clear the running flag
 * Terminate (return to background processing)
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
		framemanager.rounds++;
	}

	fm_ComputeTimes();

	framemanager.running = 0;

#ifdef FM_NROUNDS
	if ( framemanager.rounds == FM_NROUNDS )
		fm_PrintResults();
#endif
		

	/* Fall back to idle loop
	*/
	dv_terminatetask();
}

/* fm_ComputeTimes() - computes the timing for the current frame.
 *
 * Called at the end of each frame
 * Calculates:
 *	- for frame:
 *		- act_interval		- time from previous activation to current activation
 *		- start_interval	- time from previous start to current start
 *		- latency			- time from activation to start
 *	- for each job:
 *		- latency			- time from end of previous job to start of job
 *		- runtime			- time from start to end
 *		- interval			- time from previous start to current start
*/
void fm_ComputeTimes(void)
{
	dv_id_t f = framemanager.current_frame;
	struct frame_s *fr = &framemanager.frames[f];

	fm_StoreTime(&fr->act_interval, fr->prev_activation_time, fr->activation_time);
	fm_StoreTime(&fr->start_interval, fr->prev_start_time, fr->start_time);
	fm_StoreTime(&fr->latency, fr->activation_time, fr->start_time);

	fr->prev_activation_time = fr->activation_time;
	fr->prev_start_time = fr->start_time;

	for ( dv_id_t j = 0; j < fr->n_jobs; j++)
	{
		if ( j == 0 )
		{
			/* For the first job, the latency is the time from the frame start
			*/
			fm_StoreTime(&fr->jobs[j].latency, fr->start_time, fr->jobs[j].start_time);
		}
		else
		{
			fm_StoreTime(&fr->jobs[j].latency, fr->jobs[j-1].end_time, fr->jobs[j].start_time);
		}

		fm_StoreTime(&fr->jobs[j].runtime, fr->jobs[j].start_time, fr->jobs[j].end_time);
		fm_StoreTime(&fr->jobs[j].interval, fr->jobs[j].prev_start_time, fr->jobs[j].start_time);

		fr->jobs[j].prev_start_time = fr->jobs[j].start_time;
	}
}

/* fm_PrintTimes() - print the contents of a timing structure
 *
*/
void fm_PrintTimes(struct timing_s *t, char *descr, char *obj, dv_id_t id)
{
	if ( t->n <= 0 )
		return;

	dv_u64_t mean = (t->t_sum + (t->n/2)) / t->n;

	dv_u32_t mean32 = (mean > 0xffffffff) ? 0xffffffff : mean;
	dv_u32_t min32 = (t->t_min > 0xffffffff) ? 0xffffffff : t->t_min;
	dv_u32_t max32 = (t->t_max > 0xffffffff) ? 0xffffffff : t->t_max;

	dv_printf("%s times for %s %d: min %u, mean %u, max %u\n", descr, obj, id, min32, mean32, max32);
}

/* fm_PrintResults() - print all the timing at the end of the run
 *
*/
void fm_PrintResults(void)
{
	dv_id_t f, j;

	/* First the frame timings
	*/
	for ( f = 0; f <= framemanager.max_frame; f++ )
	{
		fm_PrintTimes(&framemanager.frames[f].act_interval, "Activation interval", "frame", f); 
		fm_PrintTimes(&framemanager.frames[f].start_interval, "Start interval", "frame", f); 
		fm_PrintTimes(&framemanager.frames[f].latency, "Latency", "frame", f); 
	}

	/* Then the individual job timings
	*/
	for ( f = 0; f <= framemanager.max_frame; f++ )
	{
		dv_printf("Job timings for frame %d:\n", f);
		for ( j = 0; j < framemanager.frames[f].n_jobs; j++)
		{
			fm_PrintTimes(&framemanager.frames[f].jobs[j].interval, "  Interval", "job", j);
			fm_PrintTimes(&framemanager.frames[f].jobs[j].runtime,  "  Runtime", "job", j);
			fm_PrintTimes(&framemanager.frames[f].jobs[j].latency,  "  Latency", "job", j);
		}
		dv_printf("\n");
	}
}
