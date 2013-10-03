/*!
 * \file traffic_repeater.c
 * \author Jan Neuzil <neuzija1@fit.cvut.cz>
 * \date 2013
 */
/*
 * Copyright (C) 2013 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <netdb.h>

#include "traffic_repeater.h"

TRAP_DEFAULT_SIGNAL_HANDLER(stop = 1)

void module_init(trap_module_info_t *module, int ifc_in, int ifc_out)
{
   module->name = "Traffic repeater";  
   module->description = "This module receive data from input interface and resend it to the output interface "
                         "based on given arguments in -i option";
   module->num_ifc_in = ifc_in;
   module->num_ifc_out = ifc_out;
}

void traffic_repeater(void)
{
   int ret;
   uint16_t data_size;
   uint64_t cnt_r, cnt_s, cnt_t, diff;
   const void *data;
   struct timespec start, end;
   
   data_size = 0;
   cnt_r = cnt_s = cnt_t = 0;
   data = NULL;
   TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();
   if (verb) {
      printf("Initializing traffic repeater...\n");
   }
   clock_gettime(CLOCK_MONOTONIC, &start);
   
   while (stop == 0) {
      ret = trap_get_data(1, &data, &data_size, TIMEOUT);
      if (ret == TRAP_E_OK) {
         cnt_r++;
         if (data_size == 1) {
            if (verb) {
               printf("Final record received, terminating repeater...\n");
            }
            stop = 1;
         }          
         ret = trap_send_data(0, data, data_size, TIMEOUT);
         if (ret == TRAP_E_OK) {
            cnt_s++;
            continue;
	     } 
         TRAP_DEFAULT_SEND_DATA_ERROR_HANDLING(ret, cnt_r++; continue, break);
      }
      TRAP_DEFAULT_GET_DATA_ERROR_HANDLING(ret, cnt_r++; continue, break);
   }
   
   clock_gettime(CLOCK_MONOTONIC, &end);
   diff = (end.tv_sec * NANOSECOND + end.tv_nsec) - (start.tv_sec * NANOSECOND + start.tv_nsec);
   printf("Flows received:  %16lu\n", cnt_r > 0 ? cnt_r - 1 : cnt_r);
   printf("Flows sent:      %16lu\n", cnt_s > 0 ? cnt_s - 1 : cnt_s);
   printf("Timeouts:        %16lu\n", cnt_t);
   printf("Time elapsed:    %12lu.%03lus\n", diff / NANOSECOND, (diff % NANOSECOND) / 1000000);
}

int main(int argc, char **argv)
{
   trap_module_info_t module_info;
   
   module_init(&module_info, IFC_DEF, IFC_DEF);
   TRAP_DEFAULT_INITIALIZATION(argc, argv, module_info);
   verb = (trap_get_verbose_level() >= 0);
   traffic_repeater();
   TRAP_DEFAULT_FINALIZATION();
   
   return EXIT_SUCCESS;
}
