#include<clone_threads.h>
#include<entry.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<mmap.h>
#include<fork.h>
#include<page.h>

extern int destroy_user_mappings(struct exec_context *ctx);

static void set_kstack_of_thread(struct exec_context *ctx)
{
	ctx->os_stack_pfn = os_pfn_alloc(OS_PT_REG);
	ctx->os_rsp = (((u64) ctx->os_stack_pfn) << PAGE_SHIFT) + PAGE_SIZE;
	stats->num_processes++;
	ctx->type = EXEC_CTX_USER_TH;	
}

//XXX Do not modify anything above this line

/*
   system call handler for clone, create thread like execution contexts
   */
long do_clone(void *th_func, void *user_stack, void *user_arg) 
{
	int ctr;
	struct exec_context *new_ctx = get_new_ctx();  //This is to be used for the newly created thread
	struct exec_context *ctx = get_current_ctx();
	u32 pid = new_ctx->pid;
	struct thread *n_thread;

	// 	printk("Thread is created with pid: %d, tpid: %d\n", ctx->pid, new_ctx->pid)	;

	if(!ctx->ctx_threads){  // This is the first thread
		ctx->ctx_threads = os_alloc(sizeof(struct ctx_thread_info));
		bzero((char *)ctx->ctx_threads, sizeof(struct ctx_thread_info));
		ctx->ctx_threads->pid = ctx->pid;
	}

	/* XXX Do not modify anything above. Your implementation goes here */

	// TODO your code goes here


	n_thread = find_unused_thread(ctx);


	if(n_thread == NULL) return -1;

	*new_ctx = *ctx;
	new_ctx->pid = pid;
	new_ctx->ppid = ctx->pid;
	new_ctx->type = EXEC_CTX_USER_TH;
	new_ctx->ctx_threads = NULL;
	new_ctx->ticks_to_sleep = ctx->ticks_to_sleep;
	new_ctx->pending_signal_bitmap = ctx->pending_signal_bitmap;
	new_ctx->alarm_config_time = ctx->alarm_config_time;
	new_ctx->ticks_to_alarm = ctx->ticks_to_alarm; 

	new_ctx->lock = ctx->lock;	


	for(int j = 0; j < MAX_SIGNALS; j++) 
		new_ctx->sighandlers[j] = ctx->sighandlers[j];

	for(int i = 0; i < MAX_MM_SEGS; i++)
		new_ctx->mms[i] = ctx->mms[i];


	new_ctx->regs = ctx->regs;
	
	

	new_ctx->regs.entry_rip = (u64) th_func;
	new_ctx->regs.rbp = (u64) user_stack;
	new_ctx->regs.entry_rsp = (u64) user_stack;	
	new_ctx->regs.rdi = (u64) user_arg;
	// copy files

	for(int i = 0; i < MAX_OPEN_FILES; i++) 
		new_ctx->files[i] = ctx->files[i];

	new_ctx->vm_area = ctx->vm_area;
	new_ctx->pgd = ctx->pgd;


	// Thread info
	n_thread->pid = pid;
	n_thread->status = TH_USED;
	n_thread->parent_ctx = ctx;

	 

	new_ctx->state = READY;


	//End of your logic

	//XXX The following two lines should be there. 

	set_kstack_of_thread(new_ctx);  //Allocate kstack for the thread
	


	return pid;
}



//handler for exit()
void do_exit(u8 normal)
{

	struct exec_context* ctx = get_current_ctx();

	if(ctx->type == EXEC_CTX_USER_TH) {
		dprintk("Thread is exiting with pid: %d - ppid: %d\n", ctx->pid, ctx->ppid);	
		handle_thread_exit(ctx, normal);
	}
	else {
		cleanup_all_threads(ctx);
		destroy_user_mappings(ctx);
		do_vma_exit(ctx);
		do_file_exit(ctx);  



		// cleanup of this process

		if(!put_pfn(ctx->pgd)) 
			os_pfn_free(OS_PT_REG, ctx->pgd);   //XXX Now its fine as it is a single core system
		if(!put_pfn(ctx->os_stack_pfn))
			os_pfn_free(OS_PT_REG, ctx->os_stack_pfn);
	}
	release_context(ctx); 
	struct exec_context* new_ctx = pick_next_context(ctx);


	// printk("Scheduling %s:%d  state of ctx %d\n", new_ctx->name, new_ctx->pid, new_ctx->state);


	schedule(new_ctx);
	


	return;
}

