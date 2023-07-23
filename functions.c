#include <rtthread.h>
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <stdlib.h> //for atoi, the function that change char to int.
#include "functions.h"
#include <rthw.h> 	//for interrupt
#include <rtdevice.h>
#include <finsh.h>


//*************initialization the storage of USB*********
void mnt_init(void)
{
    rt_thread_delay(RT_TICK_PER_SECOND);

    if (dfs_mount("sd1", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("file system1 initialization done!\n");
    }else{
    	rt_kprintf("file system1 initialization failed!\n");
    }

    rt_thread_delay(RT_TICK_PER_SECOND);

    if (dfs_mount("sd0", "/boot/", "elm", 0, 0) == 0)
    {
        rt_kprintf("file system2 initialization done!\n");
    }else{
    	rt_kprintf("file system2 initialization failed!\n");
    }

}

void write_data(char * file_name, int data)
{
	int fd, mode;
	char re_d[32], d[32], *p;
	p = re_d;
	
	while (data/10 > 0)
	{
		mode = data%10;
		data = data/10;
		rt_sprintf(p,"%d",mode);
		p++;
	}
	rt_sprintf(p,"%d",data);
	p++;
	*p = '\0';

	//reverse the string of data
	p = &re_d[rt_strlen(re_d) - 1];
	for (int i = 0; i < rt_strlen(re_d); i++)
	{
		rt_strncpy(&d[i],p,1);
		p--;
	}
	d[rt_strlen(re_d)] = '\0';
	//*****open file with WRITE ONLY if not existence then create it
	fd = open(file_name, O_WRONLY | O_CREAT);
	if (fd >= 0)
	{
		write(fd, d, sizeof(d));
		close(fd);
	}
}


void run_one_tick(void)
{
	int a = 150000;
	while(a--);
}

int read_line(int fd, char *buff, int len_buff)
{// read just one line from the date file.
	int length;
	char *position;
	length = read(fd, buff, len_buff);
    if (length > 0)
    {
		position = strstr(buff,"\n");
		if (position != RT_NULL)
		{
			*position = '\0';

			/*fd move back*/
			lseek(fd, -(length - (position - buff + 1)), SEEK_CUR);
			length = position - buff;
		}
		else
		{
			length = rt_strlen(buff);
		}


    }
    else
    {
    	length = 0;
    }

    return length;
}

int power(int a, int b)
{
	int j = a;
	for(int i = 1; i < b; i++)
	{
		j = j * a;
	}
	return j;
}



//***********initialize the task set: C, T, D, Processors, Topology
void task_set_init(task_p tp)
{
	// task_p tp;
	// tp = (task_p)parameter;

	char file_path[128] = "";
    char file_name[128] = "";
    // char set_number[8] = "", *sn_p;
    // char new_u[4] = "", *new_u_p; //储存小数点后的利用率
    char buff[256], *buff_start;
    int fd, length, i, j = 0; //i for raw j for column tt for save taskset number provisionally
    char thread_name_base[16] = "DAG_", * thread_name_p;
    char event_name_base[16] = "event_", * event_name_p;
    char sub_name_base[8] = "T", * sub_name_p;
    //int flag_dot; //用来作为是否加了小数点的标志符

    rt_err_t result;
    rt_event_t event_p;
	event_p = (rt_event_t)(tp->event_base);


	/************* 开始初始化大类别 *************/
	sprintf(file_path, "/data/V/%s/",tp->compare_todo);
	sprintf(file_name,"%sn_and_V.txt",file_path);


	fd = open(file_name,O_RDONLY);

	length = read_line(fd,buff,16);
	buff_start = buff;
	tp->task_numb = atoi(buff_start);

	length = read_line(fd,buff,16);
	buff_start = buff;
	tp->subtask_numb = atoi(buff_start);

    close(fd);

    // rt_kprintf("-- %s,%d,%d\n",file_name,tp->task_numb,tp->subtask_numb);


    sprintf(file_path+rt_strlen(file_path), "set_%d/RM/",tp->task_set);	

    //*****************Start Processors****************//
    sprintf(file_name,"%sTDTA.txt",file_path);
		
    i = 0;

	fd = open(file_name,O_RDONLY);
	length = 1;
	while(length)
	{
		j = 0;
		length = read_line(fd,buff,sizeof(buff));
		if (length == 0)
		{
			break;
		}
		buff_start = buff;
		do
		{
			(tp+i)->Processors[j] = atoi(buff_start);
			buff_start = strstr(buff_start+1, " ");
			j++;
		}while(buff_start != RT_NULL);
		i++;
	}
    close(fd);

    //*****************Start C****************//
    sprintf(file_name,"%sC.txt",file_path);


    
    /*开始读取文件内容*/
    i = 0;
		
	fd = open(file_name,O_RDONLY);




	length = 1;
	while(length)
	{
		j = 0;
		length = read_line(fd,buff,sizeof(buff));
		if (length == 0)
		{
			break;
		}
		buff_start = buff;
		do
		{
			(tp+i)->C[j] = atoi(buff_start);
			buff_start = strstr(buff_start+1, " ");
			j++;
		}while(buff_start != RT_NULL);
		i++;
	}
    close(fd);


    
    // rt_kprintf("\n");
    // for (int xx = 0; xx < 10; xx++)
    // {
    // 	for (int yy = 0; yy < 10; yy++)
    // 	{
    // 		rt_kprintf("-%d",(tp+xx)->C[yy]);
    // 	}
    // 	rt_kprintf("\n");
    // }


    //******************Start T*****************//
    sprintf(file_name,"%sT.txt",file_path);

	i = 0;

	fd = open(file_name,O_RDONLY);
	length = 1;
	while(length)
	{
		length = read_line(fd,buff,16);
		if (length == 0)
		{
			break;
		}
		buff_start = buff;
		(tp+i)->T = atoi(buff_start);
		i++;
	}
    close(fd);

    //******************Start D*****************//
    sprintf(file_name,"%sD.txt",file_path);

	i = 0;

	fd = open(file_name,O_RDONLY);
	length = 1;
	while(length)
	{
		length = read_line(fd,buff,16);
		if (length == 0)
		{
			break;
		}
		buff_start = buff;
		(tp+i)->D = atoi(buff_start);
		i++;
	}
    close(fd);

    

    //*****************Start Topologies****************//
    for (int task_numb = 0; task_numb < tp->task_numb; task_numb ++)
    {
	    sprintf(file_name,"%sE/%d.txt",file_path,task_numb+1);
	    //i = 0; the same function as task_numb
	    // rt_kprintf("DAG 393, sub %d topologies name is: %s\n", task_numb,file_name);

		fd = open(file_name,O_RDONLY);
		length = 1;
		i = 0;
		while(length)
		{
			 //i,j the index of matrix topologies
			j = 0;
			length = read_line(fd,buff,sizeof(buff));
			if (length == 0)
			{
				break;
			}
			buff_start = buff;
			do
			{
				(tp+task_numb)->Topologies[i][j] = atoi(buff_start);
				buff_start = strstr(buff_start+1, " ");
				j++;
			}while(buff_start != RT_NULL);
			i++;
		}
	    close(fd);
    }

    

    //************* Start structure subtasks *************************
    for (i = 0; i < tp->task_numb; i++)
    {
    	// rt_kprintf("Topologies %d\n",(tp+i)->DAG_order);
    	for (j = 0; j < tp->subtask_numb; j++)
		{

			((subtask_p)(tp+i)->sub_task_base + j)->C  = (tp+i)->C[j];
			((subtask_p)(tp+i)->sub_task_base + j)->parent_DAG_order = i;
			((subtask_p)(tp+i)->sub_task_base + j)->constraint = 0; // initialization constrain
			for (int k = 0; k < tp->subtask_numb; k++)
			{
				if ((tp+i)->Topologies[k][j] == 1)
				{
					((subtask_p)(tp+i)->sub_task_base + j)->constraint |= (1 << k);
				}
			}
			((subtask_p)(tp+i)->sub_task_base + j)->processor = (tp+i)->Processors[j];
			((subtask_p)(tp+i)->sub_task_base + j)->subtask_order = j;

	
			sub_name_p = ((subtask_p)(tp+i)->sub_task_base + j)->sub_name;
			rt_strncpy(sub_name_p,sub_name_base,rt_strlen(sub_name_base));
		    sub_name_p += rt_strlen(sub_name_base);
		    rt_sprintf(sub_name_p,"%d", i);
		    sub_name_p ++;
		    rt_strncpy(sub_name_p,"_sub",rt_strlen("_sub"));
			sub_name_p += rt_strlen("_sub");
			rt_sprintf(sub_name_p,"%d", j);
    		sub_name_p ++;

    		//   子任务的名称如果大于等于10，那么还需要把个位加上
    		if (j >= 10)
    		{
    			rt_sprintf(sub_name_p,"%d", j%10);
    			sub_name_p ++;
    		}


			*sub_name_p = '\0';

			((subtask_p)(tp+i)->sub_task_base + j)->bl = (int)tp;

		}
    }

    
    	//initialize thread_name, subtask_name and event of each DAG task
   //  if ((tp->u == 2) && (tp->task_set == 1))
  	// {   		
	    for (i = 0; i < tp->task_numb; i++)
		{
			thread_name_p = (tp+i)->task_name;
	        rt_strncpy(thread_name_p,thread_name_base,rt_strlen(thread_name_base));
	        thread_name_p += rt_strlen(thread_name_base);
	        rt_sprintf(thread_name_p,"%d", i);
	        thread_name_p ++;
	        *thread_name_p = '\0';


			event_name_p = (tp+i)->event_name;
	        rt_strncpy(event_name_p,event_name_base,rt_strlen(event_name_base));
	        event_name_p +=  rt_strlen(event_name_base);
	        rt_sprintf(event_name_p,"%d", i);
	        event_name_p ++;
	        *event_name_p = '\0';


	        /******* initialize events ********/
	//        if ((tp+i)->u == 2 && (tp+i)->task_set == 1)
	//	    {
	        result = rt_event_init(event_p+i, (tp+i)->event_name, RT_IPC_FLAG_PRIO);

	        if (result != RT_EOK)
	        {
	            rt_kprintf("init %s failed.\n", event_p+i);
	        }
	    }


	//************WCRT, schedulable and basic location of tp initialization****************
	for (i = 0; i < tp->task_numb; i++)
	{
		(tp+i)->WCRT = 0;
		(tp+i)->schedulable = 1;
	}

}

void Execution_subtask(void *parameter)
{
	subtask_p sub_tp;
	sub_tp = (subtask_p)parameter;

	task_p tp;
	tp = (task_p)sub_tp->bl;

	task_p parent_tp;
	parent_tp = tp + sub_tp->parent_DAG_order;

	rt_uint32_t e;
	int response_time;
	rt_err_t temp_err;


	while(1)
	{
		if(tp->schedulable == 0)
		{//如果不可调度了就把自己挂起
			rt_thread_suspend(rt_thread_self());
			rt_schedule();
			continue;
			// return;
		}
		response_time = 0;
		temp_err = rt_event_recv((rt_event_t)(tp->event_base) + sub_tp->parent_DAG_order, 			//event address
					sub_tp->constraint,
                  	RT_EVENT_FLAG_AND,
                  	(tp+sub_tp->parent_DAG_order)->T, &e);
		if(temp_err == -RT_ETIMEOUT)
		{
			rt_enter_critical();
			tp->schedulable = 0;
			// rt_kprintf(" %d-1b: %d ",parent_tp->DAG_order,parent_tp->WCRT);
			parent_tp->WCRT = rt_tick_get()-parent_tp->current_release_tick;
			// rt_kprintf(" %d-1a: %d ",parent_tp->DAG_order,parent_tp->WCRT);
			// rt_kprintf("subtask out time\n");
			rt_event_send((rt_event_t)(tp->over_flag) , (1 << 1));
			rt_exit_critical();
			rt_thread_suspend(rt_thread_self());
			rt_schedule();
			continue;
			// return;
		}else{
			for (int j = 0; j < sub_tp->C; j++)
			{
				run_one_tick();
			}
			rt_enter_critical();
		
			response_time = rt_tick_get() - parent_tp->current_release_tick;
			// rt_kprintf(" %d-2a: %d ",parent_tp->DAG_order,response_time);
			rt_exit_critical();
			if(response_time > parent_tp->D)
			{
				rt_enter_critical();
				tp->schedulable = 0;
				// rt_kprintf("out ddl!\n");
				// rt_kprintf(" %d-3b: %d ",parent_tp->DAG_order,parent_tp->WCRT);
				parent_tp->WCRT = response_time;
				// rt_kprintf(" %d-3a: %d ",parent_tp->DAG_order,parent_tp->WCRT);
				rt_event_send((rt_event_t)(tp->over_flag) , (1 << 1));
				rt_exit_critical();
				rt_thread_suspend(rt_thread_self());
				rt_schedule();
				continue;
				// return;
			}else{
				rt_enter_critical();
				if(response_time > parent_tp->WCRT)
				{	
					// rt_kprintf(" %d-4b: %d ",parent_tp->DAG_order,parent_tp->WCRT);
					parent_tp->WCRT = response_time;
					// rt_kprintf(" %d-4a: %d ",parent_tp->DAG_order,parent_tp->WCRT);
				}

				sub_tp->finish = 1;
				rt_event_send((rt_event_t)(tp->event_base) + sub_tp->parent_DAG_order, (1 << sub_tp->subtask_order));
				
				// if (rt_strncmp(sub_tp->sub_name,"T0_sub",5) == 0)
				// {
				// 	rt_kprintf("-%s done %d-",sub_tp->sub_name,sub_tp->constraint);
				// }

				rt_exit_critical();
				rt_thread_suspend(rt_thread_self());
				rt_schedule();
				continue;
			}
		}
	}

}

void insert_after(rt_list_t *x, rt_list_t *y)
{//把x插入到y的后面
	x->next = y->next;
	y->next->prev = x;

	y->next = x;
	x->prev = y;
}

void remove_list(rt_list_t *x)
{
	x->next->prev = x->prev;
	x->prev->next = x->next;

	x->next = x;
	x->prev = x;
}

void subtasks_first_release(task_p tp)
{
	subtask_p sub_tp;
	rt_err_t  err;

	rt_list_t *self_timer_list;
	self_timer_list = tp->list_head_base;

	/*用于插入list*/
	task_p temp_tp;
	rt_list_t *current_list_p;

	int flag = 0;
	for(int i = 0; i < tp->task_numb; i++)
	{		
		for (int j = 0; j < tp->subtask_numb; j++)
		{
			sub_tp = (subtask_p)(tp+i)->sub_task_base + j;
			err = rt_thread_init((rt_thread_t)(tp+i)->sub_thread_base + j,
								sub_tp->sub_name,
								Execution_subtask,
								(void*)sub_tp,
								(void*)sub_tp->sub_stack_base,
								SUBTASK_STACK_SIZE,
								(tp+i)->Priority,
								10);
			if (err == RT_EOK)
			{
				rt_thread_control((rt_thread_t)(tp+i)->sub_thread_base + j, 
									RT_THREAD_CTRL_BIND_CPU, 
									(void *)(sub_tp->processor - 1));
			}else{
				rt_kprintf("init %s subthread failed\n",sub_tp->sub_name);
			}
			// rt_kprintf("%s init is: %d\n",((subtask_p)(tp+i)->sub_task_base + j)->sub_name
										 // ,((rt_thread_t)(tp+i)->sub_thread_base + j)->thread_timer.init_tick);
		}


		/*释放完一个DAG任务，确定下次DAG任务的释放时间（next release time insert to the list）*/
		current_list_p = self_timer_list; //从表头开始
		while(current_list_p->next != self_timer_list) //不是表的末尾，同时表也是非空的
		{
			current_list_p = current_list_p->next;  //下一个节点
			temp_tp = rt_list_entry(current_list_p, struct DAG_task, list); //得到当前节点的DAG结构体指针

			if((tp+i)->next_release_tick >= temp_tp->next_release_tick)
			{	//	刚释放的DAG下次释放时间小于或等于当前指向的节点，继续向下搜寻。
				continue;
			}else{//否则表示，当前指向的节点刚刚比刚释放完的节点大，插到他前面
				insert_after(&(tp+i)->list,current_list_p->prev);
				flag = 1;
				break;

			}
		}

		if (flag == 0)
		{//到了表末尾，还没插入，就直接插入表的末尾
			insert_after(&(tp+i)->list,current_list_p);
		}

	}
	/*查看下链表*/
	// current_list_p = self_timer_list;
	// while(current_list_p->next != self_timer_list)
	// {
	// 	current_list_p = current_list_p->next;  //下一个节点
	// 	temp_tp = rt_list_entry(current_list_p, struct DAG_task, list); //得到当前节点的DAG结构体指针
	// 	rt_kprintf("%s: next->%d ->\n", temp_tp->task_name, temp_tp->next_release_tick);
	// }
	for(int i = 0; i < tp->task_numb; i++)
	{
		for(int j = 0; j < tp->subtask_numb; j++)
		{
			rt_thread_startup((rt_thread_t)(tp+i)->sub_thread_base + j);
		}
	}

}


void DAG_Release(task_p tp)
{//本函数只判断上个释放的子任务是否完成，并重新释放任务。
	rt_event_t event_p;
	event_p = (rt_event_t)(tp->event_base) + tp->DAG_order;

	if ((tp-tp->DAG_order)->schedulable == 0)
	{
		rt_event_send((rt_event_t)(tp->over_flag) , (1 << 1));
		rt_thread_suspend(rt_thread_self());
		rt_schedule();
	}
	
	
	// rt_kprintf("\n%s release at: %d\n",tp->task_name,rt_tick_get());

    //判断该DAG任务上次释放的所有子任务是否完成执行
	for (int i = 0; i < tp->subtask_numb; i++)
	{
		if(((subtask_p)tp->sub_task_base + i)->finish == 0)
		{
			(tp-tp->DAG_order)->schedulable = 0;
			// rt_kprintf("\n%s is 0\n",((subtask_p)tp->sub_task_base + i)->sub_name);
			// rt_kprintf("c%d,n%d,now%d\n",tp->current_release_tick,tp->next_release_tick,rt_tick_get());
			tp->WCRT = rt_tick_get() - tp->current_release_tick;
			rt_event_send((rt_event_t)(tp->over_flag) , (1 << 1));
			rt_thread_suspend(rt_thread_self());
			rt_schedule();
		}
	}
	
	
	tp->current_release_tick = tp->next_release_tick;
	tp->next_release_tick = tp->current_release_tick + (rt_tick_t)(tp->T);

	event_p->set = 0;

	//恢复子任务的循环
	for (int i = 0; i < tp->subtask_numb; i++)
	{
		((subtask_p)tp->sub_task_base + i)->finish = 0;
		rt_thread_resume((rt_thread_t)(tp->sub_thread_base) + i);
	}		
	// rt_kprintf("\n%s finish release at: %d , next at: %d\n",tp->task_name,rt_tick_get(),tp->next_release_tick);	
}

void self_timer_scheduler(void *parameter)
{
	/*用来调度任务释放从而达到事件触发型周期释放*/
	task_p tp;
	tp = (task_p)parameter;

	rt_list_t *next_timeout_node;
	rt_list_t *list_head;
	list_head = tp->list_head_base;
	rt_list_t *current_list_p;

	task_p next_release_DAG, temp_tp;
	rt_tick_t  current_time;

	int delay;
	int count = 1, base_time = 500000; //count and base_timer 用来定时向外输出时间

	int flag = 0;

	while((int)(rt_tick_get() - tp->first_release_tick) < EXECUTION_LIMITATION)
	{
		current_time = rt_tick_get();
		next_timeout_node = list_head->next;//取到最近一次待释放的任务
		next_release_DAG = rt_list_entry(next_timeout_node,struct DAG_task, list); //得到下次释放的DAG结构体

		delay = (int)next_release_DAG->next_release_tick - (int)current_time;

		/*向外输出时间！*/
		// int total_WCRT = 0;
		// if((int)current_time >= (count*base_time + tp->first_release_tick))
		// {
		// 	rt_kprintf(" %d-%d: \n",current_time,count);
		// 	for (int j = 0; j < tp->task_numb; j++)
		// 	{
		// 		rt_kprintf("-%d-",(tp+j)->WCRT);
		// 	}
		// 	rt_kprintf("\n");
		// 	for (int j = 0; j < tp->task_numb; j++)
		// 	{
		// 		rt_kprintf("-%d-",(tp+j)->D);
		// 	}
		// 	rt_kprintf("\n");
		// 	/* WCRT*/
		// 	for (int j = 0; j < tp->task_numb; j++)
		// 	{
		// 		total_WCRT += (tp+j)->WCRT;
		// 	}
		// 	rt_kprintf("\n");

		// 	rt_kprintf("*** %d\n", total_WCRT);
		// 	count++;
		// }

		if ((0 - delay) >= (int)next_release_DAG->next_release_tick && delay < 0)
		{
			rt_kprintf("delay %d, now: %d, next: %d", delay, rt_tick_get(),next_release_DAG->next_release_tick);
			rt_event_send((rt_event_t)(tp->over_flag) , (1 << 2));
			rt_thread_suspend(rt_thread_self());
			rt_schedule();
		}
		// 	/*查看下链表*/
		// current_list_p = list_head;
		// while(current_list_p->next != list_head)
		// {
		// 	current_list_p = current_list_p->next;  //下一个节点
		// 	temp_tp = rt_list_entry(current_list_p, struct DAG_task, list); //得到当前节点的DAG结构体指针
		// 	rt_kprintf("%s: next->%d ->\n", temp_tp->task_name, temp_tp->next_release_tick);
		// }

		if ((int)delay <= 0) 
		{
			//到时间了，开始释放新的任务
			remove_list(&next_release_DAG->list); //先从列表中移除
			// rt_kprintf("%d",rt_tick_get());
			// if (rt_strncmp(next_release_DAG->task_name,"DAG_0",5) == 0)
			// {
			// 	rt_kprintf("\n %s, %d, %d,%d\n",next_release_DAG->task_name, rt_tick_get(),next_release_DAG->WCRT,next_release_DAG->T);
			// }
			DAG_Release(next_release_DAG);

			flag = 0;
			//释放完毕开始计算下次释放时间，并插入self_timer队列
			current_list_p = list_head; //从表头开始
			while(current_list_p->next != list_head) //不是表的末尾，同时表也是非空的
			{
				current_list_p = current_list_p->next;  //下一个节点
				temp_tp = rt_list_entry(current_list_p, struct DAG_task, list); //得到当前节点的DAG结构体指针

				if(next_release_DAG->next_release_tick >= temp_tp->next_release_tick)
				{	//	刚释放的DAG下次释放时间小于或等于当前指向的节点，继续向下搜寻。
					continue;
				}else{//否则表示，当前指向的节点刚刚比刚释放完的节点大，插到他前面
					insert_after(&next_release_DAG->list,current_list_p->prev);
					flag = 1;
					break;

				}
			}

			if (flag == 0)
			{//到了表末尾，还没插入，就直接插入表的末尾
				insert_after(&next_release_DAG->list,current_list_p);
			}

		}else{
			rt_thread_delay(next_release_DAG->next_release_tick - current_time);
			continue;
		}
	}
	rt_event_send((rt_event_t)(tp->over_flag) , (1 << 3));
	rt_thread_suspend(rt_thread_self());
	rt_schedule();
}


