//********************************************************************
// Mehmet Ozkan
// Advanced Operating Systems
// Project3: Part A: Logical clock
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
	int rank,num_of_procs;
	int manager = 0; // assign manager process as 0
	int logical_time = 0;
	int logical_clock_values[40], procs_total, sender, acknowledgement;
	char message[100], temp[100]; 
	int i, line_counter=0;
	string line;
	
	struct // create struct data to store parameters 
	{
     int number;
     int recv;
     int message_length;
	}mission;
	
	
	
	
	MPI_Datatype ptype;
	MPI_Status status;
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_procs); 
	MPI_Type_contiguous(3,MPI_INT,&ptype);
	MPI_Type_commit(&ptype);
	
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
			
		
			if(num_of_procs != procs_total+1) // check if the number of processes in the system is not larger by 1 from the total processes in file
			{
				printf("[Error]::%d processes should be in this system to run it!!\n", procs_total+1);
				
				MPI_Abort(MPI_COMM_WORLD, 911);
				MPI_Finalize();
				return 0;
			}
		
			}
			
			else if(line.compare(0,3,"end")==0)		
			{
				
				mission.number = 3; // end mission 
				
				for(i=1; i <= procs_total; i++)
				MPI_Send(&mission, 1, ptype, i, 44, MPI_COMM_WORLD); // request logical clock values from other processes 
				for(i=1; i <= procs_total; i++)
				{
					MPI_Recv(&logical_time, 2, MPI_INT, MPI_ANY_SOURCE, 55, MPI_COMM_WORLD, &status); // receive logical clock values from the processes
					logical_clock_values[status.MPI_SOURCE] = logical_time;
				}
				
				
				for( i=1; i<= procs_total; i++)
				{
				printf("At the end, Logical clock at process %d is %d\n", i, logical_clock_values[i]); // displaying the logical clock values of the processes
				}
				
				
				MPI_Finalize(); 
				return 0;
				
			}
			
		else if (line_counter !=0)
		{
			
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
				mission.message_length = k-1;  // store the length of the message 
				
				
				MPI_Send(&mission, 1, ptype, sender, 44, MPI_COMM_WORLD); // send the mission of send, receiver process and length of the message
				MPI_Send(message, mission.message_length , MPI_CHAR, sender, 66, MPI_COMM_WORLD); // send message with the length to the other processes
				MPI_Recv(&acknowledgement, 1, MPI_INT, MPI_ANY_SOURCE, 77, MPI_COMM_WORLD, &status); // receive acknowledgement from the receiver  processes
			}
			
		 }
		 line_counter++;
		}
		file.close();		
	}
	}
	
	else //when the processes are other than manager process
	{
		
		while(true) 
		{
			MPI_Recv(&mission, 1, ptype, MPI_ANY_SOURCE, 44, MPI_COMM_WORLD, &status); // receive missions from the manager process 
			
			
				
			
			if(mission.number == 3) // finalize the system if the mission is finalize or end 
			{
				MPI_Send(&logical_time, 1, MPI_INT, manager, 55, MPI_COMM_WORLD); // send logical_time values to the manager process 
				MPI_Finalize();
				return 0;
			}
			
			
			logical_time++; // logical_time increment by 1
			
			
			if(mission.number == 1) // if the mission is execute
			{
			printf("Execution event in process %d\n", rank); //display the process which is in execution
			}
			
			
			else if(mission.number == 2 || mission.number == 4) // if the mission is send or receive
			{
				MPI_Recv(message, mission.message_length, MPI_CHAR, MPI_ANY_SOURCE, 66, MPI_COMM_WORLD, &status); // receive the message from sender 
				
				message[mission.message_length] = '\0'; 
				
				if(mission.number == 4) //  if the mission is receive
				{
					if(logical_time <= mission.recv) // if  the logical_time value is smaller than sender and equal
					logical_time = mission.recv + 1; // increment the logical_time value of the sender by 1
					
					printf("Message received from process %d by process %d: %s\n", status.MPI_SOURCE, rank, message);
					
					MPI_Send(&acknowledgement, 1, MPI_INT, manager, 77, MPI_COMM_WORLD); // send acknowledgement to the manager process 
				}
				else // send mission
				{
					mission.number = 4; // if the mission is receive
					sender = mission.recv; // receiver process rank 
					
					mission.recv = logical_time; // logical_time value to be sent to the receiver
					
					printf("Message sent from process %d to process %d: %s\n", rank, sender, message);// displaying the message which is sending from current process to the sender
					MPI_Send(&mission, 1, ptype, sender, 44, MPI_COMM_WORLD); // sending the receive mission, length of the message and logical clock 
					MPI_Send(message, mission.message_length, MPI_CHAR, sender, 66, MPI_COMM_WORLD); // sending the message to the receiver process
				}
			}
			printf("Logical time at process %d is %d\n", rank, logical_time); // displaying the current logical_time of the processes 
		}
	}
	
	MPI_Finalize();
	return 0;
}
