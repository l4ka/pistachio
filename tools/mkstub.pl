#!/usr/bin/perl -w

use strict;

my $name_base = "__l4_stub_";
my $num_ipc_regs = 8;


sub min
{
    my($a, $b) = @_;

    return $a < $b ? $a : $b;
}

sub max
{
    my($a, $b) = @_;

    return $a > $b ? $a : $b;
}

# This only takes care of the variable argumets --- dest, from, timeout etc. are
# taken care of by other functions
sub mkarglist
{
    my($argin, $argout) = @_;
    my @args = ();

    push @args, "word_t *ret_mr0";

    # start at 1 because mr0 is used for other things
    for (1..$argin) {
	push @args, "word_t in$_";
    }

    for (1..$argout) {
	push @args, "word_t *out$_";
    }
    
    return join ', ', @args;
}

sub mkvarlist
{
    my($argin, $argout, $dest, $from, $timeout) = @_;

    # Doesn't include mr0
    my @vars = ();

    push @vars, "r_dest    asm(R_TO_TID) = $dest" . ".raw";
    push @vars, "r_from    asm(R_FROM_TID) = $from" . ".raw";
    push @vars, "r_timeout asm(R_TIMEOUT) = $timeout";
    push @vars, "result    asm(R_RESULT)";
    push @vars, "mr0       asm(R_MR0) = $argin";

    for (1..$argin) {
	push @vars, "mr$_ asm(R_MR$_) = in$_";
    }

    for ($argin + 1..$argout) {
	push @vars, "mr$_ asm(R_MR$_)";
    }

    return "\tL4_ThreadId_t ret;\n" . join( '',  map("\tregister word_t " . $_ . ";\n", @vars));
}

sub mkoutassgn 
{
    my($argin, $argout) = @_;
    my @assgn = ();

    push @assgn, "ret.raw = result";
    push @assgn, "*ret_mr0 = mr0";

    for (1..$argout) {
	push @assgn, "*out$_ = mr$_";
    }

    return map "\t$_;\n", @assgn;
}

sub mkasmargs
{
    my($argin, $argout) = @_;    
    my(@in, @out, @scratch) = ();
    
    my $min = min($argin, $argout);
    my $max = max($argin, $argout);

    push @scratch, "IPC_ASM_SCRATCH";
    push @out, '"=r" (result)';

    push @in, '"r" (r_dest)';
    push @in, '"r" (r_from)';
    push @in, '"r" (r_timeout)';

    # 0 to include mr0
    for (0..$min) {
	push @out, '"+r" ' . "(mr$_)";
    }

    for (($min + 1) .. $argout) {
	push @out, '"=r" ' . "(mr$_)";
    }

    for (($min + 1) .. $argin) {
	push @in, '"r" ' . "(mr$_)";
    }

    for ($argout + 1 .. $num_ipc_regs) {
	push @scratch, "R_MR$_";
    }

    return ': ' . join(", ", @out) . "\n\t: " . join(", ", @in) . "\n\t: " . join(", ", @scratch);
}

sub mkfunc_replywait
{
    my($argin, $argout) = @_;    

    print("static inline L4_ThreadId_t ", $name_base, "replywait_", $argin, "in", $argout, 
	  "out(L4_ThreadId_t dest, \n\t",
	  mkarglist($argin, $argout), ")\n{\n",
	  mkvarlist($argin, $argout, 'dest', 'L4_anythread', '0'), "\n",
	  "\tasm volatile (IPC_ASM_STUB  \n\t",
	  mkasmargs($argin, $argout), ");\n\n",
	  mkoutassgn($argin, $argout), "\n",
	  "\treturn ret;\n}\n\n");
}

sub mkfunc_call
{
    my($argin, $argout) = @_;    

    print("static inline L4_ThreadId_t ", $name_base, "call_", $argin, "in", $argout, 
	  "out(L4_ThreadId_t dest, L4_ThreadId_t from,\n\t",
	  mkarglist($argin, $argout), ")\n{\n",
	  mkvarlist($argin, $argout, 'dest', 'from', '0'), "\n",
	  "\tasm volatile (IPC_ASM_STUB  \n\t",
	  mkasmargs($argin, $argout), ");\n\n",
	  mkoutassgn($argin, $argout), "\n",
	  "\treturn ret;\n}\n\n");
}

# header 
print "/* WARNING --- automatically generated file, do not edit! */ \n";

my($in, $out);
foreach $in (0..8) {
    foreach $out (0..8) {
	mkfunc_replywait($in, $out);
	mkfunc_call($in, $out);
    }
}
