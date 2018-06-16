/*
  falab - free algorithm lab 
  Copyright (C) 2012 luolongzhi 罗龙智 (Chengdu, China)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  filename: fa_timeprofile.c 
  version : v1.0.0
  time    : 2012/10/20 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/



#include <time.h>
#include <stdio.h>

float global_time_start[100] = {0};
float global_time_end[100]   = {0};
float global_time_cost[100]  = {0};

void fa_clock_start(int index)
{	
	global_time_start[index] = ((float)clock() / CLOCKS_PER_SEC);
}

void fa_clock_end(int index)
{	
	global_time_end[index] = ((float)clock() / CLOCKS_PER_SEC);
}

void fa_clock_cost(int index)
{	
	global_time_cost[index] += (global_time_end[index] - global_time_start[index]);
}

void fa_prt_time_cost(int index)
{
	printf("The number of %d index cost time is %f sec\n", index, global_time_cost[index]);
}








