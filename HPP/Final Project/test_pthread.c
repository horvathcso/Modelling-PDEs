#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
    
struct arg_struct
{
    int* table;
    int N;
    int* head;
    int idx;
    int* idxs;
};

pthread_mutex_t lock;
int solve_table(int* __restrict table, int N, int* __restrict head, int idx, int * idxs);

static inline double get_wall_seconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
  return seconds;
}

static inline void read_data(int* data, const char* filename, int NN){
    FILE *file;
	file=fopen(filename,"rb");
	fread(data,(2*NN*NN+1)*sizeof(int),1,file)!=0;
	fclose(file);
}

static inline void cpy_table(int* t1, int*t2, int NN){
    for (int i = 0; i < NN*NN; i++)
        t2[i]=t1[i];
}

static inline void print_table(int* table, int NN)
{
    int i,j;
    for(i = 0; i < NN; i++){
        for(j=0; j < NN; j++)
            if(table[i*NN+j]==0){
                printf(" & ");}
            else 
                printf("%d& ",table[i*NN+j]);
        printf("\n");}
}

static inline int check(int* table, int N){
    int NN=N*N;
    
    for (int i = 0; i < NN; i++)
        for (int j = 0; j < NN; j++)
                for (int k = j; k < NN; k++) // row column
                {
                    if (table[i*NN+j]!=0 && table[i*NN+j]==table[i*NN+k] && j!=k)
                        return 0;
                    if(table[j*NN+i] !=0 && table[j*NN+i]==table[k*NN+i] && j!=k)
                        return 0;
                }      
    //block validation
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                for (int l = k+1; l < N; l++)
                {
                    if (table[N*(i*N+k/N)+(j*N+k%N)]!=0 && table[N*(i*N+k/N)+(j*N+k%N)]==table[N*(i*N+l/N)+(j*N+l%N)])
                    {return 0;}   
                }   
    return 1;
}

int validate_table(int* table, int id, int N){
    int NN=N*N;
    int x = id/(NN); int y = id%NN;

    for(int i=0;i<NN;i++) // row col
        if((table[x*NN+y] == table[i*NN+y] && x != i) || (table[x*NN+y] == table[x*NN+i] && y != i))
            return 0;

    int xn=x%N; int yn=y%N;
    for(int i=-(xn); i<N-(xn);i++)
        for (int j = -(yn); j  < N-(yn); j++)
        {
            if(table[x*NN+y] == table[(x+i)*NN+(y+j)] && (i!=0 || j!=0))
                return 0;
        }
    return 1;
}

int solution=0; int nr_thread=0; int * final_table;
void* thread_func(void* arg){   
    struct arg_struct *args = (struct arg_struct *)arg;
    solve_table(args->table,args->N,args->head,(args->idx)-1, (args->idxs));
    
    pthread_mutex_lock (&lock);
    nr_thread--;
    pthread_mutex_unlock (&lock);
    pthread_exit(NULL);
}


int solve_table(int* __restrict table, int N, int* __restrict head, int idx, int* idxs){
        if(idx==0){
            final_table=(int*)malloc(N*N*N*N*sizeof(int));
            cpy_table(table,final_table,N*N);
            solution=1;
            return 1;
        }
        if(solution==1){
            pthread_exit(NULL);
        }

        int id=head[idx-1];

        if(nr_thread<8)
        { 
            pthread_t threads[N*N]; 
            int cnt=0;
            int * table_cpy[N*N];
            struct arg_struct args[N*N];
            for(int i=1; i<N*N+1;i++){
                table[id]=i;
                if (validate_table(table, id, N) && solution==0){
                    
                    table_cpy[cnt] = (int*)malloc(N*N*N*N*sizeof(int));
                    cpy_table(table,table_cpy[cnt], N*N);
                    args[cnt].table=table_cpy[cnt]; args[cnt].N=N; args[cnt].head=head; args[cnt].idx=idx; args[cnt].idxs=idxs;
                    pthread_create(&threads[cnt], NULL, thread_func, (void *)&args[cnt]);
                    cnt++;
                    pthread_mutex_lock (&lock);
                        nr_thread++;
                    pthread_mutex_unlock (&lock);
                }
               
            }
            for(int t=0 ; t<cnt; t++){
                pthread_join(threads[t],NULL);
                free(table_cpy[t]);}
            
            if(solution==1) 
                return 1;
            else{
                table[id]=0;
                return 0;
            }   
        }
        else{ //serial
            for(int i=1; i<N*N+1;i++){
                table[id]=i;
                if (validate_table(table, id, N))
                    solve_table(table, N, head, idx-1, idxs);
                    if(solution==1){
                        return 1;
                    }
            }
            if(solution==1)
                return 1;
            table[id]=0;
            return 0;
        }
}


int main(int argc, char* argv[]) {
    pthread_mutex_init(&lock,NULL);
    // read args
    if(argc != 3) {
        printf("Please give 2 argument: N (if table side length is N*N), test_file (binary starting data for problem size N).\n");
        return -1;
    }
    const int N = atoi(argv[1]);
    const int NN=N*N;
	const char *filename=argv[2];
    
    // Init data
    int* data=(int*)malloc((2*NN*NN+1)*sizeof(double));
    read_data(data, filename, NN);    
    int* table=data;
    int* head=data+NN*NN;
    int idx=head[NN*NN];
    if(!check(table,N)){
        printf("Invalid table to start!");
    }
    else{
    // Run test
    int* idxs=(int*)malloc(idx*sizeof(int));
    double start=get_wall_seconds();
            solve_table(table, N, head, idx, idxs);  
            if(solution==1)
                table=final_table;
    double end=get_wall_seconds();
   

    // Check result
    if (check(table, N))
    {
        printf("%lf\n",(end-start));
    }
    else{
        //print_table(table,NN);
        printf("Generation failed");
        printf("%lf\n",(end-start));
    }
    }
    pthread_mutex_destroy(&lock);
    free(data);
}