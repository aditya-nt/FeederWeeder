#define ROWSIZE 8
#define COLSIZE 8
#include "blackline.h"
int SX=7,SY=4;
int dir=1;
int src_value=0;
int repeat=0;
int sourcefound=0;
int DX=0,DY=0;
int count=20;
int matcount=1;
int path[50];

int maze[ROWSIZE][COLSIZE]={
       		     //  A  B  C  D  E  F  G
				{10,10,10,10,10,10,10,10,10},
				{10,11,11,11,11,11,11,11,10},  //1
				{10,11,11,11,11,11,11,11,10},  //2
				{10,11,11,11,11,11,11,11,10},  //3
				{10,11,11,11,11,11,11,11,10},  //4
				{10,11,11,11,15,11,11,11,10},  //5
				{10,11,11,11,11,11,11,11,10},  //6
				{10,11,11,11,11,11,11,11,10},  //7
				{10,10,10,10,10,10,10,10,10}
};
/*
void printmatrix(){
	 int i=0,j=0;
	 for(i=0;i<ROWSIZE;i++){
	  for(j=0;j<COLSIZE;j++){
	       if(maze[i][j]!=10){
		printf(" %d",maze[i][j]);
	       }
	       else{
		printf("   ");
	       }
	  }
	  printf("\n");
	 }
}
*/
void detect(int s){
	int c=s;
	int i=0,j=0;

	for(i=0;i<ROWSIZE;i++){
	   for(j=0;j<COLSIZE;j++){
	      if(maze[i][j]==c){
		DX=i;
		DY=j;
		maze[i][j]=count;
		count++;
		break;
		}
	   }
	}
}

void solve(int dX,int dY){

	int curX=0,curY=0;
		curX=dX;
		curY=dY;

		if(maze[curX-1][curY]==11){
		maze[curX-1][curY]=count;
		repeat=1;
		}
		if(maze[curX][curY-1]==11){
		maze[curX][curY-1]=count;
		repeat=1;
		}
		if(maze[curX][curY+1]==11){
		maze[curX][curY+1]=count;
		repeat=1;
		}
		if(maze[curX+1][curY]==11){
		maze[curX+1][curY]=count;
		repeat=1;
		}

}

void detectnodes(int e){
	int v=e;

	int i=0,j=0;

	for(i=0;i<ROWSIZE;i++){
	   for(j=0;j<COLSIZE;j++){
	      if(maze[i][j]==v){
		solve(i,j);
		}
	   }
	}
	matcount++;

	if(repeat==1 && sourcefound==0){
		repeat=0;
		count++;
		detectnodes(count-1);
	}
}

int chartoint(unsigned char chr){
	int x=0;
	switch(chr){
		case '1': x=1;	break;
		case 'A': x=1;	break;
		case '2': x=2;	break;
		case 'B': x=2;	break;
		case '3': x=3;	break;
		case 'C': x=3;	break;
		case '4': x=4;	break;
		case 'D': x=4;	break;
		case '5': x=5;	break;
		case 'E': x=5;	break;
		case '6': x=6;	break;
		case 'F': x=6;	break;
		case '7': x=7;	break;
		case 'G': x=7;	break;
		default : x=0;
	}
	return x;
}

void placeobstacles(int xi,int yi){
		maze[xi][yi]=10;
		lcd_print (2, 8, xi,1 );
		lcd_print (2, 9, yi,1 );
};

void setDestination(int xj,int yj){
		DX=xj;
		DY=yj;
		maze[xj][yj]=count;
		count++;
		lcd_print (2, 5, xj,1 );
		lcd_print (2, 6, yj,1 );
}

void setSource(int xk,int yk){
		SX=xk;
		SY=yk;
}

void refresh(){
    int i=0,j=0;count=20;

	 for(i=0;i<ROWSIZE;i++){
	  for(j=0;j<COLSIZE;j++){
	       if(maze[i][j]!=10){
                maze[i][j]=11;
           }
       }
	 }
}
/*
void ReachDestinationAvoidingNode(unsigned char Xd,unsigned char Yd,unsigned char Xn,unsigned char Yn){
        refresh();
        setDestination(Xd,chartoint(Yd));
		placeobstacles(Xn,chartoint(Yn));
        detectnodes(count-1);
        //printmatrix();
        pathplotter();

		setSource(DX,DY);
}
*/
void pathplotter(){
    src_value=maze[SX][SY];

    int i=0;
    while(src_value!=20){
    src_value--;

    if(maze[SX-1][SY]==src_value){
        if(dir==1){
        path[i]=1;   }
        else if(dir==2){
        path[i]=8;   }
        else if(dir==8){
        path[i]=2;   }
        else if(dir==4){
        path[i]=4;   }
        dir=1;
        SX=SX-1;
        SY=SY;
    }
    else if(maze[SX][SY+1]==src_value){
        if(dir==1){
        path[i]=2;   }
        else if(dir==2){
        path[i]=1;   }
        else if(dir==4){
        path[i]=8;   }
        else if(dir==8){
        path[i]=4;   }
        dir=2;
        SX=SX;
        SY=SY+1;
    }
    else if(maze[SX+1][SY]==src_value){
        if(dir==2){
        path[i]=2;   }
        else if(dir==4){
        path[i]=1;   }
        else if(dir==8){
        path[i]=8;   }
        else if(dir==1){
        path[i]=4;   }
        dir=4;
        SX=SX+1;
        SY=SY;
    }
    else if(maze[SX][SY-1]==src_value){
        if(dir==1){
        path[i]=8;   }
        else if(dir==4){
        path[i]=2;   }
        else if(dir==8){
        path[i]=1;   }
        else if(dir==2){
        path[i]=4;   }
        dir=8;
        SX=SX;
        SY=SY-1;
    }
    i++;
    }

    int j=0;
    for(j=0;j<i;j++){
            if(path[j]==1){
            line();
			}
            else if(path[j]==2){
            precision_right();
			line();
			}
            else if(path[j]==8){
            precision_left ();
			line();
			}
            else if(path[j]==4)
            {
            left_degrees(175);
            adjust_left();
            }
    }

}
/*
void main(){

	ReachDestinationAvoidingNode(5,'D',6,'D');
    ReachDestinationAvoidingNode(2,'F',2,'D');
    ReachDestinationAvoidingNode(2,'A',2,'C');

}
*/
