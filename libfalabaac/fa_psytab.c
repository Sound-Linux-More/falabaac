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


  filename: fa_psytab.c
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#include "fa_psytab.h"

int   fa_psy_32k_long_wlow[FA_PSY_32k_LONG_NUM+1] =
{
    0   , 3     , 6     , 9     , 12    , 15    , 18    , 21    , 24    , 27    ,
    30  , 33    , 36    , 39    , 42    , 45    , 48    , 51    , 54    , 57    ,
    61  , 65    , 69    , 73    , 77    , 81    , 85    , 90    , 95    , 100   ,
    105 , 111   , 117   , 123   , 130   , 137   , 145   , 153   , 162   , 172   ,
    182 , 193   , 205   , 218   , 232   , 247   , 263   , 280   , 299   , 319   ,
    341 , 364   , 389   , 416   , 445   , 476   , 509   , 544   , 582   , 623   ,
    668 , 716   , 769   , 827   , 891   , 962   , 1024
};

float fa_psy_32k_long_barkval[FA_PSY_32k_LONG_NUM] =
{
    0.24    , 0.71  , 1.18  , 1.65  , 2.12  , 2.58  , 3.03  , 3.48  , 3.92  , 4.35  ,
    4.77    , 5.19  , 5.59  , 5.99  , 6.37  , 6.74  , 7.1   , 7.45  , 7.8   , 8.18  ,
    8.6     , 9     , 9.39  , 9.76  , 10.11 , 10.45 , 10.81 , 11.19 , 11.55 , 11.9  ,
    12.25   , 12.62 , 12.96 , 13.31 , 13.66 , 14.01 , 14.36 , 14.71 , 15.07 , 15.42 ,
    15.76   , 16.1  , 16.45 , 16.8  , 17.14 , 17.48 , 17.82 , 18.15 , 18.49 , 18.84 ,
    19.17   , 19.51 , 19.85 , 20.19 , 20.53 , 20.87 , 21.2  , 21.53 , 21.86 , 22.2  ,
    22.53   , 22.86 , 23.2  , 23.53 , 23.86 , 24
};

float fa_psy_32k_long_qsthr[FA_PSY_32k_LONG_NUM] =
{
    42.05   , 42.05 , 37.05 , 37.05 , 34.05 , 34.05 , 29.05 , 29.05 , 29.05 , 27.05 ,
    27.05   , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 28.3  ,
    28.3    , 28.3  , 28.3  , 28.3  , 28.3  , 28.3  , 29.27 , 29.27 , 29.27 , 29.27 ,
    30.06   , 30.06 , 30.06 , 30.73 , 30.73 , 31.31 , 31.31 , 31.82 , 32.28 , 32.28 ,
    32.69   , 33.07 , 33.42 , 33.74 , 34.04 , 34.32 , 34.58 , 35.06 , 35.29 , 35.7  ,
    35.89   , 36.26 , 36.59 , 39.9  , 40.19 , 40.46 , 42.72 , 43.07 , 43.4  , 48.81 ,
    49.09   , 49.52 , 59.91 , 60.34 , 60.79 , 65.89
};

int   fa_psy_32k_short_wlow[FA_PSY_32k_SHORT_NUM+1] =
{
    0   , 1     , 2     , 3     , 4     , 5     , 6     , 7     , 8     , 9     ,
    10  , 11    , 12    , 13    , 14    , 15    , 16    , 17    , 18    , 19    ,
    21  , 23    , 25    , 27    , 29    , 31    , 33    , 36    , 39    , 42    ,
    45  , 49    , 53    , 57    , 61    , 66    , 71    , 76    , 82    , 88    ,
    95  , 103   , 111   , 120   , 128
};

float fa_psy_32k_short_barkval[FA_PSY_32k_SHORT_NUM] =
{
    0       , 1.26  , 2.5   , 3.7   , 4.85  , 5.92  , 6.93  , 7.85  , 8.7   , 9.49  ,
    10.2    , 10.85 , 11.45 , 12    , 12.5  , 12.96 , 13.39 , 13.78 , 14.15 , 14.8  ,
    15.38   , 15.89 , 16.36 , 16.77 , 17.15 , 17.5  , 17.9  , 18.34 , 18.74 , 19.11 ,
    19.5    , 19.92 , 20.3  , 20.65 , 21.02 , 21.4  , 21.75 , 22.1  , 22.45 , 22.8  ,
    23.16   , 23.51 , 23.85 , 24
};

float fa_psy_32k_short_qsthr[FA_PSY_32k_SHORT_NUM] =
{
    27.28   , 22.28 , 19.28 , 14.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 ,
    12.28   , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 15.29 ,
    15.29   , 15.29 , 15.29 , 15.29 , 15.29 , 15.29 , 17.05 , 17.05 , 17.05 , 17.05 ,
    18.3    , 18.3  , 21.3  , 21.3  , 24.27 , 24.27 , 24.27 , 30.06 , 30.06 , 30.73 ,
    41.31   , 41.31 , 41.82 , 60.47
};

