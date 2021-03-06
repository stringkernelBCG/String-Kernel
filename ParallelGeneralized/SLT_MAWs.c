#include"stdlib.h"
#include"stdio.h"
#include"SLT_MAWs.h"
#include <string.h>
#include"SLT.h"
#include <math.h>
#include "basic_bitvec.h"


#define alloc_growth_num 4
#define alloc_growth_denom 3
static unsigned int length1;
static unsigned int length2;
static unsigned int F1;
static unsigned int F2;


typedef struct 
{
	unsigned int minlen;
	unsigned int nMAW_capacity;
	unsigned char * MAW_buffer;
	unsigned int MAW_buffer_idx;
	unsigned int char_stack_capacity;
	unsigned char * char_stack;
	unsigned int nMAWs;
	unsigned int nMAWs1;
	unsigned int * bitvec_capacity;
	unsigned int ** bitvec1;
	unsigned int nMAWs2;
	double LW;
	unsigned int pos;
	double N;
	double D1;
	double D2;
	double * prefix_sum1;
	double * prefix_sum2;
	double * prefix_sumN;
	unsigned int prefix_capacity;
	double * KL;
	unsigned int KL_capacity;

	FILE *file;

} MAWs_callback_state_t;


static inline unsigned int is_powof2(unsigned int x)
{
	return ((x&(x-1))==0);

};
static unsigned char alpha4_to_ACGT[4]={'A','C','G','T'};


