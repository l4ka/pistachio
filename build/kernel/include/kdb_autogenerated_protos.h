/* Machine-generated file - DO NOT EDIT! */
/* File will be included from kdb/kdb.h  */
STATIC void dump_table (mdb_t * mdb, mdb_table_t * t, word_t depth);
STATIC void dump_resource_table (mdb_t * mdb, mdb_table_t * table, word_t addr, word_t depth);
STATIC void dump_resource_map (mdb_t * mdb, mdb_node_t * node, word_t addr, word_t depth);
void entry (void * param);
void SECTION (".init") init (void);
STATIC void dump_vrt_table (vrt_t * vrt, vrt_table_t * t, word_t depth);
STATIC void dump_vrt (vrt_t * vrt);
bool pre();
void post();
