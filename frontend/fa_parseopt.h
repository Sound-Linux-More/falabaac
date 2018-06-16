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


  filename: fa_getopt.h 
  version : v1.0.0
  time    : 2012/07/08 01:01 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

  comment : this file is the simple template which will be used in falab,
            it will be changed according to the project.

*/

#ifndef _FA_GETOPT_H
#define _FA_GETOPT_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  



/*
    Below macro maybe defined in fa_print, if you do not want to use fa_print,
    you can define any other printf functions which you like.  Moreover, if you
    dont want print anything (for the effienece purpose of the program), you 
    can also define null, and the program will print nothing when running.
*/
#ifndef FA_PRINT 
//#define FA_PRINT(...)       
#define FA_PRINT       printf
#define FA_PRINT_ERR   FA_PRINT
#define FA_PRINT_DBG   FA_PRINT
#endif


extern char  opt_inputfile[] ;
extern char  opt_outputfile[];
extern int   opt_bitrate;
extern float opt_quality;
extern int   opt_vbrflag;
extern int   opt_speedlevel;
extern float opt_bandwidth;
extern int   opt_lfeenable;
extern int   opt_time_resolution_first;

extern int   opt_vbr_enable;

int fa_parseopt(int argc, char *argv[]);

#ifdef __cplusplus 
}
#endif  


#endif
