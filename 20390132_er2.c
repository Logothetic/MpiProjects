#include <stdio.h>
#include "mpi.h"

struct vector{
    float delta;
    int pos;
};

int main(int argc, char** argv){
    int my_rank;
    int nop; // number of processes
    int root = 0;
    int data[100];
    int local_data[100];
    int final_greater,final_less;
    int min, max,n,sum;
    float loc_disp,final_disp;
    float avrg;
    struct vector tmp_vec;
    struct vector max_vec;
    float tmpd;
    int k,ch,tmp;
    int tag=100;
    int prefix[100];
    int local_prefix[100];
    int loc_num_arr[100];
    int displacement[100];
    int lta; //less than average counter
    int gta; //greater than average counter
    
    MPI_Status status;
    
    
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nop);
    
    do{
        avrg=0;
        sum=0;
        if (my_rank == 0){
            // system("clear");
            printf("Input how many numbers: ");
            scanf("%d", &n);
            

            printf("Input the elements of the array: "); 
            for(int i=0; i<n; i++)
                scanf("%d", &data[i]);
            
            //calculating the min max and average in p0
            max=data[0];
            min=data[0];
            sum+=data[0];
            
            for(int i=1; i<n; i++){
                sum+=data[i];
                if (data[i]>max){
                    max=data[i];
                }
                if (data[i]<min){
                    min=data[i];
                }
            }
            avrg=sum/(float)n;

            //calculating the amount of numbers each process is going to take
            for(int i=0;i<nop-1;i++)
                loc_num_arr[i]=n/nop;
            loc_num_arr[nop-1]=(n/nop)+(n%nop); 
            //last process takes also the numbers that are left if n%nop!=0

            //calculating displacement for scatterv and gatherv
            for(int i=0;i<nop;i++)
                displacement[i]=(n/nop)*i;
        }
        
        //bcasting all the variables that we will need
        MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Bcast(&max, 1, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Bcast(&min, 1, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Bcast(&avrg, 1, MPI_FLOAT, root, MPI_COMM_WORLD);
        MPI_Bcast(loc_num_arr,100,MPI_INT,root,MPI_COMM_WORLD);
        MPI_Bcast(displacement,100,MPI_INT,root,MPI_COMM_WORLD);        
        
        //scattering the data to each process
        MPI_Scatterv(data, loc_num_arr ,displacement, MPI_INT, 
        &local_data,loc_num_arr[my_rank], MPI_INT, root, MPI_COMM_WORLD);
        
        //counters
        lta=0;
        gta=0;
        loc_disp=0;

        for(int i=0; i< loc_num_arr[my_rank]; i++){
            if(local_data[i]<avrg){
                lta++; //less than avrg counter
            }
            if(local_data[i]>avrg){
                gta++; //greater than avrg counter
            }
            loc_disp+=(local_data[i]-avrg)*(local_data[i]-avrg); //calculating dispersion

            tmpd=((float)(local_data[i]-min)/(max-min))*100; //calculating delta
            printf("Delta [%d] : %.2f\n",i + (n/nop)*my_rank,tmpd); 

            //finding max delta and its pos in original data table
            if(i==0){
                tmp_vec.delta=tmpd;
                tmp_vec.pos=i + (n/nop)*my_rank;
            }else if(tmpd>tmp_vec.delta){
                tmp_vec.delta=tmpd;
                tmp_vec.pos=i + (n/nop)*my_rank;
            }
        }    

        //if amount of numbers is greater than number of processes
        if(n>nop){
                
            //calculating the first step for the prefix
            local_prefix[0]=local_data[0];                    
            for(int i=1;i<loc_num_arr[my_rank];i++)
                local_prefix[i]=local_prefix[i-1]+local_data[i];
            
            //rank 0 only sends but doesnt recv
            if(my_rank==0)
                MPI_Send(&local_prefix[loc_num_arr[my_rank]-1], 1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD);
            
            if(my_rank!=0){
                MPI_Recv(&tmp, 1, MPI_INT,my_rank-1, tag, MPI_COMM_WORLD, &status);
                for(int i=0;i<loc_num_arr[my_rank];i++){
                    local_prefix[i]+=tmp;  //calculating the second step of the prefix
                }
                if(my_rank!=nop-1){
                    MPI_Send(&local_prefix[loc_num_arr[my_rank]-1], 1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD);
                }
            }    
        
        }

        //calculating prefix for n=number of processes
        if(nop==n){
            MPI_Scan(local_data, prefix,loc_num_arr[my_rank], MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            printf("prefix[%d] == %d \n",my_rank,prefix[0]);
        }else if(n>nop){
            MPI_Gatherv(local_prefix,loc_num_arr[my_rank],MPI_INT,prefix,loc_num_arr,displacement,MPI_INT,root,MPI_COMM_WORLD);
        }
      

        //gathering the information that we need back at p0
        MPI_Reduce(&tmp_vec,&max_vec ,1, MPI_FLOAT_INT, MPI_MAXLOC,root, MPI_COMM_WORLD);
        MPI_Reduce(&loc_disp, &final_disp,1, MPI_FLOAT, MPI_SUM,root, MPI_COMM_WORLD);
        MPI_Reduce(&gta, &final_greater,1, MPI_FLOAT, MPI_SUM,root, MPI_COMM_WORLD);
        MPI_Reduce(&lta, &final_less,1, MPI_FLOAT, MPI_SUM,root, MPI_COMM_WORLD);
     
        if(my_rank==0){
            
            if(n>nop){
                for(int i=0;i<n;i++){
                      printf("prefix[%d] == %d \n",i,prefix[i]);
                }
            }
        
            printf("Final average: %.2f \n", avrg);
            printf("Final dispersion: %.2f \n", final_disp/(float)n);
            printf("Final greater: %d \n", final_greater);  
            printf("Final less: %d \n", final_less);
            printf("Final max: %d |delta: %.2f |pos: %d\n",data[max_vec.pos],max_vec.delta,max_vec.pos);
            
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
                        system("clear"); /* printing menu and getting the choice to continue or exit from the user*/
                    }else break;
            }
            for(int i=1;i<nop;i++)
                MPI_Send(&ch, 1, MPI_INT,i , tag, MPI_COMM_WORLD);
            if(ch==2){
                MPI_Finalize();    /*exiting the programm if ch==2*/
            }
        }else{
            MPI_Recv(&ch, 1, MPI_INT,0, tag, MPI_COMM_WORLD, &status);
            if(ch==2){
                MPI_Finalize();
            }
        }    
    }while(ch!=2);
}