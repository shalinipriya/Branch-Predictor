// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 8 kilobytes available
// for the CBP-2 contest; it is just an example.

class my_update : public branch_update {
public:
	unsigned int index;
	int output;
	int predict;
};

class my_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	0
#define TABLE_BITS	13
#define W_X (2^10)
#define W_Y (2^4)
#define W_Z (HISTORY_LENGTH+1)
#define THETA 10
//#define THETA (2.14*(HISTORY_LENGTH+1)+20.58)

	my_update u;
	branch_info bi;
	unsigned int history;
	unsigned int global_address[HISTORY_LENGTH];
	unsigned char tab[1<<TABLE_BITS];
	int W[W_X][W_Y][W_Z];
	
	my_predictor (void) : history(0) {
		int i,j,k; 
		memset (tab, 0, sizeof (tab));
		for(int i=0;i<HISTORY_LENGTH;i++) {
			global_address[i]=0;
		}
		srand (time(NULL));
		for(i=0;i<W_X;i++) {
			for(j=0;j<W_Y;j++) {
				for(k=0;k<W_Z;k++) {
					W[i][j][k]=rand()%256 - 127;
				}
			}
		}
	}

	branch_update *predict (branch_info & b) {
		//fprintf(stderr, "in predict\n");
		bi = b;
/*		if (b.br_flags & BR_CONDITIONAL) {
			u.index = 
				  (history << (TABLE_BITS - HISTORY_LENGTH)) 
				^ (b.address & ((1<<TABLE_BITS)-1));
			u.direction_prediction (tab[u.index] >> 1);
		} else {
			u.direction_prediction (true);
		}
		u.target_prediction (0);*/

		if (b.br_flags & BR_CONDITIONAL) {
			int x,y;
			x=b.address%W_X;
			int output = W[x][0][0];
			for(int i=0;i<HISTORY_LENGTH;i++) {
				y=global_address[i]%W_Y;
				if(history>>i & 1) {
					output = output + W[x][y][i];
				}
				else {
					output = output - W[x][y][i];
				}
			}
			if(output>=0) {
				u.direction_prediction(1);
				u.predict=1;
			}
			else { 
				u.direction_prediction(0);
				u.predict=0;
			}
			u.output = output;	
		}
	
		//fprintf(stderr, "done predict\n");*/
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
			unsigned char *c = &tab[((my_update*)u)->index];
			if (taken) {
				if (*c < 3) (*c)++;
			} else {
				if (*c > 0) (*c)--;
			}
	
			//fprintf(stderr, "in update\n");
			int x;
			if(abs(((my_update*)u)->output)<THETA || ((my_update*)u)->predict!=taken) {
				x=bi.address%W_X;
				if(taken) {
					W[x][0][0] = W[x][0][0]+1;
				}
				else {
					W[x][0][0] = W[x][0][0]-1;
				}
				for(int i =0;i<HISTORY_LENGTH;i++) {
					int y = global_address[i]%W_Y;
					if(history>>i & 1) {
						W[x][y][i] = W[x][y][i]+1;
					}
					else {
						W[x][y][i] = W[x][y][i]-1;
					}
				}
			}
			
			history <<= 1;
			history |= taken;
			history &= (1<<HISTORY_LENGTH)-1;
			
			/*global address*/	
			for(int i=HISTORY_LENGTH-1;i>=1;i--) {
				global_address[i]=global_address[i-1];
			}	
			if(HISTORY_LENGTH>0)	
			global_address[0]=target;

		}
	}
};
