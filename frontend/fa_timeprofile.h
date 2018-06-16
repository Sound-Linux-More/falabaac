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


  filename: fa_timeprofile.h 
  version : v1.0.0
  time    : 2012/10/20 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/



#ifndef _FA_TIMEPROFILE_H
#define _FA_TIMEPROFILE_H

#define USE_TIME

#define CLOCK_MAX_NUM  100

#ifdef USE_TIME

void fa_clock_start(int index);
void fa_clock_end(int index);
void fa_clock_cost(int index);
void fa_prt_time_cost(int index);

#define FA_CLOCK_START(index)      fa_clock_start(index)
#define FA_CLOCK_END(index)        fa_clock_end(index)
#define FA_CLOCK_COST(index)       fa_clock_cost(index)
#define FA_GET_TIME_COST(index)    fa_prt_time_cost(index) 

#else
	
#define FA_CLOCK_START(index) 
#define FA_CLOCK_END(index) 
#define FA_CLOCK_COST(index) 
#define FA_GET_TIME_COST(index)

#endif

#endif






