// XXX Reference implementation for a process exit
// You can refer this to implement your version of do_exit
/*
   void do_exit(u8 normal) 
   {
   int ctr;
   struct exec_context *ctx = get_current_ctx();
   struct exec_context *new_ctx;


   do_file_exit(ctx);   // Cleanup the files

// cleanup of this process
destroy_user_mappings(ctx); 
do_vma_exit(ctx);
if(!put_pfn(ctx->pgd)) 
os_pfn_free(OS_PT_REG, ctx->pgd);   //XXX Now its fine as it is a single core system
if(!put_pfn(ctx->os_stack_pfn))
os_pfn_free(OS_PT_REG, ctx->os_stack_pfn);
release_context(ctx); 
new_ctx = pick_next_context(ctx);
dprintk("Scheduling %s:%d [ptr = %x]\n", new_ctx->name, new_ctx->pid, new_ctx); 
schedule(new_ctx);  //Calling from exit
}
*/



////////////////////////////////////////////////////////// Semaphore implementation ////////////////////////////////////////////////////
//
//


// A spin lock implementation using cmpxchg
// XXX you can use it for implementing the semaphore
// Do not modify this code

static void spin_init(struct spinlock *spinlock)
{
	spinlock->value = 0;
}

static void spin_lock(struct spinlock *spinlock)
{
	unsigned long *addr = &(spinlock->value);

	asm volatile(
			"mov $1,  %%rcx;"
			"mov %0,  %%rdi;"
			"try: xor %%rax, %%rax;"
			"lock cmpxchg %%rcx, (%%rdi);"
			"jnz try;"
			:
			: "r"(addr)
			: "rcx", "rdi", "rax", "memory"
		    );
}

static void spin_unlock(struct spinlock *spinlock)
{
	spinlock->value = 0;
}

static int init_sem_metadata_in_context(struct exec_context *ctx)
{
	if(ctx->lock){
		printk("Already initialized MD. Call only for the first time\n");
		return -1;
	}
	ctx->lock = (struct lock*) os_alloc(sizeof(struct lock) * MAX_LOCKS);
	if(ctx->lock == NULL){
		printk("[pid: %d]BUG: Out of memory!\n", ctx->pid);
		return -1;
	}

	for(int i=0; i<MAX_LOCKS; i++)
		ctx->lock[i].state = LOCK_UNUSED;
}

// XXX Do not modify anything above this line

/*
   system call handler for semaphore creation
   */
int do_sem_init(struct exec_context *current, sem_t *sem_id, int value)
{



        if(current->lock == NULL)
                init_sem_metadata_in_context(current);



	for(int i = 0; i< MAX_LOCKS; i++) {

		if(current->lock[i].state == LOCK_UNUSED) {
			current->lock[i].sem.value = value;
			current->lock[i].sem.wait_queue = NULL;
			spin_init(&current->lock[i].sem.lock);
			current->lock[i].id = (u64) sem_id;
			current->lock[i].state = LOCK_USED;

			return 0;
		}


	}

	return -1;





	if(current->lock == NULL)
		init_sem_metadata_in_context(current);
	// TODO Your implementation goes here


	struct lock* free_lock; int f = 0;

	for(int i = 0; i < MAX_LOCKS; i++) {
		free_lock = current->lock + i;
		if(free_lock->state == LOCK_UNUSED) { f = 1;  break;}
	}	


	if(!f) 
		return -1;

	struct semaphore sem;

	sem.value = value;
	sem.wait_queue = NULL;
	spin_init(&sem.lock);


	free_lock->sem = sem;
	free_lock->id = (u64) sem_id; 
	free_lock->state = LOCK_USED;


	return 0;
}

int debug = 0;
/*
   system call handler for semaphore acquire
   */