int   fa_psy_44k_long_wlow[FA_PSY_44k_LONG_NUM+1] =
{
    0   , 2     , 4     , 6     , 8     , 10    , 12    , 14    , 16    , 18    ,
    20  , 22    , 24    , 26    , 28    , 30    , 32    , 34    , 36    , 39    ,
    42  , 45    , 48    , 51    , 54    , 57    , 60    , 63    , 67    , 71    ,
    75  , 79    , 83    , 88    , 93    , 98    , 104   , 110   , 117   , 124   ,
    132 , 140   , 149   , 158   , 168   , 179   , 191   , 204   , 218   , 233   ,
    249 , 266   , 284   , 304   , 325   , 348   , 372   , 398   , 426   , 456   ,
    489 , 525   , 564   , 607   , 654   , 707   , 766   , 833   , 909   , 997   , 1024
};

float fa_psy_44k_long_barkval[FA_PSY_44k_LONG_NUM] =
{
    0.22    , 0.65  , 1.09  , 1.52  , 1.95  , 2.37  , 2.79  , 3.21  , 3.62  , 4.02  ,
    4.41    , 4.8   , 5.18  , 5.55  , 5.92  , 6.27  , 6.62  , 6.95  , 7.36  , 7.83  ,
    8.28    , 8.71  , 9.12  , 9.52  , 9.89  , 10.25 , 10.59 , 10.97 , 11.38 , 11.77 ,
    12.13   , 12.48 , 12.84 , 13.22 , 13.57 , 13.93 , 14.3  , 14.67 , 15.03 , 15.4  ,
    15.76   , 16.11 , 16.45 , 16.79 , 17.13 , 17.48 , 17.83 , 18.18 , 18.52 , 18.87 ,
    19.21   , 19.54 , 19.88 , 20.22 , 20.56 , 20.9  , 21.24 , 21.57 , 21.91 , 22.24 ,
    22.58   , 22.91 , 23.25 , 23.58 , 23.91 , 24    , 24    , 24    , 24    , 24
};

float fa_psy_44k_long_qsthr[FA_PSY_44k_LONG_NUM] =
{
    40.29   , 40.29 , 35.29 , 35.29 , 35.29 , 32.29 , 32.29 , 27.29 , 27.29 , 25.29 ,
    25.29   , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 27.05 , 27.05 ,
    27.05   , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 28.3  , 28.3  , 28.3  ,
    28.3    , 28.3  , 29.27 , 29.27 , 29.27 , 30.06 , 30.06 , 30.73 , 30.73 , 31.31 ,
    31.31   , 31.82 , 31.82 , 32.28 , 32.69 , 33.07 , 33.42 , 33.74 , 34.04 , 34.32 ,
    34.58   , 34.83 , 35.29 , 38.5  , 38.89 , 39.08 , 41.43 , 41.75 , 42.05 , 47.46 ,
    47.84   , 48.19 , 58.61 , 59    , 59.52 , 69.98 , 70.54 , 71.08 , 71.72 , 72.09
};

int   fa_psy_44k_short_wlow[FA_PSY_44k_SHORT_NUM+1] =
{
    0   , 1     , 2     , 3     , 4     , 5     , 6     , 7     , 8     , 9     ,
    10  , 11    , 12    , 13    , 14    , 15    , 16    , 17    , 18    , 20    ,
    22  , 24    , 26    , 28    , 30    , 32    , 35    , 38    , 41    , 44    ,
    48  , 52    , 56    , 60    , 65    , 70    , 76    , 82    , 89    , 97    ,
    106 , 116   , 128
};

float fa_psy_44k_short_barkval[FA_PSY_44k_SHORT_NUM] =
{
    0       , 1.73  , 3.41  , 4.99  , 6.45  , 7.75  , 8.92  , 9.96  , 10.87 , 11.68 ,
    12.39   , 13.03 , 13.61 , 14.12 , 14.59 , 15.01 , 15.4  , 15.76 , 16.39 , 16.95 ,
    17.45   , 17.89 , 18.3  , 18.67 , 19.02 , 19.41 , 19.85 , 20.25 , 20.62 , 21.01 ,
    21.43   , 21.81 , 22.15 , 22.51 , 22.87 , 23.23 , 23.59 , 23.93 , 24    , 24    ,
    24      , 24
};

float fa_psy_44k_short_qsthr[FA_PSY_44k_SHORT_NUM] =
{
    27.28   , 22.28 , 14.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 ,
    12.28   , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 15.29 , 15.29 ,
    15.29   , 15.29 , 15.29 , 15.29 , 15.29 , 17.05 , 17.05 , 20.05 , 20.05 , 23.3  ,
    23.3    , 23.3  , 28.3  , 29.27 , 29.27 , 40.06 , 40.06 , 40.73 , 51.31 , 51.82 ,
    52.28   , 53.07
};