void SLT_callback_MAWs(const SLT_joint_params_t * SLT_joint_params,void * intern_state, unsigned int mem)
{
	MAWs_callback_state_t * state= (MAWs_callback_state_t*)(intern_state);
	unsigned int char_mask1;
	unsigned int char_mask2;
	unsigned int i,j,k,h;

	if(state->nMAW_capacity==0 && mem) {
		state->nMAW_capacity=1<<16;
		state->MAW_buffer=(unsigned char *) malloc(state->nMAW_capacity);
	}
	if(SLT_joint_params->string_depth!=0)
	{
		if(SLT_joint_params->string_depth>state->char_stack_capacity)
		{
			state->char_stack_capacity=(state->char_stack_capacity+1)*alloc_growth_num/alloc_growth_denom;
			state->char_stack=(unsigned char *)realloc(state->char_stack,state->char_stack_capacity);
		};
		state->char_stack[SLT_joint_params->string_depth-1]=SLT_joint_params->WL_char;
	}

	// Check that we are at a maximal repeat of length at least minlen-2
	if(SLT_joint_params->string_depth + 2 < state->minlen ||
			((SLT_joint_params->nleft_extensions1 < 2 || SLT_joint_params->nright_extensions1 < 2) &&
					(SLT_joint_params->nleft_extensions2 < 2 || SLT_joint_params->nright_extensions2 < 2)))
		return;

	double x=(double)1/((SLT_joint_params->string_depth+2)*(SLT_joint_params->string_depth+2));
	char_mask1=1;
	for(i=1;i<5;i++) {
		char_mask1<<=1;
		char_mask2=1;
		for(j=1;j<5;j++) {
			char_mask2<<=1;
			if((SLT_joint_params->right_extension_bitmap1&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap1&char_mask1)
					&& SLT_joint_params->left_right_extension_freqs1[i][j]==0) {
				// We have a MAW. Write it to the output
				state->nMAWs1++;
				state->LW+=x;
			}
			if((SLT_joint_params->right_extension_bitmap2&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap2&char_mask1)
					&& SLT_joint_params->left_right_extension_freqs2[i][j]==0) {
				// We have a MAW. Write it to the output
				state->nMAWs2++;
				state->LW+=x;
			}
			if(((SLT_joint_params->right_extension_bitmap1&char_mask2)
					&& (SLT_joint_params->right_extension_bitmap2&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap1&char_mask1)
					&& (SLT_joint_params->left_extension_bitmap2&char_mask1))
					&& (SLT_joint_params->left_right_extension_freqs1[i][j]==0
							&& SLT_joint_params->left_right_extension_freqs2[i][j]==0)) {
				// We have a MAW. Write it to the output
				state->nMAWs++;
				state->LW-=2*x;
				if(mem) {
					if(state->MAW_buffer_idx+SLT_joint_params->string_depth+3>state->nMAW_capacity) {
						#pragma omp critical
						fwrite(state->MAW_buffer, state->MAW_buffer_idx, sizeof(char), state->file);
						state->MAW_buffer_idx=0;
					}
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[i-1];
					for(k=0;k<SLT_joint_params->string_depth;k++){
						if (state->char_stack[SLT_joint_params->string_depth-k-1]-1 > 3)
							printf("%d ", state->char_stack[SLT_joint_params->string_depth-k-1]-1);
						state->MAW_buffer[state->MAW_buffer_idx++]=
								alpha4_to_ACGT[state->char_stack[SLT_joint_params->string_depth-k-1]-1];
					}
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[j-1];
					state->MAW_buffer[state->MAW_buffer_idx++]='\n';
				}
			}
		}
	}

}
void SLT_callback_RWs(const SLT_joint_params_t * SLT_joint_params,void * intern_state, unsigned int mem)
{
	MAWs_callback_state_t * state= (MAWs_callback_state_t*)(intern_state);
	unsigned int char_mask1;
	unsigned int char_mask2;
	unsigned int i,j,k,h;

	if(state->nMAW_capacity==0 && mem) {
		state->nMAW_capacity=1<<16;
		state->MAW_buffer=(unsigned char *) malloc(state->nMAW_capacity);
	}
	if(SLT_joint_params->string_depth!=0)
	{
		if(SLT_joint_params->string_depth>state->char_stack_capacity)
		{
			state->char_stack_capacity=(state->char_stack_capacity+1)*alloc_growth_num/alloc_growth_denom;
			state->char_stack=(unsigned char *)realloc(state->char_stack,state->char_stack_capacity);
		};
		state->char_stack[SLT_joint_params->string_depth-1]=SLT_joint_params->WL_char;
	}

	// Check that we are at a maximal repeat of length at least minlen-2
	if(SLT_joint_params->string_depth + 2 < state->minlen ||
			((SLT_joint_params->nleft_extensions1 < 2 || SLT_joint_params->nright_extensions1 < 2) &&
					(SLT_joint_params->nleft_extensions2 < 2 || SLT_joint_params->nright_extensions2 < 2)))
		return;

	char_mask1=1;
	for(i=1;i<5;i++) {
		char_mask1<<=1;
		char_mask2=1;
		for(j=1;j<5;j++) {
			char_mask2<<=1;
			unsigned int freqLeft1= 0;
			unsigned int freqRight1= 0;
			unsigned int freqLeft2= 0;
			unsigned int freqRight2= 0;
			for(k=1; k<5; k++) {
				freqLeft1+= SLT_joint_params->left_right_extension_freqs1[i][k];
				freqLeft2+= SLT_joint_params->left_right_extension_freqs2[i][k];
				freqRight1+= SLT_joint_params->left_right_extension_freqs1[k][j];
				freqRight2+= SLT_joint_params->left_right_extension_freqs2[k][j];
			}

			if(freqLeft1 >= F2 && freqRight1 >= F2
					&& SLT_joint_params->left_right_extension_freqs1[i][j]<=F1) {
				// We have a MAW. Write it to the output
				state->nMAWs1++;
			}
			if(freqLeft2 >= F2 && freqRight2 >= F2
					&& SLT_joint_params->left_right_extension_freqs2[i][j]<=F1) {
				// We have a MAW. Write it to the output
				state->nMAWs2++;
			}
			if(freqLeft1 >= F2 && freqRight1 >= F2
					&& freqLeft2 >= F2 && freqRight2 >= F2
					&& SLT_joint_params->left_right_extension_freqs1[i][j]<=F1
							&& SLT_joint_params->left_right_extension_freqs2[i][j]<=F1) {
				// We have a MAW. Write it to the output
				state->nMAWs++;
				if(mem) {
					if(state->MAW_buffer_idx+SLT_joint_params->string_depth+3>state->nMAW_capacity) {
						#pragma omp critical
						fwrite(state->MAW_buffer, state->MAW_buffer_idx, sizeof(char), state->file);
						state->MAW_buffer_idx=0;
					}
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[i-1];
					for(k=0;k<SLT_joint_params->string_depth;k++)
						state->MAW_buffer[state->MAW_buffer_idx++]=
								alpha4_to_ACGT[state->char_stack[SLT_joint_params->string_depth-k-1]-1];
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[j-1];
					state->MAW_buffer[state->MAW_buffer_idx++]='\n';
				}
			}
		}
	}

}

