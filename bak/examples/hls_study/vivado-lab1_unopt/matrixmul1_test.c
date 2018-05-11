//*****************************************************************************
// (c) Copyright 2009 - 2012 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,

// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
//
//*****************************************************************************
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor: Xilinx
// \   \   \/     Version: %version
//  \   \         Application: AutoESL
//  /   /         Filename: matrixmul1_test.cpp
// /___/   /\     Date Last Modified: $Date: 2012/3/30 18:53:07 $
// \   \  /  \    Date Created: Fri Mar 30 2012
//  \___\/\___\
//
//Device: All
//Design Name: maxtrixmul1
//Purpose:
//    This is the C++ test bench for the of a matrix multiplier example.
//Reference:
//Revision History:
//*****************************************************************************
//#include <iostream>
#include <stdio.h>
//#include "matrixmul1.c"
//using namespace std;
//#define HW_COSIM
//#pragma map generate_hw
void matrixmul1(
volatile      /*char*/ int a[3][3],
volatile      /*char*/ int b[3][3],
volatile      /*short*/ int res[3][3])
{
    int i,j,k;
  // Iterate over the rows of the A matrix
  Row: for(i = 0; i < 3; i++) {
    // Iterate over the columns of the B matrix
    Col: for(j = 0; j < 3; j++) {
      // Do the inner product of a row of A and col of B
      res[i][j] = 0;
      Product: for(k = 0; k < 3; k++) {
        res[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

int main(void)
{
volatile /*char*/ int in_mat_a[3][3] = {
      {11, 12, 13},
      {14, 15, 16},
      {17, 18 ,19}
   };
volatile /*char*/ int in_mat_b[3][3] = {
      {21, 22, 23},
      {24, 25, 26},
      {27, 28, 29}
   };
volatile /*short*/int hw_result[3][3], sw_result[3][3];
   int err_cnt = 0;
   int i,j,k;

   // Generate the expected result
   // Iterate over the rows of the A matrix
   for(i = 0; i < 3; i++) {
      for(j = 0; j < 3; j++) {
         // Iterate over the columns of the B matrix
         sw_result[i][j] = 0;
         // Do the inner product of a row of A and col of B
         for(k = 0; k < 3; k++) {
            sw_result[i][j] += in_mat_a[i][k] * in_mat_b[k][j];
         }
      }
   }

#ifdef HW_COSIM
   // Run the AutoESL matrix multiply block
//     #pragma map call_hw VIRTEX5 0
     matrixmul1(in_mat_a, in_mat_b, hw_result);
#endif

   // Print result matrix
   printf("{ \n");
   //cout << setw(6);
   for (i = 0; i < 3; i++) {
      printf("{");
      for (j = 0; j < 3; j++) {
#ifdef HW_COSIM
         printf("%d", hw_result[i][j]);
         // Check HW result against SW
         if (hw_result[i][j] != sw_result[i][j]) {
            err_cnt++;
            printf("*");
         }
#else
         printf("%d",sw_result[i][j]);
#endif
         if (j == 3 - 1) 
            printf("}\n");
         else 
            printf(",");
      }
   }
   printf("}\n");

#ifdef HW_COSIM
   if (err_cnt)
      printf("ERROR: %d mismatches detected!\n",err_cnt);
   else
      printf("Test passed.\n");
#endif
   return err_cnt;
}