int   fa_psy_48k_long_wlow[FA_PSY_48k_LONG_NUM+1] =
{
    0   , 2     , 4     , 6     , 8     , 10    , 12    , 14    , 16    , 18    ,
    20  , 22    , 24    , 26    , 28    , 30    , 32    , 34    , 36    , 38    ,
    41  , 44    , 47    , 50    , 53    , 56    , 59    , 62    , 66    , 70    ,
    74  , 78    , 82    , 87    , 92    , 97    , 103   , 109   , 116   , 123   ,
    131 , 139   , 148   , 158   , 168   , 179   , 191   , 204   , 218   , 233   ,
    249 , 266   , 284   , 304   , 325   , 348   , 372   , 398   , 426   , 457   ,
    491 , 528   , 568   , 613   , 663   , 719   , 782   , 854   , 938   , 1024
};

float fa_psy_48k_long_barkval[FA_PSY_48k_LONG_NUM] =
{
    0.24    , 0.71  , 1.18  , 1.65  , 2.12  , 2.58  , 3.03  , 3.48  , 3.92  , 4.35  ,
    4.77    , 5.19  , 5.59  , 5.99  , 6.37  , 6.74  , 7.1   , 7.45  , 7.8   , 8.2   ,
    8.68    , 9.13  , 9.55  , 9.96  , 10.35 , 10.71 , 11.06 , 11.45 , 11.86 , 12.25 ,
    12.62   , 12.96 , 13.32 , 13.7  , 14.05 , 14.41 , 14.77 , 15.13 , 15.49 , 15.85 ,
    16.2    , 16.55 , 16.91 , 17.25 , 17.59 , 17.93 , 18.28 , 18.62 , 18.96 , 19.3  ,
    19.64   , 19.97 , 20.31 , 20.65 , 20.99 , 21.33 , 21.66 , 21.99 , 22.32 , 22.66 ,
    23      , 23.33 , 23.67 , 24    , 24    , 24    , 24    , 24    , 24
};

float fa_psy_48k_long_qsthr[FA_PSY_48k_LONG_NUM] =
{
    40.29   , 40.29 , 35.29 , 35.29 , 32.29 , 32.29 , 27.29 , 27.29 , 27.29 , 25.29 ,
    25.29   , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 25.29 , 27.05 ,
    27.05   , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 27.05 , 28.3  , 28.3  , 28.3  ,
    28.3    , 28.3  , 29.27 , 29.27 , 29.27 , 30.06 , 30.06 , 30.73 , 30.73 , 31.31 ,
    31.31   , 31.82 , 32.28 , 32.28 , 32.69 , 33.07 , 33.42 , 33.74 , 34.04 , 34.32 ,
    34.58   , 34.83 , 38.29 , 38.5  , 38.89 , 41.08 , 41.43 , 41.75 , 47.19 , 47.59 ,
    47.96   , 58.3  , 58.81 , 69.27 , 69.76 , 70.27 , 70.85 , 71.52 , 70.2
};

int   fa_psy_48k_short_wlow[FA_PSY_48k_SHORT_NUM+1] =
{
    0   , 1     , 2     , 3     , 4     , 5     , 6     , 7     , 8     , 9     ,
    10  , 11    , 12    , 13    , 14    , 15    , 16    , 17    , 19    , 21    ,
    23  , 25    , 27    , 29    , 31    , 34    , 37    , 40    , 43    , 46    ,
    50  , 54    , 58    , 63    , 68    , 74    , 80    , 87    , 95    , 104   ,
    114 , 126   , 128
};

float fa_psy_48k_short_barkval[FA_PSY_48k_SHORT_NUM] =
{
    0       , 1.88  , 3.7   , 5.39  , 6.93  , 8.29  , 9.49  , 10.53 , 11.45 , 12.26 ,
    12.96   , 13.59 , 14.15 , 14.65 , 15.11 , 15.52 , 15.9  , 16.56 , 17.15 , 17.66 ,
    18.13   , 18.54 , 18.93 , 19.28 , 19.69 , 20.14 , 20.54 , 20.92 , 21.27 , 21.64 ,
    22.03   , 22.39 , 22.76 , 23.13 , 23.49 , 23.85 , 24    , 24    , 24    , 24    ,
    24      , 24
};

float fa_psy_48k_short_qsthr[FA_PSY_48k_SHORT_NUM] =
{
    27.28   , 22.28 , 14.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 ,
    12.28   , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 12.28 , 15.29 , 15.29 , 15.29 ,
    15.29   , 15.29 , 15.29 , 15.29 , 17.05 , 20.05 , 20.05 , 20.05 , 22.05 , 23.3  ,
    28.3    , 28.3  , 29.27 , 39.27 , 40.06 , 40.06 , 50.73 , 51.31 , 51.82 , 52.28 ,
    53.07   , 53.07
};