void SLT_callback_MAWs_present(const SLT_joint_params_t * SLT_joint_params,void * intern_state, unsigned int mem)
{
	MAWs_callback_state_t * state= (MAWs_callback_state_t*)(intern_state);
	unsigned int char_mask1;
	unsigned int char_mask2;
	unsigned int i,j,k,h;

	if(state->nMAW_capacity==0 && mem) {
		state->nMAW_capacity=1<<16;
		state->MAW_buffer=(unsigned char *) malloc(state->nMAW_capacity);
	}
	if(SLT_joint_params->string_depth!=0)
	{
		if(SLT_joint_params->string_depth>state->char_stack_capacity)
		{
			state->char_stack_capacity=(state->char_stack_capacity+1)*alloc_growth_num/alloc_growth_denom;
			state->char_stack=(unsigned char *)realloc(state->char_stack,state->char_stack_capacity);
		};
		state->char_stack[SLT_joint_params->string_depth-1]=SLT_joint_params->WL_char;
	}

	// Check that we are at a maximal repeat of length at least minlen-2
	if(SLT_joint_params->string_depth + 2 < state->minlen ||
			((SLT_joint_params->nleft_extensions1 < 2 || SLT_joint_params->nright_extensions1 < 2) &&
					(SLT_joint_params->nleft_extensions2 < 2 || SLT_joint_params->nright_extensions2 < 2)))
		return;

	char_mask1=1;
	for(i=1;i<5;i++) {
		char_mask2=1;
		for(j=1;j<5;j++) {
			if(((SLT_joint_params->right_extension_bitmap1&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap1&char_mask1)
					&& SLT_joint_params->left_right_extension_freqs1[i][j]==0
					&& SLT_joint_params->left_right_extension_freqs2[i][j]!=0)) {
				// We have a MAW. Write it to the output
				state->nMAWs++;
				if(mem) {
					if(state->MAW_buffer_idx+SLT_joint_params->string_depth+3>state->nMAW_capacity) {
#pragma omp critical
						fwrite(state->MAW_buffer, state->MAW_buffer_idx, sizeof(char), state->file);
						state->MAW_buffer_idx=0;
					}
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[i-1];
					for(k=0;k<SLT_joint_params->string_depth;k++)
						state->MAW_buffer[state->MAW_buffer_idx++]=
								alpha4_to_ACGT[state->char_stack[SLT_joint_params->string_depth-k-1]-1];
					state->MAW_buffer[state->MAW_buffer_idx++]=alpha4_to_ACGT[j-1];
					state->MAW_buffer[state->MAW_buffer_idx++]='\n';
				}
			}
		char_mask2<<=1;
		}
	char_mask1<<=1;
	}

}

