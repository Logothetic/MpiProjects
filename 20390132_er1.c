#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv)
{
    int my_rank;
    int tmpflag1,tmp;
    int p, k, flag, finflag, num;
    int source, target;
    int tag1 = 10, tag2 = 20, tag3 = 30;
    int plithos;
    int data[100];
    int data_loc[100];
    int pos;
    int tmppos;
    int i;
    int ch;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    do{
        tmpflag1=flag=finflag=0; /*Flags for checking if the sequence is sorted*/
        pos=tmppos=-1;  
          if (my_rank == 0){/*this part of the code will only be run by the 0th process*/
            system("clear");
            printf("The amount of numbers must be bigger than :: %d !\n",p); /*p is the number of processes*/
            printf("Input the amount of numbers : ");
            scanf("%d", &plithos);
            if(plithos>=p){ /*if the amount of numbers is smaller than the amount of processes then we quit the program*/
                printf("Input the %d numbers : \n", plithos);
                for (k = 0; k < plithos; k++)
                    scanf("%d", &data[k]);

                for(target=1;target<p;target++)
                    MPI_Send(&plithos, 1, MPI_INT, target, tag1,MPI_COMM_WORLD);    /*process 0 sends the amount of numbers to the rest of the processes*/
                num = plithos / p;  /*spliting the table to equal pieces*/
               

                for (k = 0; k < num; k++)
                    data_loc[k] = data[k];  /*assigning the first num numbers to data_loc from data*/

                for (target = 1; target < p; target++){
                    if(target==p-1){
                        if(plithos%p!=0){
                            num=num+plithos%p;  /* checks if  (amount of numbers)%(amount of processes)==0 */
                        }                       /* if not then add the remainder done only for the last process*/

                    }
                    MPI_Send(&data[k], num, MPI_INT, target, tag2,MPI_COMM_WORLD); /*assigning from data[k]-data[num] to data_loc[] so that each process*/
                    k += num;                                                      /*has the next num numbers*/
                }

                num = plithos/p; /*calculating num again in case the if(plithos%p!=0) was true and num changed value */
            }else{  /*p0 exiting here*/
                printf("The amount of numbers must be bigger than the amount of threads!\n");
                for(target=1;target<p;target++)
                    MPI_Send(&plithos, 1, MPI_INT, target, tag1,MPI_COMM_WORLD);
                printf("Thread %d is exiting!\n",my_rank);
                MPI_Finalize();
                exit(1);
            }
        }else{
            /*rest of processes exit here*/
            MPI_Recv(&plithos, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status);
            if(plithos<p){
                printf("Thread %d is exiting!\n",my_rank);
                MPI_Finalize();
                exit(1);
            }
            num = plithos / p;
            if(my_rank==p-1){
                if(plithos%p!=0){
                    num=num+(plithos%p);
                    // printf(":: %d %%\n",num);
                }
            }
            MPI_Recv(&data_loc[0], num, MPI_INT, 0, tag2, MPI_COMM_WORLD,&status);  /*recieveing data to data_loc from p0*/
            
            
        }
        
        /*checking if its sorted if not check the flag*/     
        for (k = 0; k < num - 1; k++){
            if (data_loc[k] > data_loc[k + 1]){
                flag = 1;
                pos=k + (plithos/p)*my_rank; /*calculating the position */
            }

        }
                    
        
        if(my_rank!=p-1){   /*if you're not the last process then send your last number to get checked with the first number from the next process*/
            MPI_Send(&data_loc[k], 1, MPI_INT, my_rank+1, tag3, MPI_COMM_WORLD);
            //   printf("my rank %d :: sending  %d \n",my_rank,data_loc[k]);
            i=k + (plithos/p)*my_rank;
            MPI_Send(&i, 1, MPI_INT, my_rank+1, tag3, MPI_COMM_WORLD);    
        }
        
        
        if(my_rank!=0){   /*recieving the last number from previous process and comparing with its own first number*/
            MPI_Recv(&tmp, 1, MPI_INT,my_rank-1, tag3, MPI_COMM_WORLD, &status);
            MPI_Recv(&tmppos, 1, MPI_INT,my_rank-1, tag3, MPI_COMM_WORLD, &status);
            if(tmp>data_loc[0]){
                flag=1;
                pos=tmppos;
            }

        }
        


        if (my_rank != 0){  /*sending your results to p0*/
            MPI_Send(&flag, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD);
            MPI_Send(&pos, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD);
        }else{
                /*printing result of p0*/
            printf("\nResult of process %d :: ", my_rank);
            if(flag==0)printf("true\n");else{ printf("false\n");finflag=1;}
            
            for (source = 1; source < p; source++){
                MPI_Recv(&tmpflag1, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
                MPI_Recv(&tmppos, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
                if (tmpflag1 == 1)
                    finflag = 1;
                
                /*if pos hasent been changed and process[source] found a mistake in the sequence*/
                /*then change the pos to tmppos so now it cannot change again thus making sure  */
                /*that we print the first mistake that has been found*/
                if(pos==-1 && tmppos!=-1)
                    pos=tmppos;

                printf("Result of process %d :: ", source);
                if(tmpflag1==0)printf("true\n");else printf("false\n");
            }
        
            printf("\nFinal result  :: ");
            if(finflag==0)
                printf("Sorted\n");
            else
                printf("Not sorted\n\nSequence breaks first at pos %d , %d > %d \n",pos,data[pos],data[pos+1]);


        }
        if(my_rank==0){
            k=0;
            while(1){
                if(k>0) printf("There is no such option such as %d , please give again!\n",ch);
                printf("\n============MENU============\n");
                printf("---Option 1  :: --Continue--\n");
                printf("---Option 2  :: ----Exit----\n");
                printf("\nGive option  :: ");
                scanf("%d",&ch);
                if(!(ch==1 || ch==2)){
                    k++;
                   system("clear");                 /* printing menu and getting the choice to continue or exit from the user*/
                }else break;
            }
            for(target=1;target<p;target++)
                MPI_Send(&ch, 1, MPI_INT,target , tag3, MPI_COMM_WORLD);
            if(ch==2){
                MPI_Finalize();    /*exiting the programm if ch==2*/
            }
        }else{
            MPI_Recv(&ch, 1, MPI_INT,0, tag3, MPI_COMM_WORLD, &status);
            if(ch==2){
                MPI_Finalize();
            }
        }    
        
    }while(ch!=2);
   
}