int do_sem_wait(struct exec_context *current, sem_t *sem_id)
{ 



	for(int i = 0; i < MAX_LOCKS; i++) {


		if(current->lock[i].id == (u64)sem_id) {

			        spin_lock(&(current->lock[i].sem.lock));

				if(current->lock->sem.value > 0) {

					current->lock[i].sem.value--;
					spin_unlock(&(current->lock[i].sem.lock));

					return 0;
				}


				 struct exec_context* temp = current->lock[i].sem.wait_queue;

  		              if(temp == NULL) {
                		        current->lock[i].sem.wait_queue = current;
 		                       temp = current;
                		        current->state = WAITING;
                     			   current->next = NULL;
                     		   struct exec_context* new_ctx = pick_next_context(current);
                 		       spin_unlock(&(current->lock[i].sem.lock));

                    		    schedule(new_ctx);
                        		return 0;
                		}	

                	while(temp->next) {
                        	temp = temp->next;
                	}
               		 
                	current->state = WAITING;
			temp->next = current;
                        current->next = NULL;

                	struct exec_context* new_ctx = pick_next_context(current);


                	spin_unlock(&(current->lock[i].sem.lock));

                	schedule(new_ctx);
                	return 0;



		}





	}




return -1;















	// dprintk("inside %s:%u | pid %u | sem_id %x\n", __func__, __LINE__, current->pid, sem_id);
	struct lock* Lock = NULL; int f = 0;
	for(int i = 0; i < MAX_LOCKS; i++) {
		Lock = current->lock + i;
		if(Lock->id == (u64)sem_id) { f = 1; break; }
	}

	if(!f) return -1;

	if (debug)
		printk("Sem waIT IS Called by %d semid : %d val: %d\n", current->pid, *sem_id, Lock->sem.value);


	// dprintk("inside %s:%u | pid %u | sem_id %x\n", __func__, __LINE__, current->pid, sem_id);

	spin_lock(&Lock->sem.lock);
	

	if(Lock->sem.value > 0)
		Lock->sem.value--;
	else {

		// printk("Sem waIT IS Called by %d", current->pid);
		//	
		struct exec_context* temp = Lock->sem.wait_queue;

		if(temp == NULL) {
			printk("Hello\n");
			temp = current;
			current->state = WAITING;
			current->next = NULL;
			struct exec_context* new_ctx = pick_next_context(current);
			spin_unlock(&Lock->sem.lock);
//			printk("unlocked2\n");
			schedule(new_ctx);
			return 0;
		}

		while(temp->next) { 
//			printk("fnweru\n");
			temp = temp->next;
		}
		temp->next = current;
		current->next = NULL;
		current->state = WAITING;
		struct exec_context* new_ctx = pick_next_context(current);
		

		spin_unlock(&Lock->sem.lock);

//		printk("unlocked3\n");	
		schedule(new_ctx);
		return 0;
	}

	spin_unlock(&Lock->sem.lock); 
//	printk("unlocked4\n");

	return 0;
}

/*
   system call handler for semaphore release
   */
int do_sem_post(struct exec_context *current, sem_t *sem_id)
{	
	

	for(int i = 0; i < MAX_LOCKS; i++) {

		if(current->lock[i].id == (u64) sem_id) {

			if(current->lock[i].sem.wait_queue == NULL) {

				current->lock[i].sem.value++;
			}
			else {

				struct exec_context* h = current->lock[i].sem.wait_queue;
				current->lock[i].sem.wait_queue = h->next;
				h->state = READY;
			//	   struct exec_context* new_ctx = pick_next_context(current);
		         //       schedule(new_ctx);



			}


			return 0;
		}


	}

	return -1;









	struct lock* Lock = NULL; int f = 0;
	for(int i = 0; i < MAX_LOCKS; i++) {
		Lock = current->lock + i;
		if(Lock->id == (u64)sem_id)  { f = 1; break; }
	}

	if(!f) return -1;

	if(Lock->sem.wait_queue == NULL) {
		Lock->sem.value++;
	}
	else {	

		struct exec_context* head = Lock->sem.wait_queue;	

		Lock->sem.wait_queue = head->next;
		head->state = READY;
		struct exec_context* new_ctx = pick_next_context(current);
		schedule(new_ctx);

	}
  	
	
	return 0;
}