void SLT_callback_kernel(const SLT_joint_params_t * SLT_joint_params,void * intern_state, unsigned int mem)
{
	MAWs_callback_state_t * state= (MAWs_callback_state_t*)(intern_state);
	unsigned int char_mask1;
	unsigned int char_mask2;
	unsigned int i,j,k,h;
	int N=0;
	int d1=0;
	int d2=0;

	if(state->nMAW_capacity==0 && mem) {
		state->nMAW_capacity=1<<16;
		state->MAW_buffer=(unsigned char *) malloc(state->nMAW_capacity);
	}
	if(SLT_joint_params->string_depth!=0)
	{
		if(SLT_joint_params->string_depth>state->char_stack_capacity)
		{
			state->char_stack_capacity=(state->char_stack_capacity+1)*alloc_growth_num/alloc_growth_denom;
			state->char_stack=(unsigned char *)realloc(state->char_stack,state->char_stack_capacity);
		};
		state->char_stack[SLT_joint_params->string_depth-1]=SLT_joint_params->WL_char;
	}

	if((SLT_joint_params->string_depth+2)>=state->prefix_capacity)
	{
		state->prefix_capacity=(state->prefix_capacity+2)*alloc_growth_num/alloc_growth_denom;
		state->prefix_sum1=(double *)realloc(state->prefix_sum1,state->prefix_capacity*sizeof(double));
		state->prefix_sum2=(double *)realloc(state->prefix_sum2,state->prefix_capacity*sizeof(double));
		state->prefix_sumN=(double *)realloc(state->prefix_sumN,state->prefix_capacity*sizeof(double));
	};

	state->prefix_sum1[SLT_joint_params->string_depth+2]= state->prefix_sum1[SLT_joint_params->string_depth+1]+(g1(SLT_joint_params->string_depth+2)-1)*(g1(SLT_joint_params->string_depth+2)-1);
	state->prefix_sum2[SLT_joint_params->string_depth+2]= state->prefix_sum2[SLT_joint_params->string_depth+1]+(g2(SLT_joint_params->string_depth+2)-1)*(g2(SLT_joint_params->string_depth+2)-1);
	state->prefix_sumN[SLT_joint_params->string_depth+2]= state->prefix_sumN[SLT_joint_params->string_depth+1]+(g1(SLT_joint_params->string_depth+2)-1)*(g2(SLT_joint_params->string_depth+2)-1);

	for(i=0; i<5; i++) {
		N+= ((SLT_joint_params->left_extension_bitmap1&SLT_joint_params->left_extension_bitmap2&(1<<i))>>i);
		d1+= (SLT_joint_params->left_extension_bitmap1&(1<<i))>>i;
		d2+= (SLT_joint_params->left_extension_bitmap2&(1<<i))>>i;
	}

	double correction1= state->prefix_sum1[SLT_joint_params->string_depth+2];
	double correction2= state->prefix_sum2[SLT_joint_params->string_depth+2];
	//frequency
	unsigned int fw=0;
	unsigned int faw=0;
	unsigned int fwb=0;
	char_mask1=1;
	for(i=0;i<5;i++) {
		char_mask2=1;
		for(j=0;j<5;j++) {
			if((SLT_joint_params->right_extension_bitmap1&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap1&char_mask1)) {
				if (SLT_joint_params->left_right_extension_freqs1[i][j]==0) {
					if(i!=0 && j!=0) {
						// We have a MAW. Write it to the output
						correction1=-1; //correction for maw in t1 (aWb)
						state->D1++;
					}
				}
				else {
					d1--; //aW has a child
				}
				// if W is a max-rep -> correction for aWb
				if(SLT_joint_params->nleft_extensions1>1 && SLT_joint_params->nright_extensions1>1
						&& SLT_joint_params->left_right_extension_freqs1[i][j]!=0) {
					fw= 0;
					faw= 0;
					fwb= 0;
					for(h=0; h<5; h++) {
						faw+= SLT_joint_params->left_right_extension_freqs1[i][h];
						fwb+= SLT_joint_params->left_right_extension_freqs1[h][j];
						for(k=0; k<5; k++)
							fw+= SLT_joint_params->left_right_extension_freqs1[h][k];
					}
					//state->KL[SLT_joint_params->string_depth+2]+= SLT_joint_params->left_right_extension_freqs1[i][j]*
					//	(log(SLT_joint_params->left_right_extension_freqs1[i][j])-log((double)faw*fwb/fw));
					correction1= (g1(SLT_joint_params->string_depth+2)*fw/faw*SLT_joint_params->left_right_extension_freqs1[i][j]/fwb-1);
					state->D1+= correction1*correction1 - (g1(SLT_joint_params->string_depth+2)-1)*(g1(SLT_joint_params->string_depth+2)-1);
				}
			}
			if((SLT_joint_params->right_extension_bitmap2&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap2&char_mask1)) {
				if (SLT_joint_params->left_right_extension_freqs2[i][j]==0) {
					if(i!=0 && j!=0) {
						// We have a MAW. Write it to the output
						correction2=-1; //correction for maw in t2 (aWb)
						state->D2++;
					}
				}
				else {
					d2--;
				}
				// if W is a max-rep -> correction for aWb
				if(SLT_joint_params->nleft_extensions2>1 && SLT_joint_params->nright_extensions2>1
						&& SLT_joint_params->left_right_extension_freqs2[i][j]!=0) {
					//frequency
					fw=0;
					faw=0;
					fwb=0;
					for(h=0; h<5; h++) {
						faw+= SLT_joint_params->left_right_extension_freqs2[i][h];
						fwb+= SLT_joint_params->left_right_extension_freqs2[h][j];
						for(k=0; k<5; k++)
							fw+= SLT_joint_params->left_right_extension_freqs2[h][k];
					}
					correction2=(g2(SLT_joint_params->string_depth+2)*fw/faw*SLT_joint_params->left_right_extension_freqs2[i][j]/fwb-1);
					state->D2+= correction2*correction2 - (g2(SLT_joint_params->string_depth+2)-1)*(g2(SLT_joint_params->string_depth+2)-1);
				}
			}
			if(((SLT_joint_params->right_extension_bitmap1&char_mask2)
					&& (SLT_joint_params->right_extension_bitmap2&char_mask2)
					&& (SLT_joint_params->left_extension_bitmap1&char_mask1)
					&& (SLT_joint_params->left_extension_bitmap2&char_mask1))) {
				if ((SLT_joint_params->left_right_extension_freqs1[i][j]==0
						&& SLT_joint_params->left_right_extension_freqs2[i][j]==0)) {
					if(i!=0 && j!=0) {
						// We have a MAW. Write it to the output
						//correction for maw-maw
						state->N++;
					}
				}
				else
					//correction for N
					state->N+= correction1*correction2-(g1(SLT_joint_params->string_depth+2)-1)*(g2(SLT_joint_params->string_depth+2)-1);

				if ((SLT_joint_params->left_right_extension_freqs1[i][j]!=0
						&& SLT_joint_params->left_right_extension_freqs2[i][j]!=0)) {
					N--;
				}
			}
			char_mask2<<=1;
		}
		char_mask1<<=1;
	}
	state->D1+= d1*state->prefix_sum1[SLT_joint_params->string_depth+1];
	state->D2+= d2*state->prefix_sum2[SLT_joint_params->string_depth+1];
	state->N+= N*state->prefix_sumN[SLT_joint_params->string_depth+1];
}

