/************************************************************
			Course 		: 	CSC456
			Source 		:	msi.cc
			Instructor	:	Ed Gehringer
			Email Id	:	efg@ncsu.edu
------------------------------------------------------------
	© Please do not replicate this code without consulting
		the owner & instructor! Thanks!
*************************************************************/ 
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
//using namespace std;
#include "main.h"
#include "moesi.h"

void MOESI::PrRd(ulong addr, int processor_number) {
    cache_state state;
	// Per-cache global counter to maintain LRU order among 
	// cache ways, updated on every cache access
    current_cycle++; 
    reads++;
    cache_line * line = find_line(addr);
       if (line == NULL) { 
		// This is a miss 
             read_misses++;
             cache_line *newline = allocate_line(addr);
            if(c2c_supplier(addr,processor_number)==0){
            		memory_transactions++;
            	if (sharers_exclude(addr,processor_number)==0)
            		{
             		I2E++;
					newline->set_state(E);}
				else{
             	I2S++; 
				newline->set_state(S);
				//cache2cache++;
				}

            }
        else {
        	cache2cache++;
        	I2S++; 
            newline->set_state(S);
        }

		// Read miss --> BusRd		
             bus_reads++;
             sendBusRd(addr, processor_number);
       }
      else { 
		// The block is cached
	     state=line->get_state();
             if (state == I){
			// The block is cached, but in invalid state.
			// Hence Read miss
			read_misses++;
            cache_line *newline = allocate_line(addr);


			 if(c2c_supplier(addr,processor_number)==0){
            		memory_transactions++;
            	if (sharers_exclude(addr,processor_number)==0)
            		{
             		I2E++;
					newline->set_state(E);}
				else{
             	I2S++; 
				newline->set_state(S);
				//cache2cache++;
				}

            }
        	else {
        	cache2cache++;
        	I2S++; 
            newline->set_state(S);
        	}
				
			bus_reads++;
			sendBusRd(addr, processor_number);
		}
         else if (state == M || state == S || state == E || state == O){
                update_LRU(line);
               }

        }  	
} 

void MOESI::PrWr(ulong addr, int processor_number) {
    cache_state state;
    current_cycle++;
    writes++;
    cache_line * line = find_line(addr);

    if (line == NULL || line->get_state() == I){ 
		/* This is a miss */
		write_misses++;
        cache_line *newline = allocate_line(addr); 	
        if(c2c_supplier(addr,processor_number)==1)
        cache2cache++;
        else	memory_transactions++;	
		I2M++;
        newline->set_state(M);
		bus_readxs++;
		sendBusRdX(addr, processor_number);
     }
    else 
	{
		state=line->get_state();
                if (state == M){
                /* Since it's a hit, update LRU and update state */
				update_LRU(line);
                }
                else if(state==S)
				{	S2M++;
					line->set_state(M);
					bus_upgrades++;
 					sendBusUpgr(addr, processor_number);
 					update_LRU(line);
				}
				else if(state==O)
				{	O2M++;
					line->set_state(M);
					bus_upgrades++;
 					sendBusUpgr(addr, processor_number);
 					update_LRU(line);

                }
                else if(state==E){
               	update_LRU(line);
               	line->set_state(M);
				E2M++;
			}
         }	
} 


void MOESI::BusRd(ulong addr) {
   cache_state state;    
    cache_line * line=find_line(addr);
    if (line!=NULL) 
       { 		
		state = line->get_state();
		if (state == M)
         {
             flushes++;
             line->set_state(O);
			 interventions++;
		     M2O++;
         }
        else if (state == O)
        {
        	flushes++;
        }	
        else if (state == E)
        {
		    line->set_state(S);
		    E2S++;
        	//write_backs++;
        }

	}
} 

void MOESI::BusRdX(ulong addr) {
    cache_line * line=find_line(addr);
    if (line!=NULL)	{ 
		cache_state state;
		state=line->get_state();
                if (state == S )
                {
                invalidations++;
                line->set_state(I);
                }
                else if(state == O)
                {
                invalidations++;
 				write_backs++;
 				flushes++;
 				memory_transactions++;
                line->set_state(I);               	
                }
                else if (state == M )
                {
				flushes++;
				invalidations++;
                line->set_state(I);
                }

                else if(state == E)
                {
                invalidations++;
                line->set_state(I);
                }
	}
} 

void MOESI::BusUpgr(ulong addr) {
    cache_line * line=find_line(addr);
    if (line!=NULL)	{
		cache_state state;
		state=line->get_state();
                if (state == S || state == O)//*
                {
					invalidations++;
	                line->set_state(I);
                }
	}
} 

bool MOESI::writeback_needed(cache_state state) {
    if (state == M || state == O)
    {
    	return true;
    } 
    else {
	return false;
    }
}
