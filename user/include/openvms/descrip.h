///http://www.tmk.com/ftp/vms-freeware/gcc-for-alpha/include/vms/descrip.h

/* http://h71000.www7.hp.com/commercial/c/docs/5492p012.html */
struct  dsc$descriptor
   {
      unsigned  short  dsc$w_length; /*  Length of data         */
      char  dsc$b_dtype              /*  Data type code         */
      char  dsc$b_class              /*  Descriptor class code  */
      char  *dsc$a_pointer           /*  Address of first byte  */
   };