void* SLT_cloner(void* p, unsigned int t){
	MAWs_callback_state_t* temp= malloc(sizeof(MAWs_callback_state_t));
	MAWs_callback_state_t* new_p= (MAWs_callback_state_t*) p;
	temp->minlen=new_p->minlen;
	temp->MAW_buffer=0;
	temp->nMAW_capacity=0;
	temp->MAW_buffer_idx=0;
	temp->char_stack_capacity=new_p->char_stack_capacity;
	temp->char_stack=(unsigned char *) malloc(new_p->char_stack_capacity);
	memcpy(temp->char_stack,new_p->char_stack,new_p->char_stack_capacity);
	temp->nMAWs=0;
	temp->nMAWs1=0;
	temp->nMAWs2=0;
	temp->LW=0;
	temp->pos=t;
	temp->file=new_p->file;
	temp->prefix_capacity= new_p->prefix_capacity;
	temp->D1=0;
	temp->D2=0;
	temp->N=0;
	temp->prefix_sum1= (double *)malloc(temp->prefix_capacity*sizeof(double));
	memcpy(temp->prefix_sum1, new_p->prefix_sum1,temp->prefix_capacity*sizeof(double));
	temp->prefix_sum2= (double *)malloc(temp->prefix_capacity*sizeof(double));
	memcpy(temp->prefix_sum2, new_p->prefix_sum2,temp->prefix_capacity*sizeof(double));
	temp->prefix_sumN= (double *)malloc(temp->prefix_capacity*sizeof(double));
	memcpy(temp->prefix_sumN, new_p->prefix_sumN,temp->prefix_capacity*sizeof(double));
	//if AAAA -> allocate! only 9
	// temp->bitvec1= (unsigned int *)malloc(36*8*sizeof(int));
	// temp->KL_capacity= new_p->KL_capacity;
	// temp->KL= (double *)malloc(temp->KL_capacity*sizeof(double));
	return temp;
};
void SLT_combiner(void** intern_state, void* state, unsigned int t,unsigned int mem) {
	unsigned int i,j;
	MAWs_callback_state_t** p=(MAWs_callback_state_t**)intern_state;
	MAWs_callback_state_t* s=(MAWs_callback_state_t*)state;
	for(i=0; i<t; i++) {
		s->nMAWs+=p[i]->nMAWs;
		s->nMAWs1+=p[i]->nMAWs1;
		s->nMAWs2+=p[i]->nMAWs2;
		s->LW+=p[i]->LW;
		s->D1+=p[i]->D1;
		s->D2+=p[i]->D2;
		s->N+=p[i]->N;
		// for(j= 2; j< s->KL_capacity; j++)
		// 	s->KL[j]+= p[i]->KL[j];
		// for(j= 0; j< 64; j++) {}
		// s->bitvec1[j]= s->bitvec1[j] | p[i]->bitvec1[j];
	}
	if (mem) {
		fclose(s->file);
	}
	free(intern_state);
};
void SLT_free(void* intern_state, unsigned int mem) {
	MAWs_callback_state_t* state= (MAWs_callback_state_t*) intern_state;
	if (mem) {
		fwrite(state->MAW_buffer, state->MAW_buffer_idx, sizeof(char), state->file);
		free(state->MAW_buffer);
	}
	free(state->char_stack);
	//free(state->KL);
}
double g1(int y) {
	return (double) (length1-y+2)/(length1-y+1)*(length1-y+2)/(length1-y+3);
}
double g2(int y) {
	return (double) (length2-y+2)/(length2-y+1)*(length2-y+2)/(length2-y+3);
}

