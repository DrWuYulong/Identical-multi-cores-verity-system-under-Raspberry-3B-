

#define TASK_NUMBER				20
#define SUBTASK_NUMBER      	20
#define SET_NUMBER				100
#define SUBTASK_STACK_SIZE		512
#define SELF_TIMER_STACK_SIZE	1024
#define EXECUTION_LIMITATION	100000

#define U_MIN					200					// *100之后的值
#define U_MAX					300
#define U_STEP					20

#define N_MIN					1000
#define N_MAX					1500
#define N_STEP					100

#define V_MIN					1000
#define V_MAX					1500
#define V_STEP					100

#define pr_MIN					20
#define pr_MAX					30
#define pr_STEP					2

struct subtask
{
	int C;						//The execution time of this subtask
	rt_uint32_t constraint;		//The constraint of this subtask
	int processor;				//The processor that this subtask is allocated to be executed.
	int subtask_order;			//The order of subtask 
	int bl;						// the basics location of DAG task set
	char sub_name[16];
	int sub_stack_base;			// the basic address of each subtask stack .
	int parent_DAG_order;		// parent DAG order
	int finish;					// wheather finished
};
typedef struct subtask *subtask_p;

struct DAG_task
{
	int sub_task_base;						//the sructure array of subtasks
	int C[SUBTASK_NUMBER];								// 10 subtasks have independent execution time
	int T;									// 10 subtasks share one T
	int D;									// 10 subtasks share one D
	int Processors[SUBTASK_NUMBER];						// 10 subtasks have independent processors
	int Topologies[SUBTASK_NUMBER][SUBTASK_NUMBER];					// 10 subtasks have a 10x10 matrix
	int Priority;							// the priority of this DAG task
	char compare_todo[8];					// which is the current comparison
	int task_set;							// which task set is. the total number of task sets is SET_NUMBER
	int WCRT;								// the worst case response time of a DAG task
	int schedulable;						// wheather this DAG task can be scheduled 1 yes 0 no
	int DAG_order;							// which DAG task in this set
	int task_numb;
	int subtask_numb;

	int event_base;							// event[0] basic address of task set
	char event_name[16];					// the name of each event

	char task_name[16];						// thread(task) name thread_1,...thread_n.

	rt_tick_t first_release_tick;			// the first release tick of each DAG task for limiting total execution time
	rt_tick_t current_release_tick;			// current release tick of each DAG task
	rt_tick_t next_release_tick;			// next release tick of each DAG task

	rt_list_t list;
	rt_list_t *list_head_base;
	
	int sub_thread_base;					// the thread for creating subtasks of each DAG task
	int over_flag;

	rt_thread_t self_timer_base;				//self_timer thread point
};
typedef struct DAG_task *task_p;

void mnt_init(void);

void run_one_tick(void);

int power(int a, int b);

// void task_set_init(void *parameter);
void task_set_init(task_p tp);

int read_line(int fd, char *buff, int len_buff);

void DAG_Release(task_p tp);

void Experiment_Begin(void* parameter);

void Execution_subtask(void *parameter);

void CLose_all(task_p tp, int x);

void write_data(char * file_name, int data);

void self_timer_scheduler(void *parameter);

void insert_after(rt_list_t *x, rt_list_t *y);

void subtasks_first_release(task_p tp);




rt_err_t test_hwtimer(void);