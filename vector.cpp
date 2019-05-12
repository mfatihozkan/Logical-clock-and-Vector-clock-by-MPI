//********************************************************************
// Mehmet Ozkan
// Advanced Operating Systems
// Project3: Part B: Vector clock
// November 19, 2018
// Instructor: Dr. Mohammed Belkouche
//********************************************************************
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <time.h>  
#include <unistd.h> 
using namespace std;

int main(int argc, char *argv[])
{
	int rank,number_of_procs; 
	int manager = 0; // assign manager process as 0
	int clock[100], sender_clock[100], clocks[100][100],  procs_total, sender, acknowledgement; 
	int line_counter =0, i, j;
	char  message[100],temp[100];
	string line;
	
	
	struct 
	{
     int number;
     int recv;
     int msg_length;
	}mission;
	
	
	
	
	MPI_Datatype ptype;
	MPI_Status status;
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_procs); 
	MPI_Type_contiguous(3,MPI_INT,&ptype);
	MPI_Type_commit(&ptype);
	
	
	procs_total = number_of_procs-1;
		
	for(i=1; i <= procs_total; i++)
	{
	clock[i] = 0; // initialize the clocks at 0
	}
	
	
	if(argc !=2) // error checking if the  number of arguments are different than 2 
	{
		printf("[Error]:: Process[%d] could not read the file! Please type the correct number of arguments!\n", rank);
		MPI_Finalize();
		return 0;
	}
	
		
	fstream file(argv[1], ios_base::in); // open the file
	
	if(file==0)
		{
			printf("[Error]:: Process[%d] could not read the file! Please type the correct name of the file!\n", rank);
			MPI_Finalize();
			return 0;
		}
	if(rank == manager) // when the process is 0
	{
			
		if(file.is_open()) // read each line for the file
		{
			
			while(getline(file,line))
			{
			
			
		   if(line_counter==0)
		   {
			
			procs_total = atoi(line.c_str()); // save the procs_total 
			
			printf("There are %d processes in the system\n", procs_total); //display the number of processes in the system
			
		
			if(number_of_procs != procs_total+1) // check if the number of processes in the system is not larger by 1 from the total processes in file
			{
				printf("[Error]::%d processes should be in this system to run it!!\n", procs_total+1);
				
				MPI_Abort(MPI_COMM_WORLD, 911);
				MPI_Finalize();
				return 0;
			}
		
			}
			
			else if(line.compare(0,3,"end")==0)		
			{
				printf("END\n");
				mission.number = 3; // end mission 
				
				for(i=1; i<number_of_procs; i++)
				MPI_Send(&mission, 1, ptype, i, 44, MPI_COMM_WORLD); // request other processes for clock values
				
				for(i=1; i<number_of_procs; i++)
				{
				  MPI_Recv(clock, number_of_procs, MPI_INT, MPI_ANY_SOURCE, 55, MPI_COMM_WORLD, &status); // receive clock values
					for(j=1; j<number_of_procs; j++)
					clocks[status.MPI_SOURCE][j] = clock[j];
				}
				for(i=1; i<number_of_procs; i++)
				{
					printf("At the end, Vector clock at process %d is ( ", i); // display vector clock values of the processes
					for(j=1; j<number_of_procs; j++)
					printf("%d ", clocks[i][j]);
					printf(")\n");
				}
				MPI_Finalize(); 
				return 0;
				
			}
			
		else if (line_counter !=0)
		{
			printf("END\n");
			 if(line[0] == 'e') // if the mission is exec
			{
				istringstream stream(line);
				mission.number = 1; // assign 1 for exec mission 
				i=0;
				
				while (stream)
				{
					string ch;
					stream >> ch;
					
					if(i==1)
					{
					sender=atoi(ch.c_str());	
					}
					i=i+1;
				}
				MPI_Send(&mission, 1, ptype, sender, 44, MPI_COMM_WORLD); // send execute mission
			}
			else if(line[0] == 's') // if the mission is send 
			{
				int k=1;
				mission.number = 2; // assign 2 for send mission
				
				temp[0] = '\0';
				
				i=0;
				istringstream stream(line);
				while (stream)
				{
					string ch;
					stream>>ch;
					if( i==1)
					{
						sender=atoi(ch.c_str());
						
					}
					if( i==2)
					{
						mission.recv =atoi(ch.c_str());
						
					}
					if( i>2)
					{
						strcat(temp,ch.c_str()); // read the message 
						strcat(temp," ");
						
					}
					i++;
					
				}
				
				for(k = 1; temp[k] !='\"'; k++)
				{
				 message[k-1] = temp[k];
				}
												
				message[k-1] = '\0'; // assign the last character of the message is null 
				mission.msg_length = k-1;  // store the length of the message 
				
				
				MPI_Send(&mission, 1, ptype, sender, 44, MPI_COMM_WORLD); // send the mission of send, receiver process and length of the message
				MPI_Send(message, k-1, MPI_CHAR, sender, 66, MPI_COMM_WORLD); // send message with the length to the other processes
				MPI_Recv(&acknowledgement, 1, MPI_INT, MPI_ANY_SOURCE, 77, MPI_COMM_WORLD, &status); // receive acknowledgement from the receiver  processes
			}
			
		 }
		 line_counter++;
		}
		file.close();		
	}
	}
	else 
	{
		
		
		while(true) 
		{
			MPI_Recv(&mission, 1, ptype, MPI_ANY_SOURCE, 44, MPI_COMM_WORLD, &status); // receive the missions from the manager 
			
			
			if(mission.number == 1) // if the mission is execute
			{
			printf("Execution event in process %d\n", rank);
			}
			if(mission.number == 3) // if the mission is  end 
			{
				MPI_Send(clock, number_of_procs, MPI_INT, manager, 55, MPI_COMM_WORLD); // send clock values
				MPI_Finalize(); // finalize the system 
				return 0;
			}
			
			
			 
			clock[rank]++;  //increase clock
			
			
			
			if(mission.number == 2 || mission.number == 4) // if the missions are send or receive 
			{
				MPI_Recv(message, mission.msg_length, MPI_CHAR, MPI_ANY_SOURCE, 66, MPI_COMM_WORLD, &status); // receive the message from the sender 
				
				message[mission.msg_length] = NULL; 
				
								
				if(mission.number == 4) // if the mission is receive 
				{
					MPI_Recv(sender_clock, number_of_procs, MPI_INT, MPI_ANY_SOURCE, 77, MPI_COMM_WORLD, &status);
					
					for(i=1; i<= procs_total; i++)
						
					{
					if(clock[i] < sender_clock[i])
					clock[i] = sender_clock[i];
					}
					
					printf("Message received from process %d by process %d: %s\n", status.MPI_SOURCE, rank, message);
					MPI_Send(&acknowledgement, 1, MPI_INT, manager, 77, MPI_COMM_WORLD); // send the acknowledgement to the manager process 
				}
				else // if it is send mission
				{
					mission.number = 4; // if the mission is receive
					
					sender = mission.recv; // receiver process rank 
					
					printf("Message sent from process %d to process %d: %s\n", rank, sender, message); // displaying the message which is sending from current process to the sender
					
					MPI_Send(&mission, 1, ptype, sender, 44, MPI_COMM_WORLD); // sending the receive mission, length of the message and vector clock 
					MPI_Send(message, mission.msg_length, MPI_CHAR, sender, 66, MPI_COMM_WORLD); // sending the message to the receiver process
					MPI_Send(clock, number_of_procs, MPI_INT, sender, 77, MPI_COMM_WORLD); // sending vector clock values to the receiver
				}
			}
			
				printf("Vector time at process %d is ( ", rank); // displaying the current vector time of the processes 
				for(i=1; i <= procs_total; i++)
				{
				printf("%d ", clock[i]);
				}
				printf(")\n");
		}
	}
	
	MPI_Finalize();
	return 0;
}