unsigned int SLT_find_MAWs(Basic_BWT_t * BBWT1,Basic_BWT_t * BBWT2,
		unsigned int minlen, unsigned int * _nMAWs1,
		unsigned int * _nMAWs2, double * _output_result, unsigned int mem, unsigned int cores, unsigned int result)
{
	SLT_joint_iterator_t * SLT_iterator;
	MAWs_callback_state_t state;

	unsigned int i;
	state.nMAWs=0;
	state.nMAWs1=0;
	state.nMAWs2=0;
	state.MAW_buffer=0;
	state.MAW_buffer_idx=0;
	state.nMAW_capacity=0;
	state.minlen=minlen;
	state.LW=0;
	state.char_stack_capacity=4;
	state.char_stack=(unsigned char *) malloc(state.char_stack_capacity);
	FILE *f;
	if(mem) {
		f=fopen("output.txt", "a");
		state.file=f;
	}
	length1= BBWT1->textlen+2;
	length2= BBWT2->textlen+2;
	state.N=0;
	state.D1=0;
	state.D2=0;

	//Initializing D1 and D2
	double prefix_sum= 0;
	for(i=1; i<=BBWT1->textlen+2;i++) {
		prefix_sum+= (g1(i)-1)*(g1(i)-1);
		state.D1+=prefix_sum;
	}
	prefix_sum= 0;
	for(i=1; i<=BBWT2->textlen+2;i++){
		prefix_sum+= (g2(i)-1)*(g2(i)-1);
		state.D2+=prefix_sum;
	}

	//Prefix sum allocation and initialization
	state.prefix_capacity=4;
	state.prefix_sum1=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sum2=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sumN=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sum1[1]=(g1(1)-1)*(g1(1)-1);
	state.prefix_sum2[1]=(g2(1)-1)*(g2(1)-1);
	state.prefix_sumN[1]=(g1(1)-1)*(g2(1)-1);
	//resize!
	/*if(d> state.bitvec_sizes[i]){
		state.bitvec_sizes[i]*=2;
		state.bitvec[i]=realloc(state.bitvec[i],state.bitvec_sizes[i]/8);
		memset(state.bitvec[i]+state.bitvec_sizes[i]/16,0,state.bitvec_sizes[i]/16);
	state.bitvec_capacity=(unsigned int *) malloc(36*sizeof(unsigned int));
	}
	for(i=0;i<36;i++) state.bitvec_capacity[i]=32;
		state.bitvec1= (unsigned int **)malloc(36*sizeof(int));
	for(i=0;i<36;i++) {
		for(i=0;i<36;i++) state.bitvec1[i]=(unsigned int *) calloc(1,sizeof(unsigned int));
	}
	state.KL_capacity= BBWT1->textlen+1;
	state.KL= (double *) malloc(state.KL_capacity*sizeof(double));
	*/
	



	switch(result) {
	case 1:
		SLT_iterator=new_SLT_joint_iterator(SLT_callback_MAWs,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
		SLT_joint_execute_iterator(SLT_iterator);
		break;
	case 3:
		SLT_iterator=new_SLT_joint_iterator(SLT_callback_MAWs_present,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
		SLT_joint_execute_iterator(SLT_iterator);
		break;
	case 4:
		SLT_iterator=new_SLT_joint_iterator(SLT_callback_MAWs,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
		SLT_joint_execute_iterator(SLT_iterator);
		break;
	case 5:
		SLT_iterator=new_SLT_joint_iterator(SLT_callback_MAWs,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
		SLT_joint_execute_iterator(SLT_iterator);
		*_output_result= state.LW;
		break;
	case 6:

		SLT_iterator=new_SLT_joint_iterator(SLT_callback_kernel,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
		SLT_joint_execute_iterator(SLT_iterator);
		*_output_result= state.N/sqrt(state.D1*state.D2);
		printf("Markovian kernel: %f D1: %f, %f, %f \n", state.N/sqrt(state.D1*state.D2), state.D1, state.D2, state.N);
		break;


	}

	//printf("KL2: %f  KL3: %f\n", state.KL[2], state.KL[3]);

	*_nMAWs1=state.nMAWs1;
	*_nMAWs2=state.nMAWs2;
	return state.nMAWs;
};


unsigned int SLT_find_RWs(Basic_BWT_t * BBWT1,Basic_BWT_t * BBWT2,
		unsigned int minlen, unsigned int * _nMAWs1,
		unsigned int * _nMAWs2, double * _output_result, unsigned int mem, unsigned int cores, unsigned int result, unsigned int f1, unsigned int f2)
{
	SLT_joint_iterator_t * SLT_iterator;
	MAWs_callback_state_t state;

	unsigned int i;
	state.nMAWs=0;
	state.nMAWs1=0;
	state.nMAWs2=0;
	state.MAW_buffer=0;
	state.MAW_buffer_idx=0;
	state.nMAW_capacity=0;
	state.minlen=minlen;
	state.LW=0;
	state.char_stack_capacity=4;
	state.char_stack=(unsigned char *) malloc(state.char_stack_capacity);
	FILE *f;
	if(mem) {
		f=fopen("output.txt", "a");
		state.file=f;
	}
	length1= BBWT1->textlen+2;
	length2= BBWT2->textlen+2;
	F1= f1;
	F2= f2;
	state.N=0;
	state.D1=0;
	state.D2=0;

	//Initializing D1 and D2
	double prefix_sum= 0;
	for(i=1; i<=BBWT1->textlen+2;i++) {
		prefix_sum+= (g1(i)-1)*(g1(i)-1);
		state.D1+=prefix_sum;
	}
	prefix_sum= 0;
	for(i=1; i<=BBWT2->textlen+2;i++){
		prefix_sum+= (g2(i)-1)*(g2(i)-1);
		state.D2+=prefix_sum;
	}

	//Prefix sum allocation and initialization
	state.prefix_capacity=4;
	state.prefix_sum1=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sum2=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sumN=(double *)malloc(state.prefix_capacity*sizeof(double));
	state.prefix_sum1[1]=(g1(1)-1)*(g1(1)-1);
	state.prefix_sum2[1]=(g2(1)-1)*(g2(1)-1);
	state.prefix_sumN[1]=(g1(1)-1)*(g2(1)-1);
	//resize!
	/*if(d> state.bitvec_sizes[i]){
		state.bitvec_sizes[i]*=2;
		state.bitvec[i]=realloc(state.bitvec[i],state.bitvec_sizes[i]/8);
		memset(state.bitvec[i]+state.bitvec_sizes[i]/16,0,state.bitvec_sizes[i]/16);
	state.bitvec_capacity=(unsigned int *) malloc(36*sizeof(unsigned int));
	}
	for(i=0;i<36;i++) state.bitvec_capacity[i]=32;
		state.bitvec1= (unsigned int **)malloc(36*sizeof(int));
	for(i=0;i<36;i++) {
		for(i=0;i<36;i++) state.bitvec1[i]=(unsigned int *) calloc(1,sizeof(unsigned int));
	}
	state.KL_capacity= BBWT1->textlen+1;
	state.KL= (double *) malloc(state.KL_capacity*sizeof(double));
	*/
	
	SLT_iterator=new_SLT_joint_iterator(SLT_callback_RWs,SLT_cloner, SLT_combiner,SLT_free,&state,BBWT1,BBWT2,SLT_stack_trick, mem, cores);
	SLT_joint_execute_iterator(SLT_iterator);

	*_nMAWs1=state.nMAWs1;
	*_nMAWs2=state.nMAWs2;
	return state.nMAWs;
};

void convert_MAWs_to_ACGT(unsigned char ** MAW_ptr,unsigned int nMAWs)
{
	unsigned int i;
	unsigned char * str;
	for(i=0;i<nMAWs;i++)
	{
		str=MAW_ptr[i];
		while(*str)
		{
			if((*str)<5)
				(*str)=alpha4_to_ACGT[(*str)-1];
			else
				(*str)='Z';
			str++;
		};
	};
};
